/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2015 Inviwo Foundation
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

#include <inviwo/core/properties/transferfunctionproperty.h>
#include <inviwo/core/network/processornetwork.h>

namespace inviwo {

PropertyClassIdentifier(TransferFunctionProperty, "org.inviwo.TransferFunctionProperty");

TransferFunctionProperty::TransferFunctionProperty(const std::string &identifier,
                                                   const std::string &displayName,
                                                   const TransferFunction &value,
                                                   VolumeInport* volumeInport,
                                                   InvalidationLevel invalidationLevel,
                                                   PropertySemantics semantics)
    : TemplateProperty<TransferFunction>(identifier, displayName, value, invalidationLevel, semantics)
    , TransferFunctionObserver()
    , zoomH_("zoomH_", vec2(0.0f, 1.0f))
    , zoomV_("zoomV_", vec2(0.0f, 1.0f))
    , showHistogram_("showHistogram_", true)
    , volumeInport_(volumeInport) {
    
    // rename the "value" to make the serialized file easier to understand.
    this->value_.name = "transferFunction";
    this->value_.value.addObserver(this);
}

TransferFunctionProperty::TransferFunctionProperty(const TransferFunctionProperty& rhs)
    : TemplateProperty<TransferFunction>(rhs)
    , TransferFunctionObserver()
    , zoomH_(rhs.zoomH_)
    , zoomV_(rhs.zoomV_)
    , showHistogram_(rhs.showHistogram_)
    , volumeInport_(rhs.volumeInport_) {

    this->value_.value.addObserver(this);
}

TransferFunctionProperty& TransferFunctionProperty::operator=(const TransferFunctionProperty& that) {
    if (this != &that) {
        this->value_.value.removeObserver(this);
        TemplateProperty<TransferFunction>::operator=(that);
        this->value_.value.addObserver(this);
        zoomH_ = that.zoomH_;
        zoomV_ = that.zoomV_;
        showHistogram_ = that.showHistogram_;
        volumeInport_ = that.volumeInport_;
    }
    return *this;
}

TransferFunctionProperty* TransferFunctionProperty::clone() const {
    return new TransferFunctionProperty(*this);
}

TransferFunctionProperty::~TransferFunctionProperty() { volumeInport_ = nullptr; }

void TransferFunctionProperty::setShowHistogram(int type) {
    showHistogram_ = type;
}

int TransferFunctionProperty::getShowHistogram() {
    return showHistogram_;
}

VolumeInport* TransferFunctionProperty::getVolumeInport() {
    return volumeInport_;
}

void TransferFunctionProperty::resetToDefaultState() {
    NetworkLock lock;
    zoomH_.reset();
    zoomV_.reset();
    showHistogram_.reset();
    TemplateProperty<TransferFunction>::resetToDefaultState();
}
void TransferFunctionProperty::setCurrentStateAsDefault() {
    TemplateProperty<TransferFunction>::setCurrentStateAsDefault();
    zoomH_.setAsDefault();
    zoomV_.setAsDefault();
    showHistogram_.setAsDefault();
}

void TransferFunctionProperty::serialize(IvwSerializer& s) const {
    TemplateProperty<TransferFunction>::serialize(s);
    zoomH_.serialize(s, this->serializationMode_);
    zoomV_.serialize(s, this->serializationMode_);
    showHistogram_.serialize(s, this->serializationMode_);
}

void TransferFunctionProperty::deserialize(IvwDeserializer& d) {
    TemplateProperty<TransferFunction>::deserialize(d);
    zoomH_.deserialize(d);
    zoomV_.deserialize(d);
    showHistogram_.deserialize(d);
    propertyModified();
}

void TransferFunctionProperty::setMask(float maskMin, float maskMax) {
    if (maskMax < maskMin) {
        maskMax = maskMin;
    }
    this->value_.value.setMaskMin(maskMin);
    this->value_.value.setMaskMax(maskMax);
    propertyModified();
}

const vec2 TransferFunctionProperty::getMask() const {
    return vec2(this->value_.value.getMaskMin(), this->value_.value.getMaskMax());
}

const vec2& TransferFunctionProperty::getZoomH() const {
    return zoomH_;
}

void TransferFunctionProperty::setZoomH(float zoomHMin, float zoomHMax) {
    if (zoomHMax < zoomHMin) {
        zoomHMax = zoomHMin;
    }
    zoomH_ = vec2(zoomHMin, zoomHMax);
}

const vec2& TransferFunctionProperty::getZoomV() const {
    return zoomV_;
}

void TransferFunctionProperty::setZoomV(float zoomVMin, float zoomVMax) {
    if (zoomVMax < zoomVMin) {
        zoomVMax = zoomVMin;
    }
    zoomV_ = vec2(zoomVMin, zoomVMax);
}

void TransferFunctionProperty::set(const TransferFunction& value) {
    this->value_.value.removeObserver(this);
    TemplateProperty<TransferFunction>::set(value);
    this->value_.value.addObserver(this);
}

void TransferFunctionProperty::set(const Property *property) {
    if (auto tfp = dynamic_cast<const TransferFunctionProperty*>(property)) {
        set(tfp->get());
        TemplateProperty<TransferFunction>::set(tfp);
    }
}

void TransferFunctionProperty::onControlPointAdded(TransferFunctionDataPoint* p) {
    propertyModified();
}
void TransferFunctionProperty::onControlPointRemoved(TransferFunctionDataPoint* p) {
    propertyModified();
}
void TransferFunctionProperty::onControlPointChanged(const TransferFunctionDataPoint* p) {
    propertyModified();
}


} // namespace
