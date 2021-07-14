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
#include <fmt/format.h>

#include <stack>

namespace inviwo {
namespace discretedata {

template <unsigned int SpatialDims>
CellTree<SpatialDims>::CellTree(std::shared_ptr<const Connectivity> grid,
                                std::shared_ptr<const DataChannel<double, SpatialDims>> coordinates,
                                const Interpolant<SpatialDims>& interpolant)
    : DataSetSampler<SpatialDims>(grid, coordinates, interpolant) {
    if (!coordinates) LogError("Invalid coordinates for CellTree building.");
    if (coordinates_->getGridPrimitiveType() != GridPrimitive::Vertex ||
        coordinates_->getNumComponents() != SpatialDims ||
        coordinates_->size() != grid->getNumElements()) {
        LogError("Incompatible grid and coordinate channel given for celltree, aborting.");
        return;
    }

    buildCellTree(coordinates);
}

template <unsigned int SpatialDims>
CellTree<SpatialDims>::CellTree(const CellTree<SpatialDims>& tree)
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
CellTree<SpatialDims>& CellTree<SpatialDims>::operator=(const CellTree<SpatialDims>& tree) {
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
SpatialEntity<SpatialDims>* CellTree<SpatialDims>::clone() const {
    std::cout << "Cloned CellTree (being a good spatial entity, but a bad cell tree!)" << std::endl;
    return new CellTree<SpatialDims>(*this);
}

template <unsigned int SpatialDims>
// template <typename T>
bool CellTree<SpatialDims>::sampleCell(ind cellId, const std::array<float, SpatialDims>& pos,
                                       std::vector<double>& weights,
                                       InterpolationType interpolationType) const {
    if (cellId >= coordinates_->size()) return false;
    auto coordinatesT = dynamic_cast<const DataChannel<double, SpatialDims>*>(coordinates_.get());
    ivwAssert(coordinatesT, "Coordinate channel not of the detected type.");
    if (!coordinatesT) return false;

    // Get cell vertices.
    std::vector<ind> vertIndices;
    grid_->getConnections(vertIndices, cellId, GridPrimitive(SpatialDims), GridPrimitive::Vertex);
    if (vertIndices.size() == 0) return false;

    std::vector<std::array<float, SpatialDims>> vertices;
    // vertices.resize(vertIndices.size());
    std::array<double, SpatialDims> originalCoord;
    for (ind v : vertIndices) {
        coordinatesT->fill(originalCoord, v);
        vertices.emplace_back();
        for (unsigned dim = 0; dim < SpatialDims; ++dim)
            vertices.back()[dim] = static_cast<float>(originalCoord[dim]);
    }

    // TODO: interpolant
    // return dynamic_cast<Interpolant<SpatialDims>>(grid_->getInterpolant())
    //     .getWeights(interpolationType, coordinatesT, weights, pos);

    return true;
}

template <unsigned int SpatialDims>
ind CellTree<SpatialDims>::locateAndSampleCell(const std::array<float, SpatialDims>& pos,
                                               std::vector<double>& returnWeights,
                                               std::vector<ind>& returnVertices,
                                               InterpolationType interpolationType) const {
    static bool firstSample = true;
    if (firstSample) std::cout << "Sampling first cell" << std::endl;
    firstSample = false;
    // glm::vec<SpatialDims, float> posVec;
    // for (unsigned i = 0; i < SpatialDims; ++i) posVec[i] = pos[i];
    // const glm::vec<SpatialDims, float>&
    // return std::invoke<decltype(locateCellFunction), CellTree<SpatialDims>&,
    //                    const glm::vec<SpatialDims, float>&>(locateCellFunction, *this, posVec);
    //     return (this->*locateCellFunction)(pos);
    // }

    // template <unsigned int SpatialDims>
    // template <typename T, ind N>
    // ind CellTree<SpatialDims>::locateAndSampleCell(const std::array<float, SpatialDims>& pos,
    //                                                std::vector<double>& returnWeights,
    //                                                std::vector<ind>& returnVertices,
    //                                                InterpolationType interpolationType) const {
    // return -1;
    for (size_t dim = 0; dim < SpatialDims; ++dim) {
        if (pos[dim] <= coordsMin_[dim] || pos[dim] >= coordsMax_[dim]) return -1;
    }

    std::stack<ind> todoNodes;
    todoNodes.push(0);
    do {
        const CellTree<SpatialDims>::Node* node = &nodes_[todoNodes.top()];
        todoNodes.pop();
        while (!node->isLeaf()) {
            // nodes_[node.child]
            bool inLeftChild = pos[node->dim] <= node->node.Lmax;
            bool inRightChild = pos[node->dim] >= node->node.Rmin;
            if (inLeftChild && inRightChild) {
                float percBetweenMinMax =
                    (pos[node->dim] - node->node.Rmin) / (node->node.Lmax - node->node.Rmin);
                if (percBetweenMinMax < 0.5) {
                    node = &nodes_[node->child];
                    todoNodes.push(node->child + 1);
                } else {
                    node = &nodes_[node->child + 1];
                    todoNodes.push(node->child);
                }
                continue;
            }
            if (inLeftChild) {
                node = &nodes_[node->child];
                continue;
            }
            node = &nodes_[node->child + 1];
            continue;
        }

        static bool firstBox = true;
        if (firstBox) std::cout << "Testing " << node->leaf.size << " cells!" << std::endl;
        firstBox = false;

        for (ind c = node->leaf.start; c < node->leaf.start + node->leaf.size; ++c) {
            if (sampleCell(cells_[c], pos, returnWeights, interpolationType)) return c;
        }
    } while (!todoNodes.empty());

    return -1;
}

template <unsigned int SpatialDims>
void CellTree<SpatialDims>::buildCellTree(
    std::shared_ptr<const DataChannel<double, SpatialDims>> coordinates) {
    using Coord = std::array<double, SpatialDims>;
    using FloatCoord = std::array<float, SpatialDims>;

    std::cout << "Making a cell tree!" << std::endl;

    // Initialize data structures.
    // We assume approximately as many inner nodes as leaves, and leaves half full on average.
    ind numVertices = grid_->getNumElements(grid_->getDimension());
    ind numNodesEstimate = numVertices / MAX_CELLS_PER_NODE * 4;
    nodes_.reserve(numNodesEstimate);
    cells_.resize(numVertices);
    std::iota(cells_.begin(), cells_.end(), 0);

    Coord coordsMinDouble, coordsMaxDouble;
    coordinates->getMinMax(coordsMinDouble, coordsMaxDouble);
    for (unsigned dim = 0; dim < SpatialDims; ++dim) {
        coordsMin_[dim] = static_cast<float>(coordsMinDouble[dim]);
        coordsMax_[dim] = static_cast<float>(coordsMaxDouble[dim]);
    }

    Node rootNode;
    rootNode.leaf.start = 0;
    rootNode.leaf.size = numVertices;
    nodes_.push_back(std::move(rootNode));

    struct NodeInfo {
        ind Index;
        FloatCoord Min, Max;
    };
    std::stack<NodeInfo> todoNodes;
    todoNodes.push({0, coordsMin_, coordsMax_});

    std::array<ind, SpatialDims * NUM_SPLIT_BUCKETS> bucketsPerSplitDimension;  //{0};
    std::vector<ind> vertices;
    Coord pos;

    // std::cout << "\t= while todoNodes.size()" << std::endl;

    while (todoNodes.size() > 0) {
        // Get the next node to work on.
        auto nodeInfo = todoNodes.top();
        todoNodes.pop();
        auto& node = nodes_[nodeInfo.Index];
        unsigned& numCells = node.leaf.size;

        std::cout << "\t\t= Node " << nodeInfo.Index << " :" << std::endl;
        std::cout << "\t\t= Number of cells: " << numCells << std::endl;
        // Should be leaf?
        if (numCells <= MAX_CELLS_PER_NODE) {
            node.child = 0;
            std::cout << "\t\t= is a leafy! " << nodeInfo.Index << std::endl;
            continue;
        }

        std::fill(std::begin(bucketsPerSplitDimension), std::end(bucketsPerSplitDimension), 0);

        // Assemble number of cells per bucket.
        for (unsigned c = node.leaf.start; c < node.leaf.start + numCells; ++c) {
            ind cellIdx = cells_[c];
            vertices.clear();
            // Get "normal" cells for now, i.e., ignore periodic or wrapping behavior.
            grid_->getConnections(vertices, cellIdx, GridPrimitive(SpatialDims),
                                  GridPrimitive::Vertex, true);
            if (!vertices.size()) {
                // std::cout << "No vertices in cell " << c - node.leaf.start << std::endl;
                numCells--;
                cells_[c] = cells_.back();
                cells_.resize(numCells);
                continue;
            }
            ivwAssert(vertices.size() > 0, "Encountered cell without vertices.");

            coordinates->fill(pos, vertices[0]);
            for (unsigned dim = 0; dim < SpatialDims; ++dim) {
                ind bucketIdx = (((float(pos[dim]) - nodeInfo.Min[dim]) * NUM_SPLIT_BUCKETS) /
                                 (nodeInfo.Max[dim] - nodeInfo.Min[dim]));
                bucketIdx = std::min(bucketIdx, ind(NUM_SPLIT_BUCKETS - 1));
                bucketIdx = std::max(bucketIdx, ind(0));

                bucketsPerSplitDimension[dim * NUM_SPLIT_BUCKETS + bucketIdx]++;
                // if (dim == 1 && nodeInfo.Index == 3973) {
                //     std::cout << fmt::format(
                //                      "  {} [{}]", float(pos[dim]),
                //                      (((float(pos[dim]) - nodeInfo.Min[dim]) * NUM_SPLIT_BUCKETS)
                //                      /
                //                       (nodeInfo.Max[dim] - nodeInfo.Min[dim])))
                //               << std::endl;
                // }
            }
        }

        std::cout << " (num Cells now " << numCells << ')' << std::endl;

        // Find best dimension to split in, and between which two buckets.
        // Tuple: bucket to split after, num elements in lower half, is it larger than half
        // the elements.
        struct SplitBucket {
            unsigned splitFromBucket = 0, numCellsLeft = 0;
        };

        std::array<SplitBucket, SpatialDims> optimalSplitPerDimensions;
        for (unsigned dim = 0; dim < SpatialDims; ++dim) {
            unsigned sum = bucketsPerSplitDimension[NUM_SPLIT_BUCKETS * dim];

            // First bucket containing more than half the cells? Split after first bucket.
            if (sum > numCells / 2) {
                optimalSplitPerDimensions[dim] = SplitBucket{1, sum};
                continue;
            }

            for (unsigned int bucket = 1; bucket < NUM_SPLIT_BUCKETS; ++bucket) {
                unsigned prevSum = sum;
                sum += bucketsPerSplitDimension[NUM_SPLIT_BUCKETS * dim + bucket];

                if (sum > numCells / 2) {
                    // Take the split with the most equal halfs.
                    optimalSplitPerDimensions[dim] = (prevSum >= numCells - sum)
                                                         ? SplitBucket{bucket, prevSum}
                                                         : SplitBucket{bucket + 1, sum};

                    break;
                }
            }
        }

        // Find the dimension that splits into two most equal partsi
        // (i.e., the half with less elements is as large as possible).
        auto bestDim =
            std::max_element(optimalSplitPerDimensions.begin(), optimalSplitPerDimensions.end(),
                             [&](auto& i, auto& j) {
                                 return std::abs(ind(numCells / 2) - ind(i.numCellsLeft)) >
                                        std::abs(ind(numCells / 2) - ind(j.numCellsLeft));
                             });
        // std::cout << "\t= Best Bucket: " << (100 * bestDim->numCellsLeft) / numCells << "% - "
        //           << bestDim->numCellsLeft << " - bucket " << bestDim->splitFromBucket <<
        //           std::endl;

        unsigned int bestDimIdx = std::distance(optimalSplitPerDimensions.begin(), bestDim);
        node.dim = bestDimIdx;
        float bestSplit =
            nodeInfo.Min[bestDimIdx] +
            (((nodeInfo.Max[bestDimIdx] - nodeInfo.Min[bestDimIdx]) * bestDim->splitFromBucket) /
             NUM_SPLIT_BUCKETS);

        // std::cout << "best split at " << bestSplit << std::endl;

        // ind firstHalfSize = bestDim->numCellsLeft;

        auto it = cells_.begin() + node.leaf.start;  // First elements.
        // auto itMiddle = it + firstHalfSize;          // Middle split.
        auto itEnd = it + node.leaf.size;  // Last element.

        std::cout << "Optimal Split: " << bestSplit << " in dim " << bestDimIdx << " ("
                  << (100 * bestDim->numCellsLeft) / numCells << "%)" << std::endl;

        // bool initialized[2] = {false, false};

        bool badSplit = (bestDim->numCellsLeft == numCells || bestDim->numCellsLeft == 0);
        if (badSplit) {
            std::cout << "Splitting " << bestDim->numCellsLeft << " to "
                      << numCells - bestDim->numCellsLeft << " from bucket "
                      << bestDim->splitFromBucket << std::endl;
            for (size_t b = 0; b < optimalSplitPerDimensions.size(); ++b) {
                auto& bucket = optimalSplitPerDimensions[b];
                std::cout << "\t= Alternatives " << bucket.numCellsLeft << " to "
                          << numCells - bucket.numCellsLeft << " in dim " << b << " ("
                          << (100 * bucket.numCellsLeft) / numCells << "%)\n\t  for range "
                          << nodeInfo.Min[b] << " -> " << nodeInfo.Max[b] << std::endl;
            }

            for (auto buckCount : bucketsPerSplitDimension)
                std::cout << "      bucket count " << buckCount << std::endl;

            for (ind b = 0; b < NUM_SPLIT_BUCKETS; ++b) {
                std::cout << "\t" << b << ": "
                          << nodeInfo.Min[bestDimIdx] +
                                 ((nodeInfo.Max[bestDimIdx] - nodeInfo.Min[bestDimIdx]) * b) /
                                     NUM_SPLIT_BUCKETS
                          << std::endl;
            }
        }

        NodeInfo children[2] = {NodeInfo{ind(nodes_.size()), FloatCoord{0}, FloatCoord{0}},
                                NodeInfo{ind(nodes_.size()) + 1, FloatCoord{0}, FloatCoord{0}}};

        children[0].Min = nodeInfo.Max;
        children[1].Min = nodeInfo.Max;
        children[0].Max = nodeInfo.Min;
        children[1].Max = nodeInfo.Min;

        // Split the cell indices by the splitting plane we found.
        // Decided against std::partition since we can re-use the vertex coordinates.
        while (it != itEnd) {
            // Decide which side it's on by the first vertex position of a cell.
            vertices.clear();
            grid_->getConnections(vertices, *it, GridPrimitive(SpatialDims), GridPrimitive::Vertex);
            coordinates->fill(pos, vertices[0]);

            if (badSplit) {
                std::cout << "\t";
                for (ind d = 0; d < SpatialDims; ++d) std::cout << pos[d] << " - ";
                std::cout << std::endl;
            }

            // If necessary, swap to the right:
            // Move this cell to the end, and make sure we don't process it twice.
            size_t childIdx = (pos[bestDimIdx] >= bestSplit) ? 1 : 0;

            if (childIdx) {
                itEnd--;
                std::iter_swap(it, itEnd);
            } else {
                it++;
            }

            // Update the cell range of the side we're on.
            // for (size_t vertIdx = 0; vertIdx < vertices.size(); ++vertIdx) {
            // if (vertIdx > 0) coordinates->fill(pos, vertices[vertIdx]);
            for (ind dim = 0; dim < SpatialDims; ++dim) {
                children[childIdx].Min[dim] =
                    std::min(children[childIdx].Min[dim], float(pos[dim]));
                children[childIdx].Max[dim] =
                    std::max(children[childIdx].Max[dim], float(pos[dim]));
            }
            // };
        }
        ind firstHalfSize = std::distance(cells_.begin() + node.leaf.start, it);
        // std::cout << "Split size: " << firstHalfSize << std::endl;
        // std::cout << "Ranges of children:" << std::endl;
        // for (ind dim = 0; dim < SpatialDims; ++dim) {
        //     std::cout << "\t # " << children[0].Min[dim] << "\t - " << children[0].Max[dim]
        //               << "\t  and  " << children[1].Min[dim] << "\t - " << children[1].Max[dim]
        //               << std::endl;
        // }

        // Create child nodes, add them to the todo list.
        Node childNode;
        childNode.leaf.start = node.leaf.start;
        childNode.leaf.size = firstHalfSize;
        childNode.child = 0;
        nodes_.push_back(childNode);

        childNode.leaf.start = node.leaf.start + firstHalfSize;
        childNode.leaf.size = numCells - firstHalfSize;
        childNode.child = 0;
        nodes_.push_back(childNode);

        node.child = nodes_.size() - 2;

        // auto& leftChild = nodes_[nodes_.size() - 2];
        // auto& rightChild = nodes_.back();
        // std::cout << fmt::format("Children: [{} -> {}] and [{} -> {}]", leftChild.leaf.start,
        //                          leftChild.leaf.start + leftChild.leaf.size,
        //                          rightChild.leaf.start, rightChild.leaf.start +
        //                          rightChild.leaf.size)
        //           << std::endl;

        // TEMPORARY DEBUG!!!!!
        unsigned numWrongBelow = 0;
        unsigned numWrongAbove = 0;
        unsigned numOutsideRangeLeft = 0;
        unsigned numOutsideRangeRight = 0;

        for (unsigned cellIdx = node.leaf.start; cellIdx < node.leaf.start + node.leaf.size;
             ++cellIdx) {
            bool rightSide = (cellIdx - node.leaf.start) >= firstHalfSize;
            vertices.clear();
            grid_->getConnections(vertices, cells_[cellIdx], GridPrimitive(SpatialDims),
                                  GridPrimitive::Vertex);
            coordinates->fill(pos, vertices[0]);
            // bool rightSide = pos[bestDimIdx] >= bestSplit;

            if (!rightSide && (pos[bestDimIdx] < children[0].Min[bestDimIdx] ||
                               pos[bestDimIdx] > children[0].Max[bestDimIdx])) {
                numOutsideRangeLeft++;
                std::cout << fmt::format("{} : Left side, but {} < {} OR {} > {}",
                                         cellIdx - node.leaf.start, pos[bestDimIdx],
                                         children[0].Min[bestDimIdx], pos[bestDimIdx],
                                         children[0].Max[bestDimIdx])
                          << std::endl;
            }
            if (!rightSide && pos[bestDimIdx] >= bestSplit) {
                numWrongBelow++;
            }

            if (rightSide && (pos[bestDimIdx] < children[1].Min[bestDimIdx] ||
                              pos[bestDimIdx] > children[1].Max[bestDimIdx])) {
                numOutsideRangeRight++;
                std::cout << fmt::format("{} : Right side, but {} < {} OR {} > {}",
                                         cellIdx - node.leaf.start, pos[bestDimIdx],
                                         children[1].Min[bestDimIdx], pos[bestDimIdx],
                                         children[1].Max[bestDimIdx])
                          << std::endl;
            }
            if (rightSide && pos[bestDimIdx] < bestSplit) {
                numWrongAbove++;
            }
        }
        if (numWrongBelow || numWrongAbove || numOutsideRangeLeft || numOutsideRangeRight)
            std::cout << fmt::format("= Num Wrong below: {} ({}) - Num Wrong above: {} ({})",
                                     numWrongBelow, numOutsideRangeLeft, numWrongAbove,
                                     numOutsideRangeRight)
                      << std::endl;
        // std::cout << "@ it is " << it - (cells_.begin() + leftChild.leaf.start) << "\n  itEnd is"
        //           << itEnd - (cells_.begin() + leftChild.leaf.start) << std::endl;

        node.node.Lmax = children[0].Max[bestDimIdx];
        node.node.Rmin = children[1].Min[bestDimIdx];
        todoNodes.push(children[0]);
        todoNodes.push(children[1]);

        std::cout << "\t= Done iterating!\n" << std::endl;
    }
    std::cout << "\t= made a cell tree!" << std::endl;
}

template <unsigned int SpatialDims>
Mesh* CellTree<SpatialDims>::getDebugMesh() const {
    std::cout << "Making a mesh!" << std::endl;

    static const float ALPHA = 0.2f;
    std::vector<vec3> posVec;
    std::vector<vec4> colorVec;
    std::function<void(ind, vec3, vec3, ind)> processNode;

    if constexpr (SpatialDims >= 3) {
        processNode = [&](ind nodeIdx, const vec3 minRange, const vec3 maxRange, ind depth) {
            const Node& node = this->nodes_[nodeIdx];
            if (node.isLeaf()) return;

            std::cout << "#\trendering node " << nodeIdx << " for dim " << node.dim << std::endl;
            vec3 min = minRange;
            vec3 max = maxRange;

            if (node.dim < 3) {
                unsigned dim0 = (node.dim + 1) % 3;
                unsigned dim1 = (node.dim + 2) % 3;
                std::cout << fmt::format("#\tdim {}, {} - {}", node.dim, dim0, dim1) << std::endl;
                // size_t vertIdx = posVec.size();

                vec4 color;
                color[node.dim] = 1;
                color.a = ALPHA + 0.1 * depth;

                for (size_t side = 0; side < 2; ++side) {
                    float cutoff = side ? node.node.Lmax : node.node.Rmin;
                    min[node.dim] = cutoff;
                    max[node.dim] = cutoff;

                    // Building a quad.
                    posVec.push_back(min);
                    posVec.push_back(min);
                    posVec.back()[dim0] = max[dim0];
                    posVec.push_back(min);
                    posVec.back()[dim1] = max[dim1];

                    posVec.push_back(min);
                    posVec.back()[dim1] = max[dim1];
                    posVec.push_back(min);
                    posVec.back()[dim0] = max[dim0];
                    posVec.push_back(max);

                    for (int i = 0; i < 6; ++i) colorVec.push_back(color);

                    processNode(node.child + side, side ? minRange : min, side ? max : maxRange,
                                depth + 1);
                    color[dim0] = 0.2f;
                    color[dim1] = 0.2f;
                }
            }
        };
    } else if constexpr (SpatialDims == 2) {
        processNode = [&](ind nodeIdx, const vec3 minRange, const vec3 maxRange, ind depth) {
            const Node& node = this->nodes_[nodeIdx];
            if (node.isLeaf()) return;
            if (depth > 3) return;

            vec3 min = minRange;
            vec3 max = maxRange;

            unsigned otherDim = 1 - node.dim;
            // size_t vertIdx = posVec.size();

            vec4 color;
            color[node.dim] = 1;
            color.a = ALPHA + 0.1 * depth;

            for (size_t side = 0; side < 2; ++side) {
                float cutoff = side ? node.node.Lmax : node.node.Rmin;
                min[node.dim] = cutoff;
                max[node.dim] = cutoff;

                // Building a quad.
                posVec.push_back(min);
                posVec.push_back(max);

                colorVec.push_back(color);
                colorVec.push_back(color);

                processNode(node.child + side, side ? minRange : min, side ? max : maxRange,
                            depth + 1);
                color[otherDim] = 0.2f;
                color[2] = 0.2f;
            }
        };
    } else {
        return nullptr;
    }

    vec3 minRange(0), maxRange(0);
    for (unsigned dim = 0; dim < std::min(unsigned(3), SpatialDims); ++dim) {
        minRange[dim] = coordsMin_[dim];
        maxRange[dim] = coordsMax_[dim];
    }

    processNode(0, minRange, maxRange, 0);

    auto posBuffer = util::makeBuffer(std::move(posVec));
    auto colorBuffer = util::makeBuffer(std::move(colorVec));

    DrawType primType = (DrawType)(std::min(SpatialDims, unsigned(3)));
    inviwo::Mesh* mesh = new Mesh(primType, ConnectivityType::None);
    mesh->addBuffer(BufferType::PositionAttrib, posBuffer);
    mesh->addBuffer(BufferType::ColorAttrib, colorBuffer);

    std::cout << "Made a mesh!" << std::endl;

    return mesh;
}

template <unsigned int SpatialDims>
std::string CellTree<SpatialDims>::getIdentifier() const {
    return fmt::format("CellTree_on_\"{}\"_channel", coordinates_->getName());
}

}  // namespace discretedata
}  // namespace inviwo
