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

#include <modules/vectorfieldvisualization/processors/currents/pathsbetweenregions.h>
#include <inviwo/core/datastructures/volume/volumeramprecision.h>
#include <inviwo/core/util/volumeramutils.h>
#include <inviwo/core/util/indexmapper.h>
#include <inviwo/core/util/foreach.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo PathsBetweenRegions::processorInfo_{
    "org.inviwo.PathsBetweenRegions",  // Class identifier
    "Paths Between Regions",           // Display name
    "Undefined",                       // Category
    CodeState::Experimental,           // Code state
    Tags::None,                        // Tags
};
const ProcessorInfo PathsBetweenRegions::getProcessorInfo() const { return processorInfo_; }

PathsBetweenRegions::PathsBetweenRegions()
    : PoolProcessor(pool::Option::DelayDispatch | pool::Option::QueuedDispatch)
    , sampler_("sampler")
    , startEndVolume_("startEndVolume")

    , integrationProperties_("integrationProperties", "Integration properties")
    , filter_("filter", "Filter by End Region", true)

    , validStartEndVolume_("validStartEndVolume")
    , integralLines_("streamlines") {

    addPorts(startEndVolume_, sampler_, validStartEndVolume_, integralLines_);
    addProperties(integrationProperties_, filter_);

    integrationProperties_.seedPointsSpace_.set(CoordinateSpace::Data);
    integrationProperties_.seedPointsSpace_.setVisible(false);
    integrationProperties_.seedPointsSpace_.setCurrentStateAsDefault();
}

void PathsBetweenRegions::process() {

    auto sampler = sampler_.getData();

    struct Result {
        std::shared_ptr<Volume> validStartEndVolume;
        std::shared_ptr<IntegralLineSet> lines;
    };

    std::cout << "==========================================" << std::endl;
    std::cout << startEndVolume_.getConnectedOutport()->getIdentifier() << std::endl;
    dispatchOne(
        [inputVolume = startEndVolume_.getData(), sampler = std::move(sampler),
         integrationProperties = integrationProperties_, filter = filter_.get(),
         this](pool::Progress progress) -> Result {
            auto lines = std::make_shared<IntegralLineSet>(sampler->getModelMatrix(),
                                                           sampler->getWorldMatrix());
            Tracer tracer(sampler, integrationProperties);

            std::mutex mutex;

            auto gridSize = inputVolume->getDimensions();
            size_t numCells = gridSize.x * gridSize.y * gridSize.z;

            auto validStartEndMask = std::make_shared<VolumeRAMPrecision<float>>(gridSize);
            float* resultData = validStartEndMask->getDataTyped();
            std::fill_n(resultData, numCells, 0);
            inputVolume->getRepresentation<VolumeRAM>()->dispatch<void, dispatching::filter::Vec2s>(
                [&](const auto* startEndRAM) {
                    auto* startEndData = startEndRAM->getDataTyped();
                    const util::IndexMapper3D indexMapper(gridSize);

                    // Count start cells for progress reporting.
                    std::vector<size_t> startCells;
                    for (size_t i = 0; i < numCells; ++i) {
                        if (startEndData[i][1] > 0) {
                            startCells.push_back(i);
                        }
                    }
                    size_t numStartCells = startCells.size();
                    // numStartCells = std::min(numStartCells, size_t(10));  // TMP! VERY MOST
                    // TMP!!!! startCells.resize(numStartCells);                     // TMP! VERY
                    // MOST TMP!!!!
                    std::cout << "Num cells: " << numStartCells << std::endl;
                    // for (size_t startIndex : startCells) {
                    util::forEach(startCells, [&](auto& startIndex, size_t step) {
                        if (step % (std::max(size_t(1), numStartCells / 1000)) == 0) {
                            // std::cout << "Progress: " << static_cast<float>(step) / numStartCells
                            //           << std::endl;
                            progress(static_cast<float>(step) / numStartCells);
                        }

                        size3_t startPoint = indexMapper(startIndex);
                        // util::forEachVoxelParallel(gridSize, [&](const size3_t& startPoint) {
                        //     size_t startIndex = indexMapper(startPoint);
                        // const auto& maskDataStart = startEndData[startIndex];
                        // if (lines->size() > 1000) return;  // TODO: TMP!!!

                        // if (maskDataStart[1] <= 0) return;
                        // if (startIndex % 1000 == 0) std::cout << "A node! " << startPoint <<
                        // std::endl;
                        IntegralLine streamline = tracer.traceFrom(vec3(
                            (0.5f + startPoint.x) / gridSize.x, (0.5f + startPoint.y) / gridSize.y,
                            (0.5f + startPoint.z) / gridSize.z));
                        // std::cout << "starting at "
                        //           << vec3((0.5f + startPoint.x) / gridSize.x,
                        //                   (0.5f + startPoint.y) / gridSize.y,
                        //                   (0.5f + startPoint.z) / gridSize.z)
                        //           << std::endl;
                        auto& linePositions = streamline.getPositions();
                        resultData[startIndex] = 0.5 + linePositions.size();
                        if (linePositions.size() < 2) {
                            return;
                        }

                        if (filter) {
                            for (size_t p = 1; p < linePositions.size(); ++p) {
                                // for (auto& point : linePositions) {
                                const dvec3 point = linePositions[p];
                                size3_t pointIdx = {size_t(point.x + 0.5), size_t(point.y + 0.5),
                                                    size_t(point.z + 0.5)};
                                auto linePointIndex = indexMapper(pointIdx);
                                const auto& maskDataPoint = startEndData[linePointIndex];

                                if (maskDataPoint[1] < 0) {  // Made it to end!
                                    linePositions.resize(p + 1);

                                    streamline.getMetaData<dvec3>("velocity").resize(p + 1);
                                    std::lock_guard<std::mutex> lock(mutex);
                                    lines->push_back(std::move(streamline), startIndex);

                                    resultData[startIndex] = 1;
                                    resultData[linePointIndex] = -1;
                                    return;
                                }

                                if (maskDataPoint[0] == 0) {  // Invalid region.
                                    return;
                                }
                            }
                        } else {
                            std::lock_guard<std::mutex> lock(mutex);
                            lines->push_back(std::move(streamline), step);
                            // std::cout << "Added line " << step << std::endl;
                            resultData[startIndex] = 1.0;
                        }
                    });
                });

            auto validStartEndVolume = std::make_shared<Volume>(validStartEndMask);
            validStartEndVolume->setModelMatrix(inputVolume->getModelMatrix());
            validStartEndVolume->setWorldMatrix(inputVolume->getWorldMatrix());
            validStartEndVolume->dataMap_.valueRange = dvec2(-1, 1);
            validStartEndVolume->dataMap_.dataRange = dvec2(-1, 1);
            return {std::move(validStartEndVolume), std::move(lines)};
        },
        [this](const Result& result) {
            validStartEndVolume_.setData(result.validStartEndVolume);
            integralLines_.setData(result.lines);
        });
}

}  // namespace inviwo
