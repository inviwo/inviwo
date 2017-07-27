/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2017 Inviwo Foundation
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

#include <modules/plotting/plottingmoduledefine.h>
#include <modules/plotting/datastructures/datapoint.h>
#include <modules/plotting/datastructures/column.h>

#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/datastructures/buffer/buffer.h>
#include <inviwo/core/ports/datainport.h>
#include <inviwo/core/ports/dataoutport.h>
#include <inviwo/core/util/exception.h>
#include <unordered_map>

namespace inviwo {
class DataPointBase;
class BufferBase;
class BufferRAM;

namespace plot {

class IVW_MODULE_PLOTTING_API InvalidColCount : public Exception {
public:
    InvalidColCount(const std::string &message = "", ExceptionContext context = ExceptionContext())
        : Exception(message, context) {
    }
    virtual ~InvalidColCount() throw() {}
};
class IVW_MODULE_PLOTTING_API NoColumns : public Exception {
public:
    NoColumns(const std::string &message = "", ExceptionContext context = ExceptionContext())
        : Exception(message, context) {
    }
    virtual ~NoColumns() throw() {}
};
class IVW_MODULE_PLOTTING_API DataTypeMismatch : public Exception {
public:
    DataTypeMismatch(const std::string &message = "", ExceptionContext context = ExceptionContext())
        : Exception(message, context) {}
    virtual ~DataTypeMismatch() throw() {}
};

class IVW_MODULE_PLOTTING_API DataFrame {
public:
    using DataItem = std::vector<std::shared_ptr<DataPointBase>>;
    using LookupTable = std::unordered_map<glm::u64, std::string>;

    DataFrame(const DataFrame &df);

    DataFrame(std::uint32_t size = 0);
    virtual ~DataFrame() = default;

    std::shared_ptr<Column> addColumnFromBuffer(const std::string &identifier,
                                                std::shared_ptr<const BufferBase> buffer);

    template <typename T>
    std::shared_ptr<TemplateColumn<T>> addColumn(const std::string &header, size_t size = 0);

    std::shared_ptr<CategoricalColumn> addCategoricalColumn(const std::string &header,
                                                            size_t size = 0);
    /**
     * \brief add a new row given a vector of strings
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

    std::shared_ptr<Column> getColumn(size_t index);
    std::shared_ptr<const Column> getColumn(size_t index) const;

    std::shared_ptr<TemplateColumn<std::uint32_t>> getIndexColumn();
    std::shared_ptr<const TemplateColumn<std::uint32_t>> getIndexColumn() const;

    size_t getSize() const;
    size_t getNumberOfColumns() const;
    size_t getNumberOfRows() const;

    std::vector<std::shared_ptr<Column>>::iterator begin();
    std::vector<std::shared_ptr<Column>>::iterator end();
    std::vector<std::shared_ptr<Column>>::const_iterator begin() const;
    std::vector<std::shared_ptr<Column>>::const_iterator end() const;

    void updateIndexBuffer();

private:
    std::vector<std::shared_ptr<Column>> columns_;
};

using DataFrameOutport = DataOutport<DataFrame>;
using DataFrameInport = DataInport<DataFrame>;

/**
 * \brief create a new DataFrame given a vector of strings as input
 *
 * @param exampleData  data for guessing data type of each column
 * @param colHeaders   headers for the columns. If none given, "Column 1", "Column 2", ... is used
 * @throws InvalidColCount  if column count between exampleData and colHeaders does not match
 */
std::shared_ptr<DataFrame> IVW_MODULE_PLOTTING_API createDataFrame(
    const std::vector<std::string> &exampleData, const std::vector<std::string> &colHeaders = {});

template <typename T>
std::shared_ptr<TemplateColumn<T>> DataFrame::addColumn(const std::string &header, size_t size) {
    auto col = std::make_shared<TemplateColumn<T>>(header);
    col->getTypedBuffer()->getEditableRAMRepresentation()->getDataContainer().resize(size);
    columns_.push_back(col);
    return col;
}

}  // namespace plot

template <>
struct port_traits<plot::DataFrame> {
    static std::string class_identifier() { return "DataFrame"; }
    static uvec3 color_code() { return uvec3(153, 76, 0); }
    static std::string data_info(const plot::DataFrame *data) {
        using H = utildoc::TableBuilder::Header;
        using P = Document::PathComponent;
        Document doc;
        doc.append("b", "DataFrame", {{"style", "color:white;"}});
        utildoc::TableBuilder tb(doc.handle(), P::end());
        tb(H("Number of columns: "), data->getNumberOfColumns());

        for (size_t i = 0; i < data->getNumberOfColumns(); i++) {
            std::ostringstream oss;
            oss << "Column " << (i + 1) << ": " << data->getHeader(i);
            tb(H(oss.str()), "");

            tb("size", data->getColumn(i)->getBuffer()->getSize());
            tb("Dataformat", data->getColumn(i)->getBuffer()->getDataFormat()->getString());
        }

        return doc;
    }
};

}  // namespace inviwo

#endif  // IVW_DATAFRAME_H
