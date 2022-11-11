/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2022 Inviwo Foundation
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

#include <inviwo/core/io/isovaluecollectioniivwriter.h>

namespace inviwo {


IsoValueCollectionIIVWriter::IsoValueCollectionIIVWriter() {
    addExtension({"iiv", "Inviwo Isovalues"});
}

IsoValueCollectionIIVWriter* IsoValueCollectionIIVWriter::clone() const {
    return new IsoValueCollectionIIVWriter(*this);
};

void IsoValueCollectionIIVWriter::writeData(const IsoValueCollection* data,
                                          std::string_view filePath) const {
    Serializer serializer(filePath);
    data->serialize(serializer);
    serializer.writeFile();
};

std::unique_ptr<std::vector<unsigned char>> IsoValueCollectionIIVWriter::writeDataToBuffer(
    const IsoValueCollection* data, std::string_view fileExtension) const {

    Serializer serializer{""};
    data->serialize(serializer);

    std::stringstream ss;
    serializer.writeFile(ss);
    auto xml = ss.str();

    auto buffer = std::make_unique<std::vector<unsigned char>>(xml.size());
    std::copy(xml.begin(), xml.end(), buffer->begin());
    return buffer;
};

}  // namespace inviwo
