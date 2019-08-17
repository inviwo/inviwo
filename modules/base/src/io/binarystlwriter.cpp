/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2019 Inviwo Foundation
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

#include <modules/base/io/binarystlwriter.h>
#include <inviwo/core/util/stdextensions.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/io/datawriterexception.h>

#include <fstream>
#include <sstream>

namespace inviwo {

BinarySTLWriter::BinarySTLWriter() : DataWriterType<Mesh>() {
    addExtension(FileExtension("stl", "STL Binary file format"));
}

BinarySTLWriter* BinarySTLWriter::clone() const { return new BinarySTLWriter(*this); }

void BinarySTLWriter::writeData(const Mesh* data, const std::string filePath) const {
    if (filesystem::fileExists(filePath) && !getOverwrite()) {
        throw DataWriterException("File already exists: " + filePath, IVW_CONTEXT);
    }
    auto f = filesystem::ofstream(filePath, std::ios_base::out | std::ios_base::binary);
    writeData(data, f);
}

std::unique_ptr<std::vector<unsigned char>> BinarySTLWriter::writeDataToBuffer(
    const Mesh* data, const std::string& /*fileExtension*/) const {
    std::stringstream ss(std::ios_base::binary);
    writeData(data, ss);
    auto stringdata = ss.str();
    return std::make_unique<std::vector<unsigned char>>(stringdata.begin(), stringdata.end());
}

void BinarySTLWriter::writeData(const Mesh* data, std::ostream& f) const {
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
        return glm::tvec4<T>(tmp) / tmp.w;
    };

    auto vertexprinter =
        posRam
            ->dispatch<std::function<void(std::vector<vec3>&, size_t)>, dispatching::filter::Vec3s>(
                [&](auto pb) -> std::function<void(std::vector<vec3>&, size_t)> {
                    return [&proj, pb](std::vector<vec3>& faces, size_t i) {
                        auto& pos = *pb;
                        const auto v = proj(pos[i]);
                        faces.push_back(static_cast<vec3>(v));
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
                        ->dispatch<std::function<void(std::vector<vec3>&, size_t, size_t, size_t)>,
                                   dispatching::filter::Vec3s>(
                            [&](auto nb) -> std::function<void(std::vector<vec3> & normals, size_t,
                                                               size_t, size_t)> {
                                return [&modelNormal, nb](std::vector<vec3>& normals, size_t i1,
                                                          size_t i2, size_t i3) {
                                    auto& norm = *nb;
                                    const auto n1 = modelNormal * norm[i1];
                                    const auto n2 = modelNormal * norm[i2];
                                    const auto n3 = modelNormal * norm[i3];
                                    const auto n = glm::normalize(n1 + n2 + n3);
                                    normals.push_back(static_cast<vec3>(n));
                                };
                            });
                }
            }
        }
        return std::function<void(std::vector<vec3>&, size_t, size_t, size_t)>(
            [](std::vector<vec3>& normals, size_t, size_t, size_t) -> void {
                normals.emplace_back(0.0f);
            });
    }();

    std::vector<vec3> floatdata;
    const auto triangle = [&](const auto& i1, const auto& i2, const auto& i3) {
        normalprinter(floatdata, i1, i2, i3);
        vertexprinter(floatdata, i1);
        vertexprinter(floatdata, i2);
        vertexprinter(floatdata, i3);
    };

    for (const auto& inds : data->getIndexBuffers()) {

        if (inds.first.dt != DrawType::Triangles) {
            std::stringstream err;
            err << "Draw type: \n" << inds.first.dt << "\" not supported";
            util::log(IVW_CONTEXT, err.str(), LogLevel::Warn, LogAudience::User);
            continue;
        }

        const auto& indices = inds.second->getRAMRepresentation()->getDataContainer();
        switch (inds.first.ct) {
            case ConnectivityType::None: {
                for (size_t i = 0; i < indices.size(); i += 3) {
                    triangle(indices[i], indices[i + 1], indices[i + 2]);
                }
                break;
            }
            case ConnectivityType::Strip: {
                for (size_t i = 0; i < indices.size() - 3u; i += 2) {
                    triangle(indices[i + 0], indices[i + 1], indices[i + 2]);
                    triangle(indices[i + 2], indices[i + 1], indices[i + 3]);
                }
                break;
            }
            case ConnectivityType::Fan: {
                for (size_t i = 1; i < indices.size() - 1u; i += 1) {
                    triangle(indices[0], indices[i], indices[i + 1]);
                }
                break;
            }
            case ConnectivityType::Adjacency: {
                for (size_t i = 0; i < indices.size() - 1u; i += 6) {
                    triangle(indices[i], indices[i + 2], indices[i + 4]);
                }
                break;
            }
            case ConnectivityType::StripAdjacency: {
                for (size_t i = 0; i < indices.size() - 6u; i += 4) {
                    triangle(indices[i], indices[i + 2], indices[i + 4]);
                    triangle(indices[i + 4], indices[i + 2], indices[i + 6]);
                }
                break;
            }
            case ConnectivityType::Loop:
            case ConnectivityType::NumberOfConnectivityTypes:
            default:
                break;
        }
    }

    std::uint16_t attrib{0};
    std::array<char, 80> header = {0};
    f.write(header.data(), header.size());
    std::uint32_t size = static_cast<std::uint32_t>(floatdata.size() / 4);
    f.write(reinterpret_cast<char*>(&size), sizeof(size));
    for (size_t i = 0; i < floatdata.size() - 4u; i += 4) {
        f.write(reinterpret_cast<char*>(&floatdata[i + 0]), sizeof(vec3));
        f.write(reinterpret_cast<char*>(&floatdata[i + 1]), sizeof(vec3));
        f.write(reinterpret_cast<char*>(&floatdata[i + 2]), sizeof(vec3));
        f.write(reinterpret_cast<char*>(&floatdata[i + 3]), sizeof(vec3));
        f.write(reinterpret_cast<char*>(&attrib), sizeof(attrib));
    }
}

}  // namespace inviwo
