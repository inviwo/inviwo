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

#include <modules/vectorfieldvisualization/processors/currents/streamspanningtree3d.h>
// #include <inviwo/core/datastructures/image/image.h>
// #include <inviwo/core/datastructures/image/layerram.h>
// #include <inviwo/core/datastructures/image/layerramprecision.h>
// #include <inviwo/core/util/imageramutils.h>
#include <inviwo/core/util/indexmapper.h>
#include <modules/base/algorithm/meshutils.h>
#include <inviwo/core/datastructures/geometry/basicmesh.h>
#include <inviwo/core/util/foreach.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/datastructures/volume/volumeramprecision.h>
#include <inviwo/core/util/volumeramutils.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo StreamSpanningTree3D::processorInfo_{
    "org.inviwo.StreamSpanningTree3D",  // Class identifier
    "Stream Spanning Tree 3D",          // Display name
    "Undefined",                        // Category
    CodeState::Experimental,            // Code state
    Tags::None,                         // Tags
};
const ProcessorInfo StreamSpanningTree3D::getProcessorInfo() const { return processorInfo_; }

StreamSpanningTree3D::StreamSpanningTree3D()
    : Processor()
    , velocitySampler_("velocitySampler")
    , seedMask_("seedMask")
    , lineSelection_("lineSelection")

    , beginning_("beginning", "Beginning Point", {0, 0, 0},
                 {{0, 0, 0}, ConstraintBehavior::Immutable},
                 {{128, 128, 128}, ConstraintBehavior::Mutable})
    , destination_("destination", "Destination Point", {32, 32, 0},
                   {{0, 0, 0}, ConstraintBehavior::Immutable},
                   {{128, 128, 128}, ConstraintBehavior::Mutable})
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
    , searchCurrent_("searchCurrent", "Search Current", true)
    , dataCacher_()

    , pointMesh_("pointMesh")
    , lineMesh_("lineMesh")
    , integralLines_("allLines")
    , seedPoints_("seedPoints")

    , integrationRequired_(true)
    , firstProcess_(true)
    , maxSeedIndex_(0, 0, 0) {

    integrationProperties_.seedPointsSpace_.set(CoordinateSpace::Data);
    integrationProperties_.seedPointsSpace_.setVisible(false);
    // seedMask_.setOptional(true);

    auto requireIntegration = [this]() { integrationRequired_ = true; };
    integrationProperties_.onChange(requireIntegration);
    velocitySampler_.onChange(requireIntegration);
    seedMask_.onChange(requireIntegration);
    beginAtMaxValue_.onChange(requireIntegration);
    // dataCacher_.load_.onChange([&]() { seedMask_.setOptional(dataCacher_.load_.get()); }); //
    // Require seed mask to know where to render start/end point ;)

    addPorts(velocitySampler_, seedMask_, lineSelection_, pointMesh_, lineMesh_, integralLines_,
             seedPoints_);
    addProperties(beginning_, destination_, beginAtMaxValue_, maxDestinationDistance_,
                  integrationProperties_, maxDiffusionRadius_, samplingStride_, maxNumNodes_,
                  markJumps_, integrate_, searchCurrent_, dataCacher_);

    // dataCacher_.store_.onChange([this]() {
    //     std::cout << "hej..." << std::endl;
    //     if (dataCacher_.store_.get()) dataCacher_.storeData(*this);
    // });

    dataCacher_.loadNow_.onChange([this]() { dataCacher_.loadData(*this); });
    dataCacher_.storeNow_.onChange([this]() { dataCacher_.storeData(*this); });
}

void StreamSpanningTree3D::process() {
    std::cout << "Processing!" << std::endl;
    auto lineMesh = std::make_shared<ColoredMesh>(DrawType::Lines, ConnectivityType::Strip);
    lineMesh->setModelMatrix(velocitySampler_.getData()->getModelMatrix());
    lineMesh->setWorldMatrix(velocitySampler_.getData()->getWorldMatrix());
    std::vector<ColoredMesh::Vertex> lineVerts;

    auto pointMesh = std::make_shared<ColoredMesh>(DrawType::Points, ConnectivityType::None);
    pointMesh->setModelMatrix(seedMask_.getData()->getModelMatrix());
    pointMesh->setWorldMatrix(seedMask_.getData()->getWorldMatrix());
    std::vector<ColoredMesh::Vertex> pointVerts;

    auto seedDim = getSeedGridDimensions();
    util::IndexMapper3D indexMapper(seedDim);

    pointVerts.emplace_back(
        vec3{(0.5f + beginning_.get().x) / seedDim.x, (0.5f + beginning_.get().y) / seedDim.y,
             (0.5f + beginning_.get().z) / seedDim.z},
        vec4{1.0f, 0.0f, 0.0f, 1.0f});

    pointVerts.emplace_back(
        vec3{(0.5f + destination_.get().x) / seedDim.x, (0.5f + destination_.get().y) / seedDim.y,
             (0.5f + destination_.get().z) / seedDim.z},
        vec4{0.0f, 0.0f, 1.0f, 1.0f});

    bool successfulLoad = false;
    if (firstProcess_ && dataCacher_.load_.get()) {
        successfulLoad = dataCacher_.loadData(*this);
        if (successfulLoad) {
            integrationRequired_ = false;
            std::cout << "Loaded data successfully!" << std::endl;
        } else {

            std::cout << "Did not actually load data successfully :(" << std::endl;
        }
    }
    firstProcess_ = false;
    if (!integrate_.get() && !searchCurrent_.get() && !successfulLoad) {
        lineMesh->addVertices(lineVerts);
        pointMesh->addVertices(pointVerts);
        lineMesh_.setData(lineMesh);
        pointMesh_.setData(pointMesh);
        return;
    }

    beginning_.setMaxValue(seedDim - size3_t(1, 1, 1));
    destination_.setMaxValue(seedDim - size3_t(1, 1, 1));

    if (integrationRequired_ || successfulLoad) {
        // =============== Generate Seed Points ===============
        std::cout << "Integrating (or working with loaded data)!" << std::endl;

        nodes_ = std::make_unique<std::vector<Node>>();
        auto seeds = std::make_shared<std::vector<vec3>>();
        size_t numSeeds = seedDim.x * seedDim.y * seedDim.z;

        if (integrationRequired_) {
            auto volumeRAM = seedMask_.getData()->getRepresentation<VolumeRAM>();

            // Seed indices - offset by 1, so 0 is invalid and 1 is node index 0.

            seedIndexMask_ = std::make_unique<size_t[]>(numSeeds);
            std::fill_n(seedIndexMask_.get(), numSeeds, 0);

            // Save the position with maximal mask value.
            // It might be used as starting point.
            maxSeedIndex_ = beginning_.get();
            double maxValue = 0.0;

            volumeRAM->dispatch<void>([&](auto volumePrecision) {
                auto data = volumePrecision->getDataTyped();

                std::cout << "Integrating " << numSeeds << " streamlines!" << std::endl;
                util::forEachVoxel(*volumePrecision, [&](const size3_t& pos) {
                    size_t idx = indexMapper(pos);
                    double value = util::glm_convert_normalized<double>(data[idx]);
                    if (value > 0.0) {
                        nodes_->emplace_back(pos);
                        seedIndexMask_[idx] = nodes_->size();
                        seeds->push_back({(0.5f + pos.x) / seedDim.x, (0.5f + pos.y) / seedDim.y,
                                          (0.5f + pos.z) / seedDim.z});
                        if (value > maxValue) {
                            maxValue = value;
                            maxSeedIndex_ = pos;
                        }
                    }
                });
                std::cout << "Done preparing nodes!" << std::endl;
            });

            // =============== Integrate Lines ===============
            // Integrate streamlines.
            auto sampler = velocitySampler_.getData();
            Tracer tracer(sampler, integrationProperties_);
            allLines_ = std::make_shared<IntegralLineSet>(sampler->getModelMatrix(),
                                                          sampler->getWorldMatrix());

            // size_t tmpBeginningIdx = seedIndexMask_[indexMapper(beginning_.get())] - 1;
            util::forEachParallel(*nodes_, [&](auto&, size_t i) {
                Node& node = nodes_->at(i);
                node.streamline =
                    std::move(tracer.traceFrom(vec3(float(0.5f + node.startPoint.x) / seedDim.x,
                                                    float(0.5f + node.startPoint.y) / seedDim.y,
                                                    float(0.5f + node.startPoint.z) / seedDim.z)));
                auto& positions = node.streamline.getPositions();
                auto size = positions.size();
                if (size == 0) {
                    seedIndexMask_[i] = 0;
                    nodes_->at(i).shortestPathLength = -1.0f;
                    return;
                }

                auto& velocities = node.streamline.getMetaData<dvec3>("velocity");

                // Subsample. We don't need (and can't store) all points.
                size_t stride = samplingStride_.get();
                size_t numSubsampledPoints = size / stride;

                for (size_t pointSampleIdx = 0; pointSampleIdx <= numSubsampledPoints;
                     ++pointSampleIdx) {
                    positions[pointSampleIdx] = positions[pointSampleIdx * stride];
                    velocities[pointSampleIdx] = velocities[pointSampleIdx * stride];
                }
                // if (numSubsampledPoints < 2 &&
                //     size >= 2) {  // Not one full subsample length given, but some points anyway.
                //     positions[1] = positions.back();
                //     velocities[1] = velocities.back();
                //     numSubsampledPoints = 2;
                // }
                if (size > numSubsampledPoints * stride) {
                    positions[numSubsampledPoints] = positions.back();
                    velocities[numSubsampledPoints] = velocities.back();
                    numSubsampledPoints++;
                }
                positions.resize(numSubsampledPoints);
                velocities.resize(numSubsampledPoints);
            });
        } else {  // If data was loaded
            for (size_t i = 0; i < numSeeds; ++i) {
                if (seedIndexMask_[i] > 0) {
                    size3_t pos = indexMapper(i);
                    nodes_->emplace_back(pos);
                    nodes_->back().streamline = allLines_->at(seedIndexMask_[i] - 1);
                    seeds->push_back({(0.5f + pos.x) / seedDim.x, (0.5f + pos.y) / seedDim.y,
                                      (0.5f + pos.z) / seedDim.z});
                }
            }
        }
        integrationRequired_ = false;

        if (beginAtMaxValue_.get() && maxSeedIndex_ != beginning_.get()) {
            beginning_.set(maxSeedIndex_);
            pointVerts[0] = {
                vec3{(0.5f + maxSeedIndex_.x) / seedDim.x, (0.5f + maxSeedIndex_.y) / seedDim.y,
                     (0.5f + maxSeedIndex_.z) / seedDim.z},
                vec4{1.0f, 0.0f, 0.0f, 1.0f}};
            LogInfo("Changed beginning to max value.");
            std::cout << "changed beginning to max" << std::endl;
        }

        seedPoints_.setData(seeds);

        // dataCacher_.storeData(*this);
    } else {  // No integration required. Ergo, only Dijkstra.

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

    if (beginningIdx == 0 || !searchCurrent_.get()) {
        finishOff();
        LogError("Invalid starting point OR not searching for currents.");
        return;
    }
    auto priorityCompare = [](const QueueNode& left, const QueueNode& right) {
        return left.shortestDistance > right.shortestDistance;
    };

    std::priority_queue<QueueNode, std::vector<QueueNode>, decltype(priorityCompare)> nodeQueue(
        priorityCompare);
    // Priority queue does not support removing or changing values.
    // Thus, have a secondary list to keep tabs of the visited nodes.
    std::vector<bool> visited(nodes_->size(), false);
    nodeQueue.emplace(beginningIdx - 1, beginningIdx - 1, 0.0f);

    // Time for Dijkstra.
    size_t numSteps = 0;
    bool foundDestination = false;
    while (!nodeQueue.empty()) {
        QueueNode currentNode = nodeQueue.top();
        nodeQueue.pop();

        if (visited[currentNode.nodeIndex])
            continue;  // This will happen if we found a shorter path to this node already.

        // Write shortest path information into currently handled node.
        visited[currentNode.nodeIndex] = true;
        Node& node = nodes_->at(currentNode.nodeIndex);
        node.shortestPathLength = currentNode.shortestDistance;
        node.shortestPathPreviousNode = &nodes_->at(currentNode.prevIndex);
        maxLength = node.shortestPathLength;
        //

        // Close enough to the destination to stop?
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
        // Move along streamline, take several point along line as possible end points
        // (i.e., one line, several potential length).
        // size_t streamlineSize = node.streamline.getPositions().size();
        // size_t pointSampleIdx = std::min(samplingStride_.get() - 1, streamlineSize - 1);
        // for (; pointSampleIdx < streamlineSize; pointSampleIdx += samplingStride_.get()) {
        auto positions = node.streamline.getPositions();

        for (size_t pIdx = positions.size() - 1; pIdx > 0; --pIdx) {

            dvec3 endPoint = dvec3(positions[pIdx]);

            endPoint = dvec3(endPoint.x * seedDim.x - 0.5f, endPoint.y * seedDim.y - 0.5f,
                             endPoint.z * seedDim.z - 0.5f);  // Bring into index range.
            size3_t maxSeedIndex{
                std::min<size_t>(std::ceil(endPoint.x + maxDiffusionRadius_), seedDim.x - 1),
                std::min<size_t>(std::ceil(endPoint.y + maxDiffusionRadius_), seedDim.y - 1),
                std::min<size_t>(std::ceil(endPoint.z + maxDiffusionRadius_), seedDim.z - 1)};
            size3_t minSeedIndex{std::max(endPoint.x - maxDiffusionRadius_, 0.0),
                                 std::max(endPoint.y - maxDiffusionRadius_, 0.0),
                                 std::max(endPoint.z - maxDiffusionRadius_, 0.0)};

            // Look through all potential nodes we could diffuse to.
            for (size_t z = minSeedIndex.z; z < maxSeedIndex.z; ++z)
                for (size_t y = minSeedIndex.y; y < maxSeedIndex.y; ++y)
                    for (size_t x = minSeedIndex.x; x < maxSeedIndex.x; ++x) {
                        float dist = glm::length(endPoint - dvec3(x, y, z));
                        if (dist >= maxDiffusionRadius_) continue;
                        size_t neighborIndex =
                            seedIndexMask_[x + y * seedDim.x +
                                           z * seedDim.x * seedDim.y];  // TODO: use indexMapper?

                        if (neighborIndex == 0 || visited[neighborIndex - 1])
                            continue;  // Not a seed cell, or already visited.
                        neighborIndex--;

                        float newLength =
                            node.shortestPathLength +
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
    // Create line mesh.
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

size3_t StreamSpanningTree3D::getSeedGridDimensions() const {
    auto mask = seedMask_.getData();
    if (!mask) return size3_t(0, 0, 0);
    return mask->getDimensions();
}

StreamSpanningTree3D::DataCacher::DataCacher()
    : BoolCompositeProperty("caching", "Streamline Caching", true)
    , cacheFile_("cacheFile", "File")
    , load_("load", "Load")
    // , store_("store", "Store")
    , loadNow_("loadNow", "Load now")
    , storeNow_("storeNow", "Store now") {
    addProperties(cacheFile_, load_, loadNow_, storeNow_);
}

bool StreamSpanningTree3D::DataCacher::storeData(const StreamSpanningTree3D& processor) {

    if (cacheFile_.get().empty() || processor.integrationRequired_ || !processor.allLines_ ||
        processor.allLines_->size() == 0 || !processor.seedIndexMask_) {

        return false;
    }

    auto file = std::ofstream(cacheFile_.get(),
                              std::ios_base::binary | std::ios_base::out | std::ios_base::trunc);
    if (!file.is_open()) {
        throw FileException(
            std::string("Could not open file \"" + cacheFile_.get() + "\" for storing."),
            IVW_CONTEXT);
    }
    // Store:
    // std::shared_ptr<IntegralLineSet> allLines_;
    // std::unique_ptr<size_t[]> seedIndexMask_;

    // Field size.
    size3_t seedDimensions = processor.getSeedGridDimensions();
    size_t numSeeds = seedDimensions.x * seedDimensions.y * seedDimensions.z;

    file.write((char*)&seedDimensions, sizeof(seedDimensions));
    file.write((char*)processor.seedIndexMask_.get(), sizeof(size_t) * numSeeds);
    file.write((char*)&processor.maxSeedIndex_, sizeof(maxSeedIndex_));

    size_t numLines = processor.allLines_->size();
    file.write((char*)&numLines, sizeof(numLines));

    for (IntegralLine& line : *processor.allLines_) {
        size_t lineSize = line.getPositions().size();
        file.write((char*)&lineSize, sizeof(lineSize));
        file.write((char*)line.getPositions().data(), sizeof(dvec3) * lineSize);
        file.write((char*)line.getMetaData<dvec3>("velocity").data(), sizeof(dvec3) * lineSize);
    }

    file.close();

    LogWarn("Stored successfully. Kinda.");
    std::cout << "Stored successfully. Kinda. " << numLines << " lines." << std::endl;
    return true;
}
bool StreamSpanningTree3D::DataCacher::loadData(StreamSpanningTree3D& processor) {
    std::cout << "Loading!" << std::endl;
    if (cacheFile_.get().empty()) return false;

    auto file = std::ifstream(cacheFile_.get(), std::ios_base::binary | std::ios_base::in);
    if (!file.is_open()) {
        throw FileException(
            std::string("Could not open file \"" + cacheFile_.get() + "\" for loading."),
            IVW_CONTEXT);
    }
    std::cout << "File is open" << std::endl;

    size3_t writtenSeedDimensions;
    file.read((char*)&writtenSeedDimensions, sizeof(writtenSeedDimensions));
    if (file.fail()) return false;

    size_t numSeeds = writtenSeedDimensions.x * writtenSeedDimensions.y * writtenSeedDimensions.z;
    size3_t maskSeedDimensions = processor.getSeedGridDimensions();

    if (numSeeds == 0 || (writtenSeedDimensions != maskSeedDimensions)) {
        std::cout << "Different seed grid size than we have here. Aborting." << std::endl;
        return false;
    }

    processor.seedIndexMask_ = std::make_unique<size_t[]>(numSeeds);
    file.read((char*)processor.seedIndexMask_.get(), sizeof(size_t) * numSeeds);
    file.read((char*)&processor.maxSeedIndex_, sizeof(maxSeedIndex_));

    size_t numLines;
    file.read((char*)&numLines, sizeof(numLines));
    if (file.fail()) return false;
    std::cout << "Lines: " << numLines << std::endl;
    if (!processor.allLines_) {
        auto sampler = processor.velocitySampler_.getData();
        if (!sampler) return false;
        processor.allLines_ =
            std::make_shared<IntegralLineSet>(sampler->getModelMatrix(), sampler->getWorldMatrix());
    }
    processor.allLines_->getVector().resize(numLines);
    std::cout << "Reading " << numLines << " lines." << std::endl;

    for (IntegralLine& line : *processor.allLines_) {
        size_t lineSize;
        file.read((char*)&lineSize, sizeof(lineSize));
        if (file.fail()) return false;

        line.getPositions().resize(lineSize);
        line.getMetaData<dvec3>("velocity", true).resize(lineSize);

        file.read((char*)line.getPositions().data(), sizeof(dvec3) * lineSize);
        file.read((char*)line.getMetaData<dvec3>("velocity").data(), sizeof(dvec3) * lineSize);
    }
    if (file.fail()) return false;

    file.close();

    LogWarn("Loaded successfully. Kinda.");
    return true;
}

}  // namespace inviwo
