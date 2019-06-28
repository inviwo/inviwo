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

#include <modules/base/io/wavefrontwriter.h>
#include <inviwo/core/util/stdextensions.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/io/datawriterexception.h>

#include <fstream>
#include <sstream>

namespace inviwo {

WaveFrontWriter::WaveFrontWriter() : DataWriterType<Mesh>() {
    addExtension(FileExtension("obj", "WaveFront Obj file format"));
}

WaveFrontWriter* WaveFrontWriter::clone() const { return new WaveFrontWriter(*this); }

void WaveFrontWriter::writeData(const Mesh* data, const std::string filePath) const {
    if (filesystem::fileExists(filePath) && !getOverwrite()) {
        throw DataWriterException("File already exists: " + filePath, IVW_CONTEXT);
    }
    auto f = filesystem::ofstream(filePath);
    writeData(data, f);
}

std::unique_ptr<std::vector<unsigned char>> WaveFrontWriter::writeDataToBuffer(
    const Mesh* data, const std::string& /*fileExtension*/) const {
    std::stringstream ss;
    writeData(data, ss);
    auto stringdata = ss.str();
    return std::make_unique<std::vector<unsigned char>>(stringdata.begin(), stringdata.end());
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
        auto pit = util::find_if(data->getBuffers(), [](const auto& buf) {
            return buf.first.type == BufferType::PositionAttrib;
        });

        if (pit == data->getBuffers().end()) {
            throw DataWriterException("Error: could not find a position buffer", IVW_CONTEXT);
        }
        const auto posRam = pit->second->getRepresentation<BufferRAM>();
        if (!posRam) {
            throw DataWriterException("Error: could not find a position buffer ram", IVW_CONTEXT);
        }
        if (posRam->getDataFormat()->getComponents() != 3) {
            throw DataWriterException("Error: Only 3 dimensional meshes are supported",
                                      IVW_CONTEXT);
        }

        f << "# List of vertex coordinates (" << posRam->getSize() << ")\n";
        posRam->dispatch<void, dispatching::filter::Vec3s>([&](auto pb) {
            for (const auto& elem : pb->getDataContainer()) {
                const auto v = proj(elem);
                f << "v " << v.x << " " << v.y << " " << v.z << "\n";
            }
        });
    }

    bool hasTextures = false;
    {
        auto tit = util::find_if(data->getBuffers(), [](const auto& buf) {
            return buf.first.type == BufferType::TexcoordAttrib;
        });
        if (tit != data->getBuffers().end()) {
            if (const auto texRam = tit->second->getRepresentation<BufferRAM>()) {
                f << "# List of texture coordinates (" << texRam->getSize() << ")\n";
                if (texRam->getDataFormat()->getComponents() == 3) {
                    hasTextures = true;
                    texRam->dispatch<void, dispatching::filter::Vec3s>([&](auto pb) {
                        for (const auto& elem : pb->getDataContainer()) {
                            const auto v = elem;
                            f << "vt " << v.x << " " << v.y << " " << v.z << "\n";
                        }
                    });
                } else if (texRam->getDataFormat()->getComponents() == 2) {
                    hasTextures = true;
                    texRam->dispatch<void, dispatching::filter::Vec2s>([&](auto pb) {
                        for (const auto& elem : pb->getDataContainer()) {
                            const auto v = elem;
                            f << "vt " << v.x << " " << v.y << "\n";
                        }
                    });
                }
            }
        }
    }

    bool hasNormals = false;
    {
        auto nit = util::find_if(data->getBuffers(), [](const auto& buf) {
            return buf.first.type == BufferType::NormalAttrib;
        });
        if (nit != data->getBuffers().end()) {
            if (const auto normRam = nit->second->getRepresentation<BufferRAM>()) {
                f << "# List of vertex normal (" << normRam->getSize() << ")\n";
                if (normRam->getDataFormat()->getComponents() == 3) {
                    hasNormals = true;
                    normRam->dispatch<void, dispatching::filter::Vec3s>([&](auto pb) {
                        for (const auto& elem : pb->getDataContainer()) {
                            const auto v = modelNormal * elem;
                            f << "vn " << v.x << " " << v.y << " " << v.z << "\n";
                        }
                    });
                }
            }
        }
    }

    std::function<void(const size_t& i1, const size_t& i2, const size_t& i3)> triangle = [&]() {
        if (hasTextures && hasNormals) {
            return std::function<void(const size_t& i1, const size_t& i2, const size_t& i3)>(
                [&f](const size_t& i1, const size_t& i2, const size_t& i3) -> void {
                    f << "f " << i1 + 1 << "/" << i1 + 1 << "/" << i1 + 1 << " " << i2 + 1 << "/"
                      << i2 + 1 << "/" << i2 + 1 << " " << i3 + 1 << "/" << i3 + 1 << "/" << i3 + 1
                      << "\n";
                });
        } else if (hasNormals && !hasTextures) {
            return std::function<void(const size_t& i1, const size_t& i2, const size_t& i3)>(
                [&f](const size_t& i1, const size_t& i2, const size_t& i3) -> void {
                    f << "f " << i1 + 1 << "//" << i1 + 1 << " " << i2 + 1 << "//" << i2 + 1 << " "
                      << i3 + 1 << "//" << i3 + 1 << "\n";
                });
        } else if (!hasNormals && hasTextures) {
            return std::function<void(const size_t& i1, const size_t& i2, const size_t& i3)>(
                [&f](const size_t& i1, const size_t& i2, const size_t& i3) -> void {
                    f << "f " << i1 + 1 << "/" << i1 + 1 << " " << i2 + 1 << "/" << i2 + 1 << " "
                      << i3 + 1 << "/" << i3 + 1 << "\n";
                });
        } else {  // only verties
            return std::function<void(const size_t& i1, const size_t& i2, const size_t& i3)>(
                [&f](const size_t& i1, const size_t& i2, const size_t& i3) -> void {
                    f << "f " << i1 + 1 << " " << i2 + 1 << " " << i3 + 1 << "\n";
                });
        }
    }();

    std::function<void(const size_t& i)> line = [&]() {
        if (hasTextures) {
            return std::function<void(const size_t& i)>(
                [&f](const size_t& i) -> void { f << i + 1 << "/" << i + 1; });
        } else {  // only verties
            return std::function<void(const size_t& i)>(
                [&f](const size_t& i) -> void { f << i + 1; });
        }
    }();

    f << "# list of primitives\n";
    for (const auto& inds : data->getIndexBuffers()) {
        const auto& indices = inds.second->getRAMRepresentation()->getDataContainer();
        switch (inds.first.dt) {
            case DrawType::Triangles: {
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
                        break;
                    default:
                        break;
                }
                break;
            }
            case DrawType::Lines: {
                switch (inds.first.ct) {
                    case ConnectivityType::None: {
                        for (size_t i = 0; i < indices.size() - 1u; i += 2u) {
                            f << "l ";
                            line(indices[i]);
                            f << " ";
                            line(indices[i + 1]);
                            f << "\n";
                        }
                        break;
                    }
                    case ConnectivityType::Strip: {
                        f << "l ";
                        for (size_t i = 0; i < indices.size(); i += 1u) {
                            line(indices[i]);
                            f << " ";
                        }
                        f << "\n";
                        break;
                    }
                    case ConnectivityType::Loop: {
                        f << "l ";
                        for (size_t i = 0; i < indices.size(); i += 1u) {
                            line(indices[i]);
                            f << " ";
                        }
                        line(indices[0]);
                        f << "\n";
                        break;
                    }
                    case ConnectivityType::Adjacency: {
                        for (size_t i = 0; i < indices.size() - 1u; i += 4) {
                            f << "l ";
                            line(indices[i + 1]);
                            f << " ";
                            line(indices[i + 2]);
                            f << "\n";
                        }
                        break;
                    }
                    case ConnectivityType::StripAdjacency: {
                        f << "l ";
                        for (size_t i = 1; i < indices.size() - 1u; i += 1u) {
                            line(indices[i]);
                            f << " ";
                        }
                        f << "\n";
                        break;
                    }
                    case ConnectivityType::Fan:
                    case ConnectivityType::NumberOfConnectivityTypes:
                    default:
                        break;
                }
                break;
            }

            case DrawType::Points: {
                f << "p ";
                for (size_t i = 0; i < indices.size(); i += 1) {
                    f << indices[i] << " ";
                }
                f << "\n";
                break;
            }
            case DrawType::NumberOfDrawTypes:
            case DrawType::NotSpecified:
            default: {
                std::stringstream err;
                err << "Draw type: \n" << inds.first.dt << "\" not supported";
                util::log(IVW_CONTEXT, err.str(), LogLevel::Warn, LogAudience::User);
            }
        }
    }
}

}  // namespace inviwo
