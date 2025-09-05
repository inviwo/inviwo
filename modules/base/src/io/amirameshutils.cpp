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

#include <modules/base/io/amirameshutils.h>

#include <inviwo/core/io/datareaderexception.h>
#include <inviwo/core/io/curlutils.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/util/stringconversion.h>
#include <inviwo/core/util/raiiutils.h>
#include <inviwo/core/util/glmvec.h>
#include <inviwo/core/util/logcentral.h>

#include <glm/gtx/component_wise.hpp>
#include <charconv>
#include <array>
#include <numeric>
#include <fmt/std.h>

#if !(defined(__cpp_lib_to_chars) && __cpp_lib_to_chars >= 201611L)
#include <fast_float/fast_float.h>
#endif

#pragma optimize("", off)

namespace inviwo::amira {

namespace {

/**
 * Extract data format from AmiraMesh data specifier string.
 *
 * @param str  string corresponding to the first part of Lattice enclosed by {}, e.g. "float[2]" in
 *             "Lattice { float Data } @1"
 * @return format matching the Lattice data structure
 */
DataSpecFormat parseDataSpecifier(std::string_view str, std::string_view line) {
    using namespace std::string_view_literals;
    using enum NumericType;

    // check for data dimensionality/number of components
    int dims = 1;
    if (auto pos = str.find('['); pos != std::string_view::npos) {
        if (!str.ends_with(']')) {
            throw DataReaderException(SourceContext{},
                                      "Invalid data dimensions in data specifier: {:?}", line);
        }
        fromStr(str.substr(pos, str.size() - 2 - pos), dims);
    }

    const auto components = static_cast<size_t>(dims);
    if (str.starts_with("byte"sv)) {
        return {UnsignedInteger, components, 8};
    } else if (str.starts_with("short"sv)) {
        return {SignedInteger, components, 16};
    } else if (str.starts_with("ushort"sv)) {
        return {UnsignedInteger, components, 16};
    } else if (str.starts_with("int"sv)) {
        return {SignedInteger, components, 32};
    } else if (str.starts_with("uint"sv)) {
        return {UnsignedInteger, components, 32};
    } else if (str.starts_with("float"sv)) {
        return {Float, components, 32};
    } else if (str.starts_with("double"sv)) {
        return {Float, components, 64};
    } else {
        throw DataReaderException(SourceContext{}, "Unsupported format in data specifier: {:?}",
                                  line);
    }
}

void addDefine(std::unordered_map<std::string_view, std::string_view>& defines,
               std::string_view key, std::string_view value, std::string_view line) {
    if (key.empty() || value.empty()) {
        throw DataReaderException(SourceContext{}, "Invalid define found: {}", line);
    }
    if (!defines.try_emplace(key, value).second) {
        throw DataReaderException(SourceContext{}, "Duplicate define found: {}", line);
    }
};

void addDataSpecifier(std::unordered_map<std::string_view, DataSpec>& dataSpecs,
                      std::string_view key, std::string_view str, std::string_view line) {
    auto [formatStr, type, parenthesis, identifierStr] = split<4>(str, ' ');
    if (str.empty() || parenthesis != "}" || !identifierStr.starts_with('@')) {
        throw DataReaderException(SourceContext{}, "Invalid data specifier found: {}", line);
    }
    int id = -1;
    fromStr(identifierStr.substr(1), id);
    if (!dataSpecs
             .try_emplace(key, DataSpec{.type = std::string{type},
                                        .format = parseDataSpecifier(formatStr, line),
                                        .identifier = id})
             .second) {
        throw DataReaderException(SourceContext{}, "Duplicate data specifier found: {}", line);
    }
};

/**
 * Parse the AmiraMesh Parameters block delimited by parentheses { and }.
 *
 * @parameters  map holding all parameters
 * @param header   string containing the entire header including "Parameters { ... }"
 * @param first index in @p str
 * @return position of line ending behind closing parenthesis.
 */
size_t addParameters(std::unordered_map<std::string_view, std::string_view>& parameters,
                     std::string_view header, size_t first) {
    size_t paramsStart = header.find('{', first);
    size_t paramsEnd = header.find('}', paramsStart);
    if (paramsEnd == std::string_view::npos) {
        throw DataReaderException(SourceContext{}, "Invalid Parameters block, end not found");
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
                throw DataReaderException(SourceContext{}, "Invalid parameter found: {}", line);
            }
            if (value.starts_with('"') && value.ends_with('"')) {
                value.remove_prefix(1);
                value.remove_suffix(1);
            }
            if (!parameters.try_emplace(key, value).second) {
                throw DataReaderException(SourceContext{}, "Duplicate parameter found: {}", line);
            }
        }
        first = second + (header.substr(second, 2).starts_with("\r\n") ? 2 : 1);
    }
    return second;
}

}  // namespace

/**
 * Extract the bounding box from AmiraMesh Parameters.
 *
 * @param parameters   map containing all paramaters from the AmiraMesh header
 * @return bounding box min, max
 * @throw DataReaderException if BoundingBox definition is missing or the dimensions are invalid
 */
std::pair<vec3, vec3> getBoundingBox(
    const std::unordered_map<std::string_view, std::string_view>& params) {
    auto it = params.find("BoundingBox");
    if (it == params.end()) {
        throw DataReaderException(SourceContext{}, "Missing BoundingBox in Parameters");
    }

    vec3 bboxMin{-1.0f};
    vec3 bboxMax{1.0f};
    auto tokens = split<6>(it->second, ' ');
    for (int i = 0; i < 3; ++i) {
        if (tokens[2 * i].empty() || tokens[2 * i + 1].empty()) {
            throw DataReaderException(SourceContext{}, "Invalid BoundingBox: 'BoundingBox {}'",
                                      it->second);
        }
        fromStr(tokens[2 * i], bboxMin[i]);
        fromStr(tokens[2 * i + 1], bboxMax[i]);
    }
    if (glm::any(glm::greaterThan(bboxMin, bboxMax))) {
        throw DataReaderException(SourceContext{}, "Invalid BoundingBox: 'BoundingBox {}'",
                                  it->second);
    }
    return {bboxMin, bboxMax};
}

AmiraMeshHeader parseAmiraMeshHeader(std::filesystem::path& path) {
    const auto filePath = net::downloadAndCacheIfUrl(path);

    if (!std::filesystem::is_regular_file(filePath)) {
        throw DataReaderException(SourceContext{}, "Could not find input file: {}", path);
    }

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

    try {
        return parseAmiraMeshHeader(data);
    } catch (DataReaderException& e) {
        throw DataReaderException(e.getContext(), "{} ({})", e.what(), path);
    }
}

AmiraMeshHeader parseAmiraMeshHeader(std::string_view header) {
    AmiraMeshHeader headerData{};

    static constexpr std::string_view amiraHeader = "# AmiraMesh";
    static constexpr std::string_view amiraDataSection = "Data section follows";

    if (!header.starts_with(amiraHeader)) {
        throw DataReaderException(SourceContext{},
                                  "Could not locate '# AmiraMesh' header in first line");
    }
    headerData.fileDescriptor = header.substr(0, header.find_first_of("\r\n"));
    headerData.byteOrder = headerData.fileDescriptor.contains("BIG-ENDIAN")
                               ? ByteOrder::BigEndian
                               : ByteOrder::LittleEndian;
    if (util::iCaseContains(headerData.fileDescriptor, "ASCII")) {
        headerData.dataSectionFormat = DataSectionFormat::Ascii;
    } else if (util::iCaseContains(headerData.fileDescriptor, "BINARY")) {
        headerData.dataSectionFormat = DataSectionFormat::Binary;
    }

    for (size_t first = amiraHeader.size(); first < header.size();) {
        size_t second = header.find_first_of("\r\n", first);
        std::string_view line = header.substr(first, second - first);

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
            addDefine(headerData.defines, tokens[1], tokens[2], line);
        } else if (tokens[0].starts_with('n')) {
            addDefine(headerData.defines, tokens[0].substr(1), tokens[1], line);
        } else if (tokens[0] == "Parameters" && tokens[1] == "{") {
            second = addParameters(headerData.parameters, header, first);
        } else if (!tokens[0].empty() && tokens[1] == "{") {
            // parse data format specifier
            if (!headerData.defines.contains(tokens[0])) {
                log::warn("AmiraMesh: Format specifier '{}' was not defined.", line);
            }
            addDataSpecifier(headerData.dataSpecs, tokens[0], tokens[2], line);
        }

        if (second == std::string_view::npos) {
            first = std::string_view::npos;
        } else {
            first = second + (header.substr(second, 2).starts_with("\r\n") ? 2 : 1);
        }
    }
    return headerData;
}

}  // namespace inviwo::amira
