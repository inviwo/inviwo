/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2021-2023 Inviwo Foundation
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

#include <inviwo/core/util/transformiterator.h>
#include <inviwo/core/util/exception.h>
#include <tcb/span.hpp>

#include <vector>
#include <algorithm>

namespace inviwo {

namespace util {

template <typename T>
std::vector<T> makeVectorRange(ptrdiff_t start, ptrdiff_t end, ptrdiff_t step = 1) {
    std::vector<T> res(static_cast<size_t>((end - start) / step));
    std::generate(res.begin(), res.end(), [i = start, step]() mutable {
        auto tmp = i;
        i += step;
        return static_cast<T>(tmp);
    });
    return res;
}

/**
 * Generate all the successive \p r length permutations of the elements in \p values
 * \see https://docs.python.org/3/library/itertools.html#itertools.permutations
 *
 * Example:
 * \code
 * std::array values = {0, 1, 2};
 * util::Permutations perm(util::span{values}, 2);
 * do {
 *     for (auto v : perm) {
 *         std::cout << v << " ";
 *     }
 *     std::cout << "\n";
 * } while (perm.next());
 * \endcode
 */
template <typename T>
class Permutations {
public:
    Permutations(util::span<T> values, size_t r)
        : pool{values}
        , r{r}
        , indices{makeVectorRange<size_t>(0, pool.size())}
        , cycles{makeVectorRange<size_t>(pool.size(), pool.size() - r, -1)}
        , hasMore{true} {

        if (r > pool.size() || r < 1) {
            throw Exception("Invalid permutation size larger than input");
        }
    }

    bool next() {
        if (hasMore) {
            for (ptrdiff_t i = r - 1; i >= 0; --i) {
                --cycles[i];
                if (cycles[i] == 0) {
                    std::rotate(indices.begin() + i, indices.begin() + i + 1, indices.end());
                    cycles[i] = pool.size() - i;
                } else {
                    const auto j = pool.size() - cycles[i];
                    std::swap(indices[i], indices[j]);
                    return hasMore;
                }
            }
            hasMore = false;
        }
        return hasMore;
    }

    auto begin() const { return makeTransformIterator(transform(), indices.begin()); }
    auto end() const { return makeTransformIterator(transform(), indices.begin() + r); }

private:
    auto transform() const {
        return [this](size_t i) { return pool[i]; };
    }

    util::span<T> pool;
    size_t r;
    std::vector<size_t> indices;
    std::vector<size_t> cycles;
    bool hasMore;
};

template <typename T, size_t N>
Permutations(util::span<T, N>, size_t) -> Permutations<T>;

/**
 * Generate \p r length subsequences of elements from the input \p values.
 * \see https://docs.python.org/3/library/itertools.html#itertools.combinations
 *
 * Example:
 * \code
 * std::array values = {0, 1, 2};
 * util::Combinations comb(util::span{values}, 2);
 * do {
 *     for (auto v : comb) {
 *         std::cout << v << " ";
 *     }
 *     std::cout << "\n";
 * } while (comb.next());
 * \endcode
 */
template <typename T>
class Combinations {
public:
    Combinations(util::span<T> values, size_t r)
        : pool{values}, indices{makeVectorRange<ptrdiff_t>(0, r)} {

        if (indices.size() > pool.size() || indices.empty()) {
            throw Exception("Invalid combinations size larger than input");
        }
    }

    bool next() {
        const ptrdiff_t r = indices.size();
        const ptrdiff_t n = pool.size();
        for (ptrdiff_t i = r - 1; i >= 0; --i) {
            if (indices[i] != i + n - r) {
                ++indices[i];
                for (ptrdiff_t j = i + 1; j < r; ++j) {
                    indices[j] = indices[j - 1] + 1;
                }
                return true;
            }
        }
        return false;
    }

    auto begin() const { return makeTransformIterator(transform(), indices.begin()); }
    auto end() const { return makeTransformIterator(transform(), indices.end()); }

private:
    auto transform() const {
        return [this](size_t i) { return pool[i]; };
    }

    util::span<T> pool;
    std::vector<ptrdiff_t> indices;
};

template <typename T, size_t N>
Combinations(util::span<T, N>, size_t) -> Combinations<T>;

/**
 * Generate \p r length subsequences of elements from the input \p values.
 * \see https://docs.python.org/3/library/itertools.html#itertools.product
 *
 * Example:
 * \code
 * std::array sizes = {3, 2};
 * util::IndexProduct inds(util::span{sizes}, 2);
 * do {
 *     for (auto v : inds) {
 *         std::cout << v << " ";
 *     }
 *     std::cout << "\n";
 * } while (inds.next());
 * \endcode
 */
template <typename T>
class IndexProduct {
public:
    IndexProduct(util::span<T> sizes) : sizes{sizes}, indices(sizes.size(), 0) {}

    bool next() {
        const ptrdiff_t n = indices.size();
        for (ptrdiff_t i = n - 1; i >= 0; --i) {
            ++indices[i];
            if (indices[i] == sizes[i]) {
                indices[i] = 0;
            } else {
                return true;
            }
        }
        return false;
    }

    auto begin() const { return indices.begin(); }
    auto end() const { return indices.end(); }

private:
    util::span<T> sizes;
    std::vector<T> indices;
};

template <typename T, size_t N>
IndexProduct(util::span<T, N>) -> IndexProduct<T>;

}  // namespace util

}  // namespace inviwo
