/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2019 Inviwo Foundation
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
#include <inviwo/core/properties/volumeindicatorproperty.h>

namespace inviwo {

const std::string VolumeIndicatorProperty::classIdentifier = "org.inviwo.VolumeIndicatorProperty";
std::string VolumeIndicatorProperty::getClassIdentifier() const { return classIdentifier; }

VolumeIndicatorProperty::VolumeIndicatorProperty(std::string identifier, std::string displayName,
                                                 InvalidationLevel invalidationLevel,
                                                 PropertySemantics semantics)
    : BoolCompositeProperty(identifier, displayName, false, invalidationLevel, semantics)
    , plane1_("plane1", "Plane 1", vec3(0.5f), vec3(0.0f, 0.0f, 1.0f))
    , plane2_("plane2", "Plane 2", vec3(0.5f), vec3(0.0f, 1.0f, 0.0f))
    , plane3_("plane3", "Plane 3", vec3(0.5f), vec3(1.0f, 0.0f, 0.0f)) {

    getBoolProperty()->setIdentifier("enable");
    getBoolProperty()->setInvalidationLevel(InvalidationLevel::InvalidResources);
    plane1_.getBoolProperty()->setInvalidationLevel(InvalidationLevel::InvalidResources);
    plane2_.getBoolProperty()->setInvalidationLevel(InvalidationLevel::InvalidResources);
    plane3_.getBoolProperty()->setInvalidationLevel(InvalidationLevel::InvalidResources);

    addProperty(plane1_);
    addProperty(plane2_);
    addProperty(plane3_);

    setCollapsed(true);
}

VolumeIndicatorProperty::VolumeIndicatorProperty(const VolumeIndicatorProperty& rhs)
    : BoolCompositeProperty(rhs), plane1_(rhs.plane1_), plane2_(rhs.plane2_), plane3_(rhs.plane3_) {

    addProperty(plane1_);
    addProperty(plane2_);
    addProperty(plane3_);
}

VolumeIndicatorProperty* VolumeIndicatorProperty::clone() const {
    return new VolumeIndicatorProperty(*this);
}

VolumeIndicatorProperty::~VolumeIndicatorProperty() = default;

}  // namespace inviwo
