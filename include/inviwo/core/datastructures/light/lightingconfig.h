/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2024 Inviwo Foundation
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
#include <inviwo/core/datastructures/light/lightingstate.h>
#include <inviwo/core/datastructures/spatialdata.h>

#include <optional>
#include <glm/vec3.hpp>

namespace inviwo {

struct IVW_CORE_API LightingConfig {
    std::optional<ShadingMode> shadingMode = std::nullopt;
    std::optional<glm::vec3> position = std::nullopt;
    std::optional<CoordinateSpace> referenceSpace = std::nullopt;
    std::optional<glm::vec3> ambient = std::nullopt;
    std::optional<glm::vec3> diffuse = std::nullopt;
    std::optional<glm::vec3> specular = std::nullopt;
    std::optional<float> specularExponent = std::nullopt;

    static constexpr ShadingMode defaultShadingMode{ShadingMode::BlinnPhong};
    // default position top right of the camera origin (xy plane in view coords, looking toward -z)
    static constexpr glm::vec3 defaultPosition{10.0f, 10.0f, 10.0f};
    static constexpr CoordinateSpace defaultReferenceSpace{CoordinateSpace::View};
    static constexpr glm::vec3 defaultAmbient{0.15f};
    static constexpr glm::vec3 defaultDiffuse{0.6f};
    static constexpr glm::vec3 defaultSpecular{0.4f};
    static constexpr float defaultSpecularExponent{60.0f};

    LightingConfig& updateFrom(const LightingConfig& config);

    friend bool operator==(const LightingConfig&, const LightingConfig&) = default;
};

}  // namespace inviwo
