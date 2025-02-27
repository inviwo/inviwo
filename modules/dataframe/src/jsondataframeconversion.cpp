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
#include <inviwo/dataframe/jsondataframeconversion.h>

#include <inviwo/core/datastructures/buffer/buffer.h>                   // for Buffer
#include <inviwo/core/datastructures/buffer/bufferram.h>                // for BufferRAM
#include <inviwo/core/datastructures/representationconverter.h>         // for RepresentationCon...
#include <inviwo/core/datastructures/representationconverterfactory.h>  // for RepresentationCon...
#include <inviwo/core/io/datareaderexception.h>                         // for DataReaderException
#include <inviwo/core/util/exception.h>                                 // for SourceContext
#include <inviwo/core/util/formatdispatching.h>                         // for PrecisionValueType
#include <inviwo/core/util/formats.h>                                   // for DataFormatBase
#include <inviwo/core/util/stringconversion.h>                          // for toString
#include <inviwo/core/util/zip.h>
#include <inviwo/dataframe/datastructures/column.h>     // for TemplateColumn
#include <inviwo/dataframe/datastructures/dataframe.h>  // for DataFrame

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
#include <limits>

#include <glm/gtc/type_ptr.hpp>  // for value_ptr
#include <glm/gtx/io.hpp>        // for operator<<

namespace inviwo {

namespace {

constexpr std::string_view categoricalTypeStr = "CATEGORICAL";

// Helper for adding columns based on json item type
void addDataFrameColumnHelper(json::value_t valueType, std::string_view header, DataFrame& df) {
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

void extractColumnsFromRow(const json& j, DataFrame& df) {
    auto checkEntireColumnForType = [&](size_t columnIndex, std::string_view columnHeader) {
        bool found = false;
        for (const auto& row : j["data"]) {
            const auto& item = row[columnIndex];
            if (item.type() != json::value_t::null) {
                try {
                    addDataFrameColumnHelper(item.type(), columnHeader, df);
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
            addDataFrameColumnHelper(json::value_t::number_float, columnHeader, df);
        }
    };

    const auto& firstRow = j["data"].at(0);
    for (auto&& [index, col] : util::enumerate(firstRow)) {
        const auto columnHeader = j["columns"][index].get<std::string_view>();

        switch (col.type()) {
            case json::value_t::null:  ///< null value
                // need to check more rows to know type
                checkEntireColumnForType(index, columnHeader);
                break;
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
                addDataFrameColumnHelper(col.type(), columnHeader, df);
                break;
            case json::value_t::discarded:  ///< discarded by the the parser callback function
                throw JSONConversionException(
                    "Value was discarded by the parser callback function");
                break;
            case json::value_t::binary:
                // Not supported
                throw JSONConversionException("Binary elements are unsupported");
                break;
        }
    }
}

void extractRows(const json& data, DataFrame& df) {
    for (auto colIndex : std::ranges::iota_view(size_t{1}, df.getNumberOfColumns())) {
        auto column = df.getColumn(colIndex);
        if (column->getColumnType() == ColumnType::Categorical) {
            auto* categorical = static_cast<CategoricalColumn*>(column.get());
            auto addValue = categorical->addMany();
            for (auto& row : data) {
                addValue(row[colIndex - 1].get<std::string_view>());
            }
        } else {
            column->getBuffer()
                ->getEditableRepresentation<BufferRAM>()
                ->dispatch<void, dispatching::filter::Scalars>([&](auto buffer) {
                    using T = util::PrecisionValueType<decltype(buffer)>;
                    auto transform = std::views::transform([colIndex](auto& row) {
                        if (row[colIndex - 1].is_null()) {
                            return std::numeric_limits<T>::quiet_NaN();
                        } else {
                            return row[colIndex - 1].template get<T>();
                        }
                    });
                    buffer->getDataContainer() = data | transform | std::ranges::to<std::vector>();
                });
        }
    }
}

auto generateColumnPrinters(const DataFrame& df) {
    std::vector<std::function<void(json&, size_t)>> printers;
    for (const auto& col : df) {
        if (col->getColumnType() == ColumnType::Index) continue;

        auto format = col->getBuffer()->getDataFormat();
        if (col->getColumnType() == ColumnType::Categorical) {
            printers.emplace_back(
                [cc = static_cast<const CategoricalColumn*>(col.get()), header = col->getHeader()](
                    json& list, size_t index) { list.emplace_back(cc->getAsString(index)); });
        } else if (format->getComponents() == 1) {
            col->getBuffer()
                ->getRepresentation<BufferRAM>()
                ->dispatch<void, dispatching::filter::Scalars>(
                    [&printers, header = col->getHeader()](auto br) {
                        using ValueType = util::PrecisionValueType<decltype(br)>;
                        if constexpr (std::is_floating_point_v<ValueType>) {
                            // treat NaN in floating point values as "empty" (null)
                            printers.push_back([br, header](json& list, size_t index) {
                                const auto val = br->getDataContainer()[index];
                                if (std::isnan(val)) {
                                    list.emplace_back(json());
                                } else {
                                    list.emplace_back(val);
                                }
                            });
                        } else {
                            printers.push_back([br, header](json& list, size_t index) {
                                list.emplace_back(br->getDataContainer()[index]);
                            });
                        }
                    });
        } else {
            col->getBuffer()
                ->getRepresentation<BufferRAM>()
                ->dispatch<void, dispatching::filter::Vecs>(
                    [&printers, header = col->getHeader()](auto br) {
                        printers.push_back([br, header](json& list, size_t index) {
                            list.emplace_back(toString(br->getDataContainer()[index]));
                        });
                    });
        }
    }
    return printers;
}

std::shared_ptr<Column> createColumn(std::string_view type, std::string_view header) {
    return dispatching::singleDispatch<std::shared_ptr<Column>, dispatching::filter::Scalars>(
        DataFormatBase::get(type)->getId(),
        [header]<typename T>() { return std::make_shared<TemplateColumn<T>>(header, 0u); });
}

void createColumnsFromTypes(const json& j, DataFrame& df) {
    for (auto&& [index, element] : util::enumerate(j["types"])) {
        if (!element.is_string()) {
            throw JSONConversionException(R"(expected data type 'string' in "types")");
        }

        auto columnHeader = j["columns"][index].get<std::string_view>();
        auto type = element.get<std::string_view>();
        if (type == categoricalTypeStr) {
            df.addCategoricalColumn(columnHeader, 0u);
        } else {
            df.addColumn(createColumn(type, columnHeader));
        }
    }
}

}  // namespace

void to_json(json& j, const DataFrame& df) {
    json columns = json::array();
    for (const auto& col : df) {
        if (col->getColumnType() == ColumnType::Index) continue;
        columns.emplace_back(col->getHeader());
    }
    json columnTypes = json::array();
    for (const auto& col : df) {
        if (col->getColumnType() == ColumnType::Index) continue;

        if (col->getColumnType() == ColumnType::Categorical) {
            columnTypes.emplace_back(categoricalTypeStr);
        } else {
            columnTypes.emplace_back(col->getBuffer()->getDataFormat()->getString());
        }
    }
    json data = json::array();
    auto printers = generateColumnPrinters(df);
    for (size_t row = 0; row < df.getNumberOfRows(); ++row) {
        json list = json::array();
        for (const auto& printer : printers) {
            printer(list, row);
        }
        data.emplace_back(list);
    }

    j["columns"] = columns;
    j["types"] = columnTypes;
    j["index"] = df.getIndexColumn()->getTypedBuffer()->getRAMRepresentation()->getDataContainer();
    j["data"] = data;
}

void from_json(const json& j, DataFrame& df) {
    if (j.empty() || !j.is_object()) {
        // Only support object types, i.e. {key: value}
        return;
    }

    if (!j.contains("columns") || !j["columns"].is_array()) {
        throw JSONConversionException(R"("JSON object must contain a "columns" array)");
    }
    if (!j.contains("data") || !j["data"].is_array()) {
        throw JSONConversionException(R"(JSON object must contain "data" array)");
    }
    if (j.contains("index")) {
        if (!j["index"].is_array()) {
            throw JSONConversionException(R"("index" must be an array)");
        }
        if (j["index"].size() != j["data"].size()) {
            throw JSONConversionException(SourceContext{},
                                          "number of indices ({}) differs from number of rows ({})",
                                          j["index"].size(), j["data"].size());
        }
    }
    if (j.contains("types")) {
        if (!j["types"].is_array()) {
            throw JSONConversionException(R"("types" must be an array)");
        }
        if (j["types"].size() != j["columns"].size()) {
            throw JSONConversionException(
                SourceContext{}, "number of types ({}) differs from number of columns ({})",
                j["types"].size(), j["columns"].size());
        }
    }

    if (j.contains("index")) {
        auto& indices = df.getIndexColumn()
                            ->getTypedBuffer()
                            ->getEditableRAMRepresentation()
                            ->getDataContainer();
        indices = j["index"].get<std::vector<IndexColumn::type>>();
    }

    if (j.contains("types")) {
        createColumnsFromTypes(j, df);
    } else {
        extractColumnsFromRow(j, df);
    }

    extractRows(j["data"], df);
    df.updateIndexBuffer();
}

void to_json(json& j, const DataFrameInport& port) {
    if (auto data = port.getData()) {
        j = *data;
    } else {
        j.clear();
    }
}
void from_json(const json&, DataFrameInport&) {
    throw Exception("It is not possible to assign a json object to an Inport");
}

}  // namespace inviwo
