/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019-2023 Inviwo Foundation
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

#include <inviwo/core/datastructures/buffer/buffer.h>                   // for Buffer
#include <inviwo/core/datastructures/buffer/bufferram.h>                // for BufferRAM
#include <inviwo/core/datastructures/representationconverter.h>         // for RepresentationCon...
#include <inviwo/core/datastructures/representationconverterfactory.h>  // for RepresentationCon...
#include <inviwo/core/io/datareaderexception.h>                         // for DataReaderException
#include <inviwo/core/util/exception.h>                                 // for ExceptionContext
#include <inviwo/core/util/formatdispatching.h>                         // for PrecisionValueType
#include <inviwo/core/util/formats.h>                                   // for DataFormatBase
#include <inviwo/core/util/stringconversion.h>                          // for toString
#include <inviwo/dataframe/datastructures/column.h>                     // for TemplateColumn
#include <inviwo/dataframe/datastructures/dataframe.h>                  // for DataFrame

#include <cmath>          // for isnan
#include <cstddef>        // for size_t
#include <cstdint>        // for int32_t, uint32_t
#include <functional>     // for __base, function
#include <memory>         // for shared_ptr, uniqu...
#include <optional>       // for optional
#include <sstream>        // for basic_stringbuf<>...
#include <type_traits>    // for remove_extent_t
#include <unordered_map>  // for unordered_map
#include <unordered_set>  // for unordered_set
#include <vector>         // for vector

#include <glm/gtc/type_ptr.hpp>  // for value_ptr
#include <glm/gtx/io.hpp>        // for operator<<
#include <half/half.hpp>         // for operator<<
#include <nlohmann/json.hpp>     // for iteration_proxy_v...

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
        case json::value_t::binary:
            // Not supported
            throw JSONConversionException("Binary elements is unsupported");
            break;
    }
}

}  // namespace detail

void to_json(json& j, const DataFrame& df) {
    std::vector<std::function<void(json & j, size_t)>> printers;
    for (const auto& col : df) {
        if (col->getColumnType() == ColumnType::Index) continue;

        auto format = col->getBuffer()->getDataFormat();
        if (col->getColumnType() == ColumnType::Categorical) {
            printers.push_back(
                [cc = static_cast<const CategoricalColumn*>(col.get()), header = col->getHeader()](
                    json& node, size_t index) { node[header] = cc->getAsString(index); });
        } else if (format->getComponents() == 1) {
            col->getBuffer()
                ->getRepresentation<BufferRAM>()
                ->dispatch<void, dispatching::filter::Scalars>(
                    [&printers, header = col->getHeader()](auto br) {
                        using ValueType = util::PrecisionValueType<decltype(br)>;
                        if constexpr (std::is_floating_point_v<ValueType>) {
                            // treat NaN in floating point values as "empty" (null)
                            printers.push_back([br, header](json& node, size_t index) {
                                const auto val = br->getDataContainer()[index];
                                if (std::isnan(val)) {
                                    node[header] = json();
                                } else {
                                    node[header] = val;
                                }
                            });
                        } else if constexpr (std::is_same_v<ValueType, half_float::half>) {
                            printers.push_back([br, header](json& node, size_t index) {
                                const auto val = br->getDataContainer()[index];
                                if (std::isnan(val)) {
                                    node[header] = json();
                                } else {
                                    node[header] = static_cast<float>(val);
                                }
                            });
                        } else {
                            printers.push_back([br, header](json& node, size_t index) {
                                node[header] = br->getDataContainer()[index];
                            });
                        }
                    });
        } else {
            col->getBuffer()
                ->getRepresentation<BufferRAM>()
                ->dispatch<void, dispatching::filter::Vecs>(
                    [&printers, header = col->getHeader()](auto br) {
                        printers.push_back([br, header](json& node, size_t index) {
                            node[header] = toString(br->getDataContainer()[index]);
                        });
                    });
        }
    }

    for (size_t row = 0; row < df.getNumberOfRows(); ++row) {
        json node = json::object();
        for (auto& printer : printers) {
            printer(node, row);
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
                bool found = false;
                for (const auto& row : j) {
                    auto item = row.find(col.key());
                    if (item->type() != json::value_t::null) {
                        try {
                            detail::addDataFrameColumnHelper(item.value().type(), col.key(), df);
                            found = true;
                        } catch (JSONConversionException& e) {
                            throw e;
                        }
                        // Stop searching when we found a non-null item
                        break;
                    }
                }
                if (!found) {
                    // entire column is empty, assume float
                    detail::addDataFrameColumnHelper(json::value_t::number_float, col.key(), df);
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
            case json::value_t::binary:
                // Not supported
                throw JSONConversionException("Binary elements is unsupported");
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
