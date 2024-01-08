/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2024 Inviwo Foundation
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

#include <modules/base/io/wavefrontwriter.h>

#include <inviwo/core/datastructures/buffer/buffer.h>                   // for BufferBase, Index...
#include <inviwo/core/datastructures/buffer/bufferram.h>                // for BufferRAM, IndexB...
#include <inviwo/core/datastructures/geometry/geometrytype.h>           // for BufferType, DrawType
#include <inviwo/core/datastructures/geometry/mesh.h>                   // for Mesh, Mesh::Index...
#include <inviwo/core/datastructures/representationconverter.h>         // for RepresentationCon...
#include <inviwo/core/datastructures/representationconverterfactory.h>  // for RepresentationCon...
#include <inviwo/core/io/datawriter.h>                                  // for DataWriterType
#include <inviwo/core/io/datawriterexception.h>                         // for DataWriterException
#include <inviwo/core/util/fileextension.h>                             // for FileExtension
#include <inviwo/core/util/formatdispatching.h>                         // for Vec3s, Vec2s
#include <inviwo/core/util/formats.h>                                   // for DataFormatBase
#include <inviwo/core/util/glmmat.h>                                    // for mat3
#include <inviwo/core/util/glmfmt.h>
#include <inviwo/core/util/glmutils.h>         // for Matrix
#include <inviwo/core/util/sourcecontext.h>    // for IVW_CONTEXT
#include <modules/base/algorithm/meshutils.h>  // for forEachTriangle

#include <cstdint>        // for uint32_t
#include <fstream>        // for basic_ofstream
#include <numeric>        // for iota
#include <string>         // for basic_string
#include <tuple>          // for tuple_element<>::...
#include <type_traits>    // for decay, remove_ref...
#include <unordered_set>  // for unordered_set
#include <utility>        // for move, pair

#include <fmt/core.h>                // for basic_string_view
#include <fmt/ostream.h>             // for print
#include <glm/detail/qualifier.hpp>  // for tvec4, tvec3
#include <glm/mat3x3.hpp>            // for operator*
#include <glm/mat4x4.hpp>            // for operator*
#include <glm/matrix.hpp>            // for inverse, transpose
#include <glm/vec3.hpp>              // for operator/
#include <glm/vec4.hpp>              // for operator*, operator+
#include <half/half.hpp>             // for operator/

namespace inviwo {

WaveFrontWriter::WaveFrontWriter() : DataWriterType<Mesh>() {
    addExtension(FileExtension("obj", "WaveFront Obj file format"));
}

WaveFrontWriter* WaveFrontWriter::clone() const { return new WaveFrontWriter(*this); }

void WaveFrontWriter::writeData(const Mesh* data, const std::filesystem::path& filePath) const {
    auto f = open(filePath);
    writeData(data, f);
}

std::unique_ptr<std::vector<unsigned char>> WaveFrontWriter::writeDataToBuffer(
    const Mesh* data, std::string_view /*fileExtension*/) const {
    std::stringstream ss;
    writeData(data, ss);
    auto stringdata = std::move(ss).str();
    return std::make_unique<std::vector<unsigned char>>(stringdata.begin(), stringdata.end());
}

void WaveFrontWriter::writePrimitives(std::ostream& f, const Mesh::MeshInfo& info,
                                      const IndexBuffer& indices, bool hasNormals,
                                      bool hasTextures) const {
    if (info.dt == DrawType::Triangles) {
        if (hasTextures && hasNormals) {
            meshutil::forEachTriangle(info, indices,
                                      [&](std::uint32_t i1, std::uint32_t i2, std::uint32_t i3) {
                                          fmt::print(f, "f {0}/{0}/{0} {1}/{1}/{1} {2}/{2}/{2}\n",
                                                     i1 + 1, i2 + 1, i3 + 1);
                                      });
        } else if (hasNormals && !hasTextures) {
            meshutil::forEachTriangle(
                info, indices, [&](std::uint32_t i1, std::uint32_t i2, std::uint32_t i3) {
                    fmt::print(f, "f {0}//{0} {1}//{1} {2}//{2}\n", i1 + 1, i2 + 1, i3 + 1);
                });
        } else if (!hasNormals && hasTextures) {
            meshutil::forEachTriangle(
                info, indices, [&](std::uint32_t i1, std::uint32_t i2, std::uint32_t i3) {
                    fmt::print(f, "f {0}/{0} {1}/{1} {2}/{2}\n", i1 + 1, i2 + 1, i3 + 1);
                });
        } else {  // only verties
            meshutil::forEachTriangle(info, indices,
                                      [&](std::uint32_t i1, std::uint32_t i2, std::uint32_t i3) {
                                          fmt::print(f, "f {0} {1} {2}\n", i1 + 1, i2 + 1, i3 + 1);
                                      });
        }
    } else if (info.dt == DrawType::Lines) {
        if (hasNormals) {
            meshutil::forEachLine(
                info, indices, [&]() { fmt::print(f, "l "); },
                [&](std::uint32_t i) { fmt::print(f, "{0}/{0} ", i + 1); },
                [&]() { fmt::print(f, "\n"); });

        } else {
            meshutil::forEachLine(
                info, indices, [&]() { fmt::print(f, "l "); },
                [&](std::uint32_t i) { fmt::print(f, "{} ", i + 1); },
                [&]() { fmt::print(f, "\n"); });
        }

    } else if (info.dt == DrawType::Points) {
        auto& ram = indices.getRAMRepresentation()->getDataContainer();
        fmt::print(f, "p ");
        for (auto i : ram) fmt::print(f, "{} ", i + 1);
        fmt::print(f, "\n");
    }
}

void WaveFrontWriter::writeData(const Mesh* data, std::ostream& f) const {
    const auto model = data->getModelMatrix();
    auto modelNormal = mat3(glm::transpose(glm::inverse(model)));
    const auto proj = [&](const auto& d1) {
        using GT = typename std::decay<decltype(d1)>::type;
        using T = typename GT::value_type;
        const glm::tvec4<T> tmp = model * glm::tvec4<T>(d1, 1.0);
        return glm::tvec3<T>(tmp) / tmp.w;
    };

    {
        auto [buffer, loc] = data->findBuffer(BufferType::PositionAttrib);
        if (!buffer) {
            throw DataWriterException("Error: could not find a position buffer", IVW_CONTEXT);
        }
        const auto posRam = buffer->getRepresentation<BufferRAM>();
        if (!posRam) {
            throw DataWriterException("Error: could not find a position buffer ram", IVW_CONTEXT);
        }
        if (posRam->getDataFormat()->getComponents() != 3) {
            throw DataWriterException("Error: Only 3 dimensional meshes are supported",
                                      IVW_CONTEXT);
        }

        fmt::print(f, "# List of vertex coordinates ({})\n", posRam->getSize());
        posRam->dispatch<void, dispatching::filter::Vec3s>([&](auto pb) {
            for (const auto& elem : pb->getDataContainer()) {
                const auto v = proj(elem);
                fmt::print(f, "v {} {} {}\n", v.x, v.y, v.z);
            }
        });
    }

    bool hasTextures = false;
    if (auto [buffer, loc] = data->findBuffer(BufferType::TexCoordAttrib); buffer) {
        if (const auto texRam = buffer->getRepresentation<BufferRAM>()) {
            fmt::print(f, "# List of texture coordinates ({})\n", texRam->getSize());
            if (texRam->getDataFormat()->getComponents() == 3) {
                hasTextures = true;
                texRam->dispatch<void, dispatching::filter::Vec3s>([&](auto pb) {
                    for (const auto& v : pb->getDataContainer()) {
                        fmt::print(f, "vt {} {} {}\n", v.x, v.y, v.z);
                    }
                });
            } else if (texRam->getDataFormat()->getComponents() == 2) {
                hasTextures = true;
                texRam->dispatch<void, dispatching::filter::Vec2s>([&](auto pb) {
                    for (const auto& v : pb->getDataContainer()) {
                        fmt::print(f, "vt {} {}\n", v.x, v.y);
                    }
                });
            }
        }
    }

    bool hasNormals = false;
    if (auto [buffer, loc] = data->findBuffer(BufferType::NormalAttrib); buffer) {
        if (const auto normRam = buffer->getRepresentation<BufferRAM>()) {
            fmt::print(f, "# List of vertex normal ({})\n", normRam->getSize());
            if (normRam->getDataFormat()->getComponents() == 3) {
                hasNormals = true;
                normRam->dispatch<void, dispatching::filter::Vec3s>([&](auto pb) {
                    for (const auto& elem : pb->getDataContainer()) {
                        const auto v = modelNormal * elem;
                        fmt::print(f, "vn {} {} {}\n", v.x, v.y, v.z);
                    }
                });
            }
        }
    }

    fmt::print(f, "# list of primitives\n");
    for (auto&& [info, indices] : data->getIndexBuffers()) {
        writePrimitives(f, info, *indices, hasNormals, hasTextures);
    }
    if (data->getIndexBuffers().empty()) {
        std::vector<std::uint32_t> indices(data->getBuffer(0)->getSize());
        std::iota(indices.begin(), indices.end(), 0);
        auto ib = IndexBuffer(std::make_shared<IndexBufferRAM>(std::move(indices)));
        writePrimitives(f, data->getDefaultMeshInfo(), ib, hasNormals, hasTextures);
    }
}
}  // namespace inviwo
