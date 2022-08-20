/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2022 Inviwo Foundation
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

#include <inviwo/core/datastructures/buffer/buffer.h>
#include <inviwo/core/datastructures/datatraits.h>
#include <inviwo/core/datastructures/unitsystem.h>
#include <inviwo/core/metadata/metadataowner.h>
#include <inviwo/core/ports/datainport.h>
#include <inviwo/core/ports/dataoutport.h>
#include <inviwo/core/util/exception.h>

#include <inviwo/dataframe/dataframemoduledefine.h>
#include <inviwo/dataframe/datastructures/column.h>

#include <unordered_map>

#include <fmt/format.h>

namespace inviwo {

class BufferBase;
class BufferRAM;

class IVW_MODULE_DATAFRAME_API InvalidColCount : public Exception {
public:
    InvalidColCount(const std::string& message = "", ExceptionContext context = ExceptionContext())
        : Exception(message, context) {}
    virtual ~InvalidColCount() noexcept {}
};
class IVW_MODULE_DATAFRAME_API NoColumns : public Exception {
public:
    NoColumns(const std::string& message = "", ExceptionContext context = ExceptionContext())
        : Exception(message, context) {}
    virtual ~NoColumns() noexcept {}
};
class IVW_MODULE_DATAFRAME_API DataTypeMismatch : public Exception {
public:
    DataTypeMismatch(const std::string& message = "", ExceptionContext context = ExceptionContext())
        : Exception(message, context) {}
    virtual ~DataTypeMismatch() noexcept {}
};
/**
 * Table of data for plotting where each column can have a header (title).
 * Missing float/double data is stored as Not a Number (NaN)
 * All columns must have the same number of elements for the
 * DataFrame to be valid.
 * @ingroup datastructures
 */
class IVW_MODULE_DATAFRAME_API DataFrame : public MetaDataOwner {
public:
    using LookupTable = std::unordered_map<glm::u64, std::string>;

    DataFrame(std::uint32_t size = 0);
    DataFrame(const DataFrame& df);
    DataFrame(const DataFrame& df, const std::vector<std::uint32_t>& rowSelection);
    DataFrame& operator=(const DataFrame& df);
    DataFrame(DataFrame&& df);
    DataFrame& operator=(DataFrame&& df);
    ~DataFrame() = default;

    /**
     * \brief add existing column to DataFrame
     * updateIndexBuffer() needs to be called after all columns have been added before
     * the DataFrame can be used
     */
    std::shared_ptr<Column> addColumn(std::shared_ptr<Column> column);

    /**
     * \brief add column based on the contents of the given buffer
     * updateIndexBuffer() needs to be called after all columns have been added before
     * the DataFrame can be used
     * Note: this will copy the data of buffer.
     */
    std::shared_ptr<Column> addColumnFromBuffer(std::string_view identifier,
                                                std::shared_ptr<const BufferBase> buffer,
                                                Unit unit = Unit{},
                                                std::optional<dvec2> range = std::nullopt);

    /**
     * \brief add column of type T
     * updateIndexBuffer() needs to be called after all columns have been added before
     * the DataFrame can be used
     */
    template <typename T>
    std::shared_ptr<TemplateColumn<T>> addColumn(std::string_view header, size_t size = 0,
                                                 Unit unit = Unit{},
                                                 std::optional<dvec2> range = std::nullopt);

    /**
     * \brief add column of type T from a std::vector<T>
     * updateIndexBuffer() needs to be called after all columns have been added before
     * the DataFrame can be used
     */
    template <typename T>
    std::shared_ptr<TemplateColumn<T>> addColumn(std::string_view header, std::vector<T> data,
                                                 Unit unit = Unit{},
                                                 std::optional<dvec2> range = std::nullopt);

    /**
     * \brief Drop a column from data frame
     *
     * Drops all columns with the specified header. If the data frame does not have a column with
     * the specified header, nothing happens.
     *
     * \param header Name of the column to be dropped
     */
    void dropColumn(std::string_view header);

    /**
     * \brief Drop a column from data frame
     *
     * Drops the column at the specified position.
     *
     * \param index Position of the column to be dropped
     */
    void dropColumn(size_t index);

    /**
     * \brief add a categorical column
     * updateIndexBuffer() needs to be called after all columns have been added before
     * the DataFrame can be used
     */
    std::shared_ptr<CategoricalColumn> addCategoricalColumn(std::string_view header,
                                                            size_t size = 0);
    std::shared_ptr<CategoricalColumn> addCategoricalColumn(std::string_view header,
                                                            const std::vector<std::string>& values);

    /**
     * \brief access individual columns
     * updateIndexBuffer() needs to be called if the size of the column, i.e. the row count, was
     * changed
     */
    std::shared_ptr<Column> getColumn(size_t index);
    std::shared_ptr<const Column> getColumn(size_t index) const;
    /**
     * fetch the first column where the header matches \p name.
     * @return  matching column if existing, else nullptr
     */
    std::shared_ptr<Column> getColumn(std::string_view name);
    std::shared_ptr<const Column> getColumn(std::string_view name) const;

    std::shared_ptr<IndexColumn> getIndexColumn();
    std::shared_ptr<const IndexColumn> getIndexColumn() const;

    size_t getNumberOfColumns() const;
    /**
     * Returns the number of rows of the largest column, excluding the header, or zero if no columns
     * exist.
     */
    size_t getNumberOfRows() const;

    std::vector<std::shared_ptr<Column>>::iterator begin();
    std::vector<std::shared_ptr<Column>>::iterator end();
    std::vector<std::shared_ptr<Column>>::const_iterator begin() const;
    std::vector<std::shared_ptr<Column>>::const_iterator end() const;

    /**
     * \brief update row indices. Needs to be called if the row count has changed, i.e.
     * after adding rows from the DataFrame or adding or removing rows from a particular
     * column.
     */
    void updateIndexBuffer();

private:
    std::vector<std::shared_ptr<Column>> columns_;
};

using DataFrameOutport = DataOutport<DataFrame>;
using DataFrameInport = DataInport<DataFrame>;
using DataFrameMultiInport = DataInport<DataFrame, 0>;

template <typename T>
std::shared_ptr<TemplateColumn<T>> DataFrame::addColumn(std::string_view header, size_t size,
                                                        Unit unit, std::optional<dvec2> range) {
    auto col = std::make_shared<TemplateColumn<T>>(header, size, unit, range);
    columns_.push_back(col);
    return col;
}

template <typename T>
std::shared_ptr<TemplateColumn<T>> DataFrame::addColumn(std::string_view header,
                                                        std::vector<T> data, Unit unit,
                                                        std::optional<dvec2> range) {
    auto col = std::make_shared<TemplateColumn<T>>(header, std::move(data), unit, range);
    columns_.push_back(col);
    return col;
}

template <>
struct DataTraits<DataFrame> {
    static const std::string& classIdentifier() {
        static const std::string id{"org.inviwo.DataFrame"};
        return id;
    }

    static const std::string& dataName() {
        static const std::string name{"DataFrame"};
        return name;
    }
    static uvec3 colorCode() { return uvec3(153, 76, 0); }
    static Document info(const DataFrame& data) {
        using H = utildoc::TableBuilder::Header;
        using P = Document::PathComponent;
        Document doc;
        doc.append("b", "DataFrame", {{"style", "color:white;"}});
        utildoc::TableBuilder tb(doc.handle(), P::end());
        tb(H("Number of Columns: "), data.getNumberOfColumns());
        const auto rowCount = data.getNumberOfRows();
        tb(H("Number of Rows: "), rowCount);

        utildoc::TableBuilder tb2(doc.handle(), P::end());
        tb2(H("Col"), H("Format"), H("Rows"), H("Name"), H("Column Range"), H("Min"), H("Max"),
            H("Unit"));
        // abbreviate list of columns if there are more than 20
        const size_t ncols = (data.getNumberOfColumns() > 20) ? 10 : data.getNumberOfColumns();

        auto range = [](const Column* col) {
            if (col->getCustomRange()) {
                return toString(*col->getCustomRange());
            } else {
                return std::string("-");
            }
        };

        bool inconsistenRowCount = false;
        for (size_t i = 0; i < ncols; i++) {
            inconsistenRowCount |= (data.getColumn(i)->getSize() != rowCount);
            if (auto col_c =
                    std::dynamic_pointer_cast<const CategoricalColumn>(data.getColumn(i))) {
                tb2(std::to_string(i + 1), "categorical", col_c->getBuffer()->getSize(),
                    col_c->getHeader(), range(col_c.get()));
                std::string categories;
                for (const auto& str : col_c->getCategories()) {
                    if (!categories.empty()) {
                        categories += ", ";
                    }
                    categories += str;
                    if (categories.size() > 50) {
                        // elide rest of the categories
                        categories += ", ...";
                        break;
                    }
                }
                categories += fmt::format(" [{}]", col_c->getCategories().size());
                tb2("", categories, utildoc::TableBuilder::Span_t{},
                    utildoc::TableBuilder::Span_t{}, utildoc::TableBuilder::Span_t{},
                    utildoc::TableBuilder::Span_t{}, utildoc::TableBuilder::Span_t{},
                    utildoc::TableBuilder::Span_t{});
            } else {
                auto col_q = data.getColumn(i);

                auto [minString, maxString] = [&]() -> std::pair<std::string, std::string> {
                    if (col_q->getSize() == 0) return {"-", "-"};

                    const auto minmax = col_q->getDataRange();
                    return {fmt::format("{:8.4g}", minmax.x), fmt::format("{:8.4g}", minmax.y)};
                }();

                tb2(std::to_string(i + 1), col_q->getBuffer()->getDataFormat()->getString(),
                    col_q->getBuffer()->getSize(), col_q->getHeader(), range(col_q.get()),
                    minString, maxString, fmt::to_string(col_q->getUnit()));
            }
        }
        if (ncols != data.getNumberOfColumns()) {
            doc.append("span", "... (" + std::to_string(data.getNumberOfColumns() - ncols) +
                                   " additional columns)");
        }
        if (inconsistenRowCount) {
            doc.append("span", "Inconsistent row counts");
        }

        return doc;
    }
};

}  // namespace inviwo
