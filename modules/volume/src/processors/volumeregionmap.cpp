/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2021 Inviwo Foundation
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

#include <inviwo/volume/processors/volumeregionmap.h>
#include <modules/base/algorithm/volume/volumevoronoi.h>
#include <algorithm>
#include <functional>
#include <fstream>
#include <unordered_map>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo VolumeRegionMap::processorInfo_{
    "org.inviwo.VolumeRegionMap",                               // Class identifier
    "Volume Region Map",                                        // Display name
    "Volume Operation",                                         // Category
    CodeState::Stable,                                          // Code state
    Tag::CPU | Tag{"Volume"} | Tag{"Atlas"} | Tag{"DataFrame"}  // Tags
};
const ProcessorInfo VolumeRegionMap::getProcessorInfo() const { return processorInfo_; }

VolumeRegionMap::VolumeRegionMap()
    : Processor()
    , dataFrame_("mapping_indices")
    , inport_("inport")
    , outport_("outport")
    , from_{"from", "From", dataFrame_, ColumnOptionProperty::AddNoneOption::No, 1}
    , to_{"to", "To", dataFrame_, ColumnOptionProperty::AddNoneOption::No, 2}
    , defaultMissingValue_{"setMissingValue", "Set missing values", true}
    , missingValues_{"missingValues", "Set missing values to", 0,
                     std::pair{0, ConstraintBehavior::Ignore},
                     std::pair{100, ConstraintBehavior::Ignore}} {

    addPort(inport_);
    addPort(dataFrame_);
    addPort(outport_);
    addProperties(from_, to_);
    addProperties(defaultMissingValue_);
    addProperties(missingValues_);
}

namespace {
constexpr auto copyColumn = [](const Column& col, auto& dstContainer, auto assign) {
    col.getBuffer()->getRepresentation<BufferRAM>()->dispatch<void, dispatching::filter::Scalars>(
        [&](auto buf) {
            const auto& data = buf->getDataContainer();
            for (auto&& [src, dst] : util::zip(data, dstContainer)) {
                std::invoke(assign, src, dst);
            }
        });
};
}

void VolumeRegionMap::process() {
    // Get data
    auto csv = dataFrame_.getData();
    auto oldIdx = csv->getColumn(from_.getSelectedValue());
    auto newIdx = csv->getColumn(to_.getSelectedValue());
    auto nrows = csv->getNumberOfRows();
    bool useMissingValue = defaultMissingValue_.get();
    int missingValue = missingValues_.get();

    // Store in vectors
    std::vector<uint32_t> sourceIndices(nrows);
    std::vector<uint32_t> destinationIndices(nrows);

    // Copy to vectors
    copyColumn(*oldIdx, sourceIndices,
               [](const auto& src, auto& dst) { dst = static_cast<uint32_t>(src); });
    copyColumn(*newIdx, destinationIndices,
               [](const auto& src, auto& dst) { dst = static_cast<uint32_t>(src); });

    // Make sure sizes match
    while (sourceIndices.size() != destinationIndices.size()) {
        if (sourceIndices.size() < destinationIndices.size()) {
            destinationIndices.pop_back();
        } else {
            sourceIndices.pop_back();
        }
    }

    // Volume
    auto inVolume = inport_.getData();
    auto newVolume = std::shared_ptr<Volume>(inVolume->clone());
    remap(newVolume, sourceIndices, destinationIndices, missingValue, useMissingValue);

    outport_.setData(newVolume);
}

void VolumeRegionMap::remap(std::shared_ptr<Volume>& volume, std::vector<uint32_t> src,
                            std::vector<uint32_t> dst, int missingValue, bool useMissingValue) {
    auto volRep = volume->getEditableRepresentation<VolumeRAM>();

    volRep->dispatch<void, dispatching::filter::Scalars>([&](auto volram) {
        using ValueType = util::PrecisionValueType<decltype(volram)>;
        ValueType* dataPtr = volram->getDataTyped();
        const auto& dim = volram->getDimensions();

        // Check state of dataframe
        uint32_t dataFrameState = 0;

        if (std::is_sorted(src.begin(), src.end()) &&
            (src.back() - src.front() == src.size() - 1)) {  // Sorted + continuous
            dataFrameState = 1;
        } else if (std::is_sorted(src.begin(), src.end())) {  // Sorted + non continuous
            dataFrameState = 2;
        } else {
            dataFrameState = 3;  // Unsorted + non continuous
        }

        uint32_t index = 0;
        std::unordered_map<uint32_t, uint32_t> unorderedIndexMap;
        switch (dataFrameState) {
            case 1: // Use indexing directly
                std::transform(
                    dataPtr, dataPtr + dim.x * dim.y * dim.z, dataPtr, [&](const ValueType& v) {
                        // Voxel value is inside src range
                        if (static_cast<uint32_t>(v) >= 0 &&
                            static_cast<uint32_t>(v) < static_cast<uint32_t>(src.size())) {
                            return static_cast<ValueType>(dst[static_cast<uint32_t>(v)]);
                        } else if (useMissingValue) {
                            return static_cast<ValueType>(missingValue);
                        } else {
                            return static_cast<ValueType>(v);
                        }
                    });
                break;
            case 2: // Binary search
                std::transform(
                    dataPtr, dataPtr + dim.x * dim.y * dim.z, dataPtr, [&](const ValueType& v) {
                        index = *std::lower_bound(src.begin(), src.end(), static_cast<uint32_t>(v));
                        if (index != *src.end()) {
                            return static_cast<ValueType>(dst[index]);
                        } else if (useMissingValue) {
                            return static_cast<ValueType>(missingValue);
                        } else {
                            return static_cast<ValueType>(v);
                        }
                    });
                break;
            case 3: // Use map
                for (uint32_t i = 0; i < src.size(); ++i) {
                    unorderedIndexMap[src[i]] = dst[i];
                }
                std::transform(dataPtr, dataPtr + dim.x * dim.y * dim.z, dataPtr,
                               [&](const ValueType& v) {
                                   if (unorderedIndexMap.count(static_cast<uint32_t>(v)) == 1) {
                                       return static_cast<ValueType>(
                                           unorderedIndexMap[static_cast<uint32_t>(v)]);
                                   } else if (useMissingValue) {
                                       return static_cast<ValueType>(missingValue);
                                   } else {
                                       return static_cast<ValueType>(v);
                                   }
                               });
                break;
            default: // Linear
                std::transform(dataPtr, dataPtr + dim.x * dim.y * dim.z, dataPtr,
                               [&](const ValueType& v) {
                                   for (uint32_t i = 0; i < src.size(); ++i) {
                                       if (static_cast<uint32_t>(v) == src[i]) {
                                           return static_cast<ValueType>(dst[i]);
                                       }
                                   }
                                   if (useMissingValue) {
                                       return static_cast<ValueType>(missingValue);
                                   } else {
                                       return static_cast<ValueType>(v);
                                   }
                               });
                break;
        }
    });
}
}  // namespace inviwo
