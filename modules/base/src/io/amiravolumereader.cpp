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
#include <inviwo/core/util/stringconversion.h>
#include <inviwo/core/util/stdextensions.h>
#include <inviwo/core/util/glmvec.h>
#include <inviwo/core/util/logcentral.h>
#include <inviwo/core/util/formats.h>
#include <inviwo/core/util/sourcecontext.h>
#include <modules/base/algorithm/dataminmax.h>

#include <array>
#include <string>
#include <string_view>
#include <fmt/std.h>
#include <fmt/format.h>
#include <charconv>

#if !(defined(__cpp_lib_to_chars) && __cpp_lib_to_chars >= 201611L)
#include <fast_float/fast_float.h>
#endif

#pragma optimize("", off)

namespace inviwo {

namespace {

struct DataSpec {
    std::string_view type;
    const DataFormatBase* format = nullptr;
    int identifier = -1;
};

/**
 * Split the string @p str by @p delimiter into most @t numItems. If there are more than @t numItems
 * parts, the last result entry will contain the remaining string.
 */
template <size_t numItems>
std::array<std::string_view, numItems> split(std::string_view str, char delimiter) {
    std::array<std::string_view, numItems> output;
    size_t first = 0;
    size_t index = 0;
    while (first < str.size()) {
        const auto second = str.find_first_of(delimiter, first);
        if (first != second) {
            if (index + 1 < numItems) {
                output[index] = str.substr(first, second - first);
            } else {
                output[index] = util::rtrim(str.substr(first));
            }
            ++index;
        }
        if (second == std::string_view::npos || index >= numItems) break;
        first = second + 1;
    }

    return output;
}

template <class T>
void fromStr(std::string_view value, T& dest) {
    const auto end = value.data() + value.size();

#if defined(__cpp_lib_to_chars) && __cpp_lib_to_chars >= 201611L
    auto [p, ec] = std::from_chars(value.data(), end, dest);
#else
    auto [p, ec] = fast_float::from_chars(value.data(), end, dest);
#endif
    if (ec != std::errc() || p != end) {
        if constexpr (std::is_same_v<double, T> || std::is_same_v<float, T>) {
            if (value == "inf") {
                dest = std::numeric_limits<T>::infinity();
            } else if (value == "-inf") {
                dest = -std::numeric_limits<T>::infinity();
            } else if (value == "nan") {
                dest = std::numeric_limits<T>::quiet_NaN();
            } else if (value == "-nan" || value == "-nan(ind)") {
                dest = -std::numeric_limits<T>::quiet_NaN();
            } else {
                throw DataReaderException(SourceContext{},
                                          "Error parsing floating point number ({})", value);
            }
        } else {
            throw DataReaderException(SourceContext{}, "Error parsing number ({})", value);
        }
    }
}

/**
 * Parse the AmiraMesh Parameters block delimited by parentheses { and }.
 *
 * @parameters  map holding all parameters
 * @param header   string containing the entire header including "Parameters { ... }"
 * @param first index in @p str
 * @param path  file path of the dataset
 * @return position of line ending behind closing parenthesis.
 */
size_t parseParameters(std::unordered_map<std::string_view, std::string_view>& parameters,
                       std::string_view header, size_t first, const std::filesystem::path& path) {
    size_t paramsStart = header.find('{', first);
    size_t paramsEnd = header.find('}', paramsStart);
    if (paramsEnd == std::string_view::npos) {
        throw DataReaderException(SourceContext{}, "Invalid Parameters block, end not found: {}",
                                  path);
    }

    first = paramsStart + 1;
    size_t second = first;
    while (first < paramsEnd) {
        second = header.find_first_of("\r\n", first);
        std::string_view line = header.substr(first, second - first);

        auto [str, _] = util::splitByFirst(line, '#');

        str = util::trim(str);
        if (str.ends_with(',')) {
            str.remove_suffix(1);
        }

        if (auto [key, value] = split<2>(str, ' '); !key.empty()) {
            if (value.empty()) {
                throw DataReaderException(SourceContext{}, "Invalid parameter found: {} ({})", line,
                                          path);
            }
            if (value.starts_with('"') && value.ends_with('"')) {
                value.remove_prefix(1);
                value.remove_suffix(1);
            }
            if (!parameters.try_emplace(key, value).second) {
                throw DataReaderException(SourceContext{}, "Duplicate parameter found: {} ({})",
                                          line, path);
            }
        }

        first = second + (header.substr(second, 2).starts_with("\r\n") ? 2 : 1);
    }
    return second;
}

/**
 * Extract data format from AmiraMesh data specifier string.
 *
 * @param str  string corresponding to the first part of Lattice enclosed by {}, e.g. "float[2]" in
 *             "Lattice { float Data } @1"
 * @param path file path of the dataset
 * @return DataFormat matching the Lattice data structure
 */
const DataFormatBase* parseDataSpecifier(std::string_view str, std::string_view line,
                                         const std::filesystem::path& path) {
    using namespace std::string_view_literals;
    using enum NumericType;

    // check for data dimensionality
    int dims = 1;
    if (auto pos = str.find('['); pos != std::string_view::npos) {
        if (!str.ends_with(']')) {
            throw DataReaderException(SourceContext{},
                                      "Invalid data dimensions in data specifier: {:?} ({})", line,
                                      path);
        }
        fromStr(str.substr(pos, str.size() - 2 - pos), dims);
    }

    const auto components = static_cast<size_t>(dims);
    if (str.starts_with("byte"sv)) {
        return DataFormatBase::get(UnsignedInteger, components, 8);
    } else if (str.starts_with("short"sv)) {
        return DataFormatBase::get(SignedInteger, components, 16);
    } else if (str.starts_with("ushort"sv)) {
        return DataFormatBase::get(UnsignedInteger, components, 16);
    } else if (str.starts_with("int"sv)) {
        return DataFormatBase::get(SignedInteger, components, 32);
    } else if (str.starts_with("uint"sv)) {
        return DataFormatBase::get(UnsignedInteger, components, 32);
    } else if (str.starts_with("float"sv)) {
        return DataFormatBase::get(Float, components, 32);
    } else if (str.starts_with("double"sv)) {
        return DataFormatBase::get(Float, components, 64);
    } else {
        throw DataReaderException(SourceContext{},
                                  "Unsupported format in data specifier: {:?} ({})", line, path);
    }
}

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

    ivec3 dim{0};
    auto tokens = split<3>(it->second, ' ');
    for (int i = 0; i < 3; ++i) {
        fromStr(tokens[i], dim[i]);
    }
    if (glm::compMin(dim) <= 0) {
        throw DataReaderException(SourceContext{},
                                  "Invalid Lattice dimensions: 'define Lattice {}' ({})",
                                  it->second, path);
    }
    return dim;
}

/**
 * Parse the AmiraMesh Parameters' bounding box and return it.
 *
 * @param parameters   map containing all paramaters from the AmiraMesh header
 * @param path file path of the dataset
 * @return bounding box min, max
 * @throw DataReaderException if Lattice definition is missing or the dimensions are invalid
 */
std::pair<vec3, vec3> parseBoundingBox(
    const std::unordered_map<std::string_view, std::string_view>& params,
    const std::filesystem::path& path) {
    auto it = params.find("BoundingBox");
    if (it == params.end()) {
        throw DataReaderException(SourceContext{}, "Missing BoundingBox in Parameters: {}", path);
    }

    vec3 bboxMin{-1.0f};
    vec3 bboxMax{1.0f};
    auto tokens = split<6>(it->second, ' ');
    for (int i = 0; i < 3; ++i) {
        if (tokens[2 * i].empty() || tokens[2 * i + 1].empty()) {
            throw DataReaderException(SourceContext{}, "Invalid BoundingBox: 'BoundingBox {}' ({})",
                                      it->second, path);
        }
        fromStr(tokens[2 * i], bboxMin[i]);
        fromStr(tokens[2 * i + 1], bboxMax[i]);
    }
    if (glm::any(glm::greaterThan(bboxMin, bboxMax))) {
        throw DataReaderException(SourceContext{}, "Invalid BoundingBox: 'BoundingBox {}' ({})",
                                  it->second, path);
    }
    return {bboxMin, bboxMax};
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

    // Get the file size, so we can pre-allocate the string. HUGE speed impact.
    fseek(file, 0, SEEK_END);
    const auto length = std::ftell(file);  // NOLINT(google-runtime-int)
    fseek(file, 0, SEEK_SET);

    if (length <= 0) {
        throw DataReaderException(SourceContext{}, "Empty AmiraMesh file: {}", path);
    }

    std::pmr::string data(length, '0');
    if (std::fread(data.data(), length, 1, file) != 1) {
        throw DataReaderException(SourceContext{}, "Could not read AmiraMesh file: {}", path);
    }
    std::string_view view = data;

    static constexpr std::string_view amiraHeader = "# AmiraMesh BINARY-LITTLE-ENDIAN 2.1";
    static constexpr std::string_view amiraDataSection = "Data section follows";

    if (!view.starts_with(amiraHeader)) {
        throw DataReaderException(SourceContext{}, "File is not an AmiraMesh file: {}", path);
    }

    std::unordered_map<std::string_view, std::string_view> defines;
    std::unordered_map<std::string_view, std::string_view> parameters;
    std::unordered_map<std::string_view, DataSpec> dataSpecs;

    auto addDefine = [&](std::string_view key, std::string_view value, std::string_view line) {
        if (key.empty() || value.empty()) {
            throw DataReaderException(SourceContext{}, "Invalid define found: {} ({})", line, path);
        }
        if (!defines.try_emplace(key, value).second) {
            throw DataReaderException(SourceContext{}, "Duplicate define found: {} ({})", line,
                                      path);
        }
    };

    auto addDataSpecifier = [&](std::string_view key, std::string_view str, std::string_view line) {
        auto [formatStr, type, parenthesis, identifierStr] = split<4>(str, ' ');
        if (str.empty() || parenthesis != "}" || !identifierStr.starts_with('@')) {
            throw DataReaderException(SourceContext{}, "Invalid data specifier found: {} ({})",
                                      line, path);
        }
        int id = -1;
        fromStr(identifierStr.substr(1), id);
        if (!dataSpecs
                 .try_emplace(key, DataSpec{.type = type,
                                            .format = parseDataSpecifier(formatStr, line, path),
                                            .identifier = id})
                 .second) {
            throw DataReaderException(SourceContext{}, "Duplicate data specifier found: {} ({})",
                                      line, path);
        }
    };

    for (size_t first = amiraHeader.size(); first < view.size();) {
        size_t second = view.find_first_of("\r\n", first);
        std::string_view line = view.substr(first, second - first);

        // remove comments
        auto [str, comment] = util::splitByFirst(line, '#');

        if (util::ltrim(comment).starts_with(amiraDataSection)) {  // marks end of header
            break;
        }

        str = util::trim(str);
        if (str.ends_with(',')) {
            str.remove_suffix(1);
        }

        if (auto tokens = split<3>(str, ' '); tokens[0] == "define") {
            addDefine(tokens[1], tokens[2], line);
        } else if (tokens[0].starts_with('n')) {
            addDefine(tokens[0].substr(1), tokens[1], line);
        } else if (tokens[0] == "Parameters" && tokens[1] == "{") {
            second = parseParameters(parameters, view, first, path);
        } else if (!tokens[0].empty() && tokens[1] == "{") {
            // parse data format specifier
            if (!defines.contains(tokens[0])) {
                log::warn("AmiraMesh: Format specifier '{}' was not defined. {}", line, path);
            }
            addDataSpecifier(tokens[0], tokens[2], line);
        }

        if (second == std::string_view::npos) {
            first = std::string_view::npos;
        } else {
            first = second + (view.substr(second, 2).starts_with("\r\n") ? 2 : 1);
        }
    }

    // verify volume parameters
    if (auto it = parameters.find("CoordType"); it == parameters.end()) {
        throw DataReaderException(SourceContext{}, "Missing CoordType, expected uniform: {}", path);
    } else if (it->second != "uniform") {
        throw DataReaderException(SourceContext{},
                                  "Unsupported CoordType '{}', expected 'uniform': {}", it->second,
                                  path);
    }

    const ivec3 dim = parseLatticeDefinition(defines, path);
    auto [bboxMin, bboxMax] = parseBoundingBox(parameters, path);

    const DataSpec& dataSpec = [&]() {
        if (auto it = dataSpecs.find("Lattice"); it == dataSpecs.end()) {
            throw DataReaderException(SourceContext{}, "Missing data specifier for Lattice: {}",
                                      path);
        } else {
            return it->second;
        }
    }();

    if (dataSpec.type != "Data") {
        throw DataReaderException(
            SourceContext{}, "Unsupported data specifier for Lattice '{}', expected 'Data': {}",
            dataSpec.type, path);
    }

    // locate data section and data matching the lattice identifier
    size_t offset = 0;
    if (auto dpos = view.find("# Data section follows"); dpos != std::string_view::npos) {
        const std::string id = fmt::format("@{}", dataSpec.identifier);
        if (auto pos = view.find(id, dpos); pos != std::string_view::npos) {
            // data starts after the lattice identifier followed by a newline character
            offset = pos + id.size() + 1;
        } else {
            throw DataReaderException(SourceContext{}, "Data for Lattice @{} not found: {}",
                                      dataSpec.identifier, path);
        }
    } else {
        throw DataReaderException(SourceContext{}, "Data section not found: {}", path);
    }

    const size_t bytesToRead = dataSpec.format->getSizeInBytes() * glm::compMul(size3_t{dim});
    if (offset + bytesToRead > static_cast<size_t>(length)) {
        throw DataReaderException(SourceContext{}, "Premature end of file in data section: {}",
                                  path);
    }

    auto volumeRam = createVolumeRAM(size3_t{dim}, dataSpec.format);
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
