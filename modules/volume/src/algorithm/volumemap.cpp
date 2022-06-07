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
#include <unordered_set>
#include <inviwo/core/util/exception.h>

namespace inviwo {

void remap(std::shared_ptr<Volume>& volume, std::vector<int> src, std::vector<int> dst,
           int missingValue, bool useMissingValue) {

    if (src.size() == 0 || src.size() != dst.size()) {
        throw Exception(IVW_CONTEXT_CUSTOM("Remap"), "Invalid dataframe size (src = {}, dst = {})",
                        src.size(), dst.size());
    }

    // Create sorted copy of src and check if it contains duplicates
    std::unordered_set<int> set(src.begin(), src.end());
    if (src.size() != set.size()) {
        throw Exception(IVW_CONTEXT_CUSTOM("Remap"),
                        "Duplicate elements in source row (numberOfDuplicates = {})",
                        src.size() - set.size());
    }

    auto volRep = volume->getEditableRepresentation<VolumeRAM>();

    volRep->dispatch<void, dispatching::filter::Scalars>([&](auto volram) {
        using ValueType = util::PrecisionValueType<decltype(volram)>;
        ValueType* dataPtr = volram->getDataTyped();
        const auto& dim = volram->getDimensions();

        // Check state of dataframe
        bool sorted = std::is_sorted(src.begin(), src.end());

        if (sorted && (src.back() - src.front()) ==
                          static_cast<int>(src.size()) - 1) {  // Sorted + continuous
            std::transform(dataPtr, dataPtr + glm::compMul(dim), dataPtr, [&](const ValueType& v) {
                // Voxel value is inside src range
                if (static_cast<int>(v) >= src.front() && static_cast<int>(v) <= src.back()) {
                    int index = static_cast<int>(v) - src.front();
                    return static_cast<ValueType>(dst[index]);
                } else if (useMissingValue) {
                    return static_cast<ValueType>(missingValue);
                } else {
                    return v;
                }
            });
        } else if (sorted) {  // Sorted + non continuous
            std::transform(dataPtr, dataPtr + glm::compMul(dim), dataPtr, [&](const ValueType& v) {
                auto index = std::distance(
                    src.begin(), std::lower_bound(src.begin(), src.end(), static_cast<int>(v)));
                if (index < static_cast<int>(src.size())) {
                    return static_cast<ValueType>(dst[index]);
                } else if (useMissingValue) {
                    return static_cast<ValueType>(missingValue);
                } else {
                    return v;
                }
            });
        } else {
            std::unordered_map<int, int> unorderedIndexMap;
            for (size_t i = 0; i < src.size(); ++i) {
                unorderedIndexMap[src[i]] = dst[i];
            }
            std::transform(dataPtr, dataPtr + glm::compMul(dim), dataPtr, [&](const ValueType& v) {
                if (unorderedIndexMap.count(static_cast<int>(v)) == 1) {
                    return static_cast<ValueType>(unorderedIndexMap[static_cast<int>(v)]);
                } else if (useMissingValue) {
                    return static_cast<ValueType>(missingValue);
                } else {
                    return v;
                }
            });
        }
    });
}
}  // namespace inviwo
