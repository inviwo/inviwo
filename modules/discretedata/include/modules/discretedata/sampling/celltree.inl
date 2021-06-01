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

template <unsigned int SpatialDims>
CellTree<SpatialDims>::CellTree(std::shared_ptr<const Connectivity> grid,
                                std::shared_ptr<const DataChannel<double, SpatialDims>> coordinates,
                                const Interpolant<SpatialDims>& interpolant)
    : DataSetSampler<SpatialDims>(grid, coordinates, interpolant) {
    if (coordinates_->getGridPrimitiveType() != GridPrimitive::Vertex ||
        coordinates_->getNumComponents() != SpatialDims ||
        coordinates_->size() != grid->getNumElements()) {
        LogError("Incompatible grid and coordinate channel given for celltree, aborting.");
        return;
    }

    // CellTreeBuilder dispatcher;
    // coordinates_->dispatch<void, dispatching::filter::Scalars, SpatialDims,
    // SpatialDims>(dispatcher,
    //                                                                                      *this);
}

template <unsigned int SpatialDims>
CellTree<SpatialDims>::CellTree(CellTree<SpatialDims>& tree)
    : DataSetSampler<SpatialDims>(
          tree.grid_,
          std::dynamic_pointer_cast<const DataChannel<double, SpatialDims>>(tree.coordinates_),
          *tree.interpolant_)
    , nodes_(tree.nodes_)
    , cells_(tree.cells_) {}

template <unsigned int SpatialDims>
CellTree<SpatialDims>::CellTree(CellTree<SpatialDims>&& tree)
    : DataSetSampler<SpatialDims>(tree.grid_, tree.coordinates_, tree.interpolant_)
    , nodes_(std::move(tree.nodes_))
    , cells_(std::move(tree.cells_)) {}

template <unsigned int SpatialDims>
CellTree<SpatialDims>& CellTree<SpatialDims>::operator=(CellTree<SpatialDims>& tree) {
    nodes_ = tree.nodes_;
    cells_ = tree.cells_;
    this->interpolant_ = tree.interpolant_->copy();
}

template <unsigned int SpatialDims>
CellTree<SpatialDims>& CellTree<SpatialDims>::operator=(CellTree<SpatialDims>&& tree) {
    nodes_ = std::move(tree.nodes_);
    cells_ = std::move(tree.cells_);
    this->interpolant_ = std::move(tree.interpolant);
}

template <unsigned int SpatialDims>
template <typename T>
bool CellTree<SpatialDims>::sampleCell(ind cellId, const std::array<float, SpatialDims>& pos,
                                       std::vector<double>& weights,
                                       InterpolationType interpolationType) const {
    if (cellId >= coordinates_->size()) return false;
    auto coordinatesT = dynamic_cast<const DataChannel<T, SpatialDims>*>(coordinates_.get());
    ivwAssert(coordinatesT, "Coordinate channel not of the detected type.");
    if (!coordinatesT) return false;

    // Get cell vertices.
    std::vector<ind> vertIndices;
    grid_->getConnections(vertIndices, cellId, GridPrimitive(SpatialDims), GridPrimitive::Vertex);
    if (vertIndices.size() == 0) return false;

    std::vector<std::array<float, SpatialDims>> vertices;
    // vertices.resize(vertIndices.size());
    std::array<T, SpatialDims> originalCoord;
    for (ind v : vertIndices) {
        coordinatesT->fill(originalCoord, v);
        vertices.emplace_back();
        for (unsigned dim = 0; dim < SpatialDims; ++dim)
            (*vertices.back())[dim] = static_cast<float>(originalCoord[dim]);
    }

    // TODO: interpolant
    // return dynamic_cast<Interpolant<SpatialDims>>(grid_->getInterpolant())
    //     .getWeights(interpolationType, coordinatesT, weights, pos);
}

template <unsigned int SpatialDims>
ind CellTree<SpatialDims>::locateAndSampleCell(const std::array<float, SpatialDims>& pos,
                                               std::vector<double>& returnWeights,
                                               std::vector<ind>& returnVertices,
                                               InterpolationType interpolationType) const {
    // glm::vec<SpatialDims, float> posVec;
    // for (unsigned i = 0; i < SpatialDims; ++i) posVec[i] = pos[i];
    // const glm::vec<SpatialDims, float>&
    // return std::invoke<decltype(locateCellFunction), CellTree<SpatialDims>&,
    //                    const glm::vec<SpatialDims, float>&>(locateCellFunction, *this, posVec);
    return (this->*locateCellFunction)(pos);
}

template <unsigned int SpatialDims>
template <typename T, ind N>
ind CellTree<SpatialDims>::locateAndSampleCell(const std::array<float, SpatialDims>& pos,
                                               std::vector<double>& returnWeights,
                                               std::vector<ind>& returnVertices,
                                               InterpolationType interpolationType) const {

    for (size_t dim = 0; dim < SpatialDims; ++dim) {
        if (pos[dim] <= coordsMin_[dim] || pos[dim] >= coordsMax_[dim]) return -1;
    }

    std::stack<ind> todoNodes;
    todoNodes.push(0);
    do {
        auto& node = nodes_[todoNodes.top()];
        todoNodes.pop();
        while (!node.isLeaf()) {
            // nodes_[node.child]
            bool inLeftChild = pos[node.dim] <= node.node.Lmax;
            bool inRightChild = pos[node.dim] >= node.node.Rmin;
            if (inLeftChild && inRightChild) {
                float percBetweenMinMax =
                    (pos[node.dim] - node.node.Rmin) / (node.node.Lmax - node.node.Rmin);
                if (percBetweenMinMax < 0.5) {
                    node = nodes_[node.child];
                    todoNodes.push(node.child + 1);
                } else {
                    node = nodes_[node.child + 1];
                    todoNodes.push(node.child);
                }
                continue;
            }
            if (inLeftChild) {
                node = nodes_[node.child];
                continue;
            }
            node = nodes_[node.child + 1];
            continue;
        }
        for (ind c = node.leaf.start; c < node.leaf.start + node.leaf.size; ++c) {
            if (sampleCell<T>(cells_[c], pos, returnWeights, interpolationType)) return c;
        }
    } while (!todoNodes.empty());
    // todoNodes.push(0);
    // while (todoNodes.size() > 0) {
    //     const auto& node = nodes_[todoNodes.top()];
    //     todoNodes.pop();

    // }
    return -1;
}

template <unsigned int SpatialDims>
template <typename T, ind N>
void CellTree<SpatialDims>::CellTreeBuilder::operator()(const DataChannel<T, N>* positions,
                                                        CellTree<SpatialDims>& tree) {
    using Vec = typename DataChannel<T, N>::DefaultVec;
    using Coord = std::array<float, SpatialDims>;

    // Save pointer to correct cell location function to avoid future dispatches.
    tree.locateCellFunction = &CellTree<SpatialDims>::template locateAndSampleCell<T, N>;

    // Initialize data structures.
    // We assume approximately as many inner nodes as leaves, and leaves half full on average.
    ind numVertices = grid_->getNumElements();
    ind numNodesEstimate = numVertices / MAX_CELLS_PER_NODE * 4;
    tree.nodes_.reserve(numNodesEstimate);
    tree.cells_.resize(numVertices);
    std::iota(tree.cells_.begin(), tree.cells_.end(), 0);

    positions->getMinMax(tree.coordsMin_, tree.coordsMax_);
    Node node;
    node.leaf.start = 0;
    node.leaf.size = numVertices;
    nodes_.push_back(std::move(node));

    struct NodeInfo {
        ind Index;
        Coord Min, Range;
    };
    std::stack<NodeInfo> todoNodes;
    NodeInfo info;
    // info.Index = 0;
    // info.Min = tree.coordsMin;
    // info.Range = tree.coordsMax_ - tree.coordsMin_;
    // todoNodes.push(std::move(info));
    todoNodes.push({0, tree.coordsMin_, tree.coordsMax_ - tree.coordsMin_});

    std::array<ind, SpatialDims * NUM_SPLIT_BUCKETS> bucketsPerSplitDimension;
    std::vector<ind> vertices;
    Vec pos;

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

        // Assemble number of cells per bucket.
        for (unsigned c = node.leaf.start; c < node.leaf.start + numCells; ++c) {
            ind cellIdx = tree.cells_[c];
            tree.grid_->getConnections(vertices, cellIdx, GridPrimitive(SpatialDims),
                                       GridPrimitive::Vertex);
            ivwAssert(vertices.size() > 0, "Encountered cell without vertices.");

            positions->fill(pos, vertices[0]);
            vertices.clear();
            for (unsigned dim = 0; dim < SpatialDims; ++dim) {
                ind bucketIdx = ((static_cast<float>(pos[dim]) - nodeInfo.Min) / nodeInfo.Range) *
                                NUM_SPLIT_BUCKETS;
                // bucketIdx = std::min(NUM_SPLIT_BUCKETS - 1, bucketIdx); // Safety.

                bucketsPerSplitDimension[dim * NUM_SPLIT_BUCKETS + bucketIdx]++;
            }
        }

        // Find best dimension to split in, and bewtween which two buckets.
        std::array<std::tuple<ind, ind, bool>, SpatialDims> optimalSplitPerDimensions;
        for (unsigned dim = 0; dim < SpatialDims; ++dim) {
            ind sum = 0;
            for (unsigned int bucket = 0; bucket < NUM_SPLIT_BUCKETS; ++bucket) {
                ind nextSum = sum + bucketsPerSplitDimension[NUM_SPLIT_BUCKETS * dim + bucket];
                if (nextSum > numCells / 2) {
                    // Take the split with the
                    optimalSplitPerDimensions[dim] =
                        (sum >= numCells - nextSum)
                            ? std::tuple<ind, ind, bool>{bucket - 1, sum, true}
                            : std::tuple<ind, ind, bool>{bucket, numCells - nextSum, false};
                }
            }
        }
        auto bestDim =
            std::max_element(optimalSplitPerDimensions.begin(), optimalSplitPerDimensions.end(),
                             [](auto& i, auto& j) { return i.second() < j.second(); });
        unsigned int bestDimIdx = std::distance(bestDim, optimalSplitPerDimensions.begin());
        float bestSplit = nodeInfo.Min + ((nodeInfo.Range * bestDimIdx) / NUM_SPLIT_BUCKETS);

        ind firstHalfSize = bestDim->third() ? bestDim->second() : numCells - bestDim->second();

        // Create child nodes, add them to the todo list.
        Node childNode;
        childNode.leaf.start = node.leaf.start;
        childNode.leaf.size = firstHalfSize;
        tree.nodes_.push_back(childNode);

        childNode.leaf.start = node.leaf.start + firstHalfSize;
        childNode.leaf.size = numCells - firstHalfSize;
        tree.nodes_.push_back(childNode);

        todoNodes.push({tree.nodes_.size() - 2, nodeInfo.Min, nodeInfo.Range});
        todoNodes.push({tree.nodes_.size() - 1, nodeInfo.Min, nodeInfo.Range});
        node.Child = tree.nodes_.size() - 2;

        auto& leftChild = tree.nodes_[tree.nodes_.size() - 2];
        auto& rightChild = *tree.nodes_.back();
        auto& leftTodo = todoNodes[todoNodes.size() - 2];
        auto& rightTodo = *todoNodes.back();

        // Split vertex indices and get min/max along chosen dimension.
        node.node.Lmax = nodeInfo.Min;
        node.node.Rmin = nodeInfo.Min + nodeInfo.Range;
        auto startRight = tree.cells_.begin() + rightChild.leaf.start;
        auto endRight = tree.cells_.begin() + rightChild.leaf.start + rightChild.leaf.size;
        // auto itLeft = tree.cells_.begin() + leftChild.leaf.start;
        // auto itRight = startRight;
        // Vec posLeft, posRight;

        auto it = tree.cells_.begin() + leftChild.leaf.start;
        auto itMiddle = tree.cells_.begin() + rightChild.leaf.start;
        auto itEnd = tree.cells_.begin() + rightChild.leaf.start + rightChild.leaf.size;
        bool rightSide = false;

        while (it != itEnd) {
            if (it == itMiddle) rightSide = true;

            tree.grid_->getConnections(vertices, *it, GridPrimitive(SpatialDims),
                                       GridPrimitive::Vertex);
            positions->fill(pos, vertices[0]);

            // If necessary, swap to the right.
            if (!rightSide && pos[bestDimIdx] >= bestSplit) {
                itEnd--;
                std::swap(*it, *itEnd);
            } else {
                it++;
            }

            if (rightSide || pos[bestDimIdx] >= bestSplit) {
                node.node.Rmin = std::min(node.node.Rmin, pos[bestDimIdx]);
                for (size_t v = 1; v < vertices.size(); ++v) {
                    positions->fill(pos, vertices[v]);
                    node.node.Rmin = std::min(node.node.Rmin, pos[bestDimIdx]);
                }
            } else {
                node.node.Lmax = std::max(node.node.Lmax, pos[bestDimIdx]);
                for (size_t v = 1; v < vertices.size(); ++v) {
                    positions->fill(pos, vertices[v]);
                    node.node.Lmax = std::max(node.node.Lmax, pos[bestDimIdx]);
                }
            }
        }
    }
}
}  // namespace discretedata
}  // namespace inviwo
