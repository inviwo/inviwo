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

#ifndef IVW_HALFEDGES_H
#define IVW_HALFEDGES_H

#include <modules/fancymeshrenderer/fancymeshrenderermoduledefine.h>

#include <vector>
#include <map>
#include <inviwo/core/datastructures/geometry/mesh.h>

namespace inviwo {

/**
 * \brief A half edge datastructure of the mesh topology.
 * Note: only the topology is stored, no vertex data.
 *
 * Code ideas taken from https://github.com/yig/halfedge and http://prideout.net/blog/?p=54,
 * both are public domain (11/12/2017).
 */
class IVW_MODULE_FANCYMESHRENDERER_API HalfEdges {
public:
    typedef int index_t;

    /**
     * \brief A single half edge
     */
    struct HalfEdge {
        /**
         * \brief index of the vertex the half edge points to
         */
        index_t toVertex_ = -1;
        /**
         * \brief Index of the adjacent face / triangle
         */
        index_t toFace_ = -1;
        /**
         * \brief Twin half edge, opposite direction.
         * NULL if border.
         */
        HalfEdge* twin_ = nullptr;
        /**
         * \brief Next half edge around the face
         */
        HalfEdge* next_ = nullptr;
    };

private:
    std::vector<HalfEdge> edges_;
    std::map<index_t, HalfEdge*> vertexToEdge_;

public:
    /**
     * \brief Creates the half edges from the given index buffer.
     * Must be an index buffer that defines single triangles.
     * \param indexBuffer the index buffer
     */
    HalfEdges(const IndexBuffer* const indexBuffer);

    /**
     * \brief Creates the index buffer for triangles with connectivity 'None'
     * \return the index buffer
     */
    std::shared_ptr<IndexBuffer> createIndexBuffer();

    /**
     * \brief Creates the index buffer for triangles with connectivity 'Adjacency'
     * \return the index buffer
     */
    std::shared_ptr<IndexBuffer> createIndexBufferWithAdjacency();

    HalfEdge* faceToEdge(index_t faceIndex) { return &edges_[faceIndex * 3]; }
    const HalfEdge* faceToEdge(index_t faceIndex) const { return &edges_[faceIndex * 3]; }
    HalfEdge* vertexToEdge(index_t vertexIndex) { return vertexToEdge_.at(vertexIndex); }
    const HalfEdge* vertexToEdge(index_t vertexIndex) const {
        return vertexToEdge_.at(vertexIndex);
    }
};

}  // namespace inviwo

#endif
