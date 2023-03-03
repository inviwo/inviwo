/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2022-2023 Inviwo Foundation
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

#include <inviwo/core/common/inviwocoredefine.h>

#include <utility>
#include <iterator>

namespace inviwo {

namespace util {

template <class Iter>
struct iter_range : std::pair<Iter, Iter> {
    using value_type = typename std::iterator_traits<Iter>::value_type;
    using const_iterator = Iter;
    using iterator = Iter;
    using std::pair<Iter, Iter>::pair;
    iter_range(const std::pair<Iter, Iter>& x) : std::pair<Iter, Iter>(x) {}
    Iter begin() const { return this->first; }
    Iter end() const { return this->second; }
};

template <class Iter>
inline iter_range<Iter> as_range(Iter begin, Iter end) {
    return iter_range<Iter>(std::make_pair(begin, end));
}

template <class Iter>
inline iter_range<Iter> as_range(std::pair<Iter, Iter> const& x) {
    return iter_range<Iter>(x);
}

template <class Container>
inline iter_range<typename Container::iterator> as_range(Container& c) {
    using std::begin;
    using std::end;
    return iter_range<typename Container::iterator>(std::make_pair(begin(c), end(c)));
}
template <class Container>
inline iter_range<typename Container::const_iterator> as_range(const Container& c) {
    using std::begin;
    using std::end;
    return iter_range<typename Container::const_iterator>(std::make_pair(begin(c), end(c)));
}

}  // namespace util

}  // namespace inviwo
