/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2022-2025 Inviwo Foundation
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

#include <inviwo/dataframe/io/xmlwriter.h>

#include <inviwo/core/datastructures/buffer/bufferram.h>                // for BufferRAM
#include <inviwo/core/datastructures/representationconverter.h>         // for RepresentationCon...
#include <inviwo/core/datastructures/representationconverterfactory.h>  // for RepresentationCon...
#include <inviwo/core/io/datawriter.h>                                  // for DataWriterType
#include <inviwo/core/io/serialization/serializer.h>                    // for Serializer
#include <inviwo/core/util/fileextension.h>                             // for FileExtension
#include <inviwo/dataframe/datastructures/dataframe.h>                  // for DataFrame

#include <fstream>        // for stringstream, bas...
#include <string>         // for basic_string
#include <type_traits>    // for remove_extent_t
#include <unordered_set>  // for unordered_set

namespace inviwo {

XMLWriter::XMLWriter() : DataWriterType<DataFrame>() {
    addExtension(FileExtension("xml", "XML file"));
}

XMLWriter* XMLWriter::clone() const { return new XMLWriter(*this); }

void XMLWriter::writeData(const DataFrame* data, const std::filesystem::path& filePath) const {
    auto f = open(filePath);
    writeData(data, f);
}

std::unique_ptr<std::vector<unsigned char>> XMLWriter::writeDataToBuffer(
    const DataFrame* data, std::string_view /*fileExtension*/) const {
    std::stringstream ss;
    writeData(data, ss);
    auto stringData = std::move(ss).str();
    return std::make_unique<std::vector<unsigned char>>(stringData.begin(), stringData.end());
}

void XMLWriter::writeData(const DataFrame* dataFrame, std::ostream& file) const {
    Serializer serializer{};

    for (const auto& col : *dataFrame) {
        if ((col == dataFrame->getIndexColumn()) && !exportIndexCol) {
            continue;
        }
        col->getBuffer()->getRepresentation<BufferRAM>()->dispatch<void>([&](auto br) {
            serializer.serialize(col->getHeader(), br->getDataContainer(), "Item");
        });
    }

    serializer.writeFile(file);
}
}  // namespace inviwo
