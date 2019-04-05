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

#include <inviwo/json/io/jsonreader.h>
#include <inviwo/json/jsonutils.h>

using json = nlohmann::json;

namespace inviwo {

JSONReader* JSONReader::clone() const { return new CSVReader(*this); }

std::shared_ptr<plot::DataFrame> JSONReader::JSONReader(const std::string& fileName) {
    auto file = filesystem::ifstream(fileName);

    if (!file.is_open()) {
        throw FileException(std::string("JSONReader: Could not open file \"" + fileName + "\"."),
                            IvwContext);
    }
    file.seekg(0, std::ios::end);
    std::streampos len = file.tellg();
    file.seekg(0, std::ios::beg);

    if (len == std::streampos(0)) {
        throw FileException("Empty file, no data", IvwContext);
    }

    return readData(file);
}

std::shared_ptr<plot::DataFrame> JSONReader::readData(std::istream& stream) const {
    // Skip BOM if it exists. 
    filesystem::skipByteOrderMark(stream);

    if (stream.bad() || stream.fail()) {
        throw FileException("Input stream in a bad state", IvwContext);
    }
    json j;
    stream >> j;
    auto dataFrame = std::make_shared<plot::DataFrame>();
    *dataFrame = j;

    return dataFrame;
}

}  // namespace inviwo
