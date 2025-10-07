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
#include <inviwo/core/util/charconv.h>

#include <filesystem>
#include <vector>
#include <unordered_map>
#include <string_view>
#include <optional>

namespace inviwo::amira {

struct IVW_MODULE_BASE_API DataSpecFormat {
    NumericType numericType{NumericType::NotSpecialized};
    size_t numComponents = 0;
    size_t precision = 0;
};

struct IVW_MODULE_BASE_API DataSpec {
    std::string name;            //!< name of the data spec
    DataSpecFormat format;       //!< format of the data
    int identifier = -1;         //!< dataset identifier used for lookups in Data section
    std::optional<size_t> size;  //!< number of data values matching the corresponding define
};

enum class DataSectionFormat : std::uint8_t { Unspecified, Ascii, Binary };

struct IVW_MODULE_BASE_API Material {
    std::string name;
    vec4 color{0.0f};
};

using AmiraDict = std::unordered_map<std::string_view, std::string_view>;

/**
 * Header information from an AmiraMesh file.
 */
struct IVW_MODULE_BASE_API AmiraMeshHeader {
    std::string fileDescriptor;  //!< contains the first line, e.g. "# AmiraMesh 3D ASCII 2.0"
    AmiraDict defines;
    AmiraDict parameters;
    std::vector<AmiraDict> materials;  //!< Note: materials are assigned per primitive
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

/**
 * Extract the bounding box from AmiraMesh Parameters.
 *
 * @param parameters   map containing all paramaters from the AmiraMesh header
 * @return bounding box min, max
 * @throw DataReaderException if BoundingBox definition is missing or the dimensions are invalid
 */
IVW_MODULE_BASE_API std::pair<vec3, vec3> getBoundingBox(const AmiraDict& params);

/**
 * Extract the materials from AmiraMesh Parameters.
 *
 * @param header   AmiraMesh header containing the material dictionaries
 * @return unordered map of Materials using the material ID as key
 * @throw DataReaderException if an invalid Material definition is found
 */
IVW_MODULE_BASE_API std::unordered_map<int, Material> getMaterials(const AmiraMeshHeader& header);

/**
 * Split the string @p str by @p delimiter, extract numbers from the resulting parts, and return
 * the combined parsed numbers as type @p T.
 *
 * @tparam T   The target type, either scalar value or glm type
 * @param str  The string to be parsed
 * @param delimiter  The character use for splitting (defaults to space)
 * @return value containing the corresponding parsed numbers in each component
 */
template <typename T>
T parseValue(std::string_view str, char delimiter = ' ') {
    T value{};
    const auto tokens = util::splitIntoArray<util::extent_v<T> + 1>(str, delimiter);
    for (size_t i = 0; i < util::extent_v<T>; ++i) {
        if constexpr (util::extent_v<T> == 1) {
            util::fromStr(tokens[i], value);
        } else {
            util::fromStr(tokens[i], value[i]);
        }
    }
    return value;
}

}  // namespace inviwo::amira
