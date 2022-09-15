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
#pragma once

#include <inviwo/dataframe/dataframemoduledefine.h>  // for IVW_MODULE_DATAFRAME_API

#include <cstddef>                                   // for size_t
#include <cstdint>                                   // for int64_t
#include <functional>                                // for function
#include <string>                                    // for string
#include <string_view>                               // for string_view
#include <variant>                                   // for variant
#include <vector>                                    // for vector

namespace inviwo {
template <typename T> struct EnumTraits;

namespace filters {

enum class StringComp { Equal, NotEqual, Regex, RegexPartial };

enum class NumberComp { Equal, NotEqual, Less, LessEqual, Greater, GreaterEqual };

/**
 * Predicate functor for filtering items in a specific column of a row. Column indices are
 * zero-based.
 * @p ItemFilter::filter is called once for the data item in @p ItemFilter::column of each row.
 * If @p ItemFilter::filterOnHeader is true, then this filter is applied before the header row
 * is extracted.
 */
struct ItemFilter {
    using FilterFunc = std::variant<std::function<bool(std::string_view)>,
                                    std::function<bool(std::int64_t)>, std::function<bool(double)>>;

    /**
     * Predicate function for filtering a column. The data item of @c ItemFilter::column is
     * converted from std::string to the corresponding type of the predicate function before
     * @c filter is called, that is @c std::string_view, @c int, @c float, or @c double.
     * @see FilterFunc
     */
    FilterFunc filter;
    int column;  //!< zero-based column index
    bool filterOnHeader;
};

/// create an item filter matching strings with @p match based on @p op
IVW_MODULE_DATAFRAME_API ItemFilter stringMatch(int column, filters::StringComp op,
                                                std::string_view match);

/// create an item filter matching integers with @p value using @p op
IVW_MODULE_DATAFRAME_API ItemFilter intMatch(int column, filters::NumberComp op,
                                             std::int64_t value);

/// create an item filter matching doubles with @p value using @p op, @epsilon is used for equal and
/// not equal comparisons
IVW_MODULE_DATAFRAME_API ItemFilter doubleMatch(int column, filters::NumberComp op, double value,
                                                double epsilon = 0.0);

/// create an item filter matching an inclusive integer range [@p min, @p max]
IVW_MODULE_DATAFRAME_API ItemFilter intRange(int column, std::int64_t min, std::int64_t max);

/// create an item filter matching an inclusive double range [@p min, @p max]
IVW_MODULE_DATAFRAME_API ItemFilter doubleRange(int column, double min, double max);

}  // namespace filters

template <>
struct EnumTraits<filters::StringComp> {
    static std::string name() { return "StringComp"; }
};
template <>
struct EnumTraits<filters::NumberComp> {
    static std::string name() { return "NumberComp"; }
};

/**
 * DataFrame-specific filters
 */
namespace dataframefilters {

using namespace filters;

/**
 * Filters to be applied per row when filtering a DataFrame. All respective filter predicates are
 * combined using a union operation. Individual data items of the rows are filtered for includes and
 * excludes.
 */
struct Filters {
    std::vector<ItemFilter> include;
    std::vector<ItemFilter> exclude;
};

}  // namespace dataframefilters

/**
 * CSV-specific filters when parsing CSV files
 */
namespace csvfilters {

using namespace filters;

/**
 * Predicate functor for filtering rows.
 * @p RowFilter::filter is called once per row.
 * If @p RowFilter::filterOnHeader is true, then this filter is applied before the header row is
 * extracted.
 */
struct RowFilter {
    /**
     * Predicate function for rows. The first argument is the content of the entire row, the
     * second argument holds the line number.
     */
    std::function<bool(std::string_view, size_t)> filter;
    bool filterOnHeader;
};

/**
 * Filters to be applied per row while parsing the CSV file.
 * All respective filter predicates are combined using a union operation.
 * First, rows are filtered for includes and then excludes. Secondly, individual data items of
 * the remaining rows are filtered for includes and excludes.
 */
struct Filters {
    std::vector<RowFilter> includeRows;
    std::vector<RowFilter> excludeRows;
    std::vector<ItemFilter> includeItems;
    std::vector<ItemFilter> excludeItems;
};

/// create a filter matching empty rows
IVW_MODULE_DATAFRAME_API RowFilter emptyLines(bool filterOnHeader = true);

/// create a filter matching rows starting with @p begin, for example comment rows starting with '#'
IVW_MODULE_DATAFRAME_API RowFilter rowBegin(std::string_view begin, bool filterOnHeader);

/// create a filter matching an inclusive line range [@p min, @p max]
IVW_MODULE_DATAFRAME_API RowFilter lineRange(int min, int max, bool filterOnHeader);

}  // namespace csvfilters

}  // namespace inviwo
