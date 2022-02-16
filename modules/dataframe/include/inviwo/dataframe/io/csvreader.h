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
#include <inviwo/dataframe/util/filters.h>

namespace inviwo {

/**
 * \class CSVReader
 * \ingroup dataio
 *
 * \brief A reader for comma separated value (CSV) files with customizable delimiters and filters.
 * The default delimiter is ',' and headers are included. Floating point values are stored as
 * float32 unless double precision is enabled.
 */
class IVW_MODULE_DATAFRAME_API CSVReader : public DataReaderType<DataFrame> {
public:
    CSVReader(std::string_view delimiters = defaultDelimiters,
              bool hasHeader = defaultFirstRowHeader,
              bool doublePrecision = defaultDoublePrecision);
    CSVReader(const CSVReader&) = default;
    CSVReader(CSVReader&&) noexcept = default;
    CSVReader& operator=(const CSVReader&) = default;
    CSVReader& operator=(CSVReader&&) noexcept = default;
    virtual CSVReader* clone() const override;
    virtual ~CSVReader() = default;

    /** @see CSVReader::defaultDelimiters */
    CSVReader& setDelimiters(const std::string& delim);
    const std::string& getDelimiters() const;

    /** @see CSVReader::defaultStripQuotes */
    CSVReader& setStripQuotes(bool stripQuotes);
    bool getStripQuotes() const;

    /** @see CSVReader::defaultFirstRowHeader */
    CSVReader& setFirstRowHeader(bool hasHeader);
    bool hasFirstRowHeader() const;

    /** @see CSVReader::defaultUnitInHeaders */
    CSVReader& setUnitsInHeaders(bool unitInHeaders);
    bool hasUnitsInHeaders() const;

    /** @see CSVReader::defaultUnitRegexp */
    CSVReader& setUnitRegexp(std::string_view regexp);
    const std::string& getUnitRegexp() const;

    /**
     * Sets the precision for columns containing floating point values. If @p useDoublePrecision is
     * true, values are stored as double (64 bits), otherwise float (32 bits) is used.
     * @see CSVReader::defaultDoublePrecision
     */
    CSVReader& setEnableDoublePrecision(bool useDoublePrecision);
    bool hasDoublePrecision() const;

    /** @see CSVReader::defaultNumberOfExampleRows */
    CSVReader& setNumberOfExampleRows(size_t rows);
    size_t getNumberOfExamplesRows() const;

    /** @see CSVReader::defaultLocale */
    CSVReader& setLocale(std::string_view loc);
    const std::string& getLocale() const;

    /**
     * Sets row and column filters.
     * @see Filters
     */
    CSVReader& setFilters(const csvfilters::Filters& filters);
    const csvfilters::Filters& getFilters() const;

    /**
     * How to handle missing / empty data
     */
    enum class EmptyField {
        Throw,        ///< Throw a DataReaderException
        EmptyOrZero,  ///< "" for categorical and 0 for numerical values
        NanOrZero     ///< "" for categorical, NaN for floating point, and 0 for integer values
    };

    /** @see CSVReader::defaultEmptyField */
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
     *   but they cannot be found, or if there are unmatched quotes at the end of
     *   the stream
     */
    std::shared_ptr<DataFrame> readData(std::istream& stream) const;

    /**
     * Set any of the settings supported by the reader, supported keys:
     * * Delimiters (string)
     * * StripQuotes (bool)
     * * FirstRowHeader (bool)
     * * UnitsInHeaders (bool)
     * * UnitRegexp (sting)
     * * DoublePrecision (bool)
     * * NumberOfExampleRows (size_t)
     * * Locale (string)
     * * HandleEmptyFields (EmptyField)
     */
    virtual bool setOption(std::string_view key, std::any value) override;

    /**
     * Get any of the settings supported by the reader, supported keys:
     * * Delimiters (string)
     * * StripQuotes (bool)
     * * FirstRowHeader (bool)
     * * UnitsInHeaders (bool)
     * * UnitRegexp (sting)
     * * DoublePrecision (bool)
     * * NumberOfExampleRows (size_t)
     * * Locale (string)
     * * HandleEmptyFields (EmptyField)
     */
    virtual std::any getOption(std::string_view key) override;

    /** @see CSVReader::setDelimiters */
    static constexpr std::string_view defaultDelimiters = ",";
    /** @see CSVReader::setStripQuotes */
    static constexpr bool defaultStripQuotes = true;
    /** @see CSVReader::setFirstRowHeader */
    static constexpr bool defaultFirstRowHeader = true;
    /** @see CSVReader::setUnitInHeaderss */
    static constexpr bool defaultUnitInHeaders = true;
    /** @see CSVReader::setUnitRegexp */
    static constexpr std::string_view defaultUnitRegexp = R"((.*)\[(.*)\])";
    /** @see CSVReader::setDoublePrecision */
    static constexpr bool defaultDoublePrecision = false;
    /** @see CSVReader::setNumberOfExampleRows */
    static constexpr size_t defaultNumberOfExampleRows = 50;
    /** @see CSVReader::setLocale */
    static constexpr std::string_view defaultLocale = "C";
    /** @see CSVReader::setHandleEmptyFields */
    static constexpr EmptyField defaultEmptyField = EmptyField::NanOrZero;

private:
    struct TypeCounts {
        size_t integer;
        size_t real;
        size_t string;
    };

    std::vector<TypeCounts> findCellTypes(
        size_t nCol, const std::vector<std::pair<std::string_view, size_t>>& rows,
        size_t sampleRows) const;

    std::vector<std::function<void(std::string_view, size_t, size_t)>> addColumns(
        DataFrame& df, const std::vector<TypeCounts>& types,
        const std::vector<std::string>& headers) const;

    bool skipRow(std::string_view row, size_t lineNumber, bool filterOnHeader) const;

    std::string delimiters_;
    bool stripQuotes_;
    bool firstRowHeader_;
    bool unitsInHeaders_;
    std::string unitRegexp_;
    bool doublePrecision_;
    size_t exampleRows_;
    std::string locale_;
    EmptyField emptyField_;
    csvfilters::Filters filters_;
};

}  // namespace inviwo
