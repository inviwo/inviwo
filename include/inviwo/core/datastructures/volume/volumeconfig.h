/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2024-2026 Inviwo Foundation
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

#include <inviwo/core/common/inviwocoredefine.h>

#include <inviwo/core/datastructures/image/imagetypes.h>
#include <inviwo/core/datastructures/datamapper.h>
#include <inviwo/core/datastructures/unitsystem.h>
#include <inviwo/core/util/formats.h>

#include <optional>
#include <glm/vec2.hpp>
#include <glm/mat4x4.hpp>

namespace inviwo {

class Volume;
class VolumeRepresentation;

struct IVW_CORE_API VolumeReprConfig {
    std::optional<size3_t> dimensions = std::nullopt;
    std::optional<DataFormatId> formatId = std::nullopt;
    std::optional<SwizzleMask> swizzleMask = std::nullopt;
    std::optional<InterpolationType> interpolation = std::nullopt;
    std::optional<Wrapping3D> wrapping = std::nullopt;

    constexpr bool operator==(const VolumeReprConfig&) const = default;
    bool operator==(const Volume&) const;
    bool operator==(const VolumeRepresentation&) const;
};

struct IVW_CORE_API VolumeConfig {
    std::optional<size3_t> dimensions = std::nullopt;
    std::optional<DataFormatId> formatId = std::nullopt;
    std::optional<SwizzleMask> swizzleMask = std::nullopt;
    std::optional<InterpolationType> interpolation = std::nullopt;
    std::optional<Wrapping3D> wrapping = std::nullopt;
    std::optional<Axis> xAxis = std::nullopt;
    std::optional<Axis> yAxis = std::nullopt;
    std::optional<Axis> zAxis = std::nullopt;
    std::optional<Axis> valueAxis = std::nullopt;
    std::optional<glm::dvec2> dataRange = std::nullopt;
    std::optional<glm::dvec2> valueRange = std::nullopt;
    std::optional<glm::mat4> model = std::nullopt;
    std::optional<glm::mat4> world = std::nullopt;

    static constexpr auto defaultDimensions = size3_t(128, 128, 128);
    static constexpr const DataFormatId defaultFormatId = DataUInt8::id();
    static constexpr auto defaultSwizzleMask = swizzlemasks::rgba;
    static constexpr auto defaultInterpolation = InterpolationType::Linear;
    static constexpr auto defaultWrapping = wrapping3d::clampAll;
    static inline const auto defaultXAxis = Axis{.name = "x", .unit = Unit{}};
    static inline const auto defaultYAxis = Axis{.name = "y", .unit = Unit{}};
    static inline const auto defaultZAxis = Axis{.name = "z", .unit = Unit{}};
    static inline const auto defaultValueAxis = Axis{};
    static constexpr glm::dvec2 defaultDataRange(DataFormatId formatId = defaultFormatId);
    static constexpr glm::dvec2 defaultValueRange(DataFormatId formatId = defaultFormatId);
    static constexpr auto defaultModel = glm::mat4{1.0f};
    static constexpr auto defaultWorld = glm::mat4{1.0f};

    [[nodiscard]] DataMapper dataMap() const;
    [[nodiscard]] DataMapper dataMap(const DataMapper& defaultMapper) const;
    VolumeConfig& updateFrom(const VolumeConfig& config);
    [[nodiscard]] VolumeReprConfig reprConfig() const;
    [[nodiscard]] VolumeConfig orDefault() const;

    constexpr bool operator==(const VolumeConfig&) const = default;

    constexpr auto tie() {
        return std::tie(dimensions, formatId, swizzleMask, interpolation, wrapping, xAxis, yAxis,
                        zAxis, valueAxis, dataRange, valueRange, model, world);
    }
    constexpr auto tie() const {
        return std::tie(dimensions, formatId, swizzleMask, interpolation, wrapping, xAxis, yAxis,
                        zAxis, valueAxis, dataRange, valueRange, model, world);
    }
    static constexpr auto defaults() {
        return std::tuple(defaultDimensions, defaultFormatId, defaultSwizzleMask,
                          defaultInterpolation, defaultWrapping, defaultXAxis, defaultYAxis,
                          defaultZAxis, defaultValueAxis, defaultDataRange(), defaultValueRange(),
                          defaultModel, defaultWorld);
    }
};

constexpr dvec2 VolumeConfig::defaultDataRange(DataFormatId formatId) {
    return DataMapper::defaultDataRangeFor(DataFormatBase::get(formatId));
}
constexpr dvec2 VolumeConfig::defaultValueRange(DataFormatId formatId) {
    return defaultDataRange(formatId);
}

}  // namespace inviwo
