/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2021-2025 Inviwo Foundation
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

#include <inviwo/core/properties/selectioncolorproperty.h>

namespace inviwo {

std::string_view SelectionColorProperty::getClassIdentifier() const { return classIdentifier; }

SelectionColorProperty::SelectionColorProperty(std::string_view identifier,
                                               std::string_view displayName, bool checked,
                                               vec3 color, float alpha,
                                               InvalidationLevel invalidationLevel,
                                               PropertySemantics semantics)
    : SelectionColorProperty(identifier, displayName, Document{}, checked, color, alpha,
                             invalidationLevel) {}

SelectionColorProperty::SelectionColorProperty(std::string_view identifier,
                                               std::string_view displayName, Document help,
                                               bool checked, vec3 color, float alpha,
                                               InvalidationLevel invalidationLevel,
                                               PropertySemantics semantics)
    : BoolCompositeProperty(identifier, displayName, std::move(help), checked, invalidationLevel,
                            semantics)
    , color_("color", "Color", util::ordinalColor(color).set("Overlay color"_help))
    , alpha_("alpha", "Alpha", "Determines the opacity of the overlay color"_help, alpha)
    , intensity_(
          "intensity", "Mixing",
          "Blending factor for mixing the overlay color with the item's original color."_help, 0.7f,
          {0.0f, ConstraintBehavior::Immutable}, {1.0f, ConstraintBehavior::Immutable}, 0.001f) {
    addProperties(color_, alpha_, intensity_);
}

SelectionColorProperty::SelectionColorProperty(const SelectionColorProperty& rhs)
    : BoolCompositeProperty(rhs)
    , color_(rhs.color_)
    , alpha_(rhs.alpha_)
    , intensity_(rhs.intensity_) {
    addProperties(color_, alpha_, intensity_);
}

SelectionColorProperty* SelectionColorProperty::clone() const {
    return new SelectionColorProperty(*this);
}

SelectionColorState SelectionColorProperty::getState() const {
    return {getColor(), getMixIntensity(), isChecked()};
}

vec4 SelectionColorProperty::getColor() const { return vec4{color_.get(), alpha_.get()}; }

float SelectionColorProperty::getMixIntensity() const { return intensity_; }

}  // namespace inviwo
