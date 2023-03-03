/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019-2023 Inviwo Foundation
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

const std::string LineSettingsProperty::classIdentifier = "org.inviwo.LineSettingsProperty";
std::string LineSettingsProperty::getClassIdentifier() const { return classIdentifier; }

LineSettingsProperty::LineSettingsProperty(std::string_view identifier,
                                           std::string_view displayName,
                                           InvalidationLevel invalidationLevel,
                                           PropertySemantics semantics)
    : CompositeProperty(identifier, displayName, invalidationLevel, semantics)
    , lineWidth_("lineWidth", "Line Width (pixel)", 1.0f, 0.0f, 50.0f, 0.1f)
    , antialiasing_("antialiasing", "Antialiasing (pixel)", 0.5f, 0.0f, 10.0f, 0.1f)
    , miterLimit_("miterLimit", "Miter Limit", 0.8f, 0.0f, 1.0f, 0.1f)
    , roundCaps_("roundCaps", "Round Caps", true)
    , pseudoLighting_("pseudoLighting", "Pseudo Lighting", true,
                      InvalidationLevel::InvalidResources)
    , roundDepthProfile_("roundDepthProfile", "Round Depth Profile", true,
                         InvalidationLevel::InvalidResources)
    , stippling_("stippling", "Stippling") {
    addProperties(lineWidth_, antialiasing_, miterLimit_, roundCaps_, pseudoLighting_,
                  roundDepthProfile_, stippling_);
}

LineSettingsProperty::LineSettingsProperty(const LineSettingsProperty& rhs)
    : CompositeProperty(rhs)
    , lineWidth_(rhs.lineWidth_)
    , antialiasing_(rhs.antialiasing_)
    , miterLimit_(rhs.miterLimit_)
    , roundCaps_(rhs.roundCaps_)
    , pseudoLighting_(rhs.pseudoLighting_)
    , roundDepthProfile_(rhs.roundDepthProfile_)
    , stippling_(rhs.stippling_) {
    addProperties(lineWidth_, antialiasing_, miterLimit_, roundCaps_, pseudoLighting_,
                  roundDepthProfile_, stippling_);
}

LineSettingsProperty* LineSettingsProperty::clone() const {
    return new LineSettingsProperty(*this);
}

float LineSettingsProperty::getWidth() const { return lineWidth_.get(); }
float LineSettingsProperty::getAntialiasingWidth() const { return antialiasing_.get(); }

float LineSettingsProperty::getMiterLimit() const { return miterLimit_.get(); }
bool LineSettingsProperty::getRoundCaps() const { return roundCaps_.get(); }
bool LineSettingsProperty::getPseudoLighting() const { return pseudoLighting_.get(); }
bool LineSettingsProperty::getRoundDepthProfile() const { return roundDepthProfile_.get(); }

const StipplingSettingsInterface& LineSettingsProperty::getStippling() const { return stippling_; }
}  // namespace inviwo
