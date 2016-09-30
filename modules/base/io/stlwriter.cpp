/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016 Inviwo Foundation
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
#include <inviwo/core/util/stdextensions.h>

#include <fstream>

namespace inviwo {

StlWriter::StlWriter() : DataWriterType<Mesh>() {
    addExtension(FileExtension("stl","STL ASCII file format"));
}

StlWriter* StlWriter::clone() const {
    return new StlWriter(*this);
}

void StlWriter::writeData(const Mesh* data, const std::string filePath) const {
    auto it = util::find_if(data->getBuffers(), [](const auto& buf) {
        return buf.first.type == BufferType::PositionAttrib;
    });
    
    if (it == data->getBuffers().end()) return;

    const auto posBuffer = it->second;
    const auto model = data->getModelMatrix();
    std::ofstream f(filePath.c_str());
    
    const auto proj = [&](const auto& d1) {
        using GT = typename std::decay<decltype(d1)>::type;
        using T = typename GT::value_type;
        const auto tmp = model * glm::tvec4<T>(d1, 1.0);
        return tmp.xyz() / tmp.w;
    };

    const auto triangle = [&](const auto& d1, const auto& d2, const auto& d3) {
        const auto v1 = proj(d1);
        const auto v2 = proj(d2);
        const auto v3 = proj(d3);
        f << "facet normal 0.0 0.0 0.0\n"
          << "    outer loop\n"
          << "        vertex " << v1.x << " " << v1.y << " " << v1.z << "\n"
          << "        vertex " << v2.x << " " << v2.y << " " << v2.z << "\n"
          << "        vertex " << v3.x << " " << v3.y << " " << v3.z << "\n"
          << "    endloop\n"
          << "endfacet\n";
    };

    const auto exporter = [&](auto pb) {
        auto& pos = *pb;
        for (const auto& inds : data->getIndexBuffers()) {

            if (inds.first.dt != DrawType::Triangles) continue; // Only support Triangles

            const auto& indices = inds.second->getRAMRepresentation()->getDataContainer();
            switch (inds.first.ct) {
                case ConnectivityType::None: {
                    for (size_t i = 0; i < indices.size(); i += 3) {
                        triangle(pos[indices[i]], pos[indices[i + 1]], pos[indices[i + 2]]);
                    }
                    break;
                }
                case ConnectivityType::Strip: {
                    for (size_t i = 0; i < indices.size() - 3u; i += 2) {
                        triangle(pos[indices[i + 0]], pos[indices[i + 1]], pos[indices[i + 2]]);
                        triangle(pos[indices[i + 2]], pos[indices[i + 1]], pos[indices[i + 3]]);
                    }
                    break;
                }
                case ConnectivityType::Fan: {
                    for (size_t i = 1; i < indices.size() - 1u; i += 1) {
                        triangle(pos[indices[0]], pos[indices[i]], pos[indices[i + 1]]);
                    }
                    break;
                }
                case ConnectivityType::Adjacency: {
                    for (size_t i = 0; i < indices.size() - 1u; i += 6) {
                        triangle(pos[indices[i]], pos[indices[i + 2]], pos[indices[i + 4]]);
                    }
                    break;
                }
                case ConnectivityType::StripAdjacency: {
                    for (size_t i = 0; i < indices.size() - 6u; i += 4) {
                        triangle(pos[indices[i]], pos[indices[i + 2]], pos[indices[i + 4]]);
                        triangle(pos[indices[i + 4]], pos[indices[i + 2]], pos[indices[i + 6]]);
                    }
                    break;
                }
                case ConnectivityType::Loop:
                    break;
                default:
                    break;
            }
        }
    };
    
    f << "solid inviwo stl file\n";
    
    const auto ram = posBuffer->getRepresentation<BufferRAM>();

    if (ram && ram->getDataFormat()->getComponents() == 3) {
        ram->dispatch<void, dispatching::filter::Vec3s>([&](auto pb) -> void { exporter(pb); });
    }

    f << "endsolid inviwo stl file\n";
}

} // namespace

