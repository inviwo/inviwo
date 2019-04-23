/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2019 Inviwo Foundation
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

#ifndef IVW_CSVREADER_H
#define IVW_CSVREADER_H

#include <inviwo/dataframe/dataframemoduledefine.h>
#include <inviwo/core/common/inviwo.h>

#include <inviwo/core/io/datareader.h>
#include <inviwo/core/io/datareaderexception.h>
#include <inviwo/dataframe/datastructures/dataframe.h>

namespace inviwo {

/**
 * \class CSVDataReaderException
 *
 * \brief This exception is thrown by the CSVReader in case the input is malformed.
 * This includes empty sources, unmatched quotes, missing headers.
 * \see CSVReader
 */
class IVW_MODULE_DATAFRAME_API CSVDataReaderException : public DataReaderException {
public:
    CSVDataReaderException(const std::string& message = "",
                           ExceptionContext context = ExceptionContext());
};

/**
 * \class CSVReader
 * \ingroup dataio
 *
 * \brief A reader for comma separated value (CSV) files with customizable delimiters.
 * The default delimiter is ',' and headers are included
 */
class IVW_MODULE_DATAFRAME_API CSVReader : public DataReaderType<DataFrame> {
public:
    CSVReader();
    CSVReader(const CSVReader&) = default;
    CSVReader(CSVReader&&) noexcept = default;
    CSVReader& operator=(const CSVReader&) = default;
    CSVReader& operator=(CSVReader&&) noexcept = default;
    virtual CSVReader* clone() const override;
    virtual ~CSVReader() = default;

    void setDelimiters(const std::string& delim);
    void setFirstRowHeader(bool hasHeader);
    using DataReaderType<DataFrame>::readData;

    /**
     * read a CSV file from a file
     *
     * @param fileName   name of the input CSV file
     * @return a DataFrame containing the CSV data
     * @throws FileException if the file cannot be accessed
     * @throws CSVDataReaderException if the file contains no data, the first row
     *   should hold column headers, but they cannot be found, or if there are
     *   unmatched quotes at the end of the file
     */
    virtual std::shared_ptr<DataFrame> readData(const std::string& fileName) override;

    /**
     * read a CSV file from a input stream, e.g. a std::ifstream. In case
     * file streams are used, the file must have be opened prior calling this function.
     *
     * @param stream    input stream with the CSV data
     * @return a DataFrame containing the CSV data
     * @throws CSVDataReaderException if the given stream is in a bad state,
     *   the stream contains no data, the first row should hold column headers,
     *   but they cannot be found, or if there are unmateched quotes at the end of
     *   the stream
     */
    std::shared_ptr<DataFrame> readData(std::istream& stream) const;

private:
    std::string delimiters_;
    bool firstRowHeader_;
};

}  // namespace inviwo

#endif  // IVW_CSVREADER_H
