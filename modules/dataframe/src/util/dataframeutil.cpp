/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2024 Inviwo Foundation
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

#include <inviwo/dataframe/util/dataframeutil.h>

#include <inviwo/core/datastructures/bitset.h>                          // for BitSet
#include <inviwo/core/datastructures/buffer/buffer.h>                   // for BufferBase, Buffer
#include <inviwo/core/datastructures/buffer/bufferram.h>                // for BufferRAM
#include <inviwo/core/datastructures/buffer/bufferramprecision.h>       // for BufferRAMPrecision
#include <inviwo/core/datastructures/representationconverter.h>         // for RepresentationCon...
#include <inviwo/core/datastructures/representationconverterfactory.h>  // for RepresentationCon...
#include <inviwo/core/util/assertion.h>                                 // for IVW_ASSERT
#include <inviwo/core/util/document.h>                                  // for Document, TableBu...
#include <inviwo/core/util/exception.h>                                 // for Exception
#include <inviwo/core/util/formatdispatching.h>                         // for PrecisionValueType
#include <inviwo/core/util/formats.h>                                   // for DataFormatBase
#include <inviwo/core/util/glmvec.h>                                    // for ivec2
#include <inviwo/core/util/iterrange.h>                                 // for iter_range, as_range
#include <inviwo/core/util/sourcecontext.h>                             // for IVW_CONTEXT_CUSTOM
#include <inviwo/core/util/stdextensions.h>                             // for transform, contains
#include <inviwo/core/util/stringconversion.h>                          // for toLower
#include <inviwo/core/util/transformiterator.h>                         // for TransformIterator
#include <inviwo/core/util/zip.h>                                       // for zipper, enumerate
#include <inviwo/dataframe/datastructures/column.h>                     // for CategoricalColumn
#include <inviwo/dataframe/datastructures/dataframe.h>                  // for DataFrame
#include <inviwo/dataframe/util/filters.h>                              // for ItemFilter, Filters

#include <algorithm>      // for any_of
#include <functional>     // for function
#include <iterator>       // for distance
#include <map>            // for operator==, map
#include <optional>       // for optional
#include <string_view>    // for string_view, oper...
#include <unordered_map>  // for operator==, unord...
#include <utility>        // for move, pair
#include <variant>        // for visit

#include <fmt/core.h>        // for format, basic_str...
#include <glm/gtx/hash.hpp>  // for hash<>::operator()
#include <glm/vec2.hpp>      // for operator==, opera...
#include <glm/vec3.hpp>      // for operator==, opera...
#include <glm/vec4.hpp>      // for operator==, opera...

namespace inviwo {

namespace dataframe {

std::shared_ptr<BufferBase> cloneBufferRange(std::shared_ptr<const BufferBase> buffer,
                                             ivec2 range) {
    if (range.y - range.x <= 0) {
        return nullptr;
    }

    return buffer->getRepresentation<BufferRAM>()->dispatch<std::shared_ptr<BufferBase>>(
        [&](auto typed) {
            using ValueType = util::PrecisionValueType<decltype(typed)>;
            auto newBuffer = std::make_shared<Buffer<ValueType>>();
            auto& vecOut = newBuffer->getEditableRAMRepresentation()->getDataContainer();
            auto& vecIn = typed->getDataContainer();

            vecOut.insert(vecOut.begin(), vecIn.begin() + range.x, vecIn.begin() + range.y + 1);
            return newBuffer;
        });
}

void copyBufferRange(std::shared_ptr<const BufferBase> src, std::shared_ptr<BufferBase> dst,
                     ivec2 range, size_t dstStart) {
    if (range.y - range.x <= 0) {
        return;
    }

    if (src->getDataFormat()->getId() == dst->getDataFormat()->getId()) {
        dst->getEditableRepresentation<BufferRAM>()->dispatch<void>([&](auto typed) {
            using ValueType = util::PrecisionValueType<decltype(typed)>;
            auto typedInBuf = static_cast<const Buffer<ValueType>*>(src.get());
            auto& vecIn = typedInBuf->getRAMRepresentation()->getDataContainer();
            auto& vecOut = typed->getDataContainer();

            vecOut.insert(vecOut.begin() + dstStart, vecIn.begin() + range.x,
                          vecIn.begin() + range.y + 1);
        });
    } else {
        throw Exception("Data Formats needs to be of same type",
                        IVW_CONTEXT_CUSTOM("dataframe::copyBufferRange"));
    }
}

std::shared_ptr<DataFrame> appendColumns(const DataFrame& left, const DataFrame& right,
                                         bool ignoreDuplicates, bool fillMissingRows) {
    if (!fillMissingRows && (left.getNumberOfRows() != right.getNumberOfRows())) {
        throw Exception(IVW_CONTEXT_CUSTOM("dataframe::appendColumns"),
                        "Row counts are different (top: {}, bottom: {})", left.getNumberOfRows(),
                        right.getNumberOfRows());
    }

    auto dataframe = std::make_shared<DataFrame>(left);

    for (auto srcCol : right) {
        if (srcCol->getColumnType() == ColumnType::Index) continue;
        if (auto col = left.getColumn(srcCol->getHeader()); col && ignoreDuplicates) {
            continue;
        }
        dataframe->addColumn(std::shared_ptr<Column>(srcCol->clone()));
    }
    dataframe->updateIndexBuffer();

    if (fillMissingRows) {
        const size_t numRows = dataframe->getNumberOfRows();
        for (auto col : *dataframe) {
            if (col->getSize() != numRows) {
                col->getBuffer()->getEditableRepresentation<BufferRAM>()->dispatch<void>(
                    [numRows, col](auto typedBuf) {
                        using ValueType = util::PrecisionValueType<decltype(typedBuf)>;
                        auto& data = typedBuf->getDataContainer();
                        if constexpr (std::is_same_v<ValueType, CategoricalColumn::type>) {
                            if (auto c = dynamic_cast<CategoricalColumn*>(col.get())) {
                                data.resize(numRows, c->addCategory("undefined"));
                            } else {
                                data.resize(numRows, ValueType{0});
                            }
                        } else {
                            data.resize(numRows, ValueType{0});
                        }
                    });
            }
        }
    }

    return dataframe;
}

std::shared_ptr<DataFrame> appendRows(const DataFrame& top, const DataFrame& bottom,
                                      bool matchByName) {
    if (top.getNumberOfColumns() != bottom.getNumberOfColumns()) {
        throw Exception(IVW_CONTEXT_CUSTOM("dataframe::appendRows"),
                        "Column counts are different (top: {}, bottom: {})",
                        top.getNumberOfColumns(), bottom.getNumberOfColumns());
    }

    auto dataframe = std::make_shared<DataFrame>(top);
    if (matchByName) {
        for (auto srcCol : bottom) {
            if (auto col = dataframe->getColumn(srcCol->getHeader())) {
                col->append(*srcCol);
            } else {
                throw Exception(IVW_CONTEXT_CUSTOM("dataframe::appendRows"),
                                "column '{}' not found in top DataFrame", srcCol->getHeader());
            }
        }
    } else {
        for (auto [t, b] : util::zip(*dataframe, bottom)) {
            t->append(*b);
        }
    }
    dataframe->updateIndexBuffer();

    return dataframe;
}

namespace detail {

void columnCheck(const DataFrame& left, const DataFrame& right,
                 const std::vector<std::pair<std::string, std::string>>& keyColumns,
                 const std::string& context) {
    for (const auto& [leftCol, rightCol] : keyColumns) {
        auto indexCol1 = left.getColumn(leftCol);
        auto indexCol2 = right.getColumn(rightCol);
        if (!indexCol1) {
            throw Exception(IVW_CONTEXT_CUSTOM(context),
                            "key column '{}' missing in the left data frame", leftCol);
        }
        if (!indexCol2) {
            throw Exception(IVW_CONTEXT_CUSTOM(context),
                            "key column '{}' missing in the right data frame", rightCol);
        }

        const bool catcol1 = indexCol1->getColumnType() == ColumnType::Categorical;
        const bool catcol2 = indexCol2->getColumnType() == ColumnType::Categorical;
        // check only for categorical types and do not compare column types directly.
        // This enables combining a regular column with an index column, e.g. for indexing.
        if (catcol1 != catcol2) {
            throw Exception(IVW_CONTEXT_CUSTOM(context),
                            "column type mismatch in key columns '{}' = {}, '{}' = {}", leftCol,
                            indexCol1->getColumnType(), rightCol, indexCol2->getColumnType());
        }

        if (indexCol1->getBuffer()->getDataFormat()->getId() !=
            indexCol2->getBuffer()->getDataFormat()->getId()) {
            throw Exception(IVW_CONTEXT_CUSTOM(context),
                            "format mismatch in key columns '{}' = {}, '{}' = {}", leftCol,
                            indexCol1->getBuffer()->getDataFormat()->getString(), rightCol,
                            indexCol2->getBuffer()->getDataFormat()->getString());
        }
    }
}

template <typename T, bool firstMatchOnly, typename Cont>
std::vector<std::vector<std::uint32_t>> matchingRows(const Cont& left, const Cont& right) {
    std::unordered_map<T, std::vector<std::uint32_t>> matches;
    for (auto&& [r, value] : util::enumerate<std::uint32_t>(right)) {
        if constexpr (firstMatchOnly) {
            if (matches.find(value) == matches.end()) {
                matches[value].push_back(r);
            }
        } else {
            matches[value].push_back(r);
        }
    }
    std::vector<std::vector<std::uint32_t>> rows(std::distance(left.begin(), left.end()));
    for (auto&& [i, key] : util::enumerate<std::uint32_t>(left)) {
        rows[i] = matches[key];
    }
    return rows;
}

/**
 * \brief for each row in leftCol return a list of matching row indices in rightCol
 */
template <bool firstMatchOnly = false>
std::vector<std::vector<std::uint32_t>> getMatchingRows(std::shared_ptr<const Column> leftCol,
                                                        std::shared_ptr<const Column> rightCol) {
    if (auto catCol1 = dynamic_cast<const CategoricalColumn*>(leftCol.get())) {
        // need to match values of categorical columns instead of indices stored in buffer
        auto catCol2 = dynamic_cast<const CategoricalColumn*>(rightCol.get());
        IVW_ASSERT(catCol2, "right column is not categorical");

        return matchingRows<std::string_view, firstMatchOnly>(*catCol1, *catCol2);
    } else {
        using retval = std::vector<std::vector<std::uint32_t>>;

        return leftCol->getBuffer()->getRepresentation<BufferRAM>()->dispatch<retval>(
            [rightBuffer = rightCol->getBuffer()](auto typedBuf) {
                using ValueType = util::PrecisionValueType<decltype(typedBuf)>;

                const auto& left = typedBuf->getDataContainer();
                const auto& right = static_cast<const BufferRAMPrecision<ValueType>*>(
                                        rightBuffer->getRepresentation<BufferRAM>())
                                        ->getDataContainer();

                return matchingRows<ValueType, firstMatchOnly>(left, right);
            });
    }
}

std::vector<std::vector<std::uint32_t>> getMatchingRows(
    const DataFrame& left, const DataFrame& right,
    const std::vector<std::pair<std::string, std::string>>& keyColumns) {

    auto indexCol1 = left.getColumn(keyColumns.front().first);
    auto indexCol2 = right.getColumn(keyColumns.front().second);

    auto rows = detail::getMatchingRows(indexCol1, indexCol2);
    // narrow down row selection by filtering with remaining keys
    for (auto&& [leftName, rightName] : util::as_range(keyColumns.begin() + 1, keyColumns.end())) {
        auto leftCol = left.getColumn(leftName);
        auto rightCol = right.getColumn(rightName);

        if (auto catCol1 = dynamic_cast<const CategoricalColumn*>(leftCol.get())) {
            auto catCol2 = dynamic_cast<const CategoricalColumn*>(rightCol.get());
            IVW_ASSERT(catCol2, "right column is not categorical");

            auto valuesLeftIt = catCol1->begin();
            auto valuesRightIt = catCol2->begin();
            for (auto&& [i, rowMatches] : util::enumerate<std::uint32_t>(rows)) {
                std::erase_if(rowMatches, [key = *(valuesLeftIt + i), valuesRightIt](auto row) {
                    return key != *(valuesRightIt + row);
                });
            }
        } else {
            leftCol->getBuffer()->getRepresentation<BufferRAM>()->dispatch<void>(
                [&rows, rightBuffer = rightCol->getBuffer()](auto typedBuf) {
                    using ValueType = util::PrecisionValueType<decltype(typedBuf)>;

                    const auto& left = typedBuf->getDataContainer();
                    const auto& right = static_cast<const BufferRAMPrecision<ValueType>*>(
                                            rightBuffer->getRepresentation<BufferRAM>())
                                            ->getDataContainer();

                    for (auto&& [i, rowMatches] : util::enumerate(rows)) {
                        std::erase_if(rowMatches, [key = left[i], &right](auto row) {
                            return key != right[row];
                        });
                    }
                });
        }
    }
    return rows;
}

void addColumns(std::shared_ptr<DataFrame> dst, const DataFrame& srcDataFrame,
                const std::vector<std::string>& keyColumns, bool skipKeyCol) {
    for (auto srcCol : srcDataFrame) {
        if (srcCol->getColumnType() == ColumnType::Index) continue;
        if (skipKeyCol && util::contains(keyColumns, srcCol->getHeader())) continue;

        dst->addColumn(std::shared_ptr<Column>{srcCol->clone()});
    }
}

void addColumns(std::shared_ptr<DataFrame> dst, const DataFrame& srcDataFrame,
                const std::vector<std::uint32_t>& rows, const std::vector<std::string>& keyColumns,
                bool skipKeyCol) {
    for (auto srcCol : srcDataFrame) {
        if (srcCol->getColumnType() == ColumnType::Index) continue;
        if (skipKeyCol && util::contains(keyColumns, srcCol->getHeader())) continue;

        dst->addColumn(std::shared_ptr<Column>(srcCol->clone(rows)));
    }
}

void addColumns(std::shared_ptr<DataFrame> dst, const DataFrame& srcDataFrame,
                const std::vector<std::optional<std::uint32_t>>& rows,
                const std::vector<std::string>& keyColumns, bool skipKeyCol) {
    for (auto srcCol : srcDataFrame) {
        if (srcCol->getColumnType() == ColumnType::Index) continue;
        if (skipKeyCol && util::contains(keyColumns, srcCol->getHeader())) continue;

        if (auto c = dynamic_cast<CategoricalColumn*>(srcCol.get())) {
            auto data = util::transform(rows, [range = c->values()](auto v) {
                return v.has_value() ? *(range.begin() + v.value()) : "undefined";
            });
            dst->addCategoricalColumn(c->getHeader(), data);
        } else {
            srcCol->getBuffer()->getRepresentation<BufferRAM>()->dispatch<void>(
                [dst, srcCol, header = srcCol->getHeader(), rows](auto typedBuf) {
                    using ValueType = util::PrecisionValueType<decltype(typedBuf)>;
                    auto dstData =
                        util::transform(rows, [&src = typedBuf->getDataContainer()](auto v) {
                            return v.has_value() ? src[v.value()] : ValueType{0};
                        });
                    dst->addColumn(header, std::move(dstData));
                });
        }
    }
}

}  // namespace detail

std::shared_ptr<DataFrame> innerJoin(const DataFrame& left, const DataFrame& right,
                                     const std::pair<std::string, std::string>& keyColumn) {
    detail::columnCheck(left, right, {keyColumn}, "dataframe::innerJoin");

    auto indexCol1 = left.getColumn(keyColumn.first);
    auto indexCol2 = right.getColumn(keyColumn.second);

    std::vector<std::uint32_t> rowsLeft;
    std::vector<std::uint32_t> rowsRight;
    for (auto&& [i, rowIndices] :
         util::enumerate<std::uint32_t>(detail::getMatchingRows<true>(indexCol1, indexCol2))) {
        if (!rowIndices.empty()) {
            rowsLeft.push_back(i);
            rowsRight.push_back(rowIndices.front());
        }
    }

    IVW_ASSERT(rowsLeft.size() == rowsRight.size(), "incorrect number of matching row indices");

    auto dataframe = std::make_shared<DataFrame>();
    dataframe->dropColumn(0);
    dataframe->addColumn(std::shared_ptr<Column>(left.getIndexColumn()->clone(rowsLeft)));
    detail::addColumns(dataframe, left, rowsLeft, {keyColumn.first}, false);
    detail::addColumns(dataframe, right, rowsRight, {keyColumn.second}, true);

    return dataframe;
}

std::shared_ptr<DataFrame> innerJoin(
    const DataFrame& left, const DataFrame& right,
    const std::vector<std::pair<std::string, std::string>>& keyColumns) {

    if (keyColumns.empty()) {
        throw Exception("no key columns given", IVW_CONTEXT_CUSTOM("dataframe::innerJoin"));
    }

    detail::columnCheck(left, right, keyColumns, "dataframe::innerJoin");

    std::vector<std::uint32_t> rowsLeft;
    std::vector<std::uint32_t> rowsRight;
    for (auto&& [i, rowIndices] :
         util::enumerate<std::uint32_t>(detail::getMatchingRows(left, right, keyColumns))) {
        if (!rowIndices.empty()) {
            rowsLeft.push_back(i);
            rowsRight.push_back(rowIndices.front());
        }
    }

    IVW_ASSERT(rowsLeft.size() == rowsRight.size(), "incorrect number of matching row indices");

    std::vector<std::string> leftKeys;
    std::transform(keyColumns.begin(), keyColumns.end(), std::back_inserter(leftKeys),
                   [](const auto& item) { return item.first; });
    std::vector<std::string> rightKeys;
    std::transform(keyColumns.begin(), keyColumns.end(), std::back_inserter(rightKeys),
                   [](const auto& item) { return item.second; });

    auto dataframe = std::make_shared<DataFrame>();
    dataframe->dropColumn(0);
    dataframe->addColumn(std::shared_ptr<Column>(left.getIndexColumn()->clone(rowsLeft)));
    detail::addColumns(dataframe, left, rowsLeft, leftKeys, false);
    detail::addColumns(dataframe, right, rowsRight, rightKeys, true);

    return dataframe;
}

std::shared_ptr<DataFrame> leftJoin(const DataFrame& left, const DataFrame& right,
                                    const std::pair<std::string, std::string>& keyColumn) {
    detail::columnCheck(left, right, {keyColumn}, "dataframe::leftJoin");

    auto indexCol1 = left.getColumn(keyColumn.first);
    auto indexCol2 = right.getColumn(keyColumn.second);

    auto rows = util::transform(
        detail::getMatchingRows<true>(indexCol1, indexCol2),
        [](const std::vector<std::uint32_t>& rowIndices) -> std::optional<std::uint32_t> {
            if (rowIndices.empty()) {
                return {};
            } else {
                return rowIndices.front();
            }
        });

    IVW_ASSERT(indexCol1->getSize() == rows.size(), "incorrect number of matching row indices");

    auto dataframe = std::make_shared<DataFrame>();
    dataframe->dropColumn(0);
    dataframe->addColumn(std::shared_ptr<Column>(left.getIndexColumn()->clone()));
    detail::addColumns(dataframe, left, {keyColumn.first}, false);
    detail::addColumns(dataframe, right, rows, {keyColumn.second}, true);

    return dataframe;
}

std::shared_ptr<DataFrame> leftJoin(
    const DataFrame& left, const DataFrame& right,
    const std::vector<std::pair<std::string, std::string>>& keyColumns) {

    if (keyColumns.empty()) {
        throw Exception("no key columns given", IVW_CONTEXT_CUSTOM("dataframe::leftJoin"));
    }

    detail::columnCheck(left, right, keyColumns, "dataframe::leftJoin");

    auto rows = util::transform(
        detail::getMatchingRows(left, right, keyColumns),
        [](const std::vector<std::uint32_t>& rowIndices) -> std::optional<std::uint32_t> {
            if (rowIndices.empty()) {
                return {};
            } else {
                return rowIndices.front();
            }
        });

    std::vector<std::string> leftKeys;
    std::transform(keyColumns.begin(), keyColumns.end(), std::back_inserter(leftKeys),
                   [](const auto& item) { return item.first; });
    std::vector<std::string> rightKeys;
    std::transform(keyColumns.begin(), keyColumns.end(), std::back_inserter(rightKeys),
                   [](const auto& item) { return item.second; });

    auto dataframe = std::make_shared<DataFrame>();
    dataframe->dropColumn(0);
    dataframe->addColumn(std::shared_ptr<Column>(left.getIndexColumn()->clone()));
    detail::addColumns(dataframe, left, leftKeys, false);
    detail::addColumns(dataframe, right, rows, rightKeys, true);

    return dataframe;
}

std::shared_ptr<DataFrame> combineDataFrames(std::vector<std::shared_ptr<DataFrame>> dataFrames,
                                             bool skipIndexColumn, std::string skipcol) {
    if (dataFrames.empty()) {
        throw Exception("DataFrames vector is empty",
                        IVW_CONTEXT_CUSTOM("dataframe::combineDataFrames"));
    }
    if (dataFrames.size() == 1) {  // just one df, clone it;
        return std::make_shared<DataFrame>(*dataFrames.front());
    }

    size_t newSize = 0;
    for (auto& hf : dataFrames) {
        newSize += hf->getNumberOfRows();
    }

    if (newSize == 0) {
        throw Exception("All input DataFrames are empty",
                        IVW_CONTEXT_CUSTOM("dataframe::combineDataFrames"));
    }

    auto first = *dataFrames.front();
    {
        std::map<std::string, const DataFormatBase*> columnType;

        for (auto col : first) {
            if (skipIndexColumn && toLower(col->getHeader()) == skipcol) continue;
            columnType[col->getHeader()] = col->getBuffer()->getDataFormat();
        }

        for (auto df = dataFrames.begin() + 1; df != dataFrames.end(); ++df) {
            for (auto col : *df->get()) {
                if (skipIndexColumn && toLower(col->getHeader()) == skipcol) continue;
                auto it = columnType.find(col->getHeader());
                if (it == columnType.end()) {
                    throw Exception(IVW_CONTEXT_CUSTOM("dataframe::combineDataFrames"),
                                    "Column {} did not exist in first DataFrame but in at least "
                                    "one of the others",
                                    col->getHeader());
                }
                if (it->second != col->getBuffer()->getDataFormat()) {
                    if (it == columnType.end()) {
                        throw Exception(IVW_CONTEXT_CUSTOM("dataframe::combineDataFrames"),
                                        "Column {} has different format in DataFrames ({}, {})",
                                        col->getHeader(), it->second->getString(),
                                        col->getBuffer()->getDataFormat()->getSize());
                    }
                }
            }
        }
    }

    std::unordered_map<std::string, std::shared_ptr<Column>> columns;
    std::shared_ptr<DataFrame> newDataFrame =
        std::make_shared<DataFrame>(static_cast<std::uint32_t>(newSize));
    for (auto col : first) {
        col->getBuffer()
            ->getRepresentation<BufferRAM>()
            ->dispatch<void, dispatching::filter::Scalars>([&]([[maybe_unused]] auto typedBuf) {
                using ValueType = util::PrecisionValueType<decltype(typedBuf)>;
                columns[col->getHeader()] = newDataFrame->addColumn<ValueType>(col->getHeader());
            });
    }
    for (auto& data : dataFrames) {
        for (auto col : *(data.get())) {
            if (skipIndexColumn && toLower(col->getHeader()) == skipcol) continue;

            columns[col->getHeader()]
                ->getBuffer()
                ->getEditableRepresentation<BufferRAM>()
                ->dispatch<void>([&](auto typedBuf) {
                    using ValueType = util::PrecisionValueType<decltype(typedBuf)>;
                    auto typedBuffer =
                        static_cast<const Buffer<ValueType>*>(col->getBuffer().get());
                    auto& vec = typedBuffer->getRAMRepresentation()->getDataContainer();
                    typedBuf->getDataContainer().insert(typedBuf->getDataContainer().end(),
                                                        vec.begin(), vec.end());
                });
        }
    }
    return newDataFrame;
}

#include <warn/push>
#include <warn/ignore/conversion>
std::vector<std::uint32_t> selectRows(const Column& col,
                                      const std::vector<dataframefilters::ItemFilter>& filters) {
    if (filters.empty()) return {};

    if (col.getColumnType() == ColumnType::Categorical) {
        const auto& catCol = dynamic_cast<const CategoricalColumn&>(col);
        std::vector<std::uint32_t> rows;
        for (auto&& [row, value] : util::enumerate<std::uint32_t>(catCol.values())) {
            auto test = util::overloaded{
                [v = value](const std::function<bool(std::string_view)>& func) { return func(v); },
                [](const std::function<bool(int64_t)>&) { return false; },
                [](const std::function<bool(double)>&) { return false; }};
            auto op = [&](const auto& f) { return std::visit(test, f.filter); };
            if (std::any_of(filters.begin(), filters.end(), op)) {
                rows.push_back(row);
            }
        }
        return rows;
    } else {
        return col.getBuffer()
            ->getRepresentation<BufferRAM>()
            ->dispatch<std::vector<std::uint32_t>, dispatching::filter::Scalars>(
                [filters](auto typedBuf) {
                    using ValueType = util::PrecisionValueType<decltype(typedBuf)>;
                    std::vector<std::uint32_t> rows;
                    for (auto&& [row, value] :
                         util::enumerate<std::uint32_t>(typedBuf->getDataContainer())) {
                        auto test = util::overloaded{
                            [&v = value](
                                [[maybe_unused]] const std::function<bool(std::int64_t)>& func) {
                                if constexpr (std::is_integral_v<ValueType>) {
                                    return func(static_cast<std::int64_t>(v));
                                } else {
                                    (void)v;
                                    return false;
                                }
                            },
                            [v = value]([[maybe_unused]] const std::function<bool(double)>& func) {
                                if constexpr (std::is_floating_point_v<ValueType>) {
                                    return func(v);
                                } else {
                                    (void)v;
                                    return false;
                                }
                            },
                            [](const std::function<bool(std::string_view)>&) { return false; }};
                        auto op = [&](const auto& f) { return std::visit(test, f.filter); };
                        if (std::any_of(filters.begin(), filters.end(), op)) {
                            rows.push_back(row);
                        }
                    }
                    return rows;
                });
    }
}
#include <warn/pop>

std::vector<std::uint32_t> selectRows(const DataFrame& dataframe,
                                      dataframefilters::Filters filters) {
    const int colCount = static_cast<int>(dataframe.getNumberOfColumns());
    std::unordered_map<int, dataframefilters::Filters> filterCols;
    for (auto& f : filters.include) {
        if (f.column >= 0 && f.column < colCount) filterCols[f.column].include.push_back(f);
    }
    for (auto& f : filters.exclude) {
        if (f.column >= 0 && f.column < colCount) filterCols[f.column].exclude.push_back(f);
    }
    if (filterCols.empty()) {
        auto seq = util::sequence<std::uint32_t>(
            0, static_cast<std::uint32_t>(dataframe.getNumberOfRows()), 1);
        return {seq.begin(), seq.end()};
    }

    if (filterCols.size() == 1) {
        const auto& col = *dataframe.getColumn(filterCols.begin()->first).get();
        return (BitSet(selectRows(col, filterCols.begin()->second.include)) -
                BitSet(selectRows(col, filterCols.begin()->second.exclude)))
            .toVector();
    }
    BitSet include;
    BitSet exclude;
    for (auto&& [colIndex, f] : filterCols) {
        const auto& col = *dataframe.getColumn(colIndex).get();
        include.add(selectRows(col, f.include));
        exclude.add(selectRows(col, f.exclude));
    }
    return (include - exclude).toVector();
}

std::string createToolTipForRow(const DataFrame& dataframe, size_t rowId) {
    using H = utildoc::TableBuilder::Header;
    using P = Document::PathComponent;
    Document doc;
    doc.append("b", fmt::format("Data Point {}", rowId), {{"style", "color:white;"}});
    utildoc::TableBuilder tb(doc.handle(), P::end());
    for (size_t i = 0; i < dataframe.getNumberOfColumns(); i++) {
        auto col = dataframe.getColumn(i);
        tb(H(col->getHeader()), col->getAsString(rowId));
    }

    return doc;
}

}  // namespace dataframe

}  // namespace inviwo
