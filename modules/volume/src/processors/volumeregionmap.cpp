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
#include <iostream>
#include <fstream>
#include <map>
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
    , dataFrame_("mapping_indexes")
    , inport_("inport")
    , outport_("outport")
    , from_{"from", "From", dataFrame_, ColumnOptionProperty::AddNoneOption::No, 1}
    , to_{"to", "to", dataFrame_, ColumnOptionProperty::AddNoneOption::No, 2}
    , missingValues_{"missingValues", "Set unfound values", 0, 0, 100} {

    addPort(inport_);
    addPort(dataFrame_);
    addPort(outport_);
    addProperties(from_, to_);
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
    // Get columns
    auto csv = dataFrame_.getData();
    auto oldIdx = csv->getColumn(from_.getSelectedValue());
    auto newIdx = csv->getColumn(to_.getSelectedValue());
    auto nrows = csv->getNumberOfRows();

    int missingValue = missingValues_.get();

    // Store in vectors
    std::vector<uint32_t> sourceIndices(nrows);
    std::vector<uint32_t> destinationIndices(nrows);

    copyColumn(*oldIdx, sourceIndices,
               [](const auto& src, auto& dst) { dst = static_cast<uint32_t>(src); });
    copyColumn(*newIdx, destinationIndices,
               [](const auto& src, auto& dst) { dst = static_cast<uint32_t>(src); });

    // Volume
    auto inVolume = inport_.getData();
    auto newVolume = std::shared_ptr<Volume>(inVolume->clone());
    remap(newVolume, sourceIndices, destinationIndices, missingValue);

    outport_.setData(newVolume);
}

void VolumeRegionMap::remap(std::shared_ptr<Volume>& volume, std::vector<unsigned int> src,
                            std::vector<unsigned int> dst, int missingValue) {
    auto volRep = volume->getEditableRepresentation<VolumeRAM>();

    volRep->dispatch<void, dispatching::filter::Scalars>([&](auto volram) {
        using ValueType = util::PrecisionValueType<decltype(volram)>;
        ValueType* dataPtr = volram->getDataTyped();

        const auto& dim = volram->getDimensions();

        // Check which state the dataframe input is in
        size_t dataFrameState = 0;
        if (isSortedSequence(src)) {  // Sorted + continuous
            dataFrameState = 1;
        } else if (isSorted(src)) {  // Sorted + non continuous
            dataFrameState = 2;
        } else {
            dataFrameState = 3;  // Unsorted + non continuous
        }

        size_t index = 0;
        // std::map<unsigned int, unsigned int> orderedIndexMap;
        std::unordered_map<unsigned int, unsigned int> unorderedIndexMap;
        switch (dataFrameState) {
            case 1:
                std::transform(dataPtr, dataPtr + dim.x * dim.y * dim.z, dataPtr,
                               [&](const ValueType& v) {
                                   if (dst.size() - 1 < v) {
                                       return static_cast<ValueType>(missingValue);
                                   }
                                   return static_cast<ValueType>(dst[static_cast<uint32_t>(v)]);
                               });
                break;
            case 2:
                std::transform(dataPtr, dataPtr + dim.x * dim.y * dim.z, dataPtr,
                               [&](const ValueType& v) {
                                   index = binarySearch(src, static_cast<uint32_t>(v));
                                   return static_cast<ValueType>(dst[index]);
                               });
                break;
            case 3:
                for (size_t i = 0; i < src.size(); ++i) {
                    unorderedIndexMap[src[i]] = dst[i];
                }
                std::transform(dataPtr, dataPtr + dim.x * dim.y * dim.z, dataPtr,
                               [&](const ValueType& v) {
                                   if (unorderedIndexMap.count(static_cast<uint32_t>(v)) == 1) {
                                       return static_cast<ValueType>(
                                           unorderedIndexMap[static_cast<uint32_t>(v)]);
                                   }
                                   return static_cast<ValueType>(missingValue);
                               });
                break;
            default:
                std::transform(dataPtr, dataPtr + dim.x * dim.y * dim.z, dataPtr,
                               [&](const ValueType& v) {
                                   for (size_t i = 0; i < src.size(); ++i) {
                                       if (static_cast<uint32_t>(v) == src[i]) {
                                           return static_cast<ValueType>(dst[i]);
                                       }
                                   }
                                   return static_cast<ValueType>(missingValue);
                               });
                break;
        }
    });
}

/// <summary>
/// Tests if a index vector contains sequential numbers without gaps.
/// </summary>
/// <param name="src">Index vecotr</param>
/// <returns>True if sequential. False if not sequential.</returns>
bool isSortedSequence(const std::vector<unsigned int>& src) {
    unsigned int prev = src[0];
    for (int i = 1; i < src.size(); ++i) {
        if (!(src[i] == prev + 1)) {
            return false;
        }
        prev = src[i];
    }
    return true;
}

bool isSorted(const std::vector<unsigned int>& src) {
    unsigned int prev = src[0];
    for (int i = 1; i < src.size(); ++i) {
        if (src[i] <= prev) {
            return false;
        }
        prev = src[i];
    }
    return true;
}

size_t binarySearch(const std::vector<unsigned int>& src, const unsigned int value) {
    size_t low = 0;
    size_t high = src.size();
    while (low < high) {
        size_t mid = (low + high) / 2;
        if (mid <= 0) {
            return 0;
        }

        if (src[mid] == value) {
            return mid;
        }
        if (src[mid] < value) {
            low = mid + 1;
        } else {
            high = mid - 1;
        }
    }
    return 0;
}

}  // namespace inviwo
