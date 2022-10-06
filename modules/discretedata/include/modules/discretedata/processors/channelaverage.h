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

#pragma once

#include <modules/discretedata/discretedatamoduledefine.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/properties/minmaxproperty.h>
#include <modules/discretedata/ports/datasetport.h>
#include <modules/discretedata/properties/datachannelproperty.h>
#include <modules/discretedata/processors/createuniformgrid.h>
#include <modules/discretedata/channels/bufferchannel.h>
#include <inviwo/core/util/zip.h>

namespace inviwo {
namespace discretedata {

/** \docpage{org.inviwo.ChannelAverage, Channel Average}
 * ![](org.inviwo.ChannelAverage.png?classIdentifier=org.inviwo.ChannelAverage)
 * Assemble the average over a certain dataset dimension.
 * Results in a uniform grid filling the bounding box of the given range.
 */
class IVW_MODULE_DISCRETEDATA_API ChannelAverage : public Processor {
public:
    ChannelAverage();
    virtual ~ChannelAverage() = default;

    virtual void process() override;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

private:
    static const std::string RANGE_ID;
    static const std::string GRID_SIZE_ID;

    DataSetInport dataIn_;
    DataSetOutport dataOut_;
    DataSetOutport avgOut_;

    DataChannelProperty positions_, data_;

    IntSizeTProperty averagingDim_;
    CompositeProperty dimRanges_;    // One FloatMinMaxProperty per dimension.
    CompositeProperty avgGridSize_;  // One IntSizeTProperty per dimension.
    BoolProperty relativeDiff_;

    template <ind SpatialDims>
    struct AverageDispatcher {
        template <typename T, ind N>
        std::shared_ptr<DataSet> operator()(const DataChannel<T, N>* dataChannel, DataSet* dataCopy,
                                            const ChannelAverage* processor,
                                            size_t numUnchangedDims) {

            auto positionChannel =
                processor->positions_.getCurrentChannelTyped<float, SpatialDims>();
            if (!positionChannel) {
                return nullptr;
            }

            std::array<float, SpatialDims> positionMin{0}, positionMax{0};
            positionChannel->getMinMax(positionMin, positionMax);

            // Assemble values from composite properties.
            std::array<vec2, SpatialDims> ranges;
            std::array<ind, SpatialDims> gridSize;
            ind numGridVerts = 1;
            const size_t averagingDim = processor->averagingDim_.get();

            for (size_t dim = 0; dim < SpatialDims; ++dim) {
                auto rangeProp = dynamic_cast<FloatMinMaxProperty*>(
                    processor->dimRanges_.getPropertyByIdentifier(fmt::format(RANGE_ID, dim)));
                if (!rangeProp) {
                    return nullptr;
                }

                rangeProp->setRange(vec2(positionMin[dim], positionMax[dim]));
                std::cout << fmt::format("Range: [{}, {}]", positionMin[dim], positionMax[dim])
                          << std::endl;
                if (dim >= numUnchangedDims || rangeProp->getStart() == rangeProp->getEnd()) {
                    rangeProp->setStart(positionMin[dim]);
                    rangeProp->setEnd(positionMax[dim]);
                }
                ranges[dim] = rangeProp->get();

                auto gridSizeProp =
                    dynamic_cast<IntSizeTProperty*>(processor->avgGridSize_.getPropertyByIdentifier(
                        fmt::format(GRID_SIZE_ID, dim)));
                if (!gridSizeProp) {
                    return nullptr;
                }
                gridSize[dim] = gridSizeProp->get();
                numGridVerts *= gridSize[dim];
            }

            auto findGridIndex = [&](const std::array<float, SpatialDims>& position,
                                     bool heedAvgRange) -> ind {
                // Find correct index (0 in averaging dim).
                ind increment = 1;
                ind linIdx = 0;
                for (size_t dim = 0; dim < SpatialDims; ++dim) {
                    if (dim == averagingDim) {
                        if (heedAvgRange &&
                            (position[dim] < ranges[dim].x || position[dim] > ranges[dim].y))
                            return -1;
                        increment *= gridSize[dim];
                        continue;
                    }
                    if (position[dim] < ranges[dim].x || position[dim] > ranges[dim].y) return -1;

                    ind dimIdx = ind((position[dim] - ranges[dim].x) * (gridSize[dim] - 1) /
                                         (ranges[dim].y - ranges[dim].x) +
                                     0.5);
                    linIdx += increment * dimIdx;
                    increment *= gridSize[dim];
                    if (dimIdx < 0 || dimIdx >= gridSize[dim]) return -1;
                }
                return linIdx;
            };

            auto uniformPositions = std::make_shared<
                AnalyticChannel<float, SpatialDims, std::array<float, SpatialDims>>>(
                [ranges, gridSize](std::array<float, SpatialDims>& vec, ind idx) {
                    ind remainingIdx = idx;
                    for (ind n = 0; n < SpatialDims; ++n) {
                        ind dimIdx = remainingIdx % gridSize[n];
                        remainingIdx /= gridSize[n];
                        vec[n] = ranges[n].x +
                                 ((ranges[n].y - ranges[n].x) * dimIdx) / (gridSize[n] - 1);
                    }
                },
                numGridVerts, "positions", GridPrimitive::Vertex);

            auto avgsChannel = std::make_shared<BufferChannel<T, N>>(
                numGridVerts, fmt::format("avg_{}", dataChannel->getName()));
            avgsChannel->setInvalidValue(T());
            auto sumCountChannel = std::make_shared<BufferChannel<size_t, 1>>(
                numGridVerts, fmt::format("avg_numContributors_{}", dataChannel->getName()));
            sumCountChannel->setInvalidValue(0);

            auto avgDiffChannel = std::make_shared<BufferChannel<T, N>>(
                dataCopy->getGrid()->getNumElements(),
                fmt::format("avg_difference_{}", dataChannel->getName()));
            avgDiffChannel->setInvalidValue(dataChannel->getInvalidValue());

            std::array<T, N> data;
            std::array<float, SpatialDims> pos;
            std::array<T, N>* avgData = nullptr;
            std::array<T, N>* avgDiffData = nullptr;

            for (ind idx = 0; idx < dataChannel->size(); ++idx) {

                // Check validity.
                dataChannel->fill(data, idx);  // Maybe do several idxs at once ?
                positionChannel->fill(pos, idx);
                avgDiffChannel->template get<std::array<T, N>>(idx) = data;
                if (!dataChannel->isValid(data[0]) || !positionChannel->isValid(pos[0])) continue;

                // Find correct index (0 in averaging dim).
                ind linIdx = findGridIndex(pos, true);

                // Increment counter.
                if (linIdx < 0 || linIdx >= numGridVerts) {
                    continue;
                }
                avgData = &avgsChannel->template get<std::array<T, N>>(linIdx);
                for (size_t n = 0; n < N; ++n) avgData->at(n) += data[n];
                sumCountChannel->get(linIdx)++;
            }

            // Divide sum by cell count and spread out.
            size_t numNormalVerts = 1;
            size_t numVertsBelowAvgDim = 1;
            for (size_t dim = 0; dim < size_t(SpatialDims); ++dim) {
                if (dim == averagingDim) continue;
                numNormalVerts *= gridSize[dim];
                if (dim < averagingDim) numVertsBelowAvgDim *= gridSize[dim];
            }

            for (size_t v = 0; v < numNormalVerts; ++v) {
                size_t fullDimIndex = (v % numVertsBelowAvgDim) + size_t(v / numVertsBelowAvgDim) *
                                                                      gridSize[averagingDim] *
                                                                      numVertsBelowAvgDim;
                // Average over num contibutors.
                size_t count = sumCountChannel->get(fullDimIndex);
                if (count != 0) {
                    avgData = &avgsChannel->template get<std::array<T, N>>(fullDimIndex);
                    for (size_t n = 0; n < N; ++n) avgData->at(n) /= count;
                }

                // Spread out into averaging dim.
                for (ind avgIdx = 1; avgIdx < gridSize[averagingDim]; ++avgIdx) {
                    size_t fullIdx = fullDimIndex + avgIdx * numVertsBelowAvgDim;
                    if (count != 0) {
                        avgsChannel->template get<std::array<T, N>>(fullIdx) = *avgData;
                    }
                }
            }

            // Fill difference values channel.
            for (ind idx = 0; idx < dataChannel->size(); ++idx) {

                // Check validity.
                avgDiffData = &avgDiffChannel->template get<std::array<T, N>>(idx);
                positionChannel->fill(pos, idx);
                if (!dataChannel->isValid((*avgDiffData)[0]) || !positionChannel->isValid(pos[0]))
                    continue;

                // Find correct index (0 in averaging dim).
                ind linIdx = findGridIndex(pos, false);
                if (linIdx < 0 || linIdx >= numGridVerts) continue;

                // Increment counter.
                avgData = &avgsChannel->template get<std::array<T, N>>(linIdx);
                if (!processor->relativeDiff_.get()) {
                    for (size_t n = 0; n < N; ++n) (*avgDiffData)[n] -= avgData->at(n);
                } else {
                    for (size_t n = 0; n < N; ++n)
                        (*avgDiffData)[n] = ((*avgDiffData)[n] - avgData->at(n)) / avgData->at(n);
                }
            }

            // Assemble into datasets.
            auto avgDataset = std::make_shared<DataSet>("AverageField", gridSize);
            avgDataset->addChannel(uniformPositions);
            avgDataset->addChannel(avgsChannel);
            avgDataset->addChannel(sumCountChannel);

            dataCopy->addChannel(avgDiffChannel);

            return avgDataset;
        }
    };

    struct AverageSpatialDispatcher {
        template <typename Result, ind N>
        Result operator()(DataSet* dataCopy, const ChannelAverage* processor,
                          size_t numUnchangedDims) {
            if (!processor) return nullptr;
            auto dataChannel = processor->data_.getCurrentChannel();
            if (!dataChannel) return nullptr;

            return dataChannel->dispatch<std::shared_ptr<DataSet>, dispatching::filter::Float1s, 1,
                                         DISCRETEDATA_MAX_NUM_DIMENSIONS>(
                AverageDispatcher<N>(), dataCopy, processor, numUnchangedDims);
        }
    };
};

}  // namespace discretedata
}  // namespace inviwo
