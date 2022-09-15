/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019-2022 Inviwo Foundation
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

#include <inviwo/dataframe/dataframemoduledefine.h>     // for IVW_MODULE_DATAFRAME_API

#include <inviwo/core/io/datareaderexception.h>         // for DataReaderException
#include <inviwo/core/util/exception.h>                 // for ExceptionContext
#include <inviwo/dataframe/datastructures/dataframe.h>  // for DataFrame

#include <string>                                       // for string

#include <nlohmann/json.hpp>                            // for json

using json = nlohmann::json;

namespace inviwo {

/**
 * \class JSONConversionException
 *
 * \brief This exception is thrown by the to_json(json& j, const DataFrame* df) in case the input is
 * unsupported. This includes empty sources, unmatched quotes, missing headers. \see
 * JSONDataFrameReader
 */
class IVW_MODULE_DATAFRAME_API JSONConversionException : public DataReaderException {
public:
    JSONConversionException(const std::string& message = "",
                            ExceptionContext context = ExceptionContext());
};

/**
 * Converts a DataFrame to a JSON object. This will not include the index column. NaN floating point
 * values are converted to null.
 * Will write the DataFrame to an JSON object layout:
 * \code{.json}
 * [ {"Col1": * val11, "Col2": val12 },
 *   {"Col1": val21, "Col2": val22 } ]
 * \endcode
 * The example above contains two rows and two columns.
 *
 * Usage example:
 * \code{.cpp}
 * Dataframe df;
 * json j = df;
 * \endcode
 */
IVW_MODULE_DATAFRAME_API void to_json(json& j, const DataFrame& df);

/**
 * Converts a JSON object to a DataFrame. Column types will be derived from json types. In case a
 * column only holds null values, the inferred data type will be float.
 * Expects object layout:
 * \code{.json}
 * [ {"Col1": val11, "Col2": val12 },
 *   {"Col1": val21, "Col2": val22 } ]
 * \endcode
 * The example above contains two rows and two columns.
 *
 * Usage example:
 * \code{.cpp}
 * auto df = j.get<DataFrame>();
 * \endcode
 */
IVW_MODULE_DATAFRAME_API void from_json(const json& j, DataFrame& df);

}  // namespace inviwo
