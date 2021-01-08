/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019-2021 Inviwo Foundation
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

#pragma once

#include <modules/meshrenderinggl/meshrenderingglmoduledefine.h>

#include <inviwo/core/datastructures/geometry/mesh.h>
#include <inviwo/core/datastructures/buffer/buffer.h>

#include <inviwo/core/util/transformiterator.h>
#include <inviwo/core/util/stdextensions.h>

#include <vector>
#include <unordered_map>
#include <optional>

namespace inviwo {

/**
 * \brief A half edge datastructure of the mesh topology.
 * Note: only the topology is stored, no vertex data.
 *
 * Code ideas taken from https://github.com/yig/halfedge and http://prideout.net/blog/?p=54,
 * both are public domain (11/12/2017).
 *
 *
 *             v2────────────────v3  edge │ vertex face  next  twin
 *            ╱ ╲ ◀────e5─────▲ ╱    ─────┼────────────────────────
 *           ╱ ▲ ╲ ╲         ╱ ╱      e0  │   v0    f1    e1    -
 *          ╱ ╱ ╲ ╲ ╲  f1   ╱ ╱       e1  │   v1    f1    e2    e3
 *         ╱ ╱   ╲ ╲e3    e4 ╱        e2  │   v2    f1    e0    -
 *        ╱e2    e1 ╲ ╲   ╱ ╱         e3  │   v2    f2    e4    e1
 *       ╱ ╱  f0   ╲ ╲ ╲ ╱ ╱          e4  │   v1    f2    e5    -
 *      ╱ ╱         ╲ ╲ ▼ ╱           e5  │   v3    f2    e3    -
 *     ╱ ▼────e0─────▶ ╲ ╱
 *   v0────────────────v1
 *
 */

class IVW_MODULE_MESHRENDERINGGL_API HalfEdges {
public:
    /**
     * \brief Construct from MeshInfo and index buffer.
     */
    HalfEdges(Mesh::MeshInfo info, const IndexBuffer& indexBuffer);

    /**
     * \brief Construct from Mesh, only triangles are considered
     */
    HalfEdges(const Mesh& mesh);

    /**
     * \brief Creates a index buffer for triangles with connectivity 'None'
     */
    IndexBuffer createIndexBuffer() const;

    /**
     * \brief Creates a index buffer for triangles with connectivity 'Adjacency'
     */
    IndexBuffer createIndexBufferWithAdjacency() const;

    class IVW_MODULE_MESHRENDERINGGL_API EdgeIter {
    public:
        EdgeIter() = default;
        using difference_type = std::uint32_t;
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type = EdgeIter;
        using pointer = const EdgeIter*;
        using reference = const EdgeIter&;

        std::uint32_t vertex() const;
        std::uint32_t face() const;
        EdgeIter next() const;
        EdgeIter prev() const;
        std::optional<EdgeIter> twin() const;

        EdgeIter& operator++() {
            edgeIndex_ = edges_->edges_[edgeIndex_].next;
            return *this;
        }
        EdgeIter operator++(int) {
            auto it = *this;
            operator++();
            return it;
        }

        reference operator*() const { return *this; }
        pointer operator->() const { return this; }

        bool operator==(const EdgeIter& rhs) const { return edgeIndex_ == rhs.edgeIndex_; }
        bool operator!=(const EdgeIter& rhs) const { return edgeIndex_ != rhs.edgeIndex_; }

    private:
        friend HalfEdges;
        EdgeIter(const HalfEdges* edges, std::uint32_t edgeIndex)
            : edges_{edges}, edgeIndex_{edgeIndex} {}

        const HalfEdges* edges_ = nullptr;
        std::uint32_t edgeIndex_ = 0;
    };

    EdgeIter faceToEdge(std::uint32_t faceIndex) const;
    EdgeIter vertexToEdge(std::uint32_t vertexIndex) const;

    auto faces() const;
    auto vertices() const;

private:
    friend EdgeIter;

    /**
     * \brief A single half edge
     */
    struct IVW_MODULE_MESHRENDERINGGL_API HalfEdge {
        /**
         * \brief index of the vertex the half edge points to
         */
        std::uint32_t vertex;
        /**
         * \brief Index of the adjacent face / triangle
         */
        std::uint32_t face;
        /**
         * \brief Next half edge around the face
         */
        std::uint32_t next;
        /**
         * \brief Next half edge around the face
         */
        std::uint32_t prev;

        /**
         * \brief Twin half edge, opposite direction.
         * nullopt if border.
         */
        std::optional<std::uint32_t> twin = std::nullopt;
    };

    std::vector<HalfEdge> edges_;
    std::unordered_map<std::uint32_t, std::uint32_t> vertexToEdge_;
    std::unordered_map<std::uint32_t, std::uint32_t> faceToEdge_;
};

inline auto HalfEdges::faceToEdge(std::uint32_t faceIndex) const -> EdgeIter {
    return {this, faceToEdge_.at(faceIndex)};
}

inline auto HalfEdges::vertexToEdge(std::uint32_t vertexIndex) const -> EdgeIter {
    return {this, vertexToEdge_.at(vertexIndex)};
}

inline auto HalfEdges::faces() const {
    const auto transform =
        [this](
            const std::unordered_map<std::uint32_t, std::uint32_t>::value_type& item) -> EdgeIter {
        return {this, item.second};
    };

    return util::as_range(util::makeTransformIterator(transform, faceToEdge_.begin()),
                          util::makeTransformIterator(transform, faceToEdge_.end()));
}

inline auto HalfEdges::vertices() const {
    const auto transform =
        [this](
            const std::unordered_map<std::uint32_t, std::uint32_t>::value_type& item) -> EdgeIter {
        return {this, item.second};
    };

    return util::as_range(util::makeTransformIterator(transform, vertexToEdge_.begin()),
                          util::makeTransformIterator(transform, vertexToEdge_.end()));
}

inline std::uint32_t HalfEdges::EdgeIter::vertex() const {
    return edges_->edges_[edgeIndex_].vertex;
}
inline std::uint32_t HalfEdges::EdgeIter::face() const { return edges_->edges_[edgeIndex_].face; }

inline auto HalfEdges::EdgeIter::next() const -> EdgeIter {
    return {edges_, edges_->edges_[edgeIndex_].next};
}

inline auto HalfEdges::EdgeIter::prev() const -> EdgeIter {
    return {edges_, edges_->edges_[edgeIndex_].prev};
}

inline auto HalfEdges::EdgeIter::twin() const -> std::optional<EdgeIter> {
    if (auto twin = edges_->edges_[edgeIndex_].twin) {
        return EdgeIter{edges_, *twin};
    } else {
        return std::nullopt;
    }
}

}  // namespace inviwo
