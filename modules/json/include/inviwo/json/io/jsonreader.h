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

#include <inviwo/json/jsonmoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/io/datareader.h>
#include <modules/plotting/datastructures/dataframe.h>

namespace inviwo {

/**
 * \class JSONReader
 * \ingroup dataio
 * Reads a json file into DataFrame
 */
class IVW_MODULE_JSON_API JSONReader : public DataReaderType<plot::DataFrame> {
public:
    JSONReader();
    JSONReader(const JSONReader&) = default;
    JSONReader(JSONReader&&) noexcept = default;
    JSONReader& operator=(const JSONReader&) = default;
    JSONReader& operator=(JSONReader&&) noexcept = default;
    virtual JSONReader* clone() const override;
    virtual ~JSONReader() = default;

    /**
     * read a JSON file from a file
     *
     * @param fileName   name of the input file
     * @return a plot::DataFrame containing the JSON data
     * @throws FileException if the file cannot be accessed
     */
    virtual std::shared_ptr<plot::DataFrame> readData(const std::string& fileName) override;

    /**
     * read a CSV file from a input stream, e.g. a std::ifstream. In case
     * file streams are used, the file must have be opened prior calling this function.
     *
     * @param stream    input stream with the json data
     * @return a plot::DataFrame containing the data
     */
    std::shared_ptr<plot::DataFrame> readData(std::istream& stream) const;
};

}  // namespace inviwo
