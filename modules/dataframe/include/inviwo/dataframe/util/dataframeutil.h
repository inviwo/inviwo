/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2020 Inviwo Foundation
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

#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/datastructures/buffer/buffer.h>
#include <inviwo/core/io/serialization/serializable.h>
#include <inviwo/dataframe/datastructures/dataframe.h>

namespace inviwo {

namespace dataframe {

std::shared_ptr<BufferBase> IVW_MODULE_DATAFRAME_API
cloneBufferRange(std::shared_ptr<const BufferBase> buffer, ivec2 range);

void IVW_MODULE_DATAFRAME_API copyBufferRange(std::shared_ptr<const BufferBase> src,
                                              std::shared_ptr<BufferBase> dst, ivec2 range,
                                              size_t dstStart = 0);

/**
 * \brief create a new DataFrame by appending the columns of DataFrame \p right to DataFrame \p left
 *
 * @param ignoreDuplicates duplicate columns, i.e. same column header, are ignored if true
 * @param fillMissingRows  if true, missing rows in either DataFrame are filled with 0 or
 *                         "undefined" (for categorical columns)
 * @return joined DataFrame with columns from \p left and \p right DataFrame
 * @throws Exception if number of rows is different and \p fillMissingRows is false
 */
std::shared_ptr<DataFrame> IVW_MODULE_DATAFRAME_API appendColumns(const DataFrame& left,
                                                                  const DataFrame& right,
                                                                  bool ignoreDuplicates = false,
                                                                  bool fillMissingRows = false);

/**
 * \brief create a new DataFrame by appending the rows of DataFrame \p bottom to DataFrame \p top
 *
 * @param matchByName    if true, column headers are used for matching columns. Otherwise columns
 *                       are matched by order (default)
 * @return joined DataFrame with columns from \p left and \p right DataFrame
 * @throws Exception if number of columns is different or a column cannot be found (matchByName)
 */
std::shared_ptr<DataFrame> IVW_MODULE_DATAFRAME_API appendRows(const DataFrame& top,
                                                               const DataFrame& bottom,
                                                               bool matchByName = false);

/**
 * \brief create a new DataFrame by using an inner join of DataFrame \p left and DataFrame \p right.
 * That is only rows with matching keys are kept.
 *
 * It is assumed that the entries in the key columns are unique. Otherwise results are undefined.
 *
 * @param keyColumn   header of the column used as key for the join operation (default: index
 * column)
 * @return inner join of \p left and \p right DataFrame
 * @throws Exception if keyColumn does not exist in either \p left or \p right
 */
std::shared_ptr<DataFrame> IVW_MODULE_DATAFRAME_API
innerJoin(const DataFrame& left, const DataFrame& right, const std::string& keyColumn = "index");
std::shared_ptr<DataFrame> IVW_MODULE_DATAFRAME_API innerJoin(
    const DataFrame& left, const DataFrame& right, const std::vector<std::string>& keyColumns);

/**
 * \brief create a new DataFrame by using an outer left join of DataFrame \p left and DataFrame \p
 * right. That is all rows of \p left are augmented with matching rows from \p right.
 *
 * It is assumed that the entries in the key columns of \p right are unique. Otherwise results are
 * undefined.
 *
 * @param keyColumn   header of the column used as key for the join operation (default: index
 * column)
 * @return left join of \p left and \p right DataFrame
 * @throws Exception if keyColumn does not exist in either \p left or \p right
 */
std::shared_ptr<DataFrame> IVW_MODULE_DATAFRAME_API
leftJoin(const DataFrame& left, const DataFrame& right, const std::string& keyColumn = "index");
std::shared_ptr<DataFrame> IVW_MODULE_DATAFRAME_API
leftJoin(const DataFrame& left, const DataFrame& right, const std::vector<std::string>& keyColumns);

std::shared_ptr<DataFrame> IVW_MODULE_DATAFRAME_API
combineDataFrames(std::vector<std::shared_ptr<DataFrame>> dataframes, bool skipIndexColumn = false,
                  std::string skipcol = "index");

std::string IVW_MODULE_DATAFRAME_API createToolTipForRow(const DataFrame& dataframe, size_t rowId);

}  // namespace dataframe

}  // namespace inviwo
