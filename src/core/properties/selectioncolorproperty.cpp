/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2021 Inviwo Foundation
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

const std::string SelectionColorProperty::classIdentifier = "org.inviwo.ColorSelectionProperty";
std::string SelectionColorProperty::getClassIdentifier() const { return classIdentifier; }

SelectionColorProperty::SelectionColorProperty(std::string_view identifier,
                                               std::string_view displayName, bool checked,
                                               vec3 color, float alpha,
                                               InvalidationLevel invalidationLevel,
                                               PropertySemantics semantics)
    : BoolCompositeProperty(identifier, displayName, checked, invalidationLevel, semantics)
    , color_("color", "Color", util::ordinalColor(color))
    , alpha_("alpha", "Alpha", alpha)
    , intensity_("intensity", "Mixing", 0.7f, 0.01f, 1.0f, 0.001f) {
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

vec4 SelectionColorProperty::getColor() const { return vec4{color_.get(), alpha_.get()}; }

float SelectionColorProperty::getMixIntensity() const { return intensity_; }

}  // namespace inviwo
