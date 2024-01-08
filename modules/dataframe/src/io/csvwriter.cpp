/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2022-2024 Inviwo Foundation
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

#include <inviwo/dataframe/io/csvwriter.h>

#include <inviwo/core/datastructures/buffer/bufferram.h>                // for BufferRAM
#include <inviwo/core/datastructures/representationconverter.h>         // for RepresentationCon...
#include <inviwo/core/datastructures/representationconverterfactory.h>  // for RepresentationCon...
#include <inviwo/core/io/datawriter.h>                                  // for DataWriterType
#include <inviwo/core/util/fileextension.h>                             // for FileExtension
#include <inviwo/core/util/formatdispatching.h>                         // for Vecs, PrecisionVa...
#include <inviwo/core/util/formats.h>                                   // for DataFormatBase
#include <inviwo/core/util/glmutils.h>                                  // for flat_extent
#include <inviwo/core/util/ostreamjoiner.h>                             // for make_ostream_joiner
#include <inviwo/dataframe/datastructures/column.h>                     // for CategoricalColumn
#include <inviwo/dataframe/datastructures/dataframe.h>                  // for DataFrame

#include <array>          // for array
#include <cstddef>        // for size_t
#include <fstream>        // for operator<<, ostream
#include <functional>     // for __base, function
#include <type_traits>    // for decay_t, remove_e...
#include <unordered_set>  // for unordered_set
#include <utility>        // for move

#include <fmt/core.h>      // for basic_string_view
#include <glm/gtx/io.hpp>  // for operator<<
#include <half/half.hpp>   // for operator<<

namespace inviwo {

CSVWriter::CSVWriter() : DataWriterType<DataFrame>() {
    addExtension(FileExtension("csv", "Comma separated values"));
}

CSVWriter* CSVWriter::clone() const { return new CSVWriter(*this); }

void CSVWriter::writeData(const DataFrame* data, const std::filesystem::path& filePath) const {
    auto f = open(filePath);
    writeData(data, f);
}

std::unique_ptr<std::vector<unsigned char>> CSVWriter::writeDataToBuffer(
    const DataFrame* data, std::string_view /*fileExtension*/) const {
    std::stringstream ss;
    writeData(data, ss);
    auto stringData = std::move(ss).str();
    return std::make_unique<std::vector<unsigned char>>(stringData.begin(), stringData.end());
}

void CSVWriter::writeData(const DataFrame* dataFrame, std::ostream& file) const {

    const std::string citation = quoteStrings ? "\"" : "";
    const char lineTerminator = '\n';
    const std::array<char, 4> componentNames = {'X', 'Y', 'Z', 'W'};

    // headers
    auto oj = util::make_ostream_joiner(file, delimiter);
    for (const auto& col : *dataFrame) {
        if ((col->getColumnType() == ColumnType::Index) && !exportIndexCol) {
            continue;
        }
        const auto components = col->getBuffer()->getDataFormat()->getComponents();
        if (components > 1 && separateVectorTypesIntoColumns) {
            for (size_t k = 0; k < components; k++) {
                oj = fmt::format("{0}{1} {2}{3: [}{0}", citation, col->getHeader(),
                                 componentNames[k], col->getUnit());
            }
        } else {
            oj = fmt::format("{0}{1}{2: [}{0}", citation, col->getHeader(), col->getUnit());
        }
    }
    file << lineTerminator;

    std::vector<std::function<void(std::ostream&, size_t)>> printers;
    for (const auto& col : *dataFrame) {
        if ((col->getColumnType() == ColumnType::Index) && !exportIndexCol) {
            continue;
        }
        auto df = col->getBuffer()->getDataFormat();
        if (auto cc = dynamic_cast<const CategoricalColumn*>(col.get())) {
            printers.push_back([cc, citation](std::ostream& os, size_t index) {
                os << citation << cc->getAsString(index) << citation;
            });
        } else if (df->getComponents() == 1) {
            col->getBuffer()
                ->getRepresentation<BufferRAM>()
                ->dispatch<void, dispatching::filter::Scalars>([&printers](auto br) {
                    printers.push_back([br](std::ostream& os, size_t index) {
                        os << br->getDataContainer()[index];
                    });
                });
        } else if (df->getComponents() > 1 && separateVectorTypesIntoColumns) {
            col->getBuffer()
                ->getRepresentation<BufferRAM>()
                ->dispatch<void, dispatching::filter::Vecs>([&printers, this](auto br) {
                    using ValueType = util::PrecisionValueType<decltype(br)>;
                    printers.push_back([br, this](std::ostream& os, size_t index) {
                        auto oj = util::make_ostream_joiner(os, delimiter);
                        for (size_t i = 0; i < util::flat_extent<ValueType>::value; ++i) {
                            oj = br->getDataContainer()[index][i];
                        }
                    });
                });
        } else {
            col->getBuffer()
                ->getRepresentation<BufferRAM>()
                ->dispatch<void, dispatching::filter::Vecs>([&printers, citation](auto br) {
                    printers.push_back([br, citation](std::ostream& os, size_t index) {
                        os << citation << br->getDataContainer()[index] << citation;
                    });
                });
        }
    }

    for (size_t j = 0; j < dataFrame->getNumberOfRows(); j++) {
        if (j != 0) {
            file << lineTerminator;
        }
        bool firstCol = true;
        for (auto& printer : printers) {
            if (!firstCol) {
                file << delimiter;
            }
            firstCol = false;
            printer(file, j);
        }
    }
}

}  // namespace inviwo
