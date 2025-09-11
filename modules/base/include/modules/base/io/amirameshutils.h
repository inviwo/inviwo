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
#pragma once

#include <modules/base/basemoduledefine.h>

#include <inviwo/core/io/inviwofileformattypes.h>
#include <inviwo/core/io/datareaderexception.h>
#include <inviwo/core/util/formats.h>
#include <inviwo/core/util/glmvec.h>
#include <inviwo/core/util/stringconversion.h>

#include <filesystem>
#include <unordered_map>
#include <string_view>
#include <charconv>

#if !(defined(__cpp_lib_to_chars) && __cpp_lib_to_chars >= 201611L)
#include <fast_float/fast_float.h>
#endif

namespace inviwo::amira {

struct IVW_MODULE_BASE_API DataSpecFormat {
    NumericType numericType{NumericType::NotSpecialized};
    size_t numComponents = 0;
    size_t precision = 0;
};

struct DataSpec {
    std::string name;       //!<
    DataSpecFormat format;  //!< format of the data
    int identifier = -1;    //!< dataset identifier used for lookups in Data section
};

enum class DataSectionFormat : std::uint8_t { Unspecified, Ascii, Binary };

/**
 * Header information from an AmiraMesh file.
 */

struct IVW_MODULE_BASE_API AmiraMeshHeader {
    std::string fileDescriptor;  //!< contains the first line, e.g. "# AmiraMesh 3D ASCII 2.0"
    std::unordered_map<std::string_view, std::string_view> defines;
    std::unordered_map<std::string_view, std::string_view> parameters;
    std::unordered_map<std::string_view, DataSpec>
        dataSpecs;  //!< key: type descriptor of the data, e.g. "Data", "Coordinates", "LineIdx"
    std::optional<int> dimension;
    DataSectionFormat format = DataSectionFormat::Unspecified;
    ByteOrder byteOrder = ByteOrder::LittleEndian;
    std::string version;
};

/**
 * Extract the AmiraMesh header information from file @p filepath.
 *
 * @throw DataReaderException
 */
IVW_MODULE_BASE_API AmiraMeshHeader parseAmiraMeshHeader(std::filesystem::path& filepath);

/**
 * Extract the AmiraMesh header information from the given string @p str.
 *
 * @throw DataReaderException
 */
IVW_MODULE_BASE_API AmiraMeshHeader parseAmiraMeshHeader(std::string_view str);

IVW_MODULE_BASE_API std::pair<vec3, vec3> getBoundingBox(
    const std::unordered_map<std::string_view, std::string_view>& params);

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
    const auto* const end = value.data() + value.size();

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

}  // namespace inviwo::amira
