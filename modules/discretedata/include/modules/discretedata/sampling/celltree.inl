/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2020 Inviwo Foundation
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

#include <modules/discretedata/sampling/celltree.h>
#include <inviwo/core/util/zip.h>

#include <stack>

namespace inviwo {
namespace discretedata {

// namespace dd_detail {

// }  // namespace dd_detail

template <unsigned int SpatialDims>
CellTree<SpatialDims>::CellTree(std::shared_ptr<const Connectivity> grid,
                                std::shared_ptr<const Channel> coordinates)
    : grid_(grid), coordinates_(coordinates) {
    if (coordinates_->getGridPrimitiveType() != GridPrimitive::Vertex ||
        coordinates_->getNumComponents() != SpatialDims ||
        coordinates_->size() != grid->getNumElements()) {
        LogError("Incompatible grid and coordinate channel given for celltree, aborting.");
        return;
    }

    CellTreeBuilder dispatcher;
    coordinates_->dispatch<void, dispatching::filter::Scalars, SpatialDims, SpatialDims>(dispatcher,
                                                                                         *this);
}

template <unsigned int SpatialDims>
CellTree<SpatialDims>::CellTree(CellTree&& tree)
    : grid_(tree.grid_)
    , coordinates_(tree.coordinates_)
    , nodes_(std::move(tree.nodes_))
    , cells_(std::move(cells_.nodes_)) {}

template <unsigned int SpatialDims>
ind CellTree<SpatialDims>::locateCell(const std::array<float, SpatialDims>& pos) const {
    return (this->*locateCellFunction)(pos);
}

template <unsigned int SpatialDims>
template <typename T, ind N>
ind CellTree<SpatialDims>::locateCell(const std::array<float, SpatialDims>& pos) const {
    // for (auto&& [p, min, max] : util::zip(pos, coordsMin_, coordsMax_)) {
    //     if (min >= p || p >= max) return;
    // }

    for (size_t dim = 0; dim < SpatialDims; ++dim) {
        if (pos[dim] <= coordsMin_[dim] || pos[dim] >= coordsMax_[dim]) return;
    }
}

template <unsigned int SpatialDims>
template <typename T, ind N>
void CellTree<SpatialDims>::CellTreeBuilder::operator()(const DataChannel<T, N>* positions,
                                                        CellTree<SpatialDims>& tree) {
    using Vec = DataChannel<T, N>::DefaultVec;
    using Coord = std::array<float, SpatialDims>;

    // Save pointer to correct cell location function to avoid future dispatches.
    tree.locateCellFunction = &CellTree<SpatialDims>::locateCell<T, N>;

    // Initialize data structures.
    // We assume approximately as many inner nodes as leaves, and leaves half full on average.
    ind numVertices = grid->getNumElements();
    ind numNodesEstimate = numVertices / MAX_CELLS_PER_NODE * 4;
    tree.nodes_.reserve(numNodesEstimate);
    tree.cells_.resize(numVertices);
    std::iota(tree.cells_.begin(), tree.cells_.end(), 0);

    positions->getMinMax(tree.coordsMin_, tree.coordsMax_);
    nodes_.push_back({.leaf.start = 0, .leaf.size = numVertices});

    struct NodeInfo {
        ind Index;
        Coord Min, Range;
    };
    std::stack<NodeInfo> todoNotes;  // std::tuple<ind, Coord, Coord>> todoNodes;
    todoNodes.push(
        {.Index = 0, .Min = tree.coordsMin_, .Range = tree.coordsMax_ - tree.coordsMin_});

    std::array<std::tuple<ind, float, float>, SpatialDims * NUM_SPLIT_BUCKETS>
        bucketsPerSplitDimension;
    std::vector<ind> vertices;
    Vec pos, avg, min, max;
    // std::array<ind, NUM_SPLIT_BUCKETS>

    while (todoNodes.size() > 0) {
        auto nodeInfo = todoNodes.top();
        todoNodes.pop();
        auto& node = tree.nodes_[nodeInfo.Index];
        ind numCells = node.leaf.size;

        // Should be leaf?
        if (numCells <= MAX_CELLS_PER_NODE) {
            node.child = 0;
            continue;
        }

        for (unsigned c = node.leaf.start; c < node.leaf.start + numCells) {
            ind cellIdx = tree.cells_[c];
            tree.grid_->getConnections(vertices, cellIdx, GridPrimitive(SpatialDims),
                                       GridPrimitive::Vertex);

            // Get the average, minimal and maximal dimension.
            // Could precompute once per tree build, but might exceed memory.
            for (auto& vert : vertices) {
                positions->fill(pos, vert);
                avg += pos;
                for (unsigned dim = 0; dim < SpatialDims; ++dim) {
                    min[dim] = std::min(min[dim], pos[dim]);
                    max[dim] = std::max(max[dim], pos[dim]);
                }
            }
            avg /= vertices.size();
            for (unsigned dim = 0; dim < SpatialDims; ++dim) {
                ind bucketIdx = (avg[dim] - nodeInfo.Min) / (nodeInfo.Range) * NUM_SPLIT_BUCKETS;
                bucketIdx = std::min(NUM_SPLIT_BUCKETS - 1, bucketIdx);

                auto& bucket = bucketsPerSplitDimension[dim * NUM_SPLIT_BUCKETS + bucketIdx];
                bucket.first++;
                bucket.second = std::min(bucket.second, min[dim]);
                bucket.third = std::min(bucket.third, max[dim]);
            }
        }
    }
}

}  // namespace discretedata
}  // namespace inviwo
