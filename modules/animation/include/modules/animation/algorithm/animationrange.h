/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2020 Inviwo Foundation
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

#include <modules/animation/animationmoduledefine.h>
#include <modules/animation/datastructures/animationtime.h>

#include <algorithm>
#include <iterator>

namespace inviwo {

namespace animation {

/*
 * Returns iterators for elements in the range [from, to].
 * Deals with cases where from > to.
 * @return One iterator to first item >= min(from, to) and one iterator to item > max(from, to)
 */
template <typename Iterator>
auto getRange(Iterator begin, Iterator end, Seconds from, Seconds to)
    -> std::tuple<decltype(begin), decltype(end)> {
    auto first = std::min(from, to);
    auto last = std::max(from, to);
    // 'fromIt' will be the first item with a time larger than or equal to 'from'
    auto fromIt = std::lower_bound(begin, end, first,
                                   [](const auto& it, const auto& val) { return *it < val; });
    // 'toIt' will be the first key with a time larger than 'to'
    auto toIt = std::upper_bound(begin, end, last,
                                 [](const auto& val, const auto& it) { return val < *it; });
    return {fromIt, toIt};
}

}  // namespace animation

}  // namespace inviwo
