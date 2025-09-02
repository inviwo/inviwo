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
#include <inviwo/core/util/raiiutils.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/util/glmvec.h>
#include <inviwo/core/util/logcentral.h>
#include <inviwo/core/util/formats.h>
#include <inviwo/core/util/sourcecontext.h>
#include <modules/base/algorithm/dataminmax.h>

#include <cstdio>
#include <string>
#include <string_view>
#include <fmt/std.h>
#include <fmt/format.h>

namespace inviwo {

namespace {

/**
 * Parse the AmiraMesh Lattice definition and return the dimensions.
 *
 * @param str  string containing the Lattice definition, i.e. "define Lattice x y z"
 * @param path file path of the dataset
 * @return dataset dimensions
 */
ivec3 parseLatticeDefinition(std::string_view str, const std::filesystem::path& path) {
    // Find the Lattice definition, i.e., the dimensions of the uniform grid
    if (auto pos = str.find("define Lattice"); pos != std::string_view::npos) {
        auto substr = str.substr(pos, str.find('\n') - pos);
        ivec3 dim{0};
        if (sscanf(substr.data() + 14, "%d %d %d", &dim.x, &dim.y, &dim.z) != 3) {
            throw DataReaderException(SourceContext{}, "Lattice definition is not valid: {:?} ({})",
                                      substr, path);
        }
        if (glm::compMin(dim) <= 0) {
            throw DataReaderException(SourceContext{}, "Invalid Lattice dimensions: {:?} ({})",
                                      substr, path);
        }
        return dim;
    } else {
        throw DataReaderException(SourceContext{}, "Lattice definition not found: {}", path);
    }
}

/**
 * Parse the AmiraMesh Parameters and return the bounding box. Also ensure it is a uniform grid.
 *
 * @param str  string containing the entire Parameters data structure, i.e. "Parameters { ... }"
 * @param path file path of the dataset
 * @return min and max values of the bounding box
 */
std::pair<vec3, vec3> parseParameters(std::string_view str, const std::filesystem::path& path) {

    if (auto ppos = str.find("Parameters {"); ppos != std::string_view::npos) {
        auto params = str.substr(ppos, str.find('}') - ppos);
        params.remove_prefix(12);

        vec3 bboxMin{-1.0f};
        vec3 bboxMax{1.0f};
        if (auto pos = params.find("BoundingBox"); pos != std::string_view::npos) {
            auto substr = params.substr(pos, params.find('\n') - pos);
            if (sscanf(substr.data() + 11, "%g %g %g %g %g %g", &bboxMin.x, &bboxMax.x, &bboxMin.y,
                       &bboxMax.y, &bboxMin.z, &bboxMax.z) != 6) {
                throw DataReaderException(SourceContext{}, "BoundingBox is not valid: {:?} ({})",
                                          substr, path);
            }
            if (glm::any(glm::greaterThan(bboxMin, bboxMax))) {
                throw DataReaderException(SourceContext{}, "Invalid BoundingBox: {:?} ({})", substr,
                                          path);
            }
        } else {
            throw DataReaderException(SourceContext{}, "BoundingBox not found: {}", path);
        }

        if (!params.contains(R"(CoordType "uniform")")) {
            throw DataReaderException(SourceContext{},
                                      "Unsupported/missing CoordType, expected uniform: {}", path);
        }

        return {bboxMin, bboxMax};
    } else {
        throw DataReaderException(SourceContext{}, "Parameters not found: {}", path);
    }
}

/**
 * Extract data format from AmiraMesh Lattice string.
 *
 * @param str  string corresponding to the part of Lattice enclosed by {}, e.g. "float[2] Data" in
 *             "Lattice { float Data } @1"
 * @param path file path of the dataset
 * @return DataFormat matching the Lattice data structure
 */
const DataFormatBase* getLatticeDataFormat(std::string_view str,
                                           const std::filesystem::path& path) {
    using namespace std::string_view_literals;
    using enum NumericType;

    auto tokens = util::splitStringView(str);
    if (tokens.size() < 2) {
        throw DataReaderException(SourceContext{}, "Invalid data format in Lattice: {:?} ({})", str,
                                  path);
    }

    // check for data dimensionality
    int dims = 1;
    if (auto pos = tokens[0].find('[');
        pos != std::string_view::npos && sscanf(tokens[0].data() + pos + 1, "%d", &dims) < 1) {
        throw DataReaderException(SourceContext{}, "Invalid data dimensions in Lattice: {:?} ({})",
                                  str, path);
    }

    const auto components = static_cast<size_t>(dims);
    if (tokens[0].starts_with("byte"sv)) {
        return DataFormatBase::get(UnsignedInteger, components, 8);
    } else if (tokens[0].starts_with("short"sv)) {
        return DataFormatBase::get(SignedInteger, components, 16);
    } else if (tokens[0].starts_with("ushort"sv)) {
        return DataFormatBase::get(UnsignedInteger, components, 16);
    } else if (tokens[0].starts_with("int"sv)) {
        return DataFormatBase::get(SignedInteger, components, 32);
    } else if (tokens[0].starts_with("uint"sv)) {
        return DataFormatBase::get(UnsignedInteger, components, 32);
    } else if (tokens[0].starts_with("float"sv)) {
        return DataFormatBase::get(Float, components, 32);
    } else if (tokens[0].starts_with("double"sv)) {
        return DataFormatBase::get(Float, components, 64);
    } else {
        throw DataReaderException(SourceContext{}, "Invalid data format in Lattice: {:?} ({})", str,
                                  path);
    }
}

/**
 * Parse the first AmiraMesh Lattice and return the corresponding DataFormat and lattice identifier.
 *
 * @param str  string containing the entire Lattice data structure, i.e. "Lattice { byte Data } @1"
 * @param path file path of the dataset
 * @return DataFormat matching the Lattice data structure and identifier
 */
std::pair<const DataFormatBase*, int> parseLattice(std::string_view str,
                                                   const std::filesystem::path& path) {
    if (auto lpos = str.find("Lattice {"); lpos != std::string_view::npos) {
        const size_t latticeEnd = str.find('}');

        auto lattice = str.substr(lpos, latticeEnd - lpos - 1);
        lattice.remove_prefix(9);

        auto format = getLatticeDataFormat(util::trim(lattice), path);

        int latticeIdentifier = 1;
        if (auto pos = str.find('@', latticeEnd); pos != std::string_view::npos) {
            auto substr = str.substr(pos, str.find('\n', latticeEnd) - pos);
            sscanf(substr.data() + 1, "%d", &latticeIdentifier);
        } else {
            throw DataReaderException(SourceContext{}, "Lattice ID not found: {}", path);
        }
        return {format, latticeIdentifier};
    } else {
        throw DataReaderException(SourceContext{}, "Lattice information not found: {}", path);
    }
}

}  // namespace

AmiraVolumeReader::AmiraVolumeReader() : DataReaderType<Volume>() {
    addExtension(FileExtension("am", "AMIRA volume reader"));
}

AmiraVolumeReader* AmiraVolumeReader::clone() const { return new AmiraVolumeReader(*this); }

std::shared_ptr<Volume> AmiraVolumeReader::readData(const std::filesystem::path& path) {
    const auto filePath = downloadAndCacheIfUrl(path);

    checkExists(filePath);

    FILE* file = filesystem::fopen(filePath, "rb");
    if (!file) {
        throw DataReaderException(SourceContext{}, "Could not open file: {}", path);
    }
    const util::OnScopeExit closeFile{[file]() { std::fclose(file); }};

    std::vector<char> buffer;

    // Get the file size, so we can pre-allocate the string. HUGE speed impact.
    long length = 0;
    fseek(file, 0, SEEK_END);
    length = ftell(file);
    fseek(file, 0, SEEK_SET);

    if (length <= 0) {
        throw DataReaderException(SourceContext{}, "Empty AmiraMesh file: {}", path);
    }

    std::pmr::string data(length, '0');
    if (std::fread(data.data(), length, 1, file) != 1) {
        throw DataReaderException(SourceContext{}, "Could not read AmiraMesh file: {}", path);
    }
    std::string_view view = data;

    if (!view.starts_with("# AmiraMesh BINARY-LITTLE-ENDIAN 2.1")) {
        throw DataReaderException(SourceContext{}, "File is not an AmiraMesh file: {}", path);
    }

    const ivec3 dim = parseLatticeDefinition(view, path);
    auto [bboxMin, bboxMax] = parseParameters(view, path);
    auto [format, latticeIdentifier] = parseLattice(view, path);

    // locate data section and data matching the lattice identifier
    size_t offset = 0;
    if (auto dpos = view.find("# Data section follows"); dpos != std::string_view::npos) {
        const std::string id = fmt::format("@{}", latticeIdentifier);
        if (auto pos = view.find(id, dpos); pos != std::string_view::npos) {
            // data starts after the lattice identifier followed by a newline character
            offset = pos + id.size() + 1;
        } else {
            throw DataReaderException(SourceContext{}, "Data for Lattice @{} not found: {}",
                                      latticeIdentifier, path);
        }
    } else {
        throw DataReaderException(SourceContext{}, "Data section not found: {}", path);
    }

    const size_t bytesToRead = format->getSizeInBytes() * glm::compMul(size3_t{dim});
    if (offset + bytesToRead > length) {
        throw DataReaderException(SourceContext{}, "Premature end of file in data section: {}",
                                  path);
    }

    auto volumeRam = createVolumeRAM(size3_t{dim}, format);
    std::copy(data.data() + offset, data.data() + offset + bytesToRead,
              static_cast<char*>(volumeRam->getData()));

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
