/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2023 Inviwo Foundation
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

#include <modules/vectorfieldvisualization/processors/currents/showstreampath.h>

#include <inviwo/core/util/indexmapper.h>
#include <modules/base/algorithm/meshutils.h>
#include <inviwo/core/datastructures/geometry/basicmesh.h>
#include <inviwo/core/util/foreach.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/datastructures/volume/volumeramprecision.h>
#include <inviwo/core/util/volumeramutils.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo ShowStreamPath::processorInfo_{
    "org.inviwo.ShowStreamPath",  // Class identifier
    "Show Stream Path",           // Display name
    "Undefined",                  // Category
    CodeState::Experimental,      // Code state
    Tags::None,                   // Tags
};
const ProcessorInfo ShowStreamPath::getProcessorInfo() const { return processorInfo_; }

ShowStreamPath::ShowStreamPath()
    : Processor()
    , velocitySampler_("velocitySampler")
    , indexVolume_("indexVolume")
    , destinationVolume_("destinationVolume")

    , destination_("destination", "End Point", size3_t(1, 1, 1),
                   {{0, 0, 0}, ConstraintBehavior::Immutable},
                   {{32, 32, 32}, ConstraintBehavior::Mutable})
    , integrationProperties_("integrationProperties", "Integration Properties")
    , samplingStride_("samplingStride", "Sampling Stride", 50, 1, 100)
    , lineColor_("lineColor", "Line Color", vec4(0.0f, 0.0f, 1.0f, 1.0f), vec4(0.0f), vec4(1.0f),
                 vec4(0.01f), InvalidationLevel::InvalidOutput, PropertySemantics::Color)
    , jumpColor_("jumpColor", "Jump Color", vec4(0.0f, 0.0f, 1.0f, 1.0f), vec4(0.0f), vec4(1.0f),
                 vec4(0.01f), InvalidationLevel::InvalidOutput, PropertySemantics::Color)
    , markJumps_("markJumps", "Mark Jumps")
    , integrate_("integrate", "Integrate")

    , pointMesh_("pointMesh")
    , lineMesh_("lineMesh")
    , jumpMesh_("jumpMesh")
    , integralLines_("integralLines")
    , endPointSeed_("endPointSeed") {

    velocitySampler_.setOptional(true);
    destinationVolume_.setOptional(true);

    integrationProperties_.seedPointsSpace_.set(CoordinateSpace::Data);
    integrationProperties_.seedPointsSpace_.setVisible(false);
    integrationProperties_.seedPointsSpace_.setCurrentStateAsDefault();

    addPorts(indexVolume_, velocitySampler_, destinationVolume_, lineMesh_, jumpMesh_, pointMesh_,
             integralLines_, endPointSeed_);
    addProperties(destination_, integrationProperties_, samplingStride_, lineColor_, jumpColor_,
                  markJumps_, integrate_);
}

void ShowStreamPath::process() {
    destination_.setVisible(!destinationVolume_.hasData());
    auto indexVolume = indexVolume_.getData();

    size3_t gridSize = indexVolume->getDimensions();
    destination_.setMaxValue(gridSize - size3_t(1));

    auto pointMesh = std::make_shared<ColoredMesh>(DrawType::Points, ConnectivityType::None);
    pointMesh->setModelMatrix(indexVolume->getModelMatrix());
    pointMesh->setWorldMatrix(indexVolume->getWorldMatrix());
    std::vector<ColoredMesh::Vertex> pointVerts;
    auto endPointVec =
        vec3{(0.5f + destination_.get().x) / gridSize.x, (0.5f + destination_.get().y) / gridSize.y,
             (0.5f + destination_.get().z) / gridSize.z};
    pointVerts.emplace_back(endPointVec, vec4{0.0f, 0.0f, 1.0f, 1.0f});
    pointMesh->addVertices(pointVerts);
    pointMesh_.setData(pointMesh);
    endPointSeed_.setData({endPointVec});

    // indexVolume->
    const auto* indexVolumeRAM = dynamic_cast<const VolumeRAMPrecision<glm::u64>*>(
        indexVolume->getRepresentation<VolumeRAM>());

    if (!indexVolumeRAM) {
        LogError(
            "Index volume must be of type glm::u64 (unsigned long/size_t) and of same dimensions.");
        return;
    }

    auto lineMesh = std::make_shared<ColoredMesh>(DrawType::Lines, ConnectivityType::Strip);
    lineMesh->setModelMatrix(indexVolume->getModelMatrix());
    lineMesh->setWorldMatrix(indexVolume->getWorldMatrix());
    std::vector<ColoredMesh::Vertex> lineVerts;

    auto jumpLineMesh = std::make_shared<ColoredMesh>(DrawType::Lines, ConnectivityType::None);
    jumpLineMesh->setModelMatrix(indexVolume->getModelMatrix());
    jumpLineMesh->setWorldMatrix(indexVolume->getWorldMatrix());
    std::vector<ColoredMesh::Vertex> jumpLineVerts;

    const glm::u64* indexData = indexVolumeRAM->getDataTyped();

    constexpr glm::u64 undoneIndex = std::numeric_limits<glm::u64>::max();
    constexpr glm::u64 landmassIndex = std::numeric_limits<glm::u64>::max() - 1;

    util::IndexMapper3D indexMapper(gridSize);

    const bool integrate = integrate_.get() && velocitySampler_.hasData();
    std::unique_ptr<Tracer> tracer;
    std::shared_ptr<IntegralLineSet> allLines;
    if (integrate) {
        auto sampler = velocitySampler_.getData();
        tracer = std::make_unique<Tracer>(sampler, integrationProperties_);
        allLines =
            std::make_shared<IntegralLineSet>(sampler->getModelMatrix(), sampler->getWorldMatrix());
    }

    auto setMeshOutputs = [&]() {
        lineMesh->addVertices(lineVerts);
        lineMesh_.setData(lineMesh);
        jumpLineMesh->addVertices(jumpLineVerts);
        jumpMesh_.setData(jumpLineMesh);
    };

    std::vector<glm::u64> destinations;
    if (destinationVolume_.hasData() && destinationVolume_.getData()->getDimensions() == gridSize) {
        auto destinationVolumeRAM = destinationVolume_.getData()->getRepresentation<VolumeRAM>();

        const size3_t& destinationSize = destinationVolumeRAM->getDimensions();
        util::IndexMapper3D destinationIndexMapper(destinationVolumeRAM->getDimensions());
        size_t destinationNumVoxels = destinationSize.x * destinationSize.y * destinationSize.z;
        for (size_t cellIdx = 0; cellIdx < destinationNumVoxels; ++cellIdx) {
            size3_t pos3D = destinationIndexMapper(cellIdx);
            if (destinationVolumeRAM->getAsDouble(pos3D) != 0.0) {
                destinations.push_back(cellIdx);
            }
        }
    } else {
        destinations.push_back(indexMapper(destination_.get()));
    }

    size_t numLinePoints = 0;
    for (glm::u64 currentNode : destinations) {
        // glm::u64 currentNode = indexMapper(destination_.get());
        glm::u64 previousNode = indexData[currentNode];

        if (previousNode == undoneIndex || previousNode == landmassIndex) {
            // Set outputs.
            continue;
        }

        while (currentNode != previousNode) {  // A node pointing to itself marks a seed.
            size3_t endIndex = indexMapper(currentNode);
            size3_t startIndex = indexMapper(previousNode);
            if (currentNode == undoneIndex || currentNode == landmassIndex) {
                LogError("Invalid index. Either undone or on land.");
                return;
            }
            vec3 endPoint = {float(0.5f + endIndex.x) / gridSize.x,
                             float(0.5f + endIndex.y) / gridSize.y,
                             float(0.5f + endIndex.z) / gridSize.z};
            vec3 startPoint = {float(0.5f + startIndex.x) / gridSize.x,
                               float(0.5f + startIndex.y) / gridSize.y,
                               float(0.5f + startIndex.z) / gridSize.z};
            if (jumpLineVerts.size() > 0) jumpLineVerts.emplace_back(endPoint, jumpColor_.get());
            if (markJumps_.get()) {
                lineVerts.emplace_back(endPoint, jumpColor_.get());
            }

            if (integrate) {
                IntegralLine line = tracer->traceFrom(startPoint);
                if (line.getPositions().size() == 0) {
                    LogWarn(fmt::format("Could not integrate line from {}.", startPoint));
                    break;
                }
                auto nearestPointIt =
                    std::min_element(line.getPositions().begin(), line.getPositions().end(),
                                     [&](const auto& p0, const auto& p1) {
                                         return glm::length2(vec3(p0.x, p0.y, p0.z) - endPoint) <
                                                glm::length2(vec3(p1.x, p1.y, p1.z) - endPoint);
                                     });
                if (nearestPointIt == line.getPositions().end()) {
                    LogWarn(fmt::format("Could not integrate line from {}.", startPoint));
                    break;
                }
                for (auto it = std::reverse_iterator(nearestPointIt);
                     it != line.getPositions().rend(); ++it) {
                    lineVerts.emplace_back(vec3(it->x, it->y, it->z), lineColor_.get());
                }
                allLines->push_back(std::move(line), previousNode);
            } else {
                lineVerts.push_back({endPoint, lineColor_.get()});
                lineVerts.push_back({startPoint, lineColor_.get()});
            }
            if (markJumps_.get()) {
                lineVerts.emplace_back(startPoint, jumpColor_.get());
            }
            jumpLineVerts.emplace_back(startPoint, jumpColor_.get());

            currentNode = previousNode;
            previousNode = indexData[currentNode];
        }
        if (jumpLineVerts.size() > 0) jumpLineVerts.pop_back();

        if (destinations.size() > 0) {
            auto indexBuffer = lineMesh->addIndexBuffer(DrawType::Lines, ConnectivityType::Strip);
            for (size_t idx = numLinePoints; idx < lineVerts.size(); ++idx) {
                indexBuffer->add(static_cast<std::uint32_t>(idx));
            }
        }
        numLinePoints = lineVerts.size();
    }

    // Set outputs.
    setMeshOutputs();
}

}  // namespace inviwo
