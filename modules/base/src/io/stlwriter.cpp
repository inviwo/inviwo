/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2023 Inviwo Foundation
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

#include <modules/base/io/stlwriter.h>

#include <inviwo/core/datastructures/buffer/bufferram.h>                // for BufferRAM
#include <inviwo/core/datastructures/geometry/geometrytype.h>           // for operator<<, Buffe...
#include <inviwo/core/datastructures/geometry/mesh.h>                   // for Mesh, Mesh::Index...
#include <inviwo/core/datastructures/representationconverter.h>         // for RepresentationCon...
#include <inviwo/core/datastructures/representationconverterfactory.h>  // for RepresentationCon...
#include <inviwo/core/io/datawriter.h>                                  // for DataWriterType
#include <inviwo/core/io/datawriterexception.h>                         // for DataWriterException
#include <inviwo/core/util/fileextension.h>                             // for FileExtension
#include <inviwo/core/util/formatdispatching.h>                         // for Vec3s
#include <inviwo/core/util/formats.h>                                   // for DataFormatBase
#include <inviwo/core/util/glmmat.h>                                    // for mat3
#include <inviwo/core/util/glmutils.h>                                  // for Matrix
#include <inviwo/core/util/logcentral.h>                                // for log, LogAudience
#include <inviwo/core/util/sourcecontext.h>                             // for IVW_CONTEXT
#include <inviwo/core/util/stdextensions.h>                             // for find_if
#include <modules/base/algorithm/meshutils.h>                           // for forEachTriangle

#include <cstddef>        // for size_t
#include <fstream>        // for basic_ofstream
#include <functional>     // for function, __base
#include <sstream>        // for basic_stringstrea...
#include <string>         // for basic_string
#include <type_traits>    // for remove_extent_t
#include <unordered_set>  // for unordered_set
#include <utility>        // for pair, move

#include <glm/detail/qualifier.hpp>  // for tvec4, tvec3
#include <glm/geometric.hpp>         // for normalize
#include <glm/mat3x3.hpp>            // for operator*
#include <glm/mat4x4.hpp>            // for operator*
#include <glm/matrix.hpp>            // for inverse, transpose
#include <glm/vec3.hpp>              // for operator+, operator/
#include <glm/vec4.hpp>              // for operator*, operator+
#include <half/half.hpp>             // for operator<<, opera...

namespace inviwo {

StlWriter::StlWriter() : DataWriterType<Mesh>() {
    addExtension(FileExtension("stl", "STL ASCII file format"));
}

StlWriter* StlWriter::clone() const { return new StlWriter(*this); }

void StlWriter::writeData(const Mesh* data, const std::filesystem::path& filePath) const {
    auto f = open(filePath);
    writeData(data, f);
}

std::unique_ptr<std::vector<unsigned char>> StlWriter::writeDataToBuffer(
    const Mesh* data, std::string_view /*fileExtension*/) const {
    std::stringstream ss;
    writeData(data, ss);
    auto stringdata = std::move(ss).str();
    return std::make_unique<std::vector<unsigned char>>(stringdata.begin(), stringdata.end());
}

void StlWriter::writeData(const Mesh* data, std::ostream& f) const {
    auto pit = util::find_if(data->getBuffers(), [](const auto& buf) {
        return buf.first.type == BufferType::PositionAttrib;
    });

    if (pit == data->getBuffers().end()) {
        throw DataWriterException("Error: could not find a position buffer", IVW_CONTEXT);
    }

    const auto posBuffer = pit->second;
    const auto posRam = posBuffer->getRepresentation<BufferRAM>();
    if (!posRam) {
        throw DataWriterException("Error: could not find a position buffer ram", IVW_CONTEXT);
    }
    if (posRam->getDataFormat()->getComponents() != 3) {
        throw DataWriterException("Error: Only 3 dimensional meshes are supported", IVW_CONTEXT);
    }

    const auto model = data->getModelMatrix();
    auto modelNormal = mat3(glm::transpose(glm::inverse(model)));

    const auto proj = [&](const auto& d1) {
        using GT = typename std::decay<decltype(d1)>::type;
        using T = typename GT::value_type;
        const glm::tvec4<T> tmp = model * glm::tvec4<T>(d1, 1.0);
        return glm::tvec3<T>(tmp) / tmp.w;
    };

    auto vertexprinter =
        posRam->dispatch<std::function<void(std::ostream&, size_t)>, dispatching::filter::Vec3s>(
            [&](auto pb) -> std::function<void(std::ostream&, size_t)> {
                return [&proj, pb](std::ostream& fs, size_t i) {
                    auto& pos = *pb;
                    const auto v = proj(pos[i]);
                    fs << v.x << " " << v.y << " " << v.z << "\n";
                };
            });

    auto nit = util::find_if(data->getBuffers(), [](const auto& buf) {
        return buf.first.type == BufferType::NormalAttrib;
    });
    auto normalprinter = [&]() {
        if (nit != data->getBuffers().end()) {
            if (const auto normRam = nit->second->getRepresentation<BufferRAM>()) {
                if (normRam->getDataFormat()->getComponents() == 3) {
                    return normRam
                        ->dispatch<std::function<void(std::ostream & fs, size_t, size_t, size_t)>,
                                   dispatching::filter::Vec3s>(
                            [&](auto nb)
                                -> std::function<void(std::ostream & fs, size_t, size_t, size_t)> {
                                return [&modelNormal, nb](std::ostream& fs, size_t i1, size_t i2,
                                                          size_t i3) {
                                    auto& norm = *nb;
                                    const auto n1 = modelNormal * norm[i1];
                                    const auto n2 = modelNormal * norm[i2];
                                    const auto n3 = modelNormal * norm[i3];
                                    const auto n = glm::normalize(n1 + n2 + n3);
                                    fs << n.x << " " << n.y << " " << n.z << "\n";
                                };
                            });
                }
            }
        }
        return std::function<void(std::ostream&, size_t, size_t, size_t)>(
            [](std::ostream& fs, size_t, size_t, size_t) -> void { fs << "0.0 0.0 0.0\n"; });
    }();

    const auto triangle = [&](const auto& i1, const auto& i2, const auto& i3) {
        f << "facet normal ";
        normalprinter(f, i1, i2, i3);
        f << "    outer loop\n";
        f << "        vertex ";
        vertexprinter(f, i1);
        f << "        vertex ";
        vertexprinter(f, i2);
        f << "        vertex ";
        vertexprinter(f, i3);
        f << "    endloop\n";
        f << "endfacet\n";
    };

    f << "solid inviwo stl file\n";

    for (const auto& inds : data->getIndexBuffers()) {
        if (inds.first.dt != DrawType::Triangles) {
            std::stringstream err;
            err << "Draw type: \n" << inds.first.dt << "\" not supported";
            util::log(IVW_CONTEXT, err.str(), LogLevel::Warn, LogAudience::User);
            continue;
        }

        meshutil::forEachTriangle(inds.first, *inds.second, triangle);
    }

    f << "endsolid inviwo stl file\n";
}

}  // namespace inviwo
