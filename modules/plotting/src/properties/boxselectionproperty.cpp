/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2020-2021 Inviwo Foundation
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

#include <modules/plotting/properties/boxselectionproperty.h>

namespace inviwo {

namespace plot {

const std::string BoxSelectionProperty::classIdentifier = "org.inviwo.BoxSelectionProperty";
std::string BoxSelectionProperty::getClassIdentifier() const { return classIdentifier; }

BoxSelectionProperty::BoxSelectionProperty(const std::string& identifier,
                                           const std::string& displayName,
                                           InvalidationLevel invalidationLevel,
                                           PropertySemantics semantics)
    : CompositeProperty(identifier, displayName, invalidationLevel, semantics)
    , mode_("mode", "Mode",
            {{"selection", "Selection", BoxSelectionSettingsInterface::Mode::Selection},
             {"filtering", "Filtering", BoxSelectionSettingsInterface::Mode::Filtering},
             {"none", "None", BoxSelectionSettingsInterface::Mode::None}})
    , lineColor_("lineColor", "Line Color", vec4(vec3(0.0f), 1.0f), vec4(0.0f), vec4(1.0f))
    , lineWidth_("lineWidth", "Line Width", 1.0f, 0.0f, 10.0f) {
    lineColor_.setSemantics(PropertySemantics::Color);
    addProperty(mode_);
    addProperty(lineColor_);
    addProperty(lineWidth_);
}

BoxSelectionProperty::BoxSelectionProperty(const BoxSelectionProperty& rhs)
    : CompositeProperty(rhs)
    , mode_(rhs.mode_)
    , lineColor_(rhs.lineColor_)
    , lineWidth_(rhs.lineWidth_) {

    lineColor_.setSemantics(PropertySemantics::Color);
    addProperty(mode_);
    addProperty(lineColor_);
    addProperty(lineWidth_);
}

BoxSelectionProperty* BoxSelectionProperty::clone() const {
    return new BoxSelectionProperty(*this);
}

BoxSelectionSettingsInterface::Mode BoxSelectionProperty::getMode() const { return mode_.get(); }
vec4 BoxSelectionProperty::getLineColor() const { return lineColor_.get(); }
float BoxSelectionProperty::getLineWidth() const { return lineWidth_.get(); }

}  // namespace plot

}  // namespace inviwo
