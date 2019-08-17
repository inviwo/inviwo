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

#include <inviwo/dataframe/io/jsonreader.h>
#include <inviwo/dataframe/jsondataframeconversion.h>
#include <inviwo/core/util/filesystem.h>

using json = nlohmann::json;

namespace inviwo {

JSONDataFrameReader::JSONDataFrameReader() {
    addExtension(FileExtension("json", "JavaScript Object Notation (JSON)"));
}

JSONDataFrameReader* JSONDataFrameReader::clone() const { return new JSONDataFrameReader(*this); }

std::shared_ptr<DataFrame> JSONDataFrameReader::readData(const std::string& fileName) {
    auto file = filesystem::ifstream(fileName);

    if (!file.is_open()) {
        throw FileException(
            std::string("JSONDataFrameReader: Could not open file \"" + fileName + "\"."),
            IVW_CONTEXT);
    }
    file.seekg(0, std::ios::end);
    std::streampos len = file.tellg();
    file.seekg(0, std::ios::beg);

    if (len == std::streampos(0)) {
        throw FileException("Empty file, no data", IVW_CONTEXT);
    }

    return readData(file);
}

std::shared_ptr<DataFrame> JSONDataFrameReader::readData(std::istream& stream) const {
    json j;
    stream >> j;
    auto dataFrame = std::make_shared<DataFrame>();
    *dataFrame = j;

    return dataFrame;
}

}  // namespace inviwo
