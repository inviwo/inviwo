/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019-2025 Inviwo Foundation
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

#include <inviwo/core/properties/boolproperty.h>          // for BoolProperty
#include <inviwo/core/properties/compositeproperty.h>     // for CompositeProperty
#include <inviwo/core/properties/invalidationlevel.h>     // for InvalidationLevel, Invalidation...
#include <inviwo/core/properties/ordinalproperty.h>       // for FloatProperty
#include <inviwo/core/properties/propertysemantics.h>     // for PropertySemantics
#include <modules/basegl/properties/stipplingproperty.h>  // for StipplingProperty

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

float LineSettingsProperty::getWidth() const { return lineWidth.get(); }
float LineSettingsProperty::getAntialiasingWidth() const { return antialiasing.get(); }

float LineSettingsProperty::getMiterLimit() const { return miterLimit.get(); }
bool LineSettingsProperty::getRoundCaps() const { return roundCaps.get(); }
bool LineSettingsProperty::getPseudoLighting() const { return pseudoLighting.get(); }
bool LineSettingsProperty::getRoundDepthProfile() const { return roundDepthProfile.get(); }

vec4 LineSettingsProperty::getDefaultColor() const { return defaultColor.get(); }

const StipplingSettingsInterface& LineSettingsProperty::getStippling() const { return stippling; }

bool LineSettingsProperty::getOverrideColor() const { return overrideColor.isChecked(); }
vec3 LineSettingsProperty::getOverrideColorValue() const { return color.get(); }

bool LineSettingsProperty::getOverrideAlpha() const { return overrideAlpha.isChecked(); }
float LineSettingsProperty::getOverrideAlphaValue() const { return alpha.get(); }

bool LineSettingsProperty::getUseMetaColor() const { return useMetaColor.isChecked(); }

const TransferFunction& LineSettingsProperty::getMetaColor() const { return metaColor.get(); }

}  // namespace inviwo
