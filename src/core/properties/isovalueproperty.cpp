/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2019 Inviwo Foundation
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
#include <inviwo/core/properties/isotfproperty.h>

namespace inviwo {

const std::string IsoValueProperty::classIdentifier = "org.inviwo.IsoValueProperty";
std::string IsoValueProperty::getClassIdentifier() const { return classIdentifier; }

IsoValueProperty::IsoValueProperty(const std::string& identifier, const std::string& displayName,
                                   const IsoValueCollection& value, VolumeInport* volumeInport,
                                   InvalidationLevel invalidationLevel, PropertySemantics semantics)
    : TemplateProperty<IsoValueCollection>(identifier, displayName, value, invalidationLevel,
                                           semantics)
    , zoomH_("zoomH_", dvec2(0.0, 1.0))
    , zoomV_("zoomV_", dvec2(0.0, 1.0))
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
    , zoomH_(rhs.zoomH_)
    , zoomV_(rhs.zoomV_)
    , histogramMode_(rhs.histogramMode_)
    , volumeInport_(rhs.volumeInport_) {

    value_.value.addObserver(this);
}
IsoValueProperty::~IsoValueProperty() = default;

IsoValueProperty* IsoValueProperty::clone() const { return new IsoValueProperty(*this); }

void IsoValueProperty::setZoomH(double zoomHMin, double zoomHMax) {
    if (zoomHMax < zoomHMin) {
        zoomHMax = zoomHMin;
    }

    const auto newZoomH = dvec2(zoomHMin, zoomHMax);
    if (zoomH_ != newZoomH) {
        zoomH_ = newZoomH;
        notifyZoomHChange(zoomH_);
    }
}

const dvec2& IsoValueProperty::getZoomH() const { return zoomH_; }

void IsoValueProperty::setZoomV(double zoomVMin, double zoomVMax) {
    if (zoomVMax < zoomVMin) {
        zoomVMax = zoomVMin;
    }

    const auto newZoomV = dvec2(zoomVMin, zoomVMax);
    if (zoomV_ != newZoomV) {
        zoomV_ = newZoomV;
        notifyZoomVChange(zoomV_);
    }
}

const dvec2& IsoValueProperty::getZoomV() const { return zoomV_; }

void IsoValueProperty::setHistogramMode(HistogramMode mode) {
    if (histogramMode_ != mode) {
        histogramMode_ = mode;
        notifyHistogramModeChange(histogramMode_);
    }
}

HistogramMode IsoValueProperty::getHistogramMode() { return histogramMode_; }

VolumeInport* IsoValueProperty::getVolumeInport() { return volumeInport_; }

IsoValueProperty& IsoValueProperty::setCurrentStateAsDefault() {
    TemplateProperty<IsoValueCollection>::setCurrentStateAsDefault();
    zoomH_.setAsDefault();
    zoomV_.setAsDefault();
    histogramMode_.setAsDefault();
    return *this;
}

IsoValueProperty& IsoValueProperty::resetToDefaultState() {
    NetworkLock lock(this);
    zoomH_.reset();
    zoomV_.reset();
    histogramMode_.reset();
    TemplateProperty<IsoValueCollection>::resetToDefaultState();
    return *this;
}

void IsoValueProperty::serialize(Serializer& s) const {
    Property::serialize(s);

    zoomH_.serialize(s, this->serializationMode_);
    zoomV_.serialize(s, this->serializationMode_);
    histogramMode_.serialize(s, this->serializationMode_);
    value_.serialize(s, this->serializationMode_);
}

void IsoValueProperty::deserialize(Deserializer& d) {
    Property::deserialize(d);

    bool modified = false;
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

void IsoValueProperty::set(const IsoTFProperty& p) { set(p.isovalues_.get()); }

void IsoValueProperty::set(const Property* property) {
    if (auto isoprop = dynamic_cast<const IsoValueProperty*>(property)) {
        TemplateProperty<IsoValueCollection>::set(isoprop);
    } else if (auto isotfprop = dynamic_cast<const IsoTFProperty*>(property)) {
        TemplateProperty<IsoValueCollection>::set(&isotfprop->isovalues_);
    }
}

void IsoValueProperty::onTFPrimitiveAdded(TFPrimitive&) {
    setInvalidationLevel(InvalidationLevel::InvalidResources);
    propertyModified();
}

void IsoValueProperty::onTFPrimitiveRemoved(TFPrimitive&) {
    setInvalidationLevel(InvalidationLevel::InvalidResources);
    propertyModified();
}

void IsoValueProperty::onTFPrimitiveChanged(const TFPrimitive&) {
    setInvalidationLevel(InvalidationLevel::InvalidOutput);
    propertyModified();
}

}  // namespace inviwo
