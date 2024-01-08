/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019-2024 Inviwo Foundation
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

#include <modules/meshrenderinggl/datastructures/halfedges.h>

#include <inviwo/core/datastructures/buffer/buffer.h>              // for IndexBuffer
#include <inviwo/core/datastructures/buffer/bufferramprecision.h>  // for IndexBufferRAM
#include <inviwo/core/datastructures/geometry/geometrytype.h>      // for DrawType, DrawType::Tr...
#include <inviwo/core/datastructures/geometry/mesh.h>              // for Mesh, Mesh::IndexVector
#include <inviwo/core/util/assertion.h>                            // for IVW_ASSERT
#include <inviwo/core/util/iterrange.h>                            // for iter_range
#include <inviwo/core/util/transformiterator.h>                    // for TransformIterator
#include <modules/base/algorithm/meshutils.h>                      // for forEachTriangle

#include <map>          // for map, operator!=, __map...
#include <memory>       // for make_shared, shared_ptr
#include <type_traits>  // for remove_reference<>::type
#include <utility>      // for pair, move

namespace inviwo {

HalfEdges::HalfEdges(Mesh::MeshInfo info, const IndexBuffer& indexBuffer) {
    // map to fill in the opposite direction
    // (start_vertex, end_vertex) -> edge_index
    std::map<std::pair<std::uint32_t, std::uint32_t>, std::uint32_t> edgeMap;

    std::uint32_t face = 0;
    meshutil::forEachTriangle(info, indexBuffer,
                              [&](std::uint32_t a, std::uint32_t b, std::uint32_t c) {
                                  // a-b, b-c, c-a
                                  const auto count = static_cast<std::uint32_t>(edges_.size());
                                  edges_.push_back(HalfEdge{a, face, count + 1, count + 2});
                                  edgeMap.try_emplace({a, b}, count + 0);
                                  vertexToEdge_.try_emplace(a, count + 0);

                                  edges_.push_back(HalfEdge{b, face, count + 2, count + 0});
                                  edgeMap.try_emplace({b, c}, count + 1);
                                  vertexToEdge_.try_emplace(b, count + 1);

                                  edges_.push_back(HalfEdge{c, face, count + 0, count + 1});
                                  edgeMap.try_emplace({c, a}, count + 2);
                                  vertexToEdge_.try_emplace(c, count + 2);

                                  faceToEdge_.try_emplace(face, count);

                                  ++face;
                              });

    for (auto& edge : edges_) {
        const auto a = edge.vertex;
        const auto b = edges_[edge.next].vertex;

        auto it = edgeMap.find(std::pair{b, a});
        if (it != edgeMap.end()) {
            edge.twin = it->second;
        }
    }
}

HalfEdges::HalfEdges(const Mesh& mesh) {

    std::map<std::pair<std::uint32_t, std::uint32_t>, std::uint32_t> edgeMap;
    std::uint32_t face = 0;

    for (auto [info, indexBuffer] : mesh.getIndexBuffers()) {
        if (info.dt != DrawType::Triangles) continue;
        meshutil::forEachTriangle(info, *indexBuffer,
                                  [&](std::uint32_t a, std::uint32_t b, std::uint32_t c) {
                                      // a-b, b-c, c-a
                                      const auto count = static_cast<std::uint32_t>(edges_.size());
                                      edges_.push_back(HalfEdge{a, face, count + 1, count + 2});
                                      edgeMap.try_emplace({a, b}, count + 0);
                                      vertexToEdge_.try_emplace(a, count + 0);

                                      edges_.push_back(HalfEdge{b, face, count + 2, count + 0});
                                      edgeMap.try_emplace({b, c}, count + 1);
                                      vertexToEdge_.try_emplace(b, count + 1);

                                      edges_.push_back(HalfEdge{c, face, count + 0, count + 1});
                                      edgeMap.try_emplace({c, a}, count + 2);
                                      vertexToEdge_.try_emplace(c, count + 2);

                                      faceToEdge_.try_emplace(face, count);

                                      ++face;
                                  });
    }

    for (auto& edge : edges_) {
        const auto a = edge.vertex;
        const auto b = edges_[edge.next].vertex;

        auto it = edgeMap.find(std::pair{b, a});
        if (it != edgeMap.end()) {
            edge.twin = it->second;
        }
    }
}

IndexBuffer HalfEdges::createIndexBuffer() const {
    std::vector<std::uint32_t> inds;
    inds.reserve(edges_.size());

    // walk faces
    for (auto edge : faces()) {
        auto ei = edge;
        inds.push_back((ei++).vertex());
        inds.push_back((ei++).vertex());
        inds.push_back((ei++).vertex());
        IVW_ASSERT(ei == edge, "Only triangles are supported");
    }
    return IndexBuffer(std::make_shared<IndexBufferRAM>(std::move(inds)));
}

IndexBuffer HalfEdges::createIndexBufferWithAdjacency() const {
    std::vector<std::uint32_t> inds;
    inds.reserve(edges_.size() * 2);

    // walk faces
    for (auto edge : faces()) {
        auto ei = edge;
        for (std::uint32_t v = 0; v < 3; ++v) {
            inds.push_back(ei.vertex());
            if (auto twin = ei.twin()) {
                // add opposite vertex
                inds.push_back(twin->prev().vertex());
            } else {
                // border! Add opposite vertex of own triangle (as if the tris is flipped)
                inds.push_back(ei.prev().vertex());
            }
            ++ei;
        }
        IVW_ASSERT(ei == edge, "Only triangles are supported");
    }

    return IndexBuffer(std::make_shared<IndexBufferRAM>(std::move(inds)));
}

}  // namespace inviwo
