/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019-2026 Inviwo Foundation
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

#include <modules/basegl/properties/linesettingsproperty.h>

#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/properties/compositeproperty.h>
#include <inviwo/core/properties/invalidationlevel.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/propertysemantics.h>
#include <modules/basegl/properties/stipplingproperty.h>

namespace inviwo {
class StipplingSettingsInterface;

std::string_view LineSettingsProperty::getClassIdentifier() const { return classIdentifier; }

LineSettingsProperty::LineSettingsProperty(std::string_view identifier,
                                           std::string_view displayName,
                                           InvalidationLevel invalidationLevel,
                                           PropertySemantics semantics)
    : CompositeProperty{identifier, displayName, invalidationLevel, semantics}
    , lineWidth{"lineWidth", "Line Width (pixel)",
                util::ordinalLength(1.0f, 50.0f).set("width of the rendered lines (in pixel)"_help)}
    , antialiasing{"antialiasing", "Antialiasing (pixel)",
                   util::ordinalLength(0.5f, 10.0f)
                       .set("width of the antialiased line edge (in pixel), "
                            "this determines the softness along the edge"_help)}
    , miterLimit{"miterLimit",
                 "Miter Limit",
                 "limit for cutting of sharp corners"_help,
                 0.8f,
                 {0.0f, ConstraintBehavior::Immutable},
                 {1.0f, ConstraintBehavior::Immutable},
                 0.1f}
    , roundCaps{"roundCaps", "Round Caps",
                "If enabled, round caps are drawn at the end of each line"_help, true}
    , pseudoLighting{"pseudoLighting", "Pseudo Lighting",
                     "enables radial shading as depth cue, i.e. tube like appearance"_help, true,
                     InvalidationLevel::InvalidResources}
    , roundDepthProfile{"roundDepthProfile", "Round Depth Profile",
                        "Modify line depth matching a round depth profile"_help, true,
                        InvalidationLevel::InvalidResources}
    , defaultColor{"defaultColor", "Default Color",
                   util::ordinalColor(vec4{1.0f, 0.7f, 0.2f, 1.0f})}
    , overrideColor{"overrideColor", "Override Color",
                    "If enabled, all lines will share the same custom color"_help, false,
                    InvalidationLevel::InvalidResources}
    , color{"color", "Color",
            util::ordinalColor(vec3{0.7f, 0.7f, 0.7f})
                .set("Custom color when overriding the input colors"_help)}
    , overrideAlpha{"useUniformAlpha", "Override Alpha", false, InvalidationLevel::InvalidResources}
    , alpha{"alpha", "Alpha", 1.0f, 0.0f, 1.0f, 0.1f}
    , useMetaColor{"useMetaColor", "Use meta color mapping", false,
                   InvalidationLevel::InvalidResources}
    , metaColor{"metaColor", "Meta Color Mapping"}
    , stippling{"stippling", "Stippling"} {

    overrideColor.addProperty(color);
    overrideAlpha.addProperty(alpha);
    useMetaColor.addProperty(metaColor);

    addProperties(lineWidth, antialiasing, miterLimit, roundCaps, pseudoLighting, roundDepthProfile,
                  defaultColor, overrideColor, overrideAlpha, useMetaColor, stippling);
}

LineSettingsProperty::LineSettingsProperty(const LineSettingsProperty& rhs)
    : CompositeProperty{rhs}
    , lineWidth{rhs.lineWidth}
    , antialiasing{rhs.antialiasing}
    , miterLimit{rhs.miterLimit}
    , roundCaps{rhs.roundCaps}
    , pseudoLighting{rhs.pseudoLighting}
    , roundDepthProfile{rhs.roundDepthProfile}
    , defaultColor{rhs.defaultColor}
    , overrideColor{rhs.overrideColor}
    , color{rhs.color}
    , overrideAlpha{rhs.overrideAlpha}
    , alpha{rhs.alpha}
    , useMetaColor{rhs.useMetaColor}
    , metaColor{rhs.metaColor}
    , stippling{rhs.stippling} {

    overrideColor.addProperty(color);
    overrideAlpha.addProperty(alpha);
    useMetaColor.addProperty(metaColor);

    addProperties(lineWidth, antialiasing, miterLimit, roundCaps, pseudoLighting, roundDepthProfile,
                  defaultColor, overrideColor, overrideAlpha, useMetaColor, stippling);
}

LineSettingsProperty* LineSettingsProperty::clone() const {
    return new LineSettingsProperty(*this);
}

void LineSettingsProperty::update(LineData& data) const {
    data.lineWidth = lineWidth.get();
    data.antialiasing = antialiasing.get();
    data.miterLimit = miterLimit.get();
    data.roundCaps = roundCaps.get();
    data.pseudoLighting = pseudoLighting.get();
    data.roundDepthProfile = roundDepthProfile.get();
    data.overrideColor = overrideColor.isChecked();
    data.overrideAlpha = overrideAlpha.isChecked();
    data.useMetaColor = useMetaColor.isChecked();
    stippling.update(data.stippling);
    data.defaultColor = defaultColor.get();
    data.overrideColorValue = color.get();
    data.overrideAlphaValue = alpha.get();
    data.metaColor = metaColor.get();
}

}  // namespace inviwo
