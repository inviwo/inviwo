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

#include <inviwo/core/properties/transferfunctionproperty.h>
#include <inviwo/core/network/processornetwork.h>
#include <inviwo/core/network/networklock.h>
#include <inviwo/core/properties/isotfproperty.h>

namespace inviwo {

const std::string TransferFunctionProperty::classIdentifier = "org.inviwo.TransferFunctionProperty";
std::string TransferFunctionProperty::getClassIdentifier() const { return classIdentifier; }

void TFPropertyObserver::onMaskChange(const dvec2&) {}

void TFPropertyObserver::onZoomHChange(const dvec2&) {}

void TFPropertyObserver::onZoomVChange(const dvec2&) {}

void TFPropertyObserver::onHistogramModeChange(HistogramMode) {}

void TFPropertyObservable::notifyMaskChange(const dvec2& mask) {
    forEachObserver([&](TFPropertyObserver* o) { o->onMaskChange(mask); });
}

void TFPropertyObservable::notifyZoomHChange(const dvec2& zoomH) {
    forEachObserver([&](TFPropertyObserver* o) { o->onZoomHChange(zoomH); });
}

void TFPropertyObservable::notifyZoomVChange(const dvec2& zoomV) {
    forEachObserver([&](TFPropertyObserver* o) { o->onZoomVChange(zoomV); });
}

void TFPropertyObservable::notifyHistogramModeChange(HistogramMode mode) {
    forEachObserver([&](TFPropertyObserver* o) { o->onHistogramModeChange(mode); });
}

TransferFunctionProperty::TransferFunctionProperty(
    const std::string& identifier, const std::string& displayName, const TransferFunction& value,
    VolumeInport* volumeInport, InvalidationLevel invalidationLevel, PropertySemantics semantics)
    : TemplateProperty<TransferFunction>(identifier, displayName, value, invalidationLevel,
                                         semantics)
    , zoomH_("zoomH_", dvec2(0.0, 1.0))
    , zoomV_("zoomV_", dvec2(0.0, 1.0))
    , histogramMode_("showHistogram_", HistogramMode::All)
    , volumeInport_(volumeInport) {

    // rename the "value" to make the serialized file easier to understand.
    this->value_.name = "TransferFunction";
    this->value_.value.addObserver(this);
}

TransferFunctionProperty::TransferFunctionProperty(const std::string& identifier,
                                                   const std::string& displayName,
                                                   VolumeInport* volumeInport,
                                                   InvalidationLevel invalidationLevel,
                                                   PropertySemantics semantics)
    : TransferFunctionProperty(identifier, displayName,
                               TransferFunction({{0.0, vec4(0.0f, 0.0f, 0.0f, 0.0f)},
                                                 {1.0, vec4(1.0f, 1.0f, 1.0f, 1.0f)}}),
                               volumeInport, invalidationLevel, semantics) {}

TransferFunctionProperty::TransferFunctionProperty(const TransferFunctionProperty& rhs)
    : TemplateProperty<TransferFunction>(rhs)
    , zoomH_(rhs.zoomH_)
    , zoomV_(rhs.zoomV_)
    , histogramMode_(rhs.histogramMode_)
    , volumeInport_(rhs.volumeInport_) {

    this->value_.value.addObserver(this);
}

TransferFunctionProperty* TransferFunctionProperty::clone() const {
    return new TransferFunctionProperty(*this);
}

TransferFunctionProperty::~TransferFunctionProperty() { volumeInport_ = nullptr; }

TransferFunctionProperty& TransferFunctionProperty::setHistogramMode(HistogramMode mode) {
    if (histogramMode_ != mode) {
        histogramMode_ = mode;
        notifyHistogramModeChange(histogramMode_);
    }
    return *this;
}

auto TransferFunctionProperty::getHistogramMode() -> HistogramMode { return histogramMode_; }

VolumeInport* TransferFunctionProperty::getVolumeInport() { return volumeInport_; }

TransferFunctionProperty& TransferFunctionProperty::resetToDefaultState() {
    NetworkLock lock(this);
    zoomH_.reset();
    zoomV_.reset();
    histogramMode_.reset();
    TemplateProperty<TransferFunction>::resetToDefaultState();
    return *this;
}

TransferFunctionProperty& TransferFunctionProperty::setCurrentStateAsDefault() {
    TemplateProperty<TransferFunction>::setCurrentStateAsDefault();
    zoomH_.setAsDefault();
    zoomV_.setAsDefault();
    histogramMode_.setAsDefault();
    return *this;
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

TransferFunctionProperty& TransferFunctionProperty::setMask(double maskMin, double maskMax) {
    if (maskMax < maskMin) {
        maskMax = maskMin;
    }

    if (this->value_.value.getMaskMin() != maskMax || this->value_.value.getMaskMax() != maskMax) {
        this->value_.value.setMaskMin(maskMin);
        this->value_.value.setMaskMax(maskMax);

        notifyMaskChange(dvec2(maskMin, maskMax));

        propertyModified();
    }
    return *this;
}

TransferFunctionProperty& TransferFunctionProperty::clearMask() {
    auto prevMask = getMask();

    this->value_.value.clearMask();
    if (getMask() != prevMask) {
        notifyMaskChange(getMask());
    }
    propertyModified();
    return *this;
}

const dvec2 TransferFunctionProperty::getMask() const {
    return dvec2(this->value_.value.getMaskMin(), this->value_.value.getMaskMax());
}

const dvec2& TransferFunctionProperty::getZoomH() const { return zoomH_; }

TransferFunctionProperty& TransferFunctionProperty::setZoomH(double zoomHMin, double zoomHMax) {
    if (zoomHMax < zoomHMin) {
        zoomHMax = zoomHMin;
    }

    const auto newZoomH = dvec2(zoomHMin, zoomHMax);
    if (zoomH_ != newZoomH) {
        zoomH_ = newZoomH;
        notifyZoomHChange(zoomH_);
    }
    return *this;
}

const dvec2& TransferFunctionProperty::getZoomV() const { return zoomV_; }

TransferFunctionProperty& TransferFunctionProperty::setZoomV(double zoomVMin, double zoomVMax) {
    if (zoomVMax < zoomVMin) {
        zoomVMax = zoomVMin;
    }

    const auto newZoomV = dvec2(zoomVMin, zoomVMax);
    if (zoomV_ != newZoomV) {
        zoomV_ = newZoomV;
        notifyZoomVChange(zoomV_);
    }
    return *this;
}

void TransferFunctionProperty::set(const TransferFunction& value) {
    this->value_.value.removeObserver(this);
    TemplateProperty<TransferFunction>::set(value);
    this->value_.value.addObserver(this);
}

void TransferFunctionProperty::set(const IsoTFProperty& p) { set(p.tf_.get()); }

void TransferFunctionProperty::set(const Property* property) {
    if (auto tfp = dynamic_cast<const TransferFunctionProperty*>(property)) {
        TemplateProperty<TransferFunction>::set(tfp);
    } else if (auto isotfprop = dynamic_cast<const IsoTFProperty*>(property)) {
        TemplateProperty<TransferFunction>::set(&isotfprop->tf_);
    }
}

void TransferFunctionProperty::onTFPrimitiveAdded(TFPrimitive&) { propertyModified(); }

void TransferFunctionProperty::onTFPrimitiveRemoved(TFPrimitive&) { propertyModified(); }

void TransferFunctionProperty::onTFPrimitiveChanged(const TFPrimitive&) { propertyModified(); }

void TransferFunctionProperty::onTFTypeChanged(const TFPrimitiveSet&) { propertyModified(); }

}  // namespace inviwo
