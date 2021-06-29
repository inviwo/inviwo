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
    // CellTreeBuilder dispatcher;
    // coordinates_->template dispatch<void, dispatching::filter::Scalars, SpatialDims,
    // SpatialDims>(
    //     dispatcher, this);
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
        for (ind c = node->leaf.start; c < node->leaf.start + node->leaf.size; ++c) {
            if (sampleCell(cells_[c], pos, returnWeights, interpolationType)) return c;
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
// template <typename T, ind N>
// void CellTree<SpatialDims>::CellTreeBuilder::operator()(const DataChannel<T, N>* positions,
//                                                         CellTree<SpatialDims>* tree) {
void CellTree<SpatialDims>::buildCellTree(
    std::shared_ptr<const DataChannel<double, SpatialDims>> coordinates) {
    using Coord = std::array<double, SpatialDims>;
    // typename DataChannel<double, SpatialDims>::DefaultVec;
    using FloatCoord = std::array<float, SpatialDims>;

    std::cout << "Making a cell tree!" << std::endl;

    // Save pointer to correct cell location function to avoid future dispatches.
    // locateCellFunction = &CellTree<SpatialDims>::template locateAndSampleCell<double, N>;

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

    Node node;
    node.leaf.start = 0;
    node.leaf.size = numVertices;
    nodes_.push_back(std::move(node));

    struct NodeInfo {
        ind Index;
        FloatCoord Min, Range;
    };
    std::stack<NodeInfo> todoNodes;
    // NodeInfo info;
    // info.Index = 0;
    // info.Min = coordsMin;
    // info.Range = coordsMax_ - coordsMin_;
    // todoNodes.push(std::move(info));
    FloatCoord difference;
    std::transform(coordsMax_.begin(), coordsMax_.end(), coordsMin_.begin(), difference.begin(),
                   std::minus<float>());
    todoNodes.push({0, coordsMin_, difference});

    std::array<ind, SpatialDims * NUM_SPLIT_BUCKETS> bucketsPerSplitDimension;  //{0};
    std::vector<ind> vertices;
    Coord pos;

    // std::cout << "\t= while todoNodes.size()" << std::endl;

    while (todoNodes.size() > 0) {
        // Get the next node to work on.
        auto nodeInfo = todoNodes.top();
        todoNodes.pop();
        auto& node = nodes_[nodeInfo.Index];
        ind numCells = node.leaf.size;

        std::cout << "\t\t= Node " << nodeInfo.Index << std::endl;
        std::cout << "\t\t= Number of nodes: " << node.leaf.size << std::endl;
        // Should be leaf?
        if (numCells <= MAX_CELLS_PER_NODE) {
            node.child = 0;
            std::cout << "\t\t= is a child! " << nodeInfo.Index << std::endl;
            continue;
        }

        std::fill(std::begin(bucketsPerSplitDimension), std::end(bucketsPerSplitDimension), 0);

        // Assemble number of cells per bucket.
        for (unsigned c = node.leaf.start; c < node.leaf.start + numCells; ++c) {
            ind cellIdx = cells_[c];
            vertices.clear();
            grid_->getConnections(vertices, cellIdx, GridPrimitive(SpatialDims),
                                  GridPrimitive::Vertex);
            ivwAssert(vertices.size() > 0, "Encountered cell without vertices.");

            coordinates->fill(pos, vertices[0]);
            vertices.clear();
            for (unsigned dim = 0; dim < SpatialDims; ++dim) {
                ind bucketIdx =
                    ((static_cast<float>(pos[dim]) - nodeInfo.Min[dim]) / nodeInfo.Range[dim]) *
                    NUM_SPLIT_BUCKETS;
                bucketIdx = std::min(bucketIdx, ind(NUM_SPLIT_BUCKETS - 1));
                bucketIdx = std::max(bucketIdx, ind(0));

                unsigned bucketPerDimIdx = dim * NUM_SPLIT_BUCKETS + bucketIdx;

                bucketsPerSplitDimension[dim * NUM_SPLIT_BUCKETS + bucketIdx]++;

                // if (cellIdx > 71600 && nodeInfo.Index == 0)
                //     std::cout << fmt::format("Dim {}: {} < {} < {},\tBucket {} of {}", dim,
                //                              nodeInfo.Min[dim], pos[dim],
                //                              nodeInfo.Min[dim] + nodeInfo.Range[dim], bucketIdx,
                //                              NUM_SPLIT_BUCKETS)
                //               << std::endl;
            }
        }

        for (auto i : bucketsPerSplitDimension) std::cout << "== Bucket: " << i << std::endl;

        // Find best dimension to split in, and between which two buckets.
        // Tuple: bucket to split after, num elements in lower half, is it larger than half
        // the elements.
        struct SplitBucket {
            ind splitFromBucket = 0, numCells = 0;
            bool largerFirstHalf;
        };

        std::array<SplitBucket, SpatialDims> optimalSplitPerDimensions;
        for (unsigned dim = 0; dim < SpatialDims; ++dim) {
            ind sum = 0;
            // ptimalSplitPerDimensions[dim] = SplitBucket{
            //     NUM_SPLIT_BUCKETS - 1,
            //     numCells - bucketsPerSplitDimension[NUM_SPLIT_BUCKETS * (dim + 1) - 1], true};
            for (unsigned int bucket = 0; bucket < NUM_SPLIT_BUCKETS - 1; ++bucket) {
                ind prevSum = sum;
                sum += bucketsPerSplitDimension[NUM_SPLIT_BUCKETS * dim + bucket];
                if (sum > numCells / 2) {
                    std::cout << fmt::format("At {}: prevSum {} \tsum {}", bucket, prevSum, sum)
                              << std::endl;
                    // Take the split with the most equal halfs.
                    optimalSplitPerDimensions[dim] =
                        (prevSum > numCells - sum) ? SplitBucket{bucket, prevSum, true}
                                                   : SplitBucket{bucket + 1, numCells - sum, false};
                    break;
                }
            }
        }

        for (auto& bucket : optimalSplitPerDimensions) {
            std::cout << "\t= Bucket: " << bucket.splitFromBucket << " - " << bucket.numCells
                      << " - " << bucket.largerFirstHalf << std::endl;
        }

        // Find the dimension that splits into two most equal partsi
        // (i.e., the half with less elements is as large as possible).
        auto bestDim =
            std::max_element(optimalSplitPerDimensions.begin(), optimalSplitPerDimensions.end(),
                             [](auto& i, auto& j) { return i.numCells < j.numCells; });
        std::cout << "\t= Best Bucket: " << bestDim->splitFromBucket << " - " << bestDim->numCells
                  << " - " << bestDim->largerFirstHalf << std::endl;

        unsigned int bestDimIdx = std::distance(optimalSplitPerDimensions.begin(), bestDim);
        std::cout << "Best dim idx: " << bestDimIdx << std::endl;
        float bestSplit =
            nodeInfo.Min[bestDimIdx] +
            ((nodeInfo.Range[bestDimIdx] * bestDim->splitFromBucket) / NUM_SPLIT_BUCKETS);

        std::cout << "best split at " << bestSplit << std::endl;

        ind firstHalfSize =
            bestDim->largerFirstHalf ? bestDim->numCells : numCells - bestDim->numCells;
        std::cout << "Size first bucket: " << firstHalfSize << std::endl;

        // Create child nodes, add them to the todo list.
        Node childNode;
        childNode.leaf.start = node.leaf.start;
        childNode.leaf.size = firstHalfSize;
        nodes_.push_back(childNode);

        childNode.leaf.start = node.leaf.start + firstHalfSize;
        childNode.leaf.size = numCells - firstHalfSize;
        nodes_.push_back(childNode);

        todoNodes.push({ind(nodes_.size()) - 2, nodeInfo.Min, nodeInfo.Range});
        todoNodes.push({ind(nodes_.size()) - 1, nodeInfo.Min, nodeInfo.Range});
        node.child = nodes_.size() - 2;

        auto& leftChild = nodes_[nodes_.size() - 2];
        auto& rightChild = nodes_.back();
        // auto& leftTodo = todoNodes[todoNodes.size() - 2];
        // auto& rightTodo = todoNodes.back();

        // Split vertex indices and get min/max along chosen dimension.
        node.node.Lmax = nodeInfo.Min[bestDimIdx];
        node.node.Rmin = nodeInfo.Min[bestDimIdx] + nodeInfo.Range[bestDimIdx];
        // auto startRight = cells_.begin() + rightChild.leaf.start;
        // auto endRight = cells_.begin() + rightChild.leaf.start + rightChild.leaf.size;
        // auto itLeft = cells_.begin() + leftChild.leaf.start;
        // auto itRight = startRight;
        // Vec posLeft, posRight;

        auto it = cells_.begin() + leftChild.leaf.start;  // First elements.
        auto itMiddle = it + firstHalfSize;               // Middle split.
        auto itEnd = it + rightChild.leaf.size - 1;       // Last element.
        bool rightSide = false;

        std::cout << "Check?" << std::endl;

        // Split the cell indices by the splitting plane we found.
        // Decided against std::partition since we can re-use the vertex coordinates.
        while (it != itEnd) {
            if (it >= itMiddle) rightSide = true;

            // Decide which side it's on by the first vertex position of a cell.
            vertices.clear();
            grid_->getConnections(vertices, *it, GridPrimitive(SpatialDims), GridPrimitive::Vertex);
            coordinates->fill(pos, vertices[0]);

            // If necessary, swap to the right:
            // Move this cell to the end, and make sure we don't process it twice.
            if (!rightSide && pos[bestDimIdx] >= bestSplit) {
                itEnd--;
                std::swap(*it, *itEnd);
                std::cout << "@ it is " << it - (cells_.begin() + leftChild.leaf.start)
                          << "\n  itEnd is " << itEnd - (cells_.begin() + leftChild.leaf.start)
                          << std::endl;
            } else {
                it++;
                if ((it - (cells_.begin() + leftChild.leaf.start)) % 1000 == 0)
                    std::cout << "@ it is " << it - (cells_.begin() + leftChild.leaf.start)
                              << "\n  itEnd is " << itEnd - (cells_.begin() + leftChild.leaf.start)
                              << std::endl;
            }

            // Update the cell range of the side we're on.
            if (pos[bestDimIdx] >= bestSplit) {
                node.node.Rmin = std::min(node.node.Rmin, static_cast<float>(pos[bestDimIdx]));
                for (size_t v = 1; v < vertices.size(); ++v) {
                    coordinates->fill(pos, vertices[v]);
                    node.node.Rmin = std::min(node.node.Rmin, static_cast<float>(pos[bestDimIdx]));
                }
            } else {
                node.node.Lmax = std::max(node.node.Lmax, static_cast<float>(pos[bestDimIdx]));
                for (size_t v = 1; v < vertices.size(); ++v) {
                    coordinates->fill(pos, vertices[v]);
                    node.node.Lmax = std::max(node.node.Lmax, static_cast<float>(pos[bestDimIdx]));
                }
            }
        }
        std::cout << "@ it is " << it - (cells_.begin() + leftChild.leaf.start) << "\n  itEnd is "
                  << itEnd - (cells_.begin() + leftChild.leaf.start) << std::endl;

        std::cout << "\t= Done iterating!" << std::endl;
    }
    std::cout << "\t= made a cell tree!" << std::endl;
}

template <unsigned int SpatialDims>
Mesh* CellTree<SpatialDims>::getDebugMesh() const {
    std::cout << "Making a mesh!" << std::endl;

    using Coord = std::array<float, SpatialDims>;

    static const float ALPHA = 0.2f;
    std::vector<vec3> posVec;
    std::vector<vec4> colorVec;
    std::function<void(ind, vec3, vec3, ind)> processNode;

    if constexpr (SpatialDims >= 3) {
        processNode = [&](ind nodeIdx, const vec3 minRange, const vec3 maxRange, ind depth) {
            const Node& node = this->nodes_[nodeIdx];
            if (node.isLeaf()) return;

            vec3 min = minRange;
            vec3 max = maxRange;

            if (node.dim < 3) {
                unsigned dim0 = (node.dim + 1) % 3;
                unsigned dim1 = (node.dim + 2) % 3;
                size_t vertIdx = posVec.size();

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

                    posVec.push_back(posVec.back());
                    posVec.push_back(posVec[vertIdx + 1]);
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

            vec3 min = minRange;
            vec3 max = maxRange;

            if (node.dim < 3) {
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
            }
        };
    } else {
        return nullptr;
    }

    vec3 minRange, maxRange;
    for (unsigned dim = 0; dim < std::min(unsigned(3), SpatialDims); ++dim) {
        minRange[dim] = coordsMin_[dim];
        maxRange[dim] = coordsMax_[dim];
    }

    processNode(0, minRange, maxRange, 0);

    auto posBuffer = util::makeBuffer(std::move(posVec));
    auto colorBuffer = util::makeBuffer(std::move(colorVec));

    DrawType primType = (DrawType)(std::max(SpatialDims, unsigned(3)));
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
