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

#include <modules/vectorfieldvisualization/processors/currents/streamspanningtreejit.h>
#include <inviwo/core/util/indexmapper.h>
#include <modules/base/algorithm/meshutils.h>
#include <inviwo/core/datastructures/geometry/basicmesh.h>
#include <inviwo/core/util/foreach.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/datastructures/volume/volumeramprecision.h>
#include <inviwo/core/util/volumeramutils.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo StreamSpanningTreeJIT::processorInfo_{
    "org.inviwo.StreamSpanningTreeJIT",  // Class identifier
    "Stream Spanning Tree JIT",          // Display name
    "Undefined",                         // Category
    CodeState::Experimental,             // Code state
    Tags::None,                          // Tags
};
const ProcessorInfo StreamSpanningTreeJIT::getProcessorInfo() const { return processorInfo_; }

StreamSpanningTreeJIT::StreamSpanningTreeJIT()
    : PoolProcessor(pool::Option::DelayDispatch | pool::Option::QueuedDispatch)
    , velocitySampler_("velocitySampler")
    , nodeMask_("seedMask")
    , integrationProperties_("integrationProperties", "Integration Properties")
    , maxDiffusionRadius_("maxDiffusionRadius", "Max Jump (in cells)", 2,
                          {0.71, ConstraintBehavior::Immutable}, {5, ConstraintBehavior::Ignore})
    , minSteps_("minSteps", "Min Steps", 10, 1, 1000)
    , samplingStride_("samplingStride", "Sampling Stride", 50, 1, 100)
    , maxNumNodes_("maxNumNodes", "Max Number Nodes", 10000, 0, 1000000000000, 1,
                   InvalidationLevel::InvalidOutput, PropertySemantics::Text)
    , integrate_("integrate", "Integrate?", true)
    , errorMeasure_("errorMeasure", "Error Measure",
                    {{"diffusion", "Diffusion", ErrorMeasure::Diffusion},
                     {"squaredDiffusion", "Squared Diffusion", ErrorMeasure::SquaredDiffusion},
                     {"relativeDiffusion", "Relative Diffusion", ErrorMeasure::RelativeDiffusion}},
                    1)

    , distanceVolume_("distanceVolume")
    , indexVolume_("indexVolume")

    , integrationRequired_(true) {
    integrationProperties_.seedPointsSpace_.set(CoordinateSpace::Data);
    integrationProperties_.seedPointsSpace_.setVisible(false);
    integrationProperties_.seedPointsSpace_.setCurrentStateAsDefault();

    errorMeasure_.setCurrentStateAsDefault();
    // NOTE: SQUARED distance was hard coded before!

    auto requireIntegration = [this]() { integrationRequired_ = true; };
    integrationProperties_.onChange(requireIntegration);
    velocitySampler_.onChange(requireIntegration);
    nodeMask_.onChange(requireIntegration);

    addPorts(velocitySampler_, nodeMask_, distanceVolume_, indexVolume_);
    addProperties(integrationProperties_, maxDiffusionRadius_, minSteps_, samplingStride_,
                  maxNumNodes_, errorMeasure_, integrate_);
}

void StreamSpanningTreeJIT::process() {
    struct CurrentResult {
        std::shared_ptr<VolumeRAMPrecision<float>> distanceVolume;
        std::shared_ptr<VolumeRAMPrecision<size_t>> indexVolume;
        std::shared_ptr<BasicMesh> currentMesh;
    };

    size3_t gridSize = getNodeGridDimensions();
    std::cout << "Processing!" << std::endl;
    if (integrate_.get()) {
        std::cout << fmt::format("== Doing {} steps", maxNumNodes_.get()) << std::endl;
        dispatchOne(
            [gridSize, nodeMask = nodeMask_.getData(), velocitySampler = velocitySampler_.getData(),
             integrationProperties = integrationProperties_, samplingStride = samplingStride_.get(),
             maxDiffusionRadius = maxDiffusionRadius_.get(), maxNumSteps = maxNumNodes_.get(),
             errorMeasure = errorMeasure_.get(), this](pool::Progress progress) -> CurrentResult {
                util::IndexMapper3D indexMapper(gridSize);
                size_t numCells = gridSize.x * gridSize.y * gridSize.z;

                std::cout << "Calculating! BIG TODO!" << std::endl;
                CurrentResult result{
                    std::make_shared<VolumeRAMPrecision<float>>(gridSize),
                    std::make_shared<VolumeRAMPrecision<glm::u64>>(gridSize),
                    std::make_shared<BasicMesh>(DrawType::Lines, ConnectivityType::Strip)};

                float* distanceData = result.distanceVolume->getDataTyped();
                glm::u64* indexData = result.indexVolume->getDataTyped();

                // Initalize volumes.
                constexpr glm::u64 undoneIndex = std::numeric_limits<glm::u64>::max();
                constexpr glm::u64 landmassIndex = std::numeric_limits<glm::u64>::max() - 1;
                std::fill_n(distanceData, numCells, -1);
                std::fill_n(indexData, numCells, landmassIndex);

                // Set up the priority queue for the Dijkstra search.
                auto priorityCompare = [](const QueueNode& left, const QueueNode& right) {
                    return left.shortestDistance > right.shortestDistance;
                };

                std::priority_queue<QueueNode, std::vector<QueueNode>, decltype(priorityCompare)>
                    nodeQueue(priorityCompare);

                // This volume is expected to contain:
                // U: all valid nodes for the graph search, and
                // V: the root nodes.
                auto maskVolumeRAM = nodeMask->getRepresentation<VolumeRAM>();
                if (maskVolumeRAM->getDataFormat()->getComponents() != 2) {
                    std::cout << "Node mask must have exactly two components(validity and roots)."
                              << std::endl;
                    return result;
                }
                // LogError("Node mask must have exactly two components (validity and roots).");

                std::cout << "Time to dispatch... ("
                          << maskVolumeRAM->getDataFormat()->getComponents() << " components)"
                          << std::endl;
                size_t numValidCells = 0;
                maskVolumeRAM->dispatch<void, dispatching::filter::Vec2s>(
                    [&](auto maskVolumePrecision) {
                        std::cout << "Preparing nodes..." << std::endl;
                        auto data = maskVolumePrecision->getDataTyped();

                        util::forEachVoxel(*maskVolumePrecision, [&](const size3_t& pos) {
                            size_t idx = indexMapper(pos);
                            // double value = util::glm_convert<dvec2>(data[idx]);
                            double isNode = (double)data[idx].x;
                            double isRoot = (double)data[idx].y;

                            if (isNode > 0.0) {
                                indexData[idx] = undoneIndex;
                                numValidCells++;
                            }
                            if (isRoot > 0.0) {
                                indexData[idx] = idx;
                                distanceData[idx] = 0.0f;
                                nodeQueue.emplace(idx, idx, 0.0f);
                            }
                        });
                        std::cout << fmt::format("Done preparing nodes!\n\t{} root nodes.",
                                                 nodeQueue.size())
                                  << std::endl;
                    });
                std::cout << fmt::format("Found {} valid cells", numValidCells) << std::endl;

                // ========= Error Functor ========
                std::function<float(float, float)> errorFunctor;
                switch (errorMeasure) {
                    case ErrorMeasure::Diffusion:
                        errorFunctor = [](float diffusionDistance, float) {
                            return diffusionDistance;
                        };
                        break;

                    case ErrorMeasure::SquaredDiffusion:
                        errorFunctor = [](float diffusionDistance, float) {
                            return diffusionDistance * diffusionDistance;
                        };
                        break;

                    case ErrorMeasure::RelativeDiffusion:
                        errorFunctor = [](float diffusionDistance, float streamlineLength) {
                            return diffusionDistance / streamlineLength;
                        };
                        break;

                    default:
                        LogError("Unknown error measure!");
                }

                // =========== Dijkstra ===========
                Tracer tracer(velocitySampler, integrationProperties);

                size_t numSteps = 0;
                bool foundDestination = false;

                while (!nodeQueue.empty()) {
                    QueueNode currentNode = nodeQueue.top();
                    nodeQueue.pop();
                    size_t currentIndex = currentNode.nodeIndex;

                    if (distanceData[currentIndex] > 0)
                        continue;  // This will happen if we found a shorter path to this node
                                   // already.

                    // Write shortest path information into currently handled node.
                    distanceData[currentIndex] = currentNode.shortestDistance;
                    indexData[currentIndex] = currentNode.prevIndex;

                    // Close enough to the destination to stop?
                    // TODO: not doing this right now...
                    // if (false) {
                    //     foundDestination = true;
                    //     break;
                    // }

                    // Update neighbors' distance.
                    // Integrate a streamline, take several point along line as possible end points
                    // (i.e., one line, several potential length).
                    size3_t startPoint = indexMapper(currentIndex);
                    IntegralLine streamline = tracer.traceFrom(
                        vec3((0.5f + startPoint.x) / gridSize.x, (0.5f + startPoint.y) / gridSize.y,
                             (0.5f + startPoint.z) / gridSize.z));
                    auto& positions = streamline.getPositions();
                    auto streamlineSize = positions.size();
                    if (streamlineSize == 0) continue;

                    glm::u64 pointIndex = std::min(size_t(1), minSteps_.get());
                    float streamlineLength = 0.0f;
                    size_t lastLengthIndex = 1;
                    bool lastStep = false;
                    while (!lastStep) {
                        pointIndex += samplingStride;
                        if (pointIndex > streamlineSize - 1) {
                            pointIndex = streamlineSize - 1;
                            lastStep = true;
                        }
                        for (size_t i = lastLengthIndex; i < pointIndex; i++) {
                            streamlineLength += glm::length(positions[i] - positions[i - 1]);
                        }
                        lastLengthIndex = pointIndex;

                        dvec3 streamlineEnd = dvec3(positions[pointIndex]);

                        streamlineEnd =
                            dvec3(streamlineEnd.x * gridSize.x, streamlineEnd.y * gridSize.y,
                                  streamlineEnd.z * gridSize.z);  // Bring into index range.
                        size3_t maxSeedIndex{
                            std::min<glm::u64>(std::ceil(streamlineEnd.x + maxDiffusionRadius),
                                               gridSize.x - 1),
                            std::min<glm::u64>(std::ceil(streamlineEnd.y + maxDiffusionRadius),
                                               gridSize.y - 1),
                            std::min<glm::u64>(std::ceil(streamlineEnd.z + maxDiffusionRadius),
                                               gridSize.z - 1)};
                        size3_t minSeedIndex{std::max(streamlineEnd.x - maxDiffusionRadius, 0.0),
                                             std::max(streamlineEnd.y - maxDiffusionRadius, 0.0),
                                             std::max(streamlineEnd.z - maxDiffusionRadius, 0.0)};

                        // Look through all potential nodes we could diffuse to.
                        for (glm::u64 z = minSeedIndex.z; z < maxSeedIndex.z; ++z)
                            for (glm::u64 y = minSeedIndex.y; y < maxSeedIndex.y; ++y)
                                for (glm::u64 x = minSeedIndex.x; x < maxSeedIndex.x; ++x) {
                                    float diffusionDistance =
                                        glm::length(streamlineEnd - dvec3(x, y, z));
                                    if (diffusionDistance >= maxDiffusionRadius) continue;

                                    glm::u64 nextIndex = indexMapper(x, y, z);
                                    if (indexData[nextIndex] != undoneIndex)
                                        continue;  // Not a seed cell, or already visited.

                                    float newLength =
                                        currentNode.shortestDistance +
                                        errorFunctor(diffusionDistance, streamlineLength);
                                    nodeQueue.emplace(nextIndex, currentIndex, newLength);
                                }
                        if (lastStep) break;
                    }
                    numSteps++;
                    if (maxNumSteps == 0 && numSteps % (numValidCells / 100) == 0) {
                        progress(double(numSteps) / numValidCells);
                    }
                    if (maxNumSteps != 0 && numSteps % (maxNumSteps / 20) == 0) {  // Log every 5%
                        progress(double(numSteps) / maxNumSteps);
                    }
                    if (maxNumSteps != 0 && numSteps > maxNumSteps) {
                        LogWarn("Reached max number of steps.");
                        break;
                    }
                }

                return result;
            },
            [this](CurrentResult result) {
                auto distanceVolume = std::make_shared<Volume>(result.distanceVolume);
                distanceVolume->setModelMatrix(velocitySampler_.getData()->getModelMatrix());
                distanceVolume->setWorldMatrix(velocitySampler_.getData()->getWorldMatrix());
                distanceVolume_.setData(distanceVolume);

                auto indexVolume = std::make_shared<Volume>(result.indexVolume);
                indexVolume->setModelMatrix(velocitySampler_.getData()->getModelMatrix());
                indexVolume->setWorldMatrix(velocitySampler_.getData()->getWorldMatrix());
                indexVolume_.setData(indexVolume);

                std::cout << "Yay. Volumes! Mesh!" << std::endl;
            });
    }
}

size3_t StreamSpanningTreeJIT::getNodeGridDimensions() const {
    auto mask = nodeMask_.getData();
    if (!mask) return size3_t(0, 0, 0);
    return mask->getDimensions();
}

}  // namespace inviwo
