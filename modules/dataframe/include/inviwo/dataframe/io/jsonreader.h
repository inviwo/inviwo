/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019 Inviwo Foundation
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
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/io/datareader.h>
#include <inviwo/dataframe/datastructures/dataframe.h>

namespace inviwo {

/**
 * \class JSONDataFrameReader
 * \ingroup dataio
 * Reads a json file into DataFrame
 * Expects object layout:
 * [ {"Col1": val11, "Col2": val12 },
 *   {"Col1": val21, "Col2": val22 } ]
 * The example above contains two rows and two columns.
 */
class IVW_MODULE_DATAFRAME_API JSONDataFrameReader : public DataReaderType<DataFrame> {
public:
    JSONDataFrameReader();
    JSONDataFrameReader(const JSONDataFrameReader&) = default;
    JSONDataFrameReader(JSONDataFrameReader&&) noexcept = default;
    JSONDataFrameReader& operator=(const JSONDataFrameReader&) = default;
    JSONDataFrameReader& operator=(JSONDataFrameReader&&) noexcept = default;
    virtual JSONDataFrameReader* clone() const override;
    virtual ~JSONDataFrameReader() = default;
    using DataReaderType<DataFrame>::readData;

    /**
     * read a JSON file from a file
     * Expects object layout:
     * [ {"Col1": val11, "Col2": val12 },
     *   {"Col1": val21, "Col2": val22 } ]
     * The example above contains two rows and two columns.
     *
     * @param fileName   name of the input file
     * @return a DataFrame containing the JSON data
     * @throws FileException if the file cannot be accessed
     */
    virtual std::shared_ptr<DataFrame> readData(const std::string& fileName) override;

    /**
     * read DataFrame from a JSON-encoded input stream, e.g. a std::ifstream. In case
     * file streams are used, the file must have be opened prior calling this function.
     * Expects object layout:
     * [ {"Col1": val11, "Col2": val12 },
     *   {"Col1": val21, "Col2": val22 } ]
     * The example above contains two rows and two columns.
     *
     * @param stream    input stream with the json data
     * @return a DataFrame containing the data
     */
    std::shared_ptr<DataFrame> readData(std::istream& stream) const;
};

}  // namespace inviwo
