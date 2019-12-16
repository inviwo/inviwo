/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019 Inviwo Foundation
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

#include <modules/fancymeshrenderer/calcnormals.h>

#include <inviwo/core/datastructures/buffer/buffer.h>
#include <inviwo/core/datastructures/buffer/bufferram.h>
#include <inviwo/core/datastructures/buffer/bufferramprecision.h>

namespace inviwo {

namespace meshutil {
using Mode = CalculateMeshNormalsMode;

void calculateMeshNormals(Mesh& mesh, CalculateMeshNormalsMode mode) {
    if (mode == Mode::PassThrough) {
        return;
    }

    // get input buffers
    auto [bv, bvLoc] = mesh.findBuffer(BufferType::PositionAttrib);
    auto [bn, bnLoc] = mesh.findBuffer(BufferType::NormalAttrib);

    BufferBase* asdf = const_cast<BufferBase*>(bn);

    if (!bv) {
        throw Exception("Input mesh has no position buffer",
                        IVW_CONTEXT_CUSTOM("meshutil::calculateMeshNormals"));
    }

    // Make sure we have a normal buffer of type vec3
    if (!bn || bn->getDataFormat() != DataVec3Float32::get()) {
        auto newBuf = std::make_shared<Buffer<vec3>>();
        if (bn) {
            mesh.replaceBuffer(bnLoc, {BufferType::NormalAttrib, bnLoc}, newBuf);
        } else {
            mesh.addBuffer(BufferType::NormalAttrib, newBuf);
        }
        bn = newBuf.get();
    }

    auto vertices = bv->getRepresentation<BufferRAM>();
    auto normalBuffer = static_cast<Buffer<vec3>*>(asdf);
    std::vector<vec3>& normals = normalBuffer->getEditableRAMRepresentation()->getDataContainer();
    if (normals.empty()) {
        normals.resize(vertices->getSize(), vec3(0.0f));
    } else {
        // reset normals
        std::fill(normals.begin(), normals.end(), vec3(0.0f));
    }

    auto vertexLookUp =
        vertices->dispatch<std::function<vec3(size_t)>, dispatching::filter::Floats>([](auto ram) {
            using T = util::PrecisionValueType<decltype(ram)>;
            return [&v = ram->getDataContainer()](size_t i) -> dvec3 {
                if constexpr (util::extent<T>::value == 1) {
                    return dvec3(v[i], 0, 0);
                }
                if constexpr (util::extent<T>::value == 2) {
                    return dvec3(v[i], 0);
                } else {  // extent == 3 or extent == 4
                    return dvec3(v[i]);
                }
            };
        });

    // loop over index buffers
    for (auto [meshInfo, buffer] : mesh.getIndexBuffers()) {
        if (meshInfo.dt != DrawType::Triangles) continue;
        meshutil::forEachTriangle(meshInfo, *buffer, [&](auto i0, auto i1, auto i2) {
            const auto v0 = vertexLookUp(i0);
            const auto v1 = vertexLookUp(i1);
            const auto v2 = vertexLookUp(i2);

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
                    auto edge = [](auto a, auto b) {
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
                default:
                    weightA = weightB = weightC = 1.0 / l;  // no weighting
            }
            // add it to the vertices
            normals[i0] += vec3(n * weightA);
            normals[i1] += vec3(n * weightB);
            normals[i2] += vec3(n * weightC);
        });
    }

    // normalize normals
    std::transform(normals.begin(), normals.end(), normals.begin(), [](auto n) {
        const auto l = glm::length(n);
        if (l < std::numeric_limits<float>::epsilon()) return n;
        return n / l;
    });
}
}  // namespace meshutil

}  // namespace inviwo
