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

#include <inviwo/dataframe/util/dataframeutil.h>

#include <inviwo/core/util/document.h>
#include <inviwo/core/util/stdextensions.h>
#include <inviwo/core/util/assertion.h>

#include <fmt/format.h>

#include <optional>

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
        throw Exception(fmt::format("Row counts are different (top: {}, bottom: {})",
                                    left.getNumberOfRows(), right.getNumberOfRows()),
                        IVW_CONTEXT_CUSTOM("dataframe::appendColumns"));
    }

    auto dataframe = std::make_shared<DataFrame>(left);

    for (auto srcCol : right) {
        if (srcCol == right.getIndexColumn()) continue;
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
        throw Exception(fmt::format("Column counts are different (top: {}, bottom: {})",
                                    top.getNumberOfColumns(), bottom.getNumberOfColumns()),
                        IVW_CONTEXT_CUSTOM("dataframe::appendRows"));
    }

    auto dataframe = std::make_shared<DataFrame>(top);
    if (matchByName) {
        for (auto srcCol : bottom) {
            if (auto col = dataframe->getColumn(srcCol->getHeader())) {
                col->append(*srcCol);
            } else {
                throw Exception(
                    fmt::format("column '{}' not found in top DataFrame", srcCol->getHeader()),
                    IVW_CONTEXT_CUSTOM("dataframe::appendRows"));
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
                 const std::vector<std::string>& keyColumns, const std::string& context) {
    for (const auto& col : keyColumns) {
        auto indexCol1 = left.getColumn(col);
        auto indexCol2 = right.getColumn(col);
        if (!indexCol1 || !indexCol2) {
            throw Exception(
                fmt::format("key column '{}' missing in {}", col,
                            (!indexCol1 && !indexCol2)
                                ? "both DataFrames"
                                : fmt::format("{} DataFrame", !indexCol1 ? "left" : "right")),
                IVW_CONTEXT_CUSTOM(context));
        }

        auto catcol1 = dynamic_cast<const CategoricalColumn*>(indexCol1.get());
        auto catcol2 = dynamic_cast<const CategoricalColumn*>(indexCol2.get());
        if (!catcol1 != !catcol2) {
            throw Exception(
                fmt::format("column type mismatch in key columns '{}': left = {}, right = {}", col,
                            catcol1 ? "categorical" : "regular",
                            catcol2 ? "categorical" : "regular"),
                IVW_CONTEXT_CUSTOM(context));
        }

        if (indexCol1->getBuffer()->getDataFormat()->getId() !=
            indexCol2->getBuffer()->getDataFormat()->getId()) {
            throw Exception(
                fmt::format("format mismatch in key columns '{}': left = {}, right = {}", col,
                            indexCol1->getBuffer()->getDataFormat()->getString(),
                            indexCol2->getBuffer()->getDataFormat()->getString()),
                IVW_CONTEXT_CUSTOM(context));
        }
    }
}

/**
 * \brief for each row in leftCol return a list of matching row indices in rightCol
 */
template <bool firstMatchOnly = false>
std::vector<std::vector<size_t>> getMatchingRows(std::shared_ptr<const Column> leftCol,
                                                 std::shared_ptr<const Column> rightCol) {
    std::vector<std::vector<size_t>> rows(leftCol->getSize());

    if (auto catCol1 = dynamic_cast<const CategoricalColumn*>(leftCol.get())) {
        // need to match values of categorical columns instead of indices stored in buffer
        auto catCol2 = dynamic_cast<const CategoricalColumn*>(rightCol.get());
        IVW_ASSERT(catCol2, "right column is not categorical");

        auto valuesRight = catCol2->getValues();
        for (auto&& [i, key] : util::enumerate(catCol1->getValues())) {
            // find all matching rows in right column
            std::vector<size_t> matches;
            for (auto&& [r, value] : util::enumerate(valuesRight)) {
                if (key == value) {
                    matches.emplace_back(r);
                    if constexpr (firstMatchOnly) break;
                }
            }
            rows[i] = std::move(matches);
        }
    } else {
        leftCol->getBuffer()->getRepresentation<BufferRAM>()->dispatch<void>(
            [rightBuffer = rightCol->getBuffer(), &rows](auto typedBuf) {
                using ValueType = util::PrecisionValueType<decltype(typedBuf)>;

                const auto& left = typedBuf->getDataContainer();
                const auto& right = static_cast<const BufferRAMPrecision<ValueType>*>(
                                        rightBuffer->getRepresentation<BufferRAM>())
                                        ->getDataContainer();

                for (auto&& [i, key] : util::enumerate(left)) {
                    // find all matching rows in right
                    std::vector<size_t> matches;
                    for (auto&& [r, value] : util::enumerate(right)) {
                        if (key == value) {
                            matches.emplace_back(r);
                            if constexpr (firstMatchOnly) break;
                        }
                    }
                    rows[i] = std::move(matches);
                }
            });
    }
    return rows;
}

std::vector<std::vector<size_t>> getMatchingRows(const DataFrame& left, const DataFrame& right,
                                                 const std::vector<std::string>& keyColumns) {
    auto indexCol1 = left.getColumn(keyColumns.front());
    auto indexCol2 = right.getColumn(keyColumns.front());

    auto rows = detail::getMatchingRows(indexCol1, indexCol2);
    // narrow down row selection by filtering with remaining keys
    for (auto keyColName : util::as_range(keyColumns.begin() + 1, keyColumns.end())) {
        auto leftCol = left.getColumn(keyColName);
        auto rightCol = right.getColumn(keyColName);

        if (auto catCol1 = dynamic_cast<const CategoricalColumn*>(leftCol.get())) {
            auto catCol2 = dynamic_cast<const CategoricalColumn*>(rightCol.get());
            IVW_ASSERT(catCol2, "right column is not categorical");

            auto valuesLeft = catCol1->getValues();
            auto valuesRight = catCol2->getValues();
            for (auto&& [i, rowMatches] : util::enumerate(rows)) {
                util::erase_remove_if(rowMatches, [key = valuesLeft[i], &valuesRight](auto row) {
                    return key != valuesRight[row];
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
                        util::erase_remove_if(rowMatches, [key = left[i], &right](auto row) {
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
        if (srcCol == srcDataFrame.getIndexColumn()) continue;
        if (skipKeyCol && util::contains(keyColumns, srcCol->getHeader())) {
            continue;
        }
        dst->addColumn(std::shared_ptr<Column>{srcCol->clone()});
    }
}

void addColumns(std::shared_ptr<DataFrame> dst, const DataFrame& srcDataFrame,
                const std::vector<size_t>& rows, const std::vector<std::string>& keyColumns,
                bool skipKeyCol) {
    for (auto srcCol : srcDataFrame) {
        if (srcCol == srcDataFrame.getIndexColumn()) continue;
        if (skipKeyCol && util::contains(keyColumns, srcCol->getHeader())) {
            continue;
        }

        if (auto c = dynamic_cast<CategoricalColumn*>(srcCol.get())) {
            auto data = util::transform(rows, [src = c->getValues()](size_t i) { return src[i]; });
            dst->addCategoricalColumn(c->getHeader(), data);
        } else {
            srcCol->getBuffer()->getRepresentation<BufferRAM>()->dispatch<void>(
                [dst, srcCol, header = srcCol->getHeader(), rows](auto typedBuf) {
                    auto dstData = util::transform(
                        rows, [& src = typedBuf->getDataContainer()](size_t i) { return src[i]; });
                    dst->addColumn(header, std::move(dstData));
                });
        }
    }
}

void addColumns(std::shared_ptr<DataFrame> dst, const DataFrame& srcDataFrame,
                const std::vector<std::optional<size_t>>& rows,
                const std::vector<std::string>& keyColumns, bool skipKeyCol) {
    for (auto srcCol : srcDataFrame) {
        if (srcCol == srcDataFrame.getIndexColumn()) continue;
        if (skipKeyCol && util::contains(keyColumns, srcCol->getHeader())) {
            continue;
        }

        if (auto c = dynamic_cast<CategoricalColumn*>(srcCol.get())) {
            auto data = util::transform(rows, [src = c->getValues()](auto v) {
                return v.has_value() ? src[v.value()] : "undefined";
            });
            dst->addCategoricalColumn(c->getHeader(), data);
        } else {
            srcCol->getBuffer()->getRepresentation<BufferRAM>()->dispatch<void>(
                [dst, srcCol, header = srcCol->getHeader(), rows](auto typedBuf) {
                    using ValueType = util::PrecisionValueType<decltype(typedBuf)>;
                    auto dstData =
                        util::transform(rows, [& src = typedBuf->getDataContainer()](auto v) {
                            return v.has_value() ? src[v.value()] : ValueType{0};
                        });
                    dst->addColumn(header, std::move(dstData));
                });
        }
    }
}

}  // namespace detail

std::shared_ptr<DataFrame> innerJoin(const DataFrame& left, const DataFrame& right,
                                     const std::string& keyColumn) {
    detail::columnCheck(left, right, {keyColumn}, "dataframe::innerJoin");

    auto indexCol1 = left.getColumn(keyColumn);
    auto indexCol2 = right.getColumn(keyColumn);

    std::vector<size_t> rowsLeft;
    std::vector<size_t> rowsRight;
    for (auto&& [i, rowIndices] :
         util::enumerate(detail::getMatchingRows<true>(indexCol1, indexCol2))) {
        if (!rowIndices.empty()) {
            rowsLeft.push_back(i);
            rowsRight.push_back(rowIndices.front());
        }
    }

    IVW_ASSERT(rowsLeft.size() == rowsRight.size(), "incorrect number of matching row indices");

    auto dataframe = std::make_shared<DataFrame>();
    detail::addColumns(dataframe, left, rowsLeft, {keyColumn}, false);
    detail::addColumns(dataframe, right, rowsRight, {keyColumn}, true);

    dataframe->updateIndexBuffer();
    return dataframe;
}

std::shared_ptr<DataFrame> innerJoin(const DataFrame& left, const DataFrame& right,
                                     const std::vector<std::string>& keyColumns) {
    if (keyColumns.empty()) {
        throw Exception("no key columns given", IVW_CONTEXT_CUSTOM("dataframe::innerJoin"));
    }

    detail::columnCheck(left, right, keyColumns, "dataframe::innerJoin");

    std::vector<size_t> rowsLeft;
    std::vector<size_t> rowsRight;
    for (auto&& [i, rowIndices] :
         util::enumerate(detail::getMatchingRows(left, right, keyColumns))) {
        if (!rowIndices.empty()) {
            rowsLeft.push_back(i);
            rowsRight.push_back(rowIndices.front());
        }
    }

    IVW_ASSERT(rowsLeft.size() == rowsRight.size(), "incorrect number of matching row indices");

    auto dataframe = std::make_shared<DataFrame>();
    detail::addColumns(dataframe, left, rowsLeft, keyColumns, false);
    detail::addColumns(dataframe, right, rowsRight, keyColumns, true);

    dataframe->updateIndexBuffer();
    return dataframe;
}

std::shared_ptr<DataFrame> leftJoin(const DataFrame& left, const DataFrame& right,
                                    const std::string& keyColumn) {
    detail::columnCheck(left, right, {keyColumn}, "dataframe::leftJoin");

    auto indexCol1 = left.getColumn(keyColumn);
    auto indexCol2 = right.getColumn(keyColumn);

    auto rows = util::transform(detail::getMatchingRows<true>(indexCol1, indexCol2),
                                [](const std::vector<size_t>& rowIndices) -> std::optional<size_t> {
                                    if (rowIndices.empty()) {
                                        return {};
                                    } else {
                                        return rowIndices.front();
                                    }
                                });

    IVW_ASSERT(indexCol1->getSize() == rows.size(), "incorrect number of matching row indices");

    auto dataframe = std::make_shared<DataFrame>();
    detail::addColumns(dataframe, left, {keyColumn}, false);
    detail::addColumns(dataframe, right, rows, {keyColumn}, true);

    dataframe->updateIndexBuffer();
    return dataframe;
}

std::shared_ptr<DataFrame> leftJoin(const DataFrame& left, const DataFrame& right,
                                    const std::vector<std::string>& keyColumns) {
    if (keyColumns.empty()) {
        throw Exception("no key columns given", IVW_CONTEXT_CUSTOM("dataframe::leftJoin"));
    }

    detail::columnCheck(left, right, keyColumns, "dataframe::leftJoin");

    auto rows = util::transform(detail::getMatchingRows(left, right, keyColumns),
                                [](const std::vector<size_t>& rowIndices) -> std::optional<size_t> {
                                    if (rowIndices.empty()) {
                                        return {};
                                    } else {
                                        return rowIndices.front();
                                    }
                                });

    auto dataframe = std::make_shared<DataFrame>();
    detail::addColumns(dataframe, left, keyColumns, false);
    detail::addColumns(dataframe, right, rows, keyColumns, true);

    dataframe->updateIndexBuffer();
    return dataframe;
}

std::shared_ptr<DataFrame> combineDataFrames(std::vector<std::shared_ptr<DataFrame>> dataFrames,
                                             bool skipIndexColumn, std::string skipcol) {
    if (dataFrames.empty()) {
        throw Exception("DataFrames vector is empty",
                        IVW_CONTEXT_CUSTOM("dataframe::combineDataFrames"));
    }
    if (dataFrames.size() == 1) {  // just one df, clone it;
        return std::make_shared<DataFrame>(*dataFrames.front().get());
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
                    throw Exception(
                        fmt::format("Column {} did not exist in first DataFrame but in at least "
                                    "one of the others",
                                    col->getHeader()),
                        IVW_CONTEXT_CUSTOM("dataframe::combineDataFrames"));
                }
                if (it->second != col->getBuffer()->getDataFormat()) {
                    if (it == columnType.end()) {
                        throw Exception(
                            fmt::format("Column {} has different format in DataFrames ({}, {})",
                                        col->getHeader(), it->second->getString(),
                                        col->getBuffer()->getDataFormat()->getSize()),
                            IVW_CONTEXT_CUSTOM("dataframe::combineDataFrames"));
                    }
                }
            }
        }
    }

    std::unordered_map<std::string, std::shared_ptr<Column>> columns;
    std::shared_ptr<DataFrame> newDataFrame =
        std::make_shared<DataFrame>(static_cast<glm::u32>(newSize));
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

std::string createToolTipForRow(const DataFrame& dataframe, size_t rowId) {
    using H = utildoc::TableBuilder::Header;
    using P = Document::PathComponent;
    Document doc;
    doc.append("b", fmt::format("Data Point {}", rowId), {{"style", "color:white;"}});
    utildoc::TableBuilder tb(doc.handle(), P::end());
    for (size_t i = 0; i < dataframe.getNumberOfColumns(); i++) {
        tb(H(dataframe.getHeader(i)), dataframe.getColumn(i)->getAsString(rowId));
    }

    return doc;
}

}  // namespace dataframe

}  // namespace inviwo
