/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018 Inviwo Foundation
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

#include <inviwo/core/properties/isovalueproperty.h>
#include <inviwo/core/network/networklock.h>

namespace inviwo {

PropertyClassIdentifier(IsoValueProperty, "org.inviwo.IsoValueProperty");

void IsoValuePropertyObserver::onEnabledChange(bool) {}

void IsoValuePropertyObserver::onZoomHChange(const vec2&) {}

void IsoValuePropertyObserver::onZoomVChange(const vec2&) {}

void IsoValuePropertyObserver::onHistogramModeChange(HistogramMode) {}

void IsoValuePropertyObservable::notifyEnabledChange(bool enabled) {
    forEachObserver([&](IsoValuePropertyObserver* o) { o->onEnabledChange(enabled); });
}

void IsoValuePropertyObservable::notifyZoomHChange(const vec2& zoomH) {
    forEachObserver([&](IsoValuePropertyObserver* o) { o->onZoomHChange(zoomH); });
}

void IsoValuePropertyObservable::notifyZoomVChange(const vec2& zoomV) {
    forEachObserver([&](IsoValuePropertyObserver* o) { o->onZoomVChange(zoomV); });
}

void IsoValuePropertyObservable::notifyHistogramModeChange(HistogramMode mode) {
    forEachObserver([&](IsoValuePropertyObserver* o) { o->onHistogramModeChange(mode); });
}

IsoValueProperty::IsoValueProperty(const std::string& identifier, const std::string& displayName,
                                   const IsoValueCollection& value, VolumeInport* volumeInport,
                                   InvalidationLevel invalidationLevel, PropertySemantics semantics)
    : TemplateProperty<IsoValueCollection>(identifier, displayName, value, invalidationLevel,
                                           semantics)
    , enabled_("enabled", true)
    , zoomH_("zoomH_", vec2(0.0f, 1.0f))
    , zoomV_("zoomV_", vec2(0.0f, 1.0f))
    , histogramMode_("showHistogram_", HistogramMode::All)
    , volumeInport_(volumeInport) {

    // rename the "value" to make the serialized file easier to understand.
    value_.name = "IsoValues";
    value_.value.addObserver(this);
}

IsoValueProperty::IsoValueProperty(const std::string& identifier, const std::string& displayName,
                                   VolumeInport* volumeInport, InvalidationLevel invalidationLevel,
                                   PropertySemantics semantics)
    : IsoValueProperty(identifier, displayName, {}, volumeInport, invalidationLevel, semantics) {}

IsoValueProperty::IsoValueProperty(const IsoValueProperty& rhs)
    : TemplateProperty<IsoValueCollection>(rhs)
    , enabled_(rhs.enabled_)
    , zoomH_(rhs.zoomH_)
    , zoomV_(rhs.zoomV_)
    , histogramMode_(rhs.histogramMode_)
    , volumeInport_(rhs.volumeInport_) {

    value_.value.addObserver(this);
}
IsoValueProperty::~IsoValueProperty() = default;

IsoValueProperty& IsoValueProperty::operator=(const IsoValueProperty& rhs) {
    if (this != &rhs) {
        value_.value.removeObserver(this);
        TemplateProperty<IsoValueCollection>::operator=(rhs);
        value_.value.addObserver(this);
        enabled_ = rhs.enabled_;
        zoomH_ = rhs.zoomH_;
        zoomV_ = rhs.zoomV_;
        histogramMode_ = rhs.histogramMode_;
        volumeInport_ = rhs.volumeInport_;
    }
    return *this;
}
IsoValueProperty* IsoValueProperty::clone() const { return new IsoValueProperty(*this); }

void IsoValueProperty::setEnabled(bool enable) {
    if (enabled_ != enable) {
        enabled_ = enable;
        notifyEnabledChange(enabled_);
    }
}

bool IsoValueProperty::getEnabled() const { return enabled_; }

void IsoValueProperty::setZoomH(float zoomHMin, float zoomHMax) {
    if (zoomHMax < zoomHMin) {
        zoomHMax = zoomHMin;
    }

    const auto newZoomH = vec2(zoomHMin, zoomHMax);
    if (zoomH_ != newZoomH) {
        zoomH_ = newZoomH;
        notifyZoomHChange(zoomH_);
    }
}

const vec2& IsoValueProperty::getZoomH() const { return zoomH_; }

void IsoValueProperty::setZoomV(float zoomVMin, float zoomVMax) {
    if (zoomVMax < zoomVMin) {
        zoomVMax = zoomVMin;
    }

    const auto newZoomV = vec2(zoomVMin, zoomVMax);
    if (zoomV_ != newZoomV) {
        zoomV_ = newZoomV;
        notifyZoomVChange(zoomV_);
    }
}

const vec2& IsoValueProperty::getZoomV() const { return zoomV_; }

void IsoValueProperty::setHistogramMode(HistogramMode mode) {
    if (histogramMode_ != mode) {
        histogramMode_ = mode;
        notifyHistogramModeChange(histogramMode_);
    }
}

HistogramMode IsoValueProperty::getHistogramMode() { return histogramMode_; }

VolumeInport* IsoValueProperty::getVolumeInport() { return volumeInport_; }

void IsoValueProperty::setCurrentStateAsDefault() {
    TemplateProperty<IsoValueCollection>::setCurrentStateAsDefault();
    enabled_.setAsDefault();
    zoomH_.setAsDefault();
    zoomV_.setAsDefault();
    histogramMode_.setAsDefault();
}

void IsoValueProperty::resetToDefaultState() {
    NetworkLock lock(this);
    enabled_.reset();
    zoomH_.reset();
    zoomV_.reset();
    histogramMode_.reset();
    TemplateProperty<IsoValueCollection>::resetToDefaultState();
}

void IsoValueProperty::serialize(Serializer& s) const {
    Property::serialize(s);

    enabled_.serialize(s, this->serializationMode_);
    zoomH_.serialize(s, this->serializationMode_);
    zoomV_.serialize(s, this->serializationMode_);
    histogramMode_.serialize(s, this->serializationMode_);
    value_.serialize(s, this->serializationMode_);
}

void IsoValueProperty::deserialize(Deserializer& d) {
    Property::deserialize(d);

    bool modified = false;
    modified |= enabled_.deserialize(d, this->serializationMode_);
    modified |= zoomH_.deserialize(d, this->serializationMode_);
    modified |= zoomV_.deserialize(d, this->serializationMode_);
    modified |= histogramMode_.deserialize(d, this->serializationMode_);
    modified |= value_.deserialize(d, this->serializationMode_);
    if (modified) propertyModified();
}

// Overrides
void IsoValueProperty::set(const IsoValueCollection& c) {
    this->value_.value.removeObserver(this);
    TemplateProperty<IsoValueCollection>::set(c);
    this->value_.value.addObserver(this);
}

void IsoValueProperty::set(const Property* property) {
    if (auto tfp = dynamic_cast<const IsoValueProperty*>(property)) {
        TemplateProperty<IsoValueCollection>::set(tfp);
    }
}

void IsoValueProperty::onTFPrimitiveAdded(TFPrimitive*) { propertyModified(); }

void IsoValueProperty::onTFPrimitiveRemoved(TFPrimitive*) { propertyModified(); }

void IsoValueProperty::onTFPrimitiveChanged(const TFPrimitive*) { propertyModified(); }

}  // namespace inviwo
