/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2016 Inviwo Foundation
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
    , histogramMode_("showHistogram_", HistogramMode::All)
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
    , histogramMode_(rhs.histogramMode_)
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
        histogramMode_ = that.histogramMode_;
        volumeInport_ = that.volumeInport_;
    }
    return *this;
}

TransferFunctionProperty* TransferFunctionProperty::clone() const {
    return new TransferFunctionProperty(*this);
}

TransferFunctionProperty::~TransferFunctionProperty() { volumeInport_ = nullptr; }

void TransferFunctionProperty::setHistogramMode(HistogramMode mode) { 
    if(histogramMode_ != mode) {
        histogramMode_ = mode;
        notifyHistogramModeChange(histogramMode_);
    }
}

auto TransferFunctionProperty::getHistogramMode() -> HistogramMode { return histogramMode_; }

VolumeInport* TransferFunctionProperty::getVolumeInport() { return volumeInport_; }

void TransferFunctionProperty::resetToDefaultState() {
    NetworkLock lock(this);
    zoomH_.reset();
    zoomV_.reset();
    histogramMode_.reset();
    TemplateProperty<TransferFunction>::resetToDefaultState();
}
void TransferFunctionProperty::setCurrentStateAsDefault() {
    TemplateProperty<TransferFunction>::setCurrentStateAsDefault();
    zoomH_.setAsDefault();
    zoomV_.setAsDefault();
    histogramMode_.setAsDefault();
}

void TransferFunctionProperty::serialize(Serializer& s) const {
    Property::serialize(s);

    zoomH_.serialize(s, this->serializationMode_);
    zoomV_.serialize(s, this->serializationMode_);
    histogramMode_.serialize(s, this->serializationMode_);
    value_.serialize(s, this->serializationMode_);
}

void TransferFunctionProperty::deserialize(Deserializer& d) {
    Property::deserialize(d);

    bool modified = false;
    modified |= zoomH_.deserialize(d, this->serializationMode_);
    modified |= zoomV_.deserialize(d, this->serializationMode_);
    modified |= histogramMode_.deserialize(d, this->serializationMode_);
    modified |= value_.deserialize(d, this->serializationMode_);
    if (modified) propertyModified();
}

void TransferFunctionProperty::setMask(float maskMin, float maskMax) {
    if (maskMax < maskMin) {
        maskMax = maskMin;
    }

    if (this->value_.value.getMaskMin() != maskMax || this->value_.value.getMaskMax() != maskMax) {
        this->value_.value.setMaskMin(maskMin);
        this->value_.value.setMaskMax(maskMax);

        notifyMaskChange(vec2(maskMin, maskMax));

        propertyModified();
    }
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

    const auto newZoomH = vec2(zoomHMin, zoomHMax);;
    if (zoomH_ != newZoomH) {
      zoomH_ = newZoomH;
      notifyZoomHChange(zoomH_);
    }
}

const vec2& TransferFunctionProperty::getZoomV() const {
    return zoomV_;
}

void TransferFunctionProperty::setZoomV(float zoomVMin, float zoomVMax) {
    if (zoomVMax < zoomVMin) {
        zoomVMax = zoomVMin;
    }

    const auto newZoomV = vec2(zoomVMin, zoomVMax);;
    if (zoomV_ != newZoomV) {
        zoomV_ = newZoomV;
        notifyZoomVChange(zoomV_);
    }
}

void TransferFunctionProperty::set(const TransferFunction& value) {
    this->value_.value.removeObserver(this);
    TemplateProperty<TransferFunction>::set(value);
    this->value_.value.addObserver(this);
}

void TransferFunctionProperty::set(const Property *property) {
    if (auto tfp = dynamic_cast<const TransferFunctionProperty*>(property)) {
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

void TransferFunctionPropertyObservable::notifyMaskChange(const vec2& mask) {
    forEachObserver([&](TransferFunctionPropertyObserver* o) { o->onMaskChange(mask); });
}

void TransferFunctionPropertyObservable::notifyZoomHChange(const vec2& zoomH) {
    forEachObserver([&](TransferFunctionPropertyObserver* o) { o->onZoomHChange(zoomH); });
}

void TransferFunctionPropertyObservable::notifyZoomVChange(const vec2& zoomV) {
    forEachObserver([&](TransferFunctionPropertyObserver* o) { o->onZoomVChange(zoomV); });
}

void TransferFunctionPropertyObservable::notifyHistogramModeChange(HistogramMode mode) {
    forEachObserver([&](TransferFunctionPropertyObserver* o) { o->onHistogramModeChange(mode); });
}

} // namespace
