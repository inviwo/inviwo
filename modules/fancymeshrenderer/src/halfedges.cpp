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

#include <modules/fancymeshrenderer/halfedges.h>

#include <iostream>

namespace inviwo {

HalfEdges::HalfEdges(const IndexBuffer* const indexBuffer) {
    // get CPU representation
    const auto* const indices = indexBuffer->getRAMRepresentation();
    index_t numTris = static_cast<index_t>(indices->getSize()) / 3;

    // map to fill in the opposite direction
    //(start_vertex, end_vertex) -> edge_index
    std::map<std::pair<index_t, index_t>, index_t> edgeMap;

    // allocate memory and set basic properties
    edges_.resize(3 * numTris);
    for (index_t tri = 0; tri < numTris; ++tri) {
        for (index_t v = 0; v < 3; ++v) {
            index_t i1 = 3 * tri + v;
            index_t i2 = 3 * tri + ((v + 1) % 3);
            edges_[i1].toFace_ = tri;
            edges_[i1].toVertex_ = indices->get(i2);
            edges_[i1].next_ = &edges_[i2];
            edgeMap[std::pair<index_t, index_t>(indices->get(i1), indices->get(i2))] = i1;
            vertexToEdge_[indices->get(i1)] = &edges_[i1];
        }
    }

    // fill pointers to opposite edges
    for (index_t tri = 0; tri < numTris; ++tri) {
        for (index_t v = 0; v < 3; ++v) {
            index_t i1 = 3 * tri + v;
            index_t i2 = 3 * tri + ((v + 1) % 3);
            if (edges_[i1].twin_ != nullptr) continue;
            auto it = edgeMap.find(std::pair<index_t, index_t>(indices->get(i2), indices->get(i1)));
            if (it != edgeMap.end()) {
                edges_[i1].twin_ = &edges_[it->second];
                edges_[it->second].twin_ = &edges_[i1];
            } else {
                std::cout << "Missing twin edge" << std::endl;
            }
        }
    }

    // Walk backwards around the vertices until a border is found (if at all)
    // This allows the 'walkVertex' to visit all edges from border to border
    // TODO
}

std::shared_ptr<IndexBuffer> HalfEdges::createIndexBuffer() {
    std::shared_ptr<IndexBuffer> buffer = std::make_shared<IndexBuffer>();
    auto* indices = buffer->getEditableRAMRepresentation();
    indices->reserve(edges_.size());
    // walk faces
    index_t numTris = static_cast<index_t>(edges_.size()) / 3;
    for (index_t tri = 0; tri < numTris; ++tri) {
        for (index_t v = 0; v < 3; ++v) {
            index_t i1 = 3 * tri + v;
            indices->add(edges_[i1].toVertex_);
        }
    }
    return buffer;
}

std::shared_ptr<IndexBuffer> HalfEdges::createIndexBufferWithAdjacency() {
    std::shared_ptr<IndexBuffer> buffer = std::make_shared<IndexBuffer>();
    auto* indices = buffer->getEditableRAMRepresentation();
    indices->reserve(edges_.size() * 2);
    // walk faces
    index_t numTris = static_cast<index_t>(edges_.size()) / 3;
    for (index_t tri = 0; tri < numTris; ++tri) {
        for (index_t v = 0; v < 3; ++v) {
            index_t i1 = 3 * tri + v;
            indices->add(edges_[i1].toVertex_);
            // add adjacency info
            HalfEdge* o = edges_[i1].next_->twin_;
            if (o == nullptr) {
                // border! Add opposite vertex of own triangle (as if the tris is flipped)
                indices->add(edges_[3 * tri + ((v + 2) % 3)].toVertex_);
            } else {
                // add opposite vertex
                indices->add(o->next_->toVertex_);
            }
        }
    }
    return buffer;
}
}  // namespace inviwo
