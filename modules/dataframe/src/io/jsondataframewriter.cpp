/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2025 Inviwo Foundation
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

#include <inviwo/dataframe/io/jsondataframewriter.h>

#include <inviwo/core/util/exception.h>                 // for FileException
#include <inviwo/core/util/fileextension.h>             // for FileExtension
#include <inviwo/core/util/sourcecontext.h>             // for SourceContext
#include <inviwo/dataframe/datastructures/dataframe.h>  // for DataFrame
#include <inviwo/dataframe/jsondataframeconversion.h>   // IWYU pragma: keep

#include <fstream>  // for basic_ifstream, ios, istream, str...
#include <string>   // for operator==, fpos

#include <nlohmann/json.hpp>  // for operator>>, json

using json = nlohmann::json;

namespace inviwo {

JSONDataFrameWriter::JSONDataFrameWriter() {
    addExtension(FileExtension("json", "DataFrame in JavaScript Object Notation (JSON)"));
}
JSONDataFrameWriter* JSONDataFrameWriter::clone() const { return new JSONDataFrameWriter(*this); }

void JSONDataFrameWriter::writeData(const DataFrame* data,
                                    const std::filesystem::path& filePath) const {
    auto f = open(filePath);
    writeData(data, f);
}
std::unique_ptr<std::vector<unsigned char>> JSONDataFrameWriter::writeDataToBuffer(
    const DataFrame* data, std::string_view fileExtension) const {
    std::stringstream ss;
    writeData(data, ss);
    auto stringData = std::move(ss).str();
    return std::make_unique<std::vector<unsigned char>>(stringData.begin(), stringData.end());
}

void JSONDataFrameWriter::writeData(const DataFrame* data, std::ostream& os) const {
    if (data) {
        json j;
        j = *data;
        os << j;
    }
}

}  // namespace inviwo
