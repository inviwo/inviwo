/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2019 Inviwo Foundation
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

#include <modules/discretedata/discretedatatypes.h>

namespace inviwo {
namespace discretedata {
namespace dd_util {

/** \brief A list of lists, queryable using prefix sums
 *  @param type Geometric description of the cell
 */
template <typename Type>
struct PrefixSumVector {
    PrefixSumVector() { prefixSum_.push_back(0); }
    PrefixSumVector(const std::vector<Type>& data, const std::vector<Type>& prefix)
        : data_(data), prefixSum_(prefix) {
        invAssert(prefix[0] == 0 && prefix[prefix.size() - 1] == data.size(),
                  "Not a valid prefix vector");
    }
    PrefixSumVector(const std::vector<std::vector<Type>>& data) {
        data_.reserve(data.size() * 3);
        prefixSum_.reserve(data.size() + 1);
        prefixSum_.push_back(0);
        for (const auto& block : data) {
            prefixSum_.push_back(prefixSum_[prefixSum_.size() - 1] + block.size());
            data_.insert(data_.end(), block.begin(), block.end());
        }
    }
    void addBlock(const std::vector<Type>& block) {
        prefixSum_.push_back(prefixSum_[prefixSum_.size() - 1] + block.size());
        data_.insert(data_.end(), block.begin(), block.end());
    }

    void getBlock(ind index, std::vector<Type>& outBlock) const {
        for (ind idx = prefixSum_[index]; idx < prefixSum_[index + 1]; ++idx)
            outBlock.push_back(data_[idx]);
    }

    void append(const PrefixSumVector<Type>& other) {
        data_.insert(data_.end(), other.data_.begin(), other.data_.end());
        ind prevSize = prefixSum_[prefixSum_.size() - 1];
        prefixSum_.reserve(prefixSum_.size() + other.size());

        for (ind p : other.prefixSum_) prefixSum_.push_back(p + prevSize);
    }

    ind size() { return prefixSum_.size() - 1; }
    ind size(ind idx) { return prefixSum_[idx + 1] - prefixSum_[idx]; }

    std::vector<Type> data_;
    std::vector<ind> prefixSum_;
};

}  // namespace dd_util
}  // namespace discretedata
}  // namespace inviwo
