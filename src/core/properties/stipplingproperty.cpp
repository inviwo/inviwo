/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2019 Inviwo Foundation
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

#include <inviwo/core/properties/stipplingproperty.h>

namespace inviwo {

const std::string StipplingProperty::classIdentifier = "org.inviwo.StipplingProperty";
std::string StipplingProperty::getClassIdentifier() const { return classIdentifier; }

StipplingProperty::StipplingProperty(const std::string& identifier, const std::string& displayName,
                                     InvalidationLevel invalidationLevel,
                                     PropertySemantics semantics)
    : CompositeProperty(identifier, displayName, invalidationLevel, semantics)
    , mode_("stippleMode", "Stipple Mode",
            {{"none", "None", Mode::None},
             {"screenspace", "Screen Space", Mode::ScreenSpace},
             {"worldspace", "World Space", Mode::WorldSpace}},
            0, InvalidationLevel::InvalidResources)
    , length_("stippleLen", "Length", 30.0f, 0.0f, 100.0f)
    , spacing_("stippleSpacing", "Spacing", 10.0f, 0.0f, 100.0f)
    , offset_("stippleOffset", "Offset", 0.0f, 0.0f, 100.0f)
    , worldScale_("stippleWorldScale", "World Scale", 4.0f, 1.0f, 20.0f) {
    addProperty(mode_);
    addProperty(length_);
    addProperty(spacing_);
    addProperty(offset_);
    addProperty(worldScale_);

    mode_.onChange([this]() { worldScale_.setVisible(mode_.get() == Mode::WorldSpace); });
    worldScale_.setVisible(mode_.get() == Mode::WorldSpace);
}

StipplingProperty::StipplingProperty(const StipplingProperty& rhs)
    : CompositeProperty(rhs)
    , mode_(rhs.mode_)
    , length_(rhs.length_)
    , spacing_(rhs.spacing_)
    , offset_(rhs.offset_)
    , worldScale_(rhs.worldScale_) {

    addProperty(mode_);
    addProperty(length_);
    addProperty(spacing_);
    addProperty(offset_);
    addProperty(worldScale_);

    mode_.onChange([this]() { worldScale_.setVisible(mode_.get() == Mode::WorldSpace); });
    worldScale_.setVisible(mode_.get() == Mode::WorldSpace);
}

StipplingProperty* StipplingProperty::clone() const { return new StipplingProperty(*this); }

}  // namespace inviwo
