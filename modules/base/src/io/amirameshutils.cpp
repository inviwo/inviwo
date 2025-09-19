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
#include <inviwo/core/util/charconv.h>

#include <glm/gtx/component_wise.hpp>
#include <array>
#include <numeric>
#include <fmt/std.h>

namespace inviwo::amira {

// see https://github.com/emdb-empiar/ahds/blob/master/ahds/grammar.py for grammar of AmiraMesh
// files

namespace {

enum class ParseLevel : std::uint8_t { Top, ParameterList, MaterialsList, Material };

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
        util::fromStr(str.substr(pos + 1, str.size() - 2 - pos), dims);
    }

    const auto components = static_cast<size_t>(dims);
    if (str.starts_with("byte"sv)) {
        return {.numericType = UnsignedInteger, .numComponents = components, .precision = 8};
    } else if (str.starts_with("short"sv)) {
        return {.numericType = SignedInteger, .numComponents = components, .precision = 16};
    } else if (str.starts_with("ushort"sv)) {
        return {.numericType = UnsignedInteger, .numComponents = components, .precision = 16};
    } else if (str.starts_with("int"sv)) {
        return {.numericType = SignedInteger, .numComponents = components, .precision = 32};
    } else if (str.starts_with("uint"sv)) {
        return {.numericType = UnsignedInteger, .numComponents = components, .precision = 32};
    } else if (str.starts_with("float"sv)) {
        return {.numericType = Float, .numComponents = components, .precision = 32};
    } else if (str.starts_with("double"sv)) {
        return {.numericType = Float, .numComponents = components, .precision = 64};
    } else {
        throw DataReaderException(SourceContext{}, "Unsupported format in data specifier: {:?}",
                                  line);
    }
}

void parseFileDescriptor(AmiraMeshHeader& header, std::string_view fileDescriptor) {
    // file descriptor consists of '#', 'AmiraMesh', optional dimension ("3D"), format, version, and
    // optional extra-format
    auto parts = util::splitIntoArray<6>(fileDescriptor, ' ');

    if (parts[0] != "#" || !(iCaseCmp(parts[1], "AmiraMesh") || iCaseCmp(parts[1], "Avizo"))) {
        throw DataReaderException(SourceContext{}, "File is not an AmiraMesh file: {}",
                                  fileDescriptor);
    }

    int offset = 2;
    if (parts[offset].starts_with("3D")) {
        header.dimension = 3;
        ++offset;
    }
    if (iCaseCmp(parts[offset], "BINARY-LITTLE-ENDIAN")) {
        header.format = DataSectionFormat::Binary;
        header.byteOrder = ByteOrder::LittleEndian;
    } else if (iCaseCmp(parts[offset], "BINARY")) {
        header.format = DataSectionFormat::Binary;
        header.byteOrder = ByteOrder::BigEndian;
    } else if (iCaseCmp(parts[offset], "ASCII")) {
        header.format = DataSectionFormat::Ascii;
        header.byteOrder = ByteOrder::LittleEndian;
    } else {
        log::warn("AmiraMesh: Invalid data format '{}'.", fileDescriptor);
    }
    header.version = parts[offset + 1];
}

void addEntry(AmiraDict& dict, std::string_view str, std::string_view line) {
    auto [key, value] = util::splitIntoArray<2>(str, ' ');
    if (key.empty() || value.empty()) {
        throw DataReaderException(SourceContext{}, "Invalid key/value pair found: {}", line);
    }
    if (value.starts_with('"') && value.ends_with('"')) {
        value.remove_prefix(1);
        value.remove_suffix(1);
    }
    if (!dict.try_emplace(key, value).second) {
        throw DataReaderException(SourceContext{}, "Duplicate key found: {}", line);
    }
}

void addDataSpecifier(std::unordered_map<std::string_view, DataSpec>& dataSpecs,
                      const AmiraDict& defines, std::string_view name, std::string_view str,
                      std::string_view line) {
    std::optional<int> size;
    if (auto it = defines.find(name); it == defines.end()) {
        log::warn("Format specifier '{}' was not defined.", line);
    } else {
        ivec4 dims{0};
        auto tokens = util::splitIntoArray<5>(it->second);
        for (size_t i = 0; i < 4; ++i) {
            if (!tokens[i].empty()) {
                util::fromStr(tokens[i], dims[i]);
            }
        }
        size = glm::compMul(glm::max(dims, ivec4{1}));
    }

    auto [open, formatStr, type, close, identifierStr] = util::splitIntoArray<5>(str, ' ');
    if (str.empty() || open != "{" || close != "}" || !identifierStr.starts_with('@')) {
        throw DataReaderException(SourceContext{}, "Invalid data specifier found: {}", line);
    }
    int id = -1;
    util::fromStr(identifierStr.substr(1), id);
    if (!dataSpecs
             .try_emplace(type, DataSpec{.name = std::string{name},
                                         .format = parseDataSpecifier(formatStr, line),
                                         .identifier = id,
                                         .size = size})
             .second) {
        throw DataReaderException(SourceContext{}, "Duplicate data specifier found: {}", line);
    }
}

ParseLevel parseLine(ParseLevel level, AmiraMeshHeader& header, std::string_view str,
                     std::string_view line) {
    auto tokens = util::splitIntoArray<2>(str, ' ');

    using enum ParseLevel;
    switch (level) {
        case Top:
            if (tokens[0] == "define") {
                addEntry(header.defines, tokens[1], line);
            } else if (str.starts_with('n')) {
                addEntry(header.defines, str.substr(1), line);
            } else if (tokens[0] == "Parameters") {
                if (tokens[1] != "{") {
                    throw DataReaderException(SourceContext{},
                                              "Expected '{{' after 'Parameters': {}", line);
                }
                level = ParameterList;
            } else {
                addDataSpecifier(header.dataSpecs, header.defines, tokens[0], tokens[1], line);
            }
            break;
        case ParameterList:
            if (tokens[0] == "}") {
                level = Top;
            } else if (tokens[0] == "Materials") {
                if (tokens[1] != "{") {
                    throw DataReaderException(SourceContext{},
                                              "Expected '{{' after 'Materials': {}", line);
                }
                level = MaterialsList;
            } else {
                addEntry(header.parameters, str, line);
            }
            break;
        case MaterialsList:
            if (tokens[0] == "}") {
                level = ParameterList;
            } else if (tokens[1] == "{") {
                header.materials.emplace_back(
                    AmiraDict{std::pair{std::string_view{"name"}, tokens[0]}});
                level = Material;
            } else {
                throw DataReaderException(SourceContext{}, "Invalid Material found: {}", line);
            }
            break;
        case Material:
            if (tokens[0] == "}") {
                level = MaterialsList;
            } else if (!header.materials.back().try_emplace(tokens[0], tokens[1]).second) {
                throw DataReaderException(SourceContext{},
                                          "Duplicate entry in Material '{}' found: {}",
                                          header.materials.back()["name"], line);
            }
            break;
    }
    return level;
}

}  // namespace

std::pair<vec3, vec3> getBoundingBox(const AmiraDict& params) {
    auto it = params.find("BoundingBox");
    if (it == params.end()) {
        throw DataReaderException(SourceContext{}, "Missing BoundingBox in Parameters");
    }

    vec3 bboxMin{-1.0f};
    vec3 bboxMax{1.0f};
    auto tokens = util::splitIntoArray<6>(it->second, ' ');
    // NOLINTBEGIN(readability-math-missing-parentheses)
    for (int i = 0; i < 3; ++i) {
        if (tokens[2 * i].empty() || tokens[2 * i + 1].empty()) {
            throw DataReaderException(SourceContext{}, "Invalid BoundingBox: 'BoundingBox {}'",
                                      it->second);
        }
        util::fromStr(tokens[2 * i], bboxMin[i]);
        util::fromStr(tokens[2 * i + 1], bboxMax[i]);
    }
    // NOLINTEND(readability-math-missing-parentheses)
    if (glm::any(glm::greaterThan(bboxMin, bboxMax))) {
        throw DataReaderException(SourceContext{}, "Invalid BoundingBox: 'BoundingBox {}'",
                                  it->second);
    }
    return {bboxMin, bboxMax};
}

std::unordered_map<int, Material> getMaterials(const AmiraMeshHeader& header) {
    std::unordered_map<int, Material> materials;
    for (const auto& mat : header.materials) {
        const auto name = mat.find("name")->first;

        auto it = mat.find("Id");
        if (it == mat.end()) {
            throw DataReaderException(SourceContext{}, "Material Id missing for material '{}'",
                                      name);
        }
        const int id = parseValue<int>(it->second);
        it = mat.find("Color");
        if (it == mat.end()) {
            throw DataReaderException(SourceContext{}, "Color missing for material '{}'", name);
        }
        const vec3 color = parseValue<vec3>(it->second);
        float transparency = 1.0f;
        it = mat.find("Transparency");
        if (it != mat.end()) {
            transparency = parseValue<float>(it->second);
        }
        materials.try_emplace(
            id, Material{.name = std::string{name}, .color = vec4{color, transparency}});
    }
    return materials;
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
    static constexpr std::string_view amiraDataSection = "# Data section follows";

    if (!header.starts_with(amiraHeader)) {
        throw DataReaderException(SourceContext{},
                                  "Could not locate '# AmiraMesh' header in first line");
    }

    headerData.fileDescriptor = header.substr(0, header.find_first_of("\r\n"));
    parseFileDescriptor(headerData, headerData.fileDescriptor);

    ParseLevel level = ParseLevel::Top;

    const size_t headerEnd = header.find(amiraDataSection);
    util::forEachLine(header.substr(0, headerEnd), [&level, &headerData](std::string_view line) {
        // remove comments
        auto [str, comment] = util::splitByFirst(line, '#');
        str = util::trim(str);
        if (str.ends_with(',')) {
            str.remove_suffix(1);
        }
        if (str.empty()) {
            return;
        }

        level = parseLine(level, headerData, str, line);
    });

    return headerData;
}

}  // namespace inviwo::amira
