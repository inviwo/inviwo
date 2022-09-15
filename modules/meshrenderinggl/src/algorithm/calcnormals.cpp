/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019-2022 Inviwo Foundation
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *********************************************************************************/

#include <modules/meshrenderinggl/algorithm/calcnormals.h>

#include <inviwo/core/datastructures/buffer/buffer.h>                   // for Buffer, BufferBase
#include <inviwo/core/datastructures/buffer/bufferram.h>                // for BufferRAM
#include <inviwo/core/datastructures/buffer/bufferramprecision.h>       // for BufferRAMPrecision
#include <inviwo/core/datastructures/geometry/geometrytype.h>           // for BufferType, Buffe...
#include <inviwo/core/datastructures/geometry/mesh.h>                   // for Mesh, Mesh::Index...
#include <inviwo/core/datastructures/representationconverter.h>         // for RepresentationCon...
#include <inviwo/core/datastructures/representationconverterfactory.h>  // for RepresentationCon...
#include <inviwo/core/util/exception.h>                                 // for Exception
#include <inviwo/core/util/formatdispatching.h>                         // for Floats
#include <inviwo/core/util/glmconvert.h>                                // for glm_convert
#include <inviwo/core/util/glmvec.h>                                    // for vec3, dvec3
#include <inviwo/core/util/sourcecontext.h>                             // for IVW_CONTEXT_CUSTOM
#include <modules/base/algorithm/meshutils.h>                           // for forEachTriangle

#include <algorithm>                                                    // for transform
#include <cmath>                                                        // for acos
#include <limits>                                                       // for numeric_limits
#include <string>                                                       // for string
#include <string_view>                                                  // for string_view
#include <unordered_map>                                                // for unordered_map
#include <unordered_set>                                                // for unordered_set
#include <utility>                                                      // for tuple_element<>::...
#include <vector>                                                       // for vector

#include <glm/geometric.hpp>                                            // for normalize, cross
#include <glm/vec3.hpp>                                                 // for operator-, operator/

namespace inviwo {

namespace meshutil {
using Mode = CalculateMeshNormalsMode;

void calculateMeshNormals(Mesh& mesh, CalculateMeshNormalsMode mode) {
    if (mode == Mode::PassThrough) {
        return;
    }

    // get input buffers
    auto positions = mesh.getBuffer(BufferType::PositionAttrib);

    if (!positions) {
        throw Exception("Input mesh has no position buffer",
                        IVW_CONTEXT_CUSTOM("meshutil::calculateMeshNormals"));
    }

    while (auto normals = mesh.getBuffer(BufferType::NormalAttrib)) {
        mesh.removeBuffer(normals);
    }

    auto vertices = positions->getRepresentation<BufferRAM>();
    std::vector<vec3> normals(vertices->getSize(), vec3(0.0f));

    // loop over index buffers
    for (auto [meshInfo, buffer] : mesh.getIndexBuffers()) {
        if (meshInfo.dt != DrawType::Triangles) continue;

        vertices->dispatch<void, dispatching::filter::Floats>(
            [&, mi = meshInfo, buf = buffer](auto ram) {
                const auto& vert = ram->getDataContainer();

                meshutil::forEachTriangle(mi, *buf, [&](auto i0, auto i1, auto i2) {
                    const auto v0 = util::glm_convert<dvec3>(vert[i0]);
                    const auto v1 = util::glm_convert<dvec3>(vert[i1]);
                    const auto v2 = util::glm_convert<dvec3>(vert[i2]);

                    const dvec3 n = cross(v1 - v0, v2 - v0);
                    double l = glm::length(n);
                    if (l < std::numeric_limits<float>::epsilon()) {
                        // degenerated triangle
                        return;
                    }
                    // weighting factor
                    double weightA;
                    double weightB;
                    double weightC;
                    switch (mode) {
                        case Mode::WeightArea:
                            // area = norm of cross product
                            weightA = 1;
                            weightB = 1;
                            weightC = 1;
                            break;
                        case Mode::WeightAngle: {
                            // based on the angle between the edges
                            const dvec3 e0 = glm::normalize(v1 - v2);
                            const dvec3 e1 = glm::normalize(v2 - v0);
                            const dvec3 e2 = glm::normalize(v1 - v0);
                            weightA = acos(dot(e1, e2)) / l;
                            weightB = acos(dot(e0, e2)) / l;
                            weightC = acos(dot(e0, e1)) / l;
                            break;
                        }
                        case Mode::WeightNMax: {
                            const auto edge = [](auto a, auto b) {
                                auto e = a - b;
                                auto l = glm::length(e);
                                return std::make_pair(e / l, l);
                            };
                            const auto [e0, l0] = edge(v1, v2);
                            const auto [e1, l1] = edge(v2, v0);
                            const auto [e2, l2] = edge(v1, v0);
                            weightA = sin(acos(dot(e1, e2))) / (l * l1 * l2);
                            weightB = sin(acos(dot(e0, e2))) / (l * l0 * l2);
                            weightC = sin(acos(dot(e0, e1))) / (l * l0 * l1);
                            break;
                        }
                        case Mode::NoWeighting:
                        default:
                            weightA = 1.0 / l;
                            weightB = 1.0 / l;
                            weightC = 1.0 / l;
                    }
                    // add it to the vertices
                    normals[i0] += vec3(n * weightA);
                    normals[i1] += vec3(n * weightB);
                    normals[i2] += vec3(n * weightC);
                });
            });
    }

    // normalize normals
    std::transform(normals.begin(), normals.end(), normals.begin(), [](auto n) {
        const auto l = glm::length(n);
        if (l < std::numeric_limits<float>::epsilon()) return n;
        return n / l;
    });

    auto bufferRAM = std::make_shared<BufferRAMPrecision<vec3>>(std::move(normals));
    mesh.addBuffer(BufferType::NormalAttrib, std::make_shared<Buffer<vec3>>(bufferRAM));
}
}  // namespace meshutil

}  // namespace inviwo
