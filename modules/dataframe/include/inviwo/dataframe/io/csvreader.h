/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2021 Inviwo Foundation
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

#include <inviwo/dataframe/dataframemoduledefine.h>

#include <inviwo/core/io/datareader.h>
#include <inviwo/core/io/datareaderexception.h>
#include <inviwo/dataframe/datastructures/dataframe.h>

namespace inviwo {

/**
 * \class CSVReader
 * \ingroup dataio
 *
 * \brief A reader for comma separated value (CSV) files with customizable delimiters.
 * The default delimiter is ',' and headers are included. Floating point values are stored as
 * float32.
 */
class IVW_MODULE_DATAFRAME_API CSVReader : public DataReaderType<DataFrame> {
public:
    CSVReader(std::string_view delim = defaultDelimiters, bool hasHeader = defaultFirstRowHeader,
              bool doublePrecrecision = defaultDoublePrecision);
    CSVReader(const CSVReader&) = default;
    CSVReader(CSVReader&&) noexcept = default;
    CSVReader& operator=(const CSVReader&) = default;
    CSVReader& operator=(CSVReader&&) noexcept = default;
    virtual CSVReader* clone() const override;
    virtual ~CSVReader() = default;

    CSVReader& setDelimiters(const std::string& delim);
    const std::string& getDelimiters() const;
    
    CSVReader& setStripQuotes(bool stripQuotes);
    bool getStripQuotes() const;

    CSVReader& setFirstRowHeader(bool hasHeader);
    bool hasFirstRowHeader() const;

    CSVReader& setUnitsInHeaders(bool unitInHeaders);
    bool hasUnitsInHeaders() const;

    CSVReader& setUnitRegexp(std::string_view regexp);
    const std::string& getUnitRegexp() const;

    /**
     * sets the precision for columns containing floating point values. If \p doubleprec is true,
     * values are stored as double. Otherwise float32 is used.
     */
    CSVReader& setEnableDoublePrecision(bool doubleprec);
    bool hasDoublePrecision() const;

    CSVReader& setNumberOfExampleRows(size_t rows);
    size_t getNumberOfExamplesRows() const;

    CSVReader& setLocale(std::string_view loc);
    const std::string& getLocale() const;

    enum class EmptyField { Throw, DefaultConstruct, NanOrZero };

    CSVReader& setHandleEmptyFields(EmptyField emptyField);
    EmptyField getHandleEmptyFields() const;

    using DataReaderType<DataFrame>::readData;

    /**
     * read a CSV file from a file
     *
     * @param fileName   name of the input CSV file
     * @return a DataFrame containing the CSV data
     * @throws FileException if the file cannot be accessed
     * @throws DataReaderException if the file contains no data, the first row
     *   should hold column headers, but they cannot be found, or if there are
     *   unmatched quotes at the end of the file
     */
    virtual std::shared_ptr<DataFrame> readData(std::string_view fileName) override;

    /**
     * read a CSV file from a input stream, e.g. a std::ifstream. In case
     * file streams are used, the file must have be opened prior calling this function.
     *
     * @param stream    input stream with the CSV data
     * @return a DataFrame containing the CSV data
     * @throws DataReaderException if the given stream is in a bad state,
     *   the stream contains no data, the first row should hold column headers,
     *   but they cannot be found, or if there are unmateched quotes at the end of
     *   the stream
     */
    std::shared_ptr<DataFrame> readData(std::istream& stream) const;

    virtual bool setOption(std::string_view key, std::any value) override;
    virtual std::any getOption(std::string_view key) override;

    static constexpr std::string_view defaultDelimiters = ",";
    static constexpr bool defaultStripQuotes = true;
    static constexpr bool defaultFirstRowHeader = true;
    static constexpr bool defaultUnitInHeaders = true;
    static constexpr std::string_view defaultUnitRegexp = R"((.*)\[(.*)\])";
    static constexpr bool defaultDoublePrecision = false;
    static constexpr size_t defaultNumberOfExampleRows = 50;
    static constexpr std::string_view defaultLocale = "C";
    static constexpr EmptyField defaultEmptyField = EmptyField::NanOrZero;

private:
    struct CellType {
        size_t integer;
        size_t real;
        size_t string;
    };

    std::vector<CellType> findCellTypes(
        size_t nCol, const std::vector<std::pair<std::string_view, size_t>>& rows,
        size_t sampleRows) const;

    std::vector<std::function<void(std::string_view, size_t, size_t)>> addColumns(
        DataFrame& df, const std::vector<CellType>& types,
        const std::vector<std::string>& headers) const;

    std::string delimiters_;
    bool stripQuotes_;
    bool firstRowHeader_;
    bool unitsInHeaders_;
    std::string unitRegexp_;
    bool doublePrecision_;
    size_t exampleRows_;
    std::string locale_;
    EmptyField emptyField_;
};

}  // namespace inviwo
