/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2022-2024 Inviwo Foundation
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

#include <inviwo/dataframe/util/filters.h>

#include <stdlib.h>    // for size_t, abs
#include <algorithm>   // for max
#include <functional>  // for __base, equal_to, greater, greater_equal, less, less_equal, not_eq...
#include <regex>       // for regex, regex_match, regex_search

namespace inviwo {

namespace filters {

ItemFilter stringMatch(int column, filters::StringComp op, std::string_view match) {
    switch (op) {
        case filters::StringComp::Equal:
            return ItemFilter{[str = match](std::string_view item) { return item == str; }, column,
                              false};
        case filters::StringComp::NotEqual:
            return ItemFilter{[str = match](std::string_view item) { return item != str; }, column,
                              false};
        case filters::StringComp::Regex:
            return ItemFilter{[re = std::regex(std::string{match})](std::string_view item) {
                                  return std::regex_match(item.begin(), item.end(), re);
                              },
                              column, false};
        case filters::StringComp::RegexPartial:
            return ItemFilter{[re = std::regex(std::string{match})](std::string_view item) {
                                  return std::regex_search(item.begin(), item.end(), re);
                              },
                              column, false};
        default:
            return ItemFilter{[str = match](std::string_view item) { return item == str; }, column,
                              false};
    }
}

namespace detail {

template <typename T>
ItemFilter epsilonComparison(int column, filters::NumberComp op, T value, T epsilon) {
    auto createFilter = [v = value, column](auto comp) {
        return ItemFilter{std::function<bool(T)>([v, comp](T value) { return comp(value, v); }),
                          column, false};
    };

    switch (op) {
        case filters::NumberComp::Equal:
            return ItemFilter{std::function<bool(T)>([v = value, eps = epsilon](T value) {
                                  return std::abs(value - v) <= eps;
                              }),
                              column, false};
        case filters::NumberComp::NotEqual:
            return ItemFilter{std::function<bool(T)>([v = value, eps = epsilon](T value) {
                                  return std::abs(value - v) > eps;
                              }),
                              column, false};
        case filters::NumberComp::Less:
            return createFilter(std::less<T>());
        case filters::NumberComp::LessEqual:
            return createFilter(std::less_equal<T>());
        case filters::NumberComp::Greater:
            return createFilter(std::greater<T>());
        case filters::NumberComp::GreaterEqual:
            return createFilter(std::greater_equal<T>());
        default:
            return ItemFilter{std::function<bool(T)>([v = value, eps = epsilon](T value) {
                                  return std::abs(value - v) <= eps;
                              }),
                              column, false};
    }
}

template <typename T>
ItemFilter rangeComparison(int column, T min, T max) {
    return ItemFilter{
        std::function<bool(T)>([min, max](T value) { return (value >= min) && (value <= max); }),
        column, false};
}

}  // namespace detail

ItemFilter intMatch(int column, filters::NumberComp op, std::int64_t value) {
    auto createFilter = [v = value, column](auto comp) {
        return ItemFilter{std::function<bool(std::int64_t)>(
                              [v, comp](std::int64_t value) { return comp(value, v); }),
                          column, false};
    };

    switch (op) {
        case filters::NumberComp::Equal:
            return createFilter(std::equal_to<std::int64_t>());
        case filters::NumberComp::NotEqual:
            return createFilter(std::not_equal_to<std::int64_t>());
        case filters::NumberComp::Less:
            return createFilter(std::less<std::int64_t>());
        case filters::NumberComp::LessEqual:
            return createFilter(std::less_equal<std::int64_t>());
        case filters::NumberComp::Greater:
            return createFilter(std::greater<std::int64_t>());
        case filters::NumberComp::GreaterEqual:
            return createFilter(std::greater_equal<std::int64_t>());
        default:
            return createFilter(std::equal_to<std::int64_t>());
    }
}

ItemFilter doubleMatch(int column, filters::NumberComp op, double value, double epsilon) {
    return detail::epsilonComparison(column, op, value, epsilon);
}

ItemFilter intRange(int column, std::int64_t min, std::int64_t max) {
    return detail::rangeComparison(column, min, max);
}

ItemFilter doubleRange(int column, double min, double max) {
    return detail::rangeComparison(column, min, max);
}

}  // namespace filters

namespace csvfilters {

RowFilter emptyLines(bool filterOnHeader) {
    return RowFilter{[](std::string_view row, size_t) { return row.empty(); }, filterOnHeader};
}

RowFilter rowBegin(std::string_view begin, bool filterOnHeader) {
    return RowFilter{[str = std::string{begin}](std::string_view row, size_t) {
                         return row.substr(0, str.size()) == str;
                     },
                     filterOnHeader};
}

RowFilter lineRange(int min, int max, bool filterOnHeader) {
    return RowFilter{
        [min = static_cast<size_t>(std::max(min, 0)), max = static_cast<size_t>(std::max(max, 0))](
            std::string_view, size_t line) { return (line >= min) && (line <= max); },
        filterOnHeader};
}

}  // namespace csvfilters

}  // namespace inviwo
