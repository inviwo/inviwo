/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2019 Inviwo Foundation
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

#ifndef IVW_DATAFRAME_H
#define IVW_DATAFRAME_H

#include <inviwo/dataframe/dataframemoduledefine.h>
#include <inviwo/dataframe/datastructures/datapoint.h>
#include <inviwo/dataframe/datastructures/column.h>

#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/datastructures/buffer/buffer.h>
#include <inviwo/core/datastructures/datatraits.h>
#include <inviwo/core/ports/datainport.h>
#include <inviwo/core/ports/dataoutport.h>
#include <inviwo/core/util/exception.h>
#include <unordered_map>

namespace inviwo {
class DataPointBase;
class BufferBase;
class BufferRAM;

class IVW_MODULE_DATAFRAME_API InvalidColCount : public Exception {
public:
    InvalidColCount(const std::string &message = "", ExceptionContext context = ExceptionContext())
        : Exception(message, context) {}
    virtual ~InvalidColCount() throw() {}
};
class IVW_MODULE_DATAFRAME_API NoColumns : public Exception {
public:
    NoColumns(const std::string &message = "", ExceptionContext context = ExceptionContext())
        : Exception(message, context) {}
    virtual ~NoColumns() throw() {}
};
class IVW_MODULE_DATAFRAME_API DataTypeMismatch : public Exception {
public:
    DataTypeMismatch(const std::string &message = "", ExceptionContext context = ExceptionContext())
        : Exception(message, context) {}
    virtual ~DataTypeMismatch() throw() {}
};
/**
 * \class DataFrame
 * Table of data for plotting where each column can have a header (title).
 * Missing float/double data is stored as Not a Number (NaN)
 * All columns must have the same number of elements for the
 * DataFrame to be valid.
 */
class IVW_MODULE_DATAFRAME_API DataFrame {
public:
    using DataItem = std::vector<std::shared_ptr<DataPointBase>>;
    using LookupTable = std::unordered_map<glm::u64, std::string>;

    DataFrame(const DataFrame &df);

    DataFrame(std::uint32_t size = 0);
    virtual ~DataFrame() = default;

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
     */
    std::shared_ptr<Column> addColumnFromBuffer(const std::string &identifier,
                                                std::shared_ptr<const BufferBase> buffer);

    /**
     * \brief add column of type T
     * updateIndexBuffer() needs to be called after all columns have been added before
     * the DataFrame can be used
     */
    template <typename T>
    std::shared_ptr<TemplateColumn<T>> addColumn(const std::string &header, size_t size = 0);

    /**
     * \brief add a categorical column
     * updateIndexBuffer() needs to be called after all columns have been added before
     * the DataFrame can be used
     */
    std::shared_ptr<CategoricalColumn> addCategoricalColumn(const std::string &header,
                                                            size_t size = 0);
    /**
     * \brief add a new row given a vector of strings.
     * updateIndexBuffer() needs to be called after the last row has been added.
     *
     * @param data  data for each column
     * @throws NoColumns        if the data frame has no columns defined
     * @throws InvalidColCount  if column count of DataFrame does not match the number of columns in
     * data
     * @throws DataTypeMismatch  if the data type of a column doesn't match with the input data
     */
    void addRow(const std::vector<std::string> &data);

    DataItem getDataItem(size_t index, bool getStringsAsStrings = false) const;

    const std::vector<std::pair<std::string, const DataFormatBase *>> getHeaders() const;
    std::string getHeader(size_t idx) const;

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
    std::shared_ptr<Column> getColumn(const std::string &name);
    std::shared_ptr<const Column> getColumn(const std::string &name) const;

    std::shared_ptr<TemplateColumn<std::uint32_t>> getIndexColumn();
    std::shared_ptr<const TemplateColumn<std::uint32_t>> getIndexColumn() const;

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

/**
 * \brief Create a new DataFrame by guessing the column types from a number of rows.
 *
 * @param exampleRows  Rows for guessing data type of each column.
 * @param colHeaders   Name of each column. If none are given, "Column 1", "Column 2", ... is used
 * @throws InvalidColCount  if column count between exampleRows and colHeaders does not match
 */
std::shared_ptr<DataFrame> IVW_MODULE_DATAFRAME_API
createDataFrame(const std::vector<std::vector<std::string>> &exampleRows,
                const std::vector<std::string> &colHeaders = {});

template <typename T>
std::shared_ptr<TemplateColumn<T>> DataFrame::addColumn(const std::string &header, size_t size) {
    auto col = std::make_shared<TemplateColumn<T>>(header);
    col->getTypedBuffer()->getEditableRAMRepresentation()->getDataContainer().resize(size);
    columns_.push_back(col);
    return col;
}

template <>
struct DataTraits<DataFrame> {
    static std::string classIdentifier() { return "org.inviwo.DataFrame"; }
    static std::string dataName() { return "DataFrame"; }
    static uvec3 colorCode() { return uvec3(153, 76, 0); }
    static Document info(const DataFrame &data) {
        using H = utildoc::TableBuilder::Header;
        using P = Document::PathComponent;
        Document doc;
        doc.append("b", "DataFrame", {{"style", "color:white;"}});
        utildoc::TableBuilder tb(doc.handle(), P::end());
        tb(H("Number of Columns: "), data.getNumberOfColumns());
        tb(H("Number of Rows: "), data.getNumberOfRows());

        utildoc::TableBuilder tb2(doc.handle(), P::end());
        tb2(H("Col"), H("Format"), H("Rows"), H("Name"));
        // abbreviate list of columns if there are more than 20
        const size_t ncols = (data.getNumberOfColumns() > 20) ? 10 : data.getNumberOfColumns();

        for (size_t i = 0; i < ncols; i++) {
            tb2(std::to_string(i + 1), data.getColumn(i)->getBuffer()->getDataFormat()->getString(),
                data.getColumn(i)->getBuffer()->getSize(), data.getHeader(i));
        }
        if (ncols != data.getNumberOfColumns()) {
            doc.append("span", "... (" + std::to_string(data.getNumberOfColumns() - ncols) +
                                   " additional columns)");
        }

        return doc;
    }
};

}  // namespace inviwo

#endif  // IVW_DATAFRAME_H
