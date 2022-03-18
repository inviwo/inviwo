/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2021 Inviwo Foundation
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

#include <inviwo/dataframe/dataframemoduledefine.h>

#include <inviwo/core/datastructures/buffer/buffer.h>
#include <inviwo/core/datastructures/buffer/bufferram.h>
#include <inviwo/core/io/serialization/serializable.h>
#include <inviwo/dataframe/datastructures/dataframe.h>
#include <inviwo/dataframe/util/filters.h>
#include <inviwo/core/util/zip.h>

namespace inviwo {

/**
 * Utility functions for DataFrame operations
 */
namespace dataframe {

IVW_MODULE_DATAFRAME_API std::shared_ptr<BufferBase> cloneBufferRange(
    std::shared_ptr<const BufferBase> buffer, ivec2 range);

IVW_MODULE_DATAFRAME_API void copyBufferRange(std::shared_ptr<const BufferBase> src,
                                              std::shared_ptr<BufferBase> dst, ivec2 range,
                                              size_t dstStart = 0);

/**
 * \brief create a new DataFrame by appending the columns of DataFrame \p right to DataFrame \p left
 *
 * @param left
 * @param right
 * @param ignoreDuplicates duplicate columns, i.e. same column header, are ignored if true
 * @param fillMissingRows  if true, missing rows in either DataFrame are filled with 0 or
 *                         "undefined" (for categorical columns)
 * @return joined DataFrame with columns from \p left and \p right DataFrame
 * @throws Exception if number of rows is different and \p fillMissingRows is false
 */
IVW_MODULE_DATAFRAME_API std::shared_ptr<DataFrame> appendColumns(const DataFrame& left,
                                                                  const DataFrame& right,
                                                                  bool ignoreDuplicates = false,
                                                                  bool fillMissingRows = false);

/**
 * \brief create a new DataFrame by appending the rows of DataFrame \p bottom to DataFrame \p top
 * @param top
 * @param bottom
 * @param matchByName    if true, column headers are used for matching columns. Otherwise columns
 *                       are matched by order (default)
 * @return joined DataFrame with columns from \p left and \p right DataFrame
 * @throws Exception if number of columns is different or a column cannot be found (matchByName)
 */
IVW_MODULE_DATAFRAME_API std::shared_ptr<DataFrame> appendRows(const DataFrame& top,
                                                               const DataFrame& bottom,
                                                               bool matchByName = false);

/**
 * \brief create a new DataFrame by using an inner join of DataFrame \p left and DataFrame \p right.
 * That is only rows with matching keys are kept. The row indices of \p left will be reused.
 *
 * It is assumed that the entries in the key columns are unique. Otherwise results are undefined.
 * @param left
 * @param right
 * @param keyColumn   header of the column used as key for the join operation (default: index
 * column)
 * @return inner join of \p left and \p right DataFrame
 * @throws Exception if keyColumn does not exist in either \p left or \p right
 */
IVW_MODULE_DATAFRAME_API std::shared_ptr<DataFrame> innerJoin(
    const DataFrame& left, const DataFrame& right, const std::string& keyColumn = "index");

/**
 * \brief create a new DataFrame by using an inner join of DataFrame \p left and DataFrame \p right.
 * That is only rows with matching keys are kept. The row indices of \p left will be reused.
 *
 * It is assumed that the entries in the key columns are unique. Otherwise results are undefined.
 * @param left
 * @param right
 * @param keyColumns   headers of the columns used as keys for the join operation
 * @return inner join of \p left and \p right DataFrame
 * @throws Exception if keyColumn does not exist in either \p left or \p right
 */
IVW_MODULE_DATAFRAME_API std::shared_ptr<DataFrame> innerJoin(
    const DataFrame& left, const DataFrame& right, const std::vector<std::string>& keyColumns);



/**
 * \brief create a new DataFrame by using an outer left join of DataFrame \p left and DataFrame \p
 * right. That is all rows of \p left are augmented with matching rows from \p right.  The row
 * indices of \p left will be reused.
 *
 * It is assumed that the entries in the key columns of \p right are unique. Otherwise results are
 * undefined.
 *
 * @param left
 * @param right
 * @param keyColumn   header of the column used as key for the join operation (default: index
 * column)
 * @return left join of \p left and \p right DataFrame
 * @throws Exception if keyColumn does not exist in either \p left or \p right
 */
IVW_MODULE_DATAFRAME_API std::shared_ptr<DataFrame> leftJoin(
    const DataFrame& left, const DataFrame& right, const std::string& keyColumn = "index");

/**
 * \brief create a new DataFrame by using an outer left join of DataFrame \p left and DataFrame \p
 * right. That is all rows of \p left are augmented with matching rows from \p right.  The row
 * indices of \p left will be reused.
 *
 * It is assumed that the entries in the key columns of \p right are unique. Otherwise results are
 * undefined.
 *
 * @param left
 * @param right
 * @param keyColumns   headers of the columns used as keys for the join operation
 * @return left join of \p left and \p right DataFrame
 * @throws Exception if keyColumn does not exist in either \p left or \p right
 */
IVW_MODULE_DATAFRAME_API std::shared_ptr<DataFrame> leftJoin(
    const DataFrame& left, const DataFrame& right, const std::vector<std::string>& keyColumns);

IVW_MODULE_DATAFRAME_API std::shared_ptr<DataFrame> combineDataFrames(
    std::vector<std::shared_ptr<DataFrame>> dataframes, bool skipIndexColumn = false,
    std::string skipcol = "index");

/**
 * \brief apply predicate \p pred to each value of column \p col and return the row indices where
 * the predicate evaluates to true.
 *
 * Note: the predicate function needs to take care of the different column datatypes
 * \code{.cpp}
 * auto pred = [](const auto& arg) {
 *                 if  constexpr(std::is_same_v<decltype(arg), float>) {
 *                     return  arg == 4.5f
 *                 } else {
 *                     return true;
 *                 }};
 * \endcode
 *
 * Alternatively, predicate overloads can be used.
 * \code{.cpp}
 * #include <inviwo/core/util/stdextensions.h>
 *
 * auto pred = util::overloaded{[](const std::string& arg) { return arg == "bla"; },
 *                              [](const int& arg) { return arg == 5;  },
 *                              [](const auto&) { return true; });
 * \endcode
 *
 * @param col   column containing data for filtering
 * @param pred  predicate to check values from \p col
 * @return list of row indices where rows fulfill the predicate
 * @see selectRows(const DataFrame&, dataframefilters::Filters)
 */
template <typename Pred>
std::vector<std::uint32_t> selectRows(std::shared_ptr<const Column> col, Pred pred);

/**
 * \brief apply the \p filters to each row of column \p col and return the row indices where
 * any of the filters evaluates to true.
 *
 * @param col     column containing data for filtering
 * @param filters predicate to check values from \p col
 * @return list of row indices where rows satisfy all \p filters
 * @see selectRows(const DataFrame&, dataframefilters::Filters)
 */
IVW_MODULE_DATAFRAME_API std::vector<std::uint32_t> selectRows(
    const Column& col, const std::vector<dataframefilters::ItemFilter>& filters);

/**
 * \brief apply the \p filters to each row of \p dataframe and return the row indices where
 * any of the include filters and no exclude filter evaluates to true.
 *
 * @param dataframe   column containing data for filtering
 * @param filters     predicate to check values from \p col
 * @return list of row indices where rows satisfy all \p filters
 * @see selectRows(const Column&, dataframefilters::Filters)
 */
IVW_MODULE_DATAFRAME_API std::vector<std::uint32_t> selectRows(const DataFrame& dataframe,
                                                               dataframefilters::Filters filters);

IVW_MODULE_DATAFRAME_API std::string createToolTipForRow(const DataFrame& dataframe, size_t rowId);

#include <warn/push>
#include <warn/ignore/conversion>
template <typename Pred>
std::vector<std::uint32_t> selectRows(std::shared_ptr<const Column> col, Pred pred) {
    if (auto catCol = dynamic_cast<const CategoricalColumn*>(col.get())) {
        std::vector<std::uint32_t> rows;
        for (auto&& [row, v] : util::enumerate<std::uint32_t>(catCol->values())) {
            if (pred(v)) {
                rows.push_back(row);
            }
        }
        return rows;
    } else {
        return col->getBuffer()
            ->getRepresentation<BufferRAM>()
            ->dispatch<std::vector<std::uint32_t>>([pred](auto typedBuf) {
                std::vector<std::uint32_t> rows;
                for (auto&& [row, value] :
                     util::enumerate<std::uint32_t>(typedBuf->getDataContainer())) {
                    // find all rows fulfilling the predicate
                    if (pred(value)) {
                        rows.push_back(row);
                    }
                }
                return rows;
            });
    }
}
#include <warn/pop>

}  // namespace dataframe

}  // namespace inviwo
