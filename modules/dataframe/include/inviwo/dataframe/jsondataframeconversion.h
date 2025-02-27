/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019-2025 Inviwo Foundation
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

#include <inviwo/core/io/datareaderexception.h>         // for DataReaderException
#include <inviwo/core/util/exception.h>                 // for SourceContext
#include <inviwo/dataframe/datastructures/dataframe.h>  // for DataFrame
#include <modules/json/json.h>

#include <string>  // for string

namespace inviwo {

/**
 * \class JSONConversionException
 *
 * \brief This exception is thrown by from_json(json& j, const DataFrame* df) in case the input is
 * illformated or unsupported. This includes empty sources, unmatched quotes, missing headers.
 * \see JSONDataFrameReader
 */
class IVW_MODULE_DATAFRAME_API JSONConversionException : public DataReaderException {
public:
    using DataReaderException::DataReaderException;
};

/**
 * Converts a DataFrame to a JSON object. NaN floating point values are converted to null.
 * The output is based on Pandas pandas.DataFrame.to_json(order='split') and has the following JSON
 * object layout:
 * \code{.json}
 * {
 *     "columns": [ "col1", "col2" ],
 *     "data": [
 *         [ 5.1, "category A" ],
 *         [4.9, "category B" ]
 *     ],
 *     "index": [ 0, 1 ],
 *     "types": [ "FLOAT64", "CATEGORICAL" ]
 * }
 * \endcode
 * The example above contains two rows and two columns, one with double values and one categorical.
 *
 * Usage example:
 * \code{.cpp}
 * Dataframe df;
 * json j = df;
 * \endcode
 */
IVW_MODULE_DATAFRAME_API void to_json(json& j, const DataFrame& df);

/**
 * Converts a JSON object to a DataFrame. The JSON object must contain the following elements:
 * "columns" (list of column headers) and "data" (list of rows). The elements "index" and "types"
 * are optional. If no types are provided, the column types will be derived from json types. In case
 * a column only holds null values, the inferred data type will be float. Null values are converted
 * to NaN in float columns and 0 in integer columns.
 *
 * Usage example:
 * \code{.cpp}
 * auto df = j.get<DataFrame>();
 * \endcode
 *
 * \see to_json(json&, const DataFrame&)
 */
IVW_MODULE_DATAFRAME_API void from_json(const json& j, DataFrame& df);

IVW_MODULE_DATAFRAME_API void to_json(json& j, const DataFrameInport& port);
IVW_MODULE_DATAFRAME_API void from_json(const json& j, DataFrameInport& port);

}  // namespace inviwo
