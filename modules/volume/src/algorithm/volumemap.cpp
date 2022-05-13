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

#include <inviwo/volume/algorithm/volumemap.h>

#include <algorithm>
#include <unordered_map>

namespace inviwo {

void remap(std::shared_ptr<Volume>& volume, std::vector<short> src, std::vector<short> dst,
           short missingValue, bool useMissingValue) {
    auto volRep = volume->getEditableRepresentation<VolumeRAM>();

    volRep->dispatch<void, dispatching::filter::Scalars>([&](auto volram) {
        using ValueType = util::PrecisionValueType<decltype(volram)>;
        ValueType* dataPtr = volram->getDataTyped();
        const auto& dim = volram->getDimensions();

        // Check state of dataframe
        uint32_t dataFrameState = 0;

        if (std::is_sorted(src.begin(), src.end()) &&
            (static_cast<int>(src.back() - src.front()) ==
             static_cast<int>(src.size() - 1))) {  // Sorted + continuous
            dataFrameState = 1;
        } else if (std::is_sorted(src.begin(), src.end())) {  // Sorted + non continuous
            dataFrameState = 2;
        } else {
            dataFrameState = 3;  // Unsorted + non continuous
        }

        short index = 0;
        std::unordered_map<short, short> unorderedIndexMap;
        switch (dataFrameState) {
            case 1:  // Use indexing directly
                std::transform(
                    dataPtr, dataPtr + dim.x * dim.y * dim.z, dataPtr, [&](const ValueType& v) {
                        // Voxel value is inside src range
                        if (static_cast<short>(v) >= src.front() &&
                            static_cast<short>(v) <= static_cast<short>(src.size() + src.front())) {
                            index = static_cast<short>(v) - src.front();
                            return static_cast<ValueType>(dst[index]);
                        } else if (useMissingValue) {
                            return static_cast<ValueType>(missingValue);
                        } else {
                            return static_cast<ValueType>(v);
                        }
                    });
                break;
            case 2:  // Binary search
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
            case 3:  // Use map
                for (uint32_t i = 0; i < src.size(); ++i) {
                    unorderedIndexMap[src[i]] = dst[i];
                }
                std::transform(
                    dataPtr, dataPtr + dim.x * dim.y * dim.z, dataPtr, [&](const ValueType& v) {
                        if (unorderedIndexMap.count(static_cast<short>(v)) ==
                            static_cast<short>(1)) {
                            return static_cast<ValueType>(unorderedIndexMap[static_cast<short>(v)]);
                        } else if (useMissingValue) {
                            return static_cast<ValueType>(missingValue);
                        } else {
                            return static_cast<ValueType>(v);
                        }
                    });
                break;
            default:  // Linear
                std::transform(dataPtr, dataPtr + dim.x * dim.y * dim.z, dataPtr,
                               [&](const ValueType& v) {
                                   for (uint32_t i = 0; i < src.size(); ++i) {
                                       if (static_cast<short>(v) == src[i]) {
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
