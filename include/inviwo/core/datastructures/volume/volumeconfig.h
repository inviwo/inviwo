/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2024-2025 Inviwo Foundation
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

struct IVW_CORE_API VolumeConfig {
    std::optional<size3_t> dimensions = std::nullopt;
    const DataFormatBase* format = nullptr;
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
    static inline const DataFormatBase* defaultFormat = DataUInt8::get();
    static constexpr auto defaultSwizzleMask = swizzlemasks::rgba;
    static constexpr auto defaultInterpolation = InterpolationType::Linear;
    static constexpr auto defaultWrapping = wrapping3d::clampAll;
    static inline const auto defaultXAxis = Axis{"x", Unit{}};
    static inline const auto defaultYAxis = Axis{"y", Unit{}};
    static inline const auto defaultZAxis = Axis{"z", Unit{}};
    static inline const auto defaultValueAxis = Axis{};
    static glm::dvec2 defaultDataRange(const DataFormatBase* format = defaultFormat);
    static glm::dvec2 defaultValueRange(const DataFormatBase* format = defaultFormat);
    static constexpr auto defaultModel = glm::mat4{1.0f};
    static constexpr auto defaultWorld = glm::mat4{1.0f};

    DataMapper dataMap() const;
    DataMapper dataMap(const DataMapper& defaultMapper) const;
    VolumeConfig& updateFrom(const VolumeConfig& config);

    friend bool operator==(const VolumeConfig&, const VolumeConfig&) = default;
};

struct IVW_CORE_API VolumeReprConfig {
    std::optional<size3_t> dimensions = std::nullopt;
    const DataFormatBase* format = nullptr;
    std::optional<SwizzleMask> swizzleMask = std::nullopt;
    std::optional<InterpolationType> interpolation = std::nullopt;
    std::optional<Wrapping3D> wrapping = std::nullopt;
};

}  // namespace inviwo
