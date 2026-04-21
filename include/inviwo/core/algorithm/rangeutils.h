/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2026 Inviwo Foundation
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

#include <algorithm>
#include <ranges>

namespace inviwo {

namespace views {

inline constexpr auto deref =
    std::views::transform([](auto ptr) -> decltype(auto) { return *ptr; });
}  // namespace views

namespace util {

#if defined(__cpp_lib_ranges_starts_ends_with) && __cpp_lib_ranges_starts_ends_with >= 202106L

inline constexpr decltype(std::ranges::ends_with) ends_with{};
inline constexpr decltype(std::ranges::starts_with) starts_with{};

#else

// From cpp_reference until we have this in all compilers we support
struct ends_with_fn {
    template <std::input_iterator I1, std::sentinel_for<I1> S1, std::input_iterator I2,
              std::sentinel_for<I2> S2, class Pred = std::ranges::equal_to,
              class Proj1 = std::identity, class Proj2 = std::identity>
        requires(std::forward_iterator<I1> || std::sized_sentinel_for<S1, I1>) &&
                (std::forward_iterator<I2> || std::sized_sentinel_for<S2, I2>) &&
                std::indirectly_comparable<I1, I2, Pred, Proj1, Proj2>
    constexpr bool operator()(I1 first1, S1 last1, I2 first2, S2 last2, Pred pred = {},
                              Proj1 proj1 = {}, Proj2 proj2 = {}) const {
        const auto n1 = std::ranges::distance(first1, last1);
        const auto n2 = std::ranges::distance(first2, last2);
        if (n1 < n2) return false;
        std::ranges::advance(first1, n1 - n2);
        return std::ranges::equal(std::move(first1), last1, std::move(first2), last2, pred, proj1,
                                  proj2);
    }

    template <std::ranges::input_range R1, std::ranges::input_range R2,
              class Pred = std::ranges::equal_to, class Proj1 = std::identity,
              class Proj2 = std::identity>
        requires(std::ranges::forward_range<R1> || std::ranges::sized_range<R1>) &&
                (std::ranges::forward_range<R2> || std::ranges::sized_range<R2>) &&
                std::indirectly_comparable<std::ranges::iterator_t<R1>, std::ranges::iterator_t<R2>,
                                           Pred, Proj1, Proj2>
    constexpr bool operator()(R1&& r1, R2&& r2, Pred pred = {}, Proj1 proj1 = {},
                              Proj2 proj2 = {}) const {
        const auto n1 = std::ranges::distance(r1);
        const auto n2 = std::ranges::distance(r2);
        if (n1 < n2) return false;
        return std::ranges::equal(
            std::views::drop(std::ranges::ref_view(r1), n1 - static_cast<decltype(n1)>(n2)), r2,
            pred, proj1, proj2);
    }
};

struct starts_with_fn {
    template <std::input_iterator I1, std::sentinel_for<I1> S1, std::input_iterator I2,
              std::sentinel_for<I2> S2, class Pred = std::ranges::equal_to,
              class Proj1 = std::identity, class Proj2 = std::identity>
        requires std::indirectly_comparable<I1, I2, Pred, Proj1, Proj2>
    constexpr bool operator()(I1 first1, S1 last1, I2 first2, S2 last2, Pred pred = {},
                              Proj1 proj1 = {}, Proj2 proj2 = {}) const {
        return std::ranges::mismatch(std::move(first1), last1, std::move(first2), last2,
                                     std::move(pred), std::move(proj1), std::move(proj2))
                   .in2 == last2;
    }

    template <std::ranges::input_range R1, std::ranges::input_range R2,
              class Pred = std::ranges::equal_to, class Proj1 = std::identity,
              class Proj2 = std::identity>
        requires std::indirectly_comparable<std::ranges::iterator_t<R1>,
                                            std::ranges::iterator_t<R2>, Pred, Proj1, Proj2>
    constexpr bool operator()(R1&& r1, R2&& r2, Pred pred = {}, Proj1 proj1 = {},
                              Proj2 proj2 = {}) const {
        return (*this)(std::ranges::begin(r1), std::ranges::end(r1), std::ranges::begin(r2),
                       std::ranges::end(r2), std::move(pred), std::move(proj1), std::move(proj2));
    }
};

inline constexpr ends_with_fn ends_with{};
inline constexpr starts_with_fn starts_with{};

#endif

}  // namespace util

}  // namespace inviwo
