/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2019 Inviwo Foundation
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

#ifndef IVW_DISJOINTSETS_H
#define IVW_DISJOINTSETS_H

#include <modules/base/basemoduledefine.h>
#include <inviwo/core/util/assertion.h>

#include <cstddef>
#include <vector>

namespace inviwo {

/**
 * Disjoint sets data structure with path compression and weighted unions.
 * @see https://en.m.wikipedia.org/wiki/Disjoint-set_data_structure
 */
template <typename T = int>
class DisjointSets {
public:
    /**
     * Construct size disjoint sets with 1 member in each set.
     */
    explicit DisjointSets(T size);

    /**
     * Join the sets of element r and s.
     * Requires r and s to be positive and less than size.
     * Return true it the sets were joined or false if r and s
     * already were in the same set.
     */
    bool join(T r, T s);

    /**
     * Returns name of the set for element x
     * i.e. return root of tree for element x
     * Requires x to be positive and less than size.
     */
    T find(T x);

    /**
     * Returns cardinality of the set for element x
     * Requires x to be positive and less than size.
     */
    T cardinality(T x);

    /**
     * Returns the total number of elements in all sets
     */
    std::size_t size();

private:
    std::vector<T> array_;
};

template <typename T>
inline DisjointSets<T>::DisjointSets(T size) : array_(size, T{-1}) {
    static_assert(std::is_signed<T>::value, "T must be a signed type");
    IVW_ASSERT(size > 0, "Size should be greater than 0");
}

template <typename T>
inline bool DisjointSets<T>::join(T r, T s) {
    IVW_ASSERT(r >= 0, "r should be greater than or equal to 0");
    IVW_ASSERT(s >= 0, "s should be greater than or equal to 0");

    IVW_ASSERT(r < static_cast<T>(array_.size()), "r should be less than size");
    IVW_ASSERT(s < static_cast<T>(array_.size()), "s should be less than size");

    r = find(r);
    s = find(s);
    if (r == s) return false;
    // weighted union (by cardinality)
    if (array_[r] <= array_[s]) {
        array_[r] += array_[s];
        array_[s] = r;
    } else {
        array_[s] += array_[r];
        array_[r] = s;
    }
    return true;
}

template <typename T>
inline T DisjointSets<T>::find(T x) {
    IVW_ASSERT(x >= 0, "x should be greater than or equal to 0");
    IVW_ASSERT(x < static_cast<T>(array_.size()), "x should be less than size");

    // find with path compression
    if (array_[x] < 0) {
        return x;
    } else {
        return array_[x] = find(array_[x]);
    }
}

template <typename T>
inline T DisjointSets<T>::cardinality(T x) {
    IVW_ASSERT(x >= 0, "x should be greater than or equal to 0");
    IVW_ASSERT(x < array_.size(), "x should be less than size");

    auto r = find(x);
    return -array_[r];
}

template <typename T>
inline std::size_t DisjointSets<T>::size() {
    return array_.size();
}

}  // namespace inviwo

#endif  // IVW_DISJOINTSETS_H
