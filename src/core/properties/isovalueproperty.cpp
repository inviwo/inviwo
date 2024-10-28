/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2024 Inviwo Foundation
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
std::string_view IsoValueProperty::getClassIdentifier() const { return classIdentifier; }

IsoValueProperty::IsoValueProperty(std::string_view identifier, std::string_view displayName,
                                   Document help, const IsoValueCollection& value, TFData port,
                                   PropertySemantics semantics)
    : Property(identifier, displayName, std::move(help), InvalidationLevel::InvalidResources,
               std::move(semantics))
    , iso_("IsoValues", value)
    , zoomH_("zoomH_", dvec2(0.0, 1.0))
    , zoomV_("zoomV_", dvec2(0.0, 1.0))
    , histogramMode_("showHistogram_", HistogramMode::All)
    , histogramSelection_("histogramSelection", histogramSelectionAll)
    , data_{std::move(port)} {

    iso_.value.addObserver(this);
}

IsoValueProperty::IsoValueProperty(std::string_view identifier, std::string_view displayName,
                                   const IsoValueCollection& value, TFData port,
                                   PropertySemantics semantics)
    : IsoValueProperty(identifier, displayName, Document{}, value, std::move(port),
                       std::move(semantics)) {}

IsoValueProperty::IsoValueProperty(std::string_view identifier, std::string_view displayName,
                                   TFData port, PropertySemantics semantics)
    : IsoValueProperty(identifier, displayName, {}, std::move(port), std::move(semantics)) {}

IsoValueProperty::IsoValueProperty(const IsoValueProperty& rhs)
    : Property(rhs)
    , iso_{rhs.iso_}
    , zoomH_(rhs.zoomH_)
    , zoomV_(rhs.zoomV_)
    , histogramMode_(rhs.histogramMode_)
    , histogramSelection_(rhs.histogramSelection_)
    , data_{rhs.data_} {

    iso_.value.addObserver(this);
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

IsoValueProperty& IsoValueProperty::setHistogramMode(HistogramMode mode) {
    if (histogramMode_ != mode) {
        histogramMode_ = mode;
        notifyHistogramModeChange(histogramMode_);
    }
    return *this;
}

HistogramMode IsoValueProperty::getHistogramMode() const { return histogramMode_; }

IsoValueProperty& IsoValueProperty::setHistogramSelection(HistogramSelection selection) {
    if (histogramSelection_ != selection) {
        histogramSelection_ = selection;
        notifyHistogramSelectionChange(histogramSelection_);
    }
    return *this;
}

auto IsoValueProperty::getHistogramSelection() const -> HistogramSelection {
    return histogramSelection_;
}

IsoValueProperty& IsoValueProperty::setCurrentStateAsDefault() {
    Property::setCurrentStateAsDefault();
    iso_.setAsDefault();
    zoomH_.setAsDefault();
    zoomV_.setAsDefault();
    histogramMode_.setAsDefault();
    histogramSelection_.setAsDefault();
    return *this;
}

IsoValueProperty& IsoValueProperty::setDefault(const IsoValueCollection& iso) {
    iso_.defaultValue = iso;
    return *this;
}

IsoValueProperty& IsoValueProperty::resetToDefaultState() {
    const NetworkLock lock(this);

    bool modified = false;
    modified |= iso_.reset();
    modified |= zoomH_.reset();
    modified |= zoomV_.reset();
    modified |= histogramMode_.reset();
    modified |= histogramSelection_.reset();
    if (modified) this->propertyModified();
    return *this;
}

void IsoValueProperty::serialize(Serializer& s) const {
    Property::serialize(s);

    iso_.serialize(s, this->serializationMode_);
    zoomH_.serialize(s, this->serializationMode_);
    zoomV_.serialize(s, this->serializationMode_);
    histogramMode_.serialize(s, this->serializationMode_);
    histogramSelection_.serialize(s, this->serializationMode_);
}

void IsoValueProperty::deserialize(Deserializer& d) {
    Property::deserialize(d);

    bool modified = false;
    modified |= iso_.deserialize(d, this->serializationMode_);
    modified |= zoomH_.deserialize(d, this->serializationMode_);
    modified |= zoomV_.deserialize(d, this->serializationMode_);
    modified |= histogramMode_.deserialize(d, this->serializationMode_);
    modified |= histogramSelection_.deserialize(d, this->serializationMode_);
    if (modified) propertyModified();
}

IsoValueCollection& IsoValueProperty::get() { return iso_.value; }

const IsoValueCollection& IsoValueProperty::get() const { return iso_.value; }

IsoValueProperty& IsoValueProperty::set(const IsoValueCollection& iso) {
    iso_.value.removeObserver(this);
    if (iso_.update(iso)) propertyModified();
    iso_.value.addObserver(this);
    return *this;
}

const IsoValueCollection& IsoValueProperty::operator*() const { return iso_.value; }
IsoValueCollection& IsoValueProperty::operator*() { return iso_.value; }
const IsoValueCollection* IsoValueProperty::operator->() const { return &iso_.value; }
IsoValueCollection* IsoValueProperty::operator->() { return &iso_.value; }

void IsoValueProperty::set(const Property* property) {
    if (const auto* iso = dynamic_cast<const IsoValueProperty*>(property)) {
        set(iso);
    } else if (const auto* isotf = dynamic_cast<const IsoTFProperty*>(property)) {
        set(isotf);
    }
}

void IsoValueProperty::set(const IsoTFProperty* p) { set(p->isovalues_.get()); }

void IsoValueProperty::set(const IsoValueProperty* p) { set(p->iso_.value); }

void IsoValueProperty::onTFPrimitiveAdded(const TFPrimitiveSet&, TFPrimitive&) {
    setInvalidationLevel(InvalidationLevel::InvalidResources);
    propertyModified();
}

void IsoValueProperty::onTFPrimitiveRemoved(const TFPrimitiveSet&, TFPrimitive&) {
    setInvalidationLevel(InvalidationLevel::InvalidResources);
    propertyModified();
}

void IsoValueProperty::onTFPrimitiveChanged(const TFPrimitiveSet&, const TFPrimitive&) {
    setInvalidationLevel(InvalidationLevel::InvalidOutput);
    propertyModified();
}

}  // namespace inviwo
