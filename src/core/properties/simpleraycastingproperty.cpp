/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2015 Inviwo Foundation
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

#include <inviwo/core/properties/simpleraycastingproperty.h>

namespace inviwo {

PropertyClassIdentifier(SimpleRaycastingProperty, "org.inviwo.SimpleRaycastingProperty");

SimpleRaycastingProperty::SimpleRaycastingProperty(
    std::string identifier, std::string displayName,
    InvalidationLevel invalidationLevel, PropertySemantics semantics)
    : CompositeProperty(identifier, displayName, invalidationLevel, semantics)
    , classificationMode_("classificationMode", "Classification", INVALID_RESOURCES)
    , compositingMode_("compositingMode", "Compositing", INVALID_RESOURCES)
    , gradientComputationMode_("gradientComputationMode", "Gradient",
    INVALID_RESOURCES)
    , samplingRate_("samplingRate", "Sampling rate", 2.0f, 1.0f, 10.0f)
    , isoValue_("isoValue", "Iso value", 0.5f, 0.0f, 1.0f) {

    classificationMode_.addOption("none", "None");
    classificationMode_.addOption("transfer-function", "Transfer function");
    classificationMode_.setSelectedIdentifier("transfer-function");
    classificationMode_.setCurrentStateAsDefault();
    addProperty(classificationMode_);

    compositingMode_.addOption("dvr", "Direct volume rendering");
    compositingMode_.addOption("mip", "Maximum intensity projection");
    compositingMode_.addOption("fhp", "First hit points");
    compositingMode_.addOption("fhn", "First hit normals");
    compositingMode_.addOption("fhnvs", "First hit normals (ViewSpace)");
    compositingMode_.addOption("fhd", "First hit depth");
    compositingMode_.addOption("iso", "Iso surface rendering");
    compositingMode_.addOption("ison", "Iso surface normal rendering");
    compositingMode_.setSelectedIdentifier("dvr");
    compositingMode_.setCurrentStateAsDefault();
    addProperty(compositingMode_);

    gradientComputationMode_.addOption("none", "None");
    gradientComputationMode_.addOption("forward", "Forward differences");
    gradientComputationMode_.addOption("backward", "Backward differences");
    gradientComputationMode_.addOption("central", "Central differences");
    gradientComputationMode_.addOption("central-higher", "Higher order central differences");
    gradientComputationMode_.addOption("precomputedXYZ", "Pre-computed gradients (xyz)");
    gradientComputationMode_.addOption("precomputedYZW", "Pre-computed gradients (yzw)");
    gradientComputationMode_.setSelectedIdentifier("central");
    gradientComputationMode_.setCurrentStateAsDefault();
    addProperty(gradientComputationMode_);

    addProperty(samplingRate_);
    addProperty(isoValue_);
}

SimpleRaycastingProperty::SimpleRaycastingProperty(const SimpleRaycastingProperty& rhs)
    : CompositeProperty(rhs)
    , classificationMode_(rhs.classificationMode_)
    , compositingMode_(rhs.compositingMode_)
    , gradientComputationMode_(rhs.gradientComputationMode_)
    , samplingRate_(rhs.samplingRate_) 
    , isoValue_(rhs.isoValue_) {
    
    addProperty(classificationMode_);
    addProperty(compositingMode_);
    addProperty(gradientComputationMode_);
    addProperty(samplingRate_);
    addProperty(isoValue_);
}

SimpleRaycastingProperty& SimpleRaycastingProperty::operator=(const SimpleRaycastingProperty& that) {
    if (this != &that) {
        CompositeProperty::operator=(that);
        classificationMode_ = that.classificationMode_;
        compositingMode_ = that.compositingMode_;
        gradientComputationMode_ = that.gradientComputationMode_;
        samplingRate_ = that.samplingRate_;
        isoValue_ = that.isoValue_;
    }
    return *this;
}

SimpleRaycastingProperty* SimpleRaycastingProperty::clone() const {
    return new SimpleRaycastingProperty(*this);
}

SimpleRaycastingProperty::~SimpleRaycastingProperty() {}

}  // namespace
