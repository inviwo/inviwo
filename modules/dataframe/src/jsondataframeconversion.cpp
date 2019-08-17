/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019 Inviwo Foundation
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
#include <inviwo/dataframe/jsondataframeconversion.h>
#include <inviwo/core/datastructures/buffer/bufferramprecision.h>
#include <iterator>
#include <vector>

namespace inviwo {

JSONConversionException::JSONConversionException(const std::string& message,
                                                 ExceptionContext context)
    : DataReaderException("JSONConversion: " + message, context) {}

namespace detail {

// Helper for adding columns based on json item type
void addDataFrameColumnHelper(json::value_t valueType, std::string header, DataFrame& df) {
    switch (valueType) {
        case json::value_t::null:  ///< null value
            // need to check more rows to know type
            break;
        case json::value_t::object:  ///< object (unordered set of name/value pairs)
            // Not supported
            throw JSONConversionException("Object (ordered collection of values) is unsupported");
            break;
        case json::value_t::array:  ///< array (ordered collection of values)
            // Not supported
            throw JSONConversionException("Array (ordered collection of values) is unsupported");
            break;
        case json::value_t::string:  ///< string value
            df.addCategoricalColumn(header, 0u);
            break;
        case json::value_t::boolean:  ///< boolean value
            // We do not support buffers<bool> (std::vector<bool>)  since they are packed bit
            // arrays. Use unsigned char instead.
            // df.addColumn<bool>(header, 0u);
            df.addColumn<uint8_t>(header, 0u);
            break;
        case json::value_t::number_integer:  ///< number value (signed integer)
            df.addColumn<int32_t>(header, 0u);
            break;
        case json::value_t::number_unsigned:  ///< number value (unsigned integer)
            df.addColumn<uint32_t>(header, 0u);
            break;
        case json::value_t::number_float:  ///< number value (floating-point)
            df.addColumn<float>(header, 0u);
            break;
        case json::value_t::discarded:  ///< discarded by the the parser callback function
            throw JSONConversionException(
                "Value was discarded by the the parser callback function");
            break;
    }
}

}  // namespace detail

void to_json(json& j, const DataFrame& df) {
    for (size_t row = 0; row < df.getNumberOfRows(); ++row) {
        json node = json::object();
        auto items = df.getDataItem(row, true);
        // Row 0 in the dataframe contains the row indices, which is not needed in the json object.
        int i = 1;
        for (auto col = ++items.begin(); col != items.end(); ++col) {
            node[df.getHeader(i++)] = (*col)->toString();
        }
        j.emplace_back(node);
    }
}

void from_json(const json& j, DataFrame& df) {
    // Extract header and column types
    if (j.empty() || !j.front().is_object()) {
        // Only support object types, i.e. [ {key: value} ]
        return;
    }
    // Extract header names from first object
    const auto& firstRow = j.at(0);
    for (const auto& col : firstRow.items()) {
        switch (col.value().type()) {
            case json::value_t::null: {  ///< null value
                // need to check more rows to know type
                for (const auto& row : j) {
                    auto item = row.find(col.key());
                    if (item->type() != json::value_t::null) {
                        try {
                            detail::addDataFrameColumnHelper(item.value().type(), col.key(), df);
                        } catch (JSONConversionException& e) {
                            throw e;
                        }
                        // Stop searching when we found a non-null item
                        break;
                    }
                }
                break;
            }
            case json::value_t::object:  ///< object (unordered set of name/value pairs)
                // Not supported
                throw JSONConversionException(
                    "Object (unordered set of name/value pairs) is unsupported");
                break;
            case json::value_t::array:  ///< array (ordered collection of values)
                // Not supported
                throw JSONConversionException(
                    "Array (ordered collection of values) is unsupported");
                break;
            case json::value_t::string:           ///< string value
            case json::value_t::boolean:          ///< boolean value
            case json::value_t::number_integer:   ///< number value (signed integer)
            case json::value_t::number_unsigned:  ///< number value (unsigned integer)
            case json::value_t::number_float:     ///< number value (floating-point)
                detail::addDataFrameColumnHelper(col.value().type(), col.key(), df);
                break;
            case json::value_t::discarded:  ///< discarded by the the parser callback function
                throw JSONConversionException(
                    "Value was discarded by the the parser callback function");
                break;
        }
    }
    // Extract values of each column
    for (const auto& row : j) {
        auto colIdx = 1u;  // 0 column is index column
        for (const auto& col : row.items()) {
            if (col.value().type() == json::value_t::object ||
                col.value().type() == json::value_t::array ||
                col.value().type() == json::value_t::discarded) {
                // Skip unsupported types
                continue;
            }
            std::stringstream ss;
            ss << col.value();
            df.getColumn(colIdx++)->add(ss.str());
        }
    }
    // Update index buffer when we are done
    df.updateIndexBuffer();
}

}  // namespace inviwo
