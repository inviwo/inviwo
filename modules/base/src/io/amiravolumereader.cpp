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

#include <modules/base/io/amiravolumereader.h>

#include <inviwo/core/io/datareaderexception.h>
#include <inviwo/core/datastructures/volume/volume.h>
#include <inviwo/core/datastructures/volume/volumeram.h>
#include <inviwo/core/io/bytereaderutil.h>
#include <inviwo/core/util/raiiutils.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/util/glmvec.h>
#include <inviwo/core/util/logcentral.h>
#include <inviwo/core/util/formats.h>
#include <inviwo/core/util/sourcecontext.h>
#include <inviwo/core/util/charconv.h>
#include <modules/base/algorithm/dataminmax.h>
#include <modules/base/io/amirameshutils.h>

#include <string>
#include <string_view>
#include <fmt/std.h>
#include <fmt/format.h>

namespace inviwo {

namespace {

/**
 * Parse the AmiraMesh Lattice definition and return the dimensions.
 *
 * @param defines   map containing all defines from the AmiraMesh header
 * @param path file path of the dataset
 * @return dataset dimensions
 * @throw DataReaderException if Lattice definition is missing or the dimensions are invalid
 */
ivec3 parseLatticeDefinition(const std::unordered_map<std::string_view, std::string_view>& defines,
                             const std::filesystem::path& path) {
    auto it = defines.find("Lattice");
    if (it == defines.end()) {
        throw DataReaderException(SourceContext{}, "Missing Lattice definition: {}", path);
    }
    const auto dim = amira::parseValue<ivec3>(it->second, ' ');
    if (glm::compMin(dim) <= 0) {
        throw DataReaderException(SourceContext{},
                                  "Invalid Lattice dimensions: 'define Lattice {}' ({})",
                                  it->second, path);
    }
    return dim;
}

}  // namespace

AmiraVolumeReader::AmiraVolumeReader() : DataReaderType<Volume>() {
    addExtension(FileExtension("am", "AMIRA volume reader"));
}

AmiraVolumeReader* AmiraVolumeReader::clone() const { return new AmiraVolumeReader(*this); }

std::shared_ptr<Volume> AmiraVolumeReader::readData(const std::filesystem::path& path) {
    const auto filePath = downloadAndCacheIfUrl(path);

    checkExists(filePath);

    const std::string data = readFileContents(filePath);

    amira::AmiraMeshHeader header;
    try {
        header = amira::parseAmiraMeshHeader(data);
    } catch (DataReaderException& e) {
        throw DataReaderException(e.getContext(), "{} ({})", e.what(), path);
    }

    if (header.format != amira::DataSectionFormat::Binary) {
        throw DataReaderException(SourceContext{}, "File is not a binary AmiraMesh file: {}", path);
    }
    if (header.version != "2.1" && header.version != "3.0") {
        throw DataReaderException(SourceContext{},
                                  "Unsupported AmiraMesh version '{}', expected '2.1' or '3.0': {}",
                                  header.version, path);
    }

    // verify volume parameters
    if (auto it = header.parameters.find("CoordType"); it == header.parameters.end()) {
        throw DataReaderException(SourceContext{}, "Missing CoordType: {}", path);
    } else if (!iCaseCmp(it->second, "uniform")) {
        throw DataReaderException(SourceContext{},
                                  "Unsupported CoordType '{}', expected 'uniform': {}", it->second,
                                  path);
    }

    const ivec3 dim = parseLatticeDefinition(header.defines, path);
    auto [bboxMin, bboxMax] = amira::getBoundingBox(header.parameters);

    const auto& dataSpec = [&]() {
        if (auto it = header.dataSpecs.find("Data"); it == header.dataSpecs.end()) {
            throw DataReaderException(SourceContext{}, "Missing data specifier for Lattice: {}",
                                      path);
        } else {
            return it->second;
        }
    }();

    if (dataSpec.name != "Lattice") {
        throw DataReaderException(SourceContext{},
                                  "Data format specified for '{}', expected 'Lattice': {}",
                                  dataSpec.name, path);
    }

    const auto* format = DataFormatBase::get(
        dataSpec.format.numericType, dataSpec.format.numComponents, dataSpec.format.precision);

    // locate data section and data matching the lattice identifier
    size_t offset = 0;
    if (auto dpos = data.find("# Data section follows"); dpos != std::string_view::npos) {
        const std::string id = fmt::format("@{}", dataSpec.identifier);
        if (auto pos = data.find(id, dpos); pos != std::string_view::npos) {
            // data starts after the lattice identifier followed by a newline character
            offset = data.find('\n', pos) + 1;
        } else {
            throw DataReaderException(SourceContext{}, "Data for Lattice @{} not found: {}",
                                      dataSpec.identifier, path);
        }
    } else {
        throw DataReaderException(SourceContext{}, "Data section not found: {}", path);
    }

    const size_t bytesToRead = format->getSizeInBytes() * glm::compMul(size3_t{dim});
    if (offset + bytesToRead > data.size()) {
        throw DataReaderException(SourceContext{}, "Premature end of file in data section: {}",
                                  path);
    }

    auto volumeRam = createVolumeRAM(size3_t{dim}, format);
    std::copy(data.data() + offset, data.data() + offset + bytesToRead,
              static_cast<char*>(volumeRam->getData()));

    if (header.byteOrder == ByteOrder::BigEndian && format->getSizeInBytes() > 1) {
        util::reverseByteOrder(volumeRam->getData(), bytesToRead, format->getSizeInBytes());
    }

    auto volume = std::make_shared<Volume>(volumeRam);
    volume->setBasis(glm::scale(bboxMax - bboxMin));
    volume->setOffset(bboxMin);

    auto [min, max] = util::volumeMinMax(volumeRam.get());
    auto compMinMax = dvec2{glm::compMin(min), glm::compMax(max)};
    volume->dataMap.dataRange = compMinMax;
    volume->dataMap.valueRange = compMinMax;

    log::info("Loaded AmiraMesh volume: {}, dimensions: {}", filePath, dim);

    return volume;
}

}  // namespace inviwo
