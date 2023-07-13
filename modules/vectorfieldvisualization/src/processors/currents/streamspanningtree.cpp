/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2022 Inviwo Foundation
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

#include <modules/vectorfieldvisualization/processors/currents/streamspanningtree.h>
#include <inviwo/core/datastructures/image/image.h>
#include <inviwo/core/datastructures/image/layerram.h>
#include <inviwo/core/datastructures/image/layerramprecision.h>
#include <inviwo/core/util/imageramutils.h>
#include <inviwo/core/util/indexmapper.h>
#include <modules/base/algorithm/meshutils.h>
#include <inviwo/core/datastructures/geometry/basicmesh.h>
#include <inviwo/core/util/foreach.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo StreamSpanningTree::processorInfo_{
    "org.inviwo.StreamSpanningTree",  // Class identifier
    "Stream Spanning Tree",           // Display name
    "Undefined",                      // Category
    CodeState::Experimental,          // Code state
    Tags::None,                       // Tags
};
const ProcessorInfo StreamSpanningTree::getProcessorInfo() const { return processorInfo_; }

StreamSpanningTree::StreamSpanningTree()
    : Processor()
    , velocitySampler_("velocitySampler")
    , seedMask_("seedMask")
    , lineSelection_("lineSelection")

    , beginning_("beginning", "Beginning Point", {0, 0}, {{0, 0}, ConstraintBehavior::Immutable},
                 {{128, 128}, ConstraintBehavior::Mutable})
    , destination_("destination", "Destination Point", {32, 32},
                   {{0, 0}, ConstraintBehavior::Immutable},
                   {{128, 128}, ConstraintBehavior::Mutable})
    , beginAtMaxValue_("beginAtMaxValue", "Begin at Max Value", false)
    , maxDestinationDistance_("maxDestinationDistance", "Max Destination Distance (in cells)", 2.0f,
                              0.0f, 128.0f)
    , integrationProperties_("integrationProperties", "Integration Properties")
    , maxDiffusionRadius_("maxDiffusionRadius", "Max Jump (in cells)", 2,
                          {0.71, ConstraintBehavior::Immutable}, {5, ConstraintBehavior::Ignore})
    , samplingStride_("samplingStride", "Sampling Stride", 50, 1, 100)
    , maxNumNodes_("maxNumNodes", "Max Number Nodes", 10000, 0, 1000000000000, 1,
                   InvalidationLevel::InvalidOutput, PropertySemantics::Text)
    , markJumps_("markJumps", "Mark Jumps", false)
    , integrate_("integrate", "Integrate?", true)

    , pointMesh_("pointMesh")
    , lineMesh_("lineMesh")
    , integralLines_("allLines")
    , seedPoints_("seedPoints")

    , integrationRequired_(true) {

    seedMask_.setOutportDeterminesSize(true);
    integrationProperties_.seedPointsSpace_.set(CoordinateSpace::Data);
    integrationProperties_.seedPointsSpace_.setVisible(false);

    auto requireIntegration = [this]() { integrationRequired_ = true; };
    integrationProperties_.onChange(requireIntegration);
    velocitySampler_.onChange(requireIntegration);
    seedMask_.onChange(requireIntegration);
    beginAtMaxValue_.onChange(requireIntegration);

    addPorts(velocitySampler_, seedMask_, lineSelection_, pointMesh_, lineMesh_, integralLines_,
             seedPoints_);
    addProperties(beginning_, destination_, beginAtMaxValue_, maxDestinationDistance_,
                  integrationProperties_, maxDiffusionRadius_, samplingStride_, maxNumNodes_,
                  markJumps_, integrate_);
}

void StreamSpanningTree::process() {
    auto lineMesh = std::make_shared<ColoredMesh>(DrawType::Lines, ConnectivityType::Strip);
    lineMesh->setModelMatrix(velocitySampler_.getData()->getModelMatrix());
    lineMesh->setWorldMatrix(velocitySampler_.getData()->getWorldMatrix());
    std::vector<ColoredMesh::Vertex> lineVerts;

    auto pointMesh = std::make_shared<ColoredMesh>(DrawType::Points, ConnectivityType::None);
    pointMesh->setModelMatrix(velocitySampler_.getData()->getModelMatrix());
    pointMesh->setWorldMatrix(velocitySampler_.getData()->getWorldMatrix());
    std::vector<ColoredMesh::Vertex> pointVerts;

    auto seedDim = seedMask_.getData()->getDimensions();
    util::IndexMapper2D indexMapper(seedDim);

    pointVerts.emplace_back(vec3{(0.5f + beginning_.get().x) / seedDim.x,
                                 (0.5f + beginning_.get().y) / seedDim.y, 0.0f},
                            vec4{1.0f, 0.0f, 0.0f, 1.0f});

    pointVerts.emplace_back(vec3{(0.5f + destination_.get().x) / seedDim.x,
                                 (0.5f + destination_.get().y) / seedDim.y, 0.0f},
                            vec4{0.0f, 0.0f, 1.0f, 1.0f});

    if (!integrate_.get()) {
        lineMesh->addVertices(lineVerts);
        pointMesh->addVertices(pointVerts);
        lineMesh_.setData(lineMesh);
        pointMesh_.setData(pointMesh);
        return;
    }

    if (integrationRequired_) {
        // =============== Generate Seed Points ===============
        std::cout << "Integrating!" << std::endl;
        beginning_.setMaxValue(seedDim - size2_t(1, 1));
        destination_.setMaxValue(seedDim - size2_t(1, 1));
        nodes_ = std::make_unique<std::vector<Node>>();
        auto seeds = std::make_shared<std::vector<vec2>>();

        auto layer = seedMask_.getData()->getColorLayer();
        auto layerRAM = layer->getRepresentation<LayerRAM>();

        // Seed indices - offset by 1, so 0 is invalid and 1 is node index 0.
        seedIndexMask_ = std::make_unique<size_t[]>(seedDim.x * seedDim.y);
        std::fill_n(seedIndexMask_.get(), seedDim.x * seedDim.y, 0);

        size2_t maxValueIdx = beginning_.get();
        double maxValue = 0.0;

        layerRAM->dispatch<void>([&](auto imgPrecision) {
            auto data = imgPrecision->getDataTyped();

            util::forEachPixel(*imgPrecision, [&](const size2_t& pos) {
                size_t idx = indexMapper(pos);
                double value = util::glm_convert_normalized<double>(data[idx]);
                if (value > 0.0) {
                    nodes_->emplace_back(pos);
                    seedIndexMask_[idx] = nodes_->size();
                    seeds->push_back({(0.5f + pos.x) / seedDim.x, (0.5f + pos.y) / seedDim.y});
                    if (value > maxValue) {
                        maxValue = value;
                        maxValueIdx = pos;
                    }
                }
            });
        });
        if (beginAtMaxValue_.get() && maxValueIdx != beginning_.get()) {
            std::cout << "Found value " << maxValue << std::endl;
            beginning_.set(maxValueIdx);
            LogInfo("Changed beginning to max value.");
        }

        // =============== Integrate Lines ===============
        // Integrate streamlines.
        auto sampler = velocitySampler_.getData();
        Tracer tracer(sampler, integrationProperties_);
        allLines_ =
            std::make_shared<IntegralLineSet>(sampler->getModelMatrix(), sampler->getWorldMatrix());

        size_t tmpBeginningIdx = seedIndexMask_[indexMapper(beginning_.get())] - 1;
        util::forEachParallel(*nodes_, [&](auto&, size_t i) {
            Node& node = nodes_->at(i);
            node.streamline =
                std::move(tracer.traceFrom(vec2(float(0.5f + node.startPoint.x) / seedDim.x,
                                                float(0.5f + node.startPoint.y) / seedDim.y)));
            if (i == tmpBeginningIdx) {
            }
            auto size = node.streamline.getPositions().size();
            if (size == 0) {
                seedIndexMask_[i] = 0;
                nodes_->at(i).shortestPathLength = -1.0f;
            }
        });
        integrationRequired_ = false;
        seedPoints_.setData(seeds);
    } else {
        for (Node& node : *nodes_) {
            node.shortestPathLength = std::numeric_limits<float>::infinity();
            node.shortestPathPreviousNode = nullptr;
        }
    }

    size_t beginningIdx = seedIndexMask_[indexMapper(beginning_.get())];
    size_t destinationIdx = seedIndexMask_[indexMapper(destination_)] -
                            1;  // In case this index is invalid, we just do the whole search.
    float maxLength = 0.0;

    // Outputs to be set, even if something goes wrong with the algorithm.
    auto finishOff = [&]() {
        std::unordered_set<size_t> filteredLines, selectedLines;
        for (size_t n = 0; n < nodes_->size(); ++n) {
            allLines_->push_back(nodes_->at(n).streamline, n);
            if (nodes_->at(n).shortestPathLength > maxLength) {
                filteredLines.insert(n);
            }
        }

        lineSelection_.sendSelectionEvent(selectedLines);
        lineSelection_.sendFilterEvent(filteredLines);
        integralLines_.setData(allLines_);

        lineMesh->addVertices(lineVerts);
        pointMesh->addVertices(pointVerts);
        lineMesh_.setData(lineMesh);
        pointMesh_.setData(pointMesh);
    };

    // ================ Dijkstra ================

    if (beginningIdx == 0) {
        finishOff();
        LogError("Invalid starting point.");
        return;
    }
    auto priorityCompare = [](const QueueNode& left, const QueueNode& right) {
        return left.shortestDistance > right.shortestDistance;
    };

    std::priority_queue<QueueNode, std::vector<QueueNode>, decltype(priorityCompare)> nodeQueue(
        priorityCompare);
    // Priority queue does not support changing removing or changing values.
    // Thus, have a secondary list to keep tabs of the visited nodes.
    std::vector<bool> visited(nodes_->size(), false);
    nodeQueue.emplace(beginningIdx - 1, beginningIdx - 1, 0.0f);

    // Time for Dijkstra.
    size_t numSteps = 0;
    bool foundDestination = false;
    while (!nodeQueue.empty()) {
        QueueNode currentNode = nodeQueue.top();
        nodeQueue.pop();
        if (visited[currentNode.nodeIndex]) continue;

        // Write shortest path information into currently handled node.
        visited[currentNode.nodeIndex] = true;
        Node& node = nodes_->at(currentNode.nodeIndex);
        node.shortestPathLength = currentNode.shortestDistance;
        node.shortestPathPreviousNode = &nodes_->at(currentNode.prevIndex);
        maxLength = node.shortestPathLength;

        if (currentNode.nodeIndex == destinationIdx ||
            glm::length(vec2(node.startPoint - destination_.get())) <
                maxDestinationDistance_.get()) {
            LogInfo("Destination was found! Length: " << currentNode.shortestDistance);
            std::cout << "Destination Found!" << std::endl;
            destinationIdx = currentNode.nodeIndex;
            foundDestination = true;
            break;
        }

        // Update neighbors' distance.
        size_t streamlineSize = node.streamline.getPositions().size();
        size_t pointSampleIdx = std::min(samplingStride_.get() - 1, streamlineSize - 1);
        for (; pointSampleIdx < streamlineSize;
             pointSampleIdx +=
             samplingStride_
                 .get()) {  // TODO: Kinda wrong.... Why are we walking up from last element?

            dvec2 endPoint = dvec2(node.streamline.getPositions()[pointSampleIdx]);

            endPoint = dvec2(endPoint.x * seedDim.x - 0.5f,
                             endPoint.y * seedDim.y - 0.5f);  // Bring into index range.
            size2_t maxSeedIndex{std::ceil(endPoint.x + maxDiffusionRadius_),
                                 std::ceil(endPoint.y + maxDiffusionRadius_)};
            size2_t minSeedIndex{endPoint.x - maxDiffusionRadius_,
                                 endPoint.y - maxDiffusionRadius_};

            for (size_t y = minSeedIndex.y; y < maxSeedIndex.y; ++y)
                for (size_t x = minSeedIndex.x; x < maxSeedIndex.x; ++x) {
                    float dist = glm::length(endPoint - dvec2(x, y));
                    if (dist >= maxDiffusionRadius_) continue;

                    size_t neighborIndex = seedIndexMask_[x + y * seedDim.x];
                    if (neighborIndex == 0 || visited[neighborIndex - 1])
                        continue;  // Not a seed cell, or already visited.
                    neighborIndex--;

                    float newLength = node.shortestPathLength +
                                      dist * dist;  // The diffusion edge costs its square distance.
                    Node& neighbor = nodes_->at(neighborIndex);
                    if (neighbor.shortestPathLength > newLength) {
                        neighbor.shortestPathLength = newLength;
                        nodeQueue.emplace(neighborIndex, currentNode.nodeIndex, newLength);
                    }
                }
        }
        numSteps++;
        if (maxNumNodes_.get() != 0 && numSteps > maxNumNodes_.get()) {
            LogWarn("Reached max number of steps.");
            break;
        }
    }
    nodes_->at(beginningIdx - 1).shortestPathPreviousNode = nullptr;

    // =========== Generate Shortest Path ========
    if (foundDestination) {
        Node* tailNode = &nodes_->at(destinationIdx);
        float destinationDistance = tailNode->shortestPathLength;
        dvec3 previousSeedPoint = tailNode->streamline.getPositions().back();

        for (; tailNode != nullptr; tailNode = tailNode->shortestPathPreviousNode) {
            const auto& positions = tailNode->streamline.getPositions();
            auto pointIt =
                std::min_element(positions.rbegin(), positions.rend(), [&](auto& a, auto& b) {
                    return glm::length(a - previousSeedPoint) < glm::length(b - previousSeedPoint);
                });
            if (markJumps_.get()) lineVerts.emplace_back(*pointIt, vec4(1.0f));
            for (; pointIt != positions.rend(); ++pointIt) {
                lineVerts.emplace_back(
                    *pointIt,
                    vec4(tailNode->shortestPathLength / destinationDistance, 0.0f, 0.0f, 1.0f));
            }
            if (markJumps_.get()) lineVerts.emplace_back(std::get<0>(lineVerts.back()), vec4(1.0f));

            previousSeedPoint = positions[0];
        }
    }

    finishOff();
}

}  // namespace inviwo
