/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2019 Inviwo Foundation
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

#include <modules/base/algorithm/mesh/meshconverter.h>
#include <inviwo/core/datastructures/buffer/buffer.h>
#include <inviwo/core/datastructures/geometry/geometrytype.h>
#include <inviwo/core/util/stdextensions.h>

namespace inviwo {

namespace meshutil {

std::unique_ptr<Mesh> toPointMesh(const Mesh& mesh) {
    auto res = std::make_unique<Mesh>();
    res->copyMetaDataFrom(mesh);
    res->setModelMatrix(mesh.getModelMatrix());
    res->setWorldMatrix(mesh.getWorldMatrix());

    auto posIt = util::find_if(mesh.getBuffers(), [](const auto& buf) {
        return buf.first.type == BufferType::PositionAttrib;
    });

    if (posIt == mesh.getBuffers().end()) {
        throw Exception("Error: could not find a position buffer",
                        IVW_CONTEXT_CUSTOM("toPointMesh"));
    }

    res->addBuffer(posIt->first, std::shared_ptr<BufferBase>(posIt->second->clone()));

    auto colorIt = util::find_if(mesh.getBuffers(), [](const auto& buf) {
        return buf.first.type == BufferType::ColorAttrib;
    });
    if (colorIt != mesh.getBuffers().end()) {
        res->addBuffer(colorIt->first, std::shared_ptr<BufferBase>(colorIt->second->clone()));
    }

    for (const auto& inds : mesh.getIndexBuffers()) {
        res->addIndices(Mesh::MeshInfo{DrawType::Points, ConnectivityType::None},
                        std::shared_ptr<IndexBuffer>(inds.second->clone()));
    }
    return res;
}

std::unique_ptr<Mesh> toLineMesh(const Mesh& mesh) {
    auto res = std::make_unique<Mesh>();
    res->copyMetaDataFrom(mesh);
    res->setModelMatrix(mesh.getModelMatrix());
    res->setWorldMatrix(mesh.getWorldMatrix());

    auto posIt = util::find_if(mesh.getBuffers(), [](const auto& buf) {
        return buf.first.type == BufferType::PositionAttrib;
    });

    if (posIt == mesh.getBuffers().end()) {
        throw Exception("Error: could not find a position buffer",
                        IVW_CONTEXT_CUSTOM("toPointMesh"));
    }

    res->addBuffer(posIt->first, std::shared_ptr<BufferBase>(posIt->second->clone()));

    auto colorIt = util::find_if(mesh.getBuffers(), [](const auto& buf) {
        return buf.first.type == BufferType::ColorAttrib;
    });
    if (colorIt != mesh.getBuffers().end()) {
        res->addBuffer(colorIt->first, std::shared_ptr<BufferBase>(colorIt->second->clone()));
    }

    for (const auto& inds : mesh.getIndexBuffers()) {
        if (inds.first.dt == DrawType::Points) {
            continue;
        }

        if (inds.first.dt == DrawType::Lines) {
            res->addIndices(inds.first, std::shared_ptr<IndexBuffer>(inds.second->clone()));
        }

        if (inds.first.dt == DrawType::Triangles) {
            const auto& indices = inds.second->getRAMRepresentation()->getDataContainer();

            auto& ib =
                res->addIndexBuffer(DrawType::Lines, ConnectivityType::None)->getDataContainer();
            const auto triangle = [&](const auto& i1, const auto& i2, const auto& i3) {
                ib.push_back(i1);
                ib.push_back(i2);
                ib.push_back(i2);
                ib.push_back(i3);
                ib.push_back(i3);
                ib.push_back(i1);
            };

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
    }
    return res;
}
}  // namespace meshutil

}  // namespace inviwo
