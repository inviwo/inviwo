/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2023-2025 Inviwo Foundation
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

#include <modules/basegl/baseglmoduledefine.h>

#include <inviwo/core/ports/imageport.h>
#include <modules/basegl/datastructures/meshshadercache.h>
#include <inviwo/core/properties/boolcompositeproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/optionproperty.h>

namespace inviwo {

class Shader;
class TextureUnitContainer;

// Define an enum for blend modes
enum class BlendMode {
    Source,         // Use source
    Destination,    // Use destination
    Alpha,          // Standard alpha blending
    Additive,       // Glowing effects (fire, lights)
    Multiply,       // Darkening effects (shadows)
    Screen,         // Lightening effects (highlights)
    Subtractive,    // Removing light (dark smoke, dirt)
    Premultiplied,  // Correct alpha blending (HDR)
    Overlay         // Contrast enhancement
};

// Convert BlendMode to string for logging/debugging
constexpr std::string_view format_as(BlendMode mode) {
    switch (mode) {
        case BlendMode::Source:
            return "Source Blending";
        case BlendMode::Destination:
            return "Destination Blending";
        case BlendMode::Alpha:
            return "Alpha Blending";
        case BlendMode::Additive:
            return "Additive Blending";
        case BlendMode::Multiply:
            return "Multiplicative Blending";
        case BlendMode::Screen:
            return "Screen Blending";
        case BlendMode::Subtractive:
            return "Subtractive Blending";
        case BlendMode::Premultiplied:
            return "Premultiplied Alpha Blending";
        case BlendMode::Overlay:
            return "Overlay Blending";
        default:
            return "Unknown Blend Mode";
    }
}

class IVW_MODULE_BASEGL_API MeshTexturing {
public:
    MeshTexturing(std::string_view identifier, Document help);
    void bind(TextureUnitContainer& cont);

    void setUniforms(Shader& shader) const;
    MeshShaderCache::Requirement getRequirement() const;

    void addDefines(Shader& shader) const;

    ImageInport inport;
    int unitNumber;

    BoolCompositeProperty texture;
    OptionProperty<BlendMode> blendMode;
    BoolProperty swap;
    FloatProperty mix;
};

}  // namespace inviwo
