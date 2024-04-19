/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2024 Inviwo Foundation
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

void TFPropertyObserver::onZoomHChange(const dvec2&) {}

void TFPropertyObserver::onZoomVChange(const dvec2&) {}

void TFPropertyObserver::onHistogramModeChange(HistogramMode) {}

void TFPropertyObserver::onHistogramSelectionChange(HistogramSelection) {}

void TFPropertyObservable::notifyZoomHChange(const dvec2& zoomH) {
    forEachObserver([&](TFPropertyObserver* o) { o->onZoomHChange(zoomH); });
}

void TFPropertyObservable::notifyZoomVChange(const dvec2& zoomV) {
    forEachObserver([&](TFPropertyObserver* o) { o->onZoomVChange(zoomV); });
}

void TFPropertyObservable::notifyHistogramModeChange(HistogramMode mode) {
    forEachObserver([&](TFPropertyObserver* o) { o->onHistogramModeChange(mode); });
}

void TFPropertyObservable::notifyHistogramSelectionChange(HistogramSelection selection) {
    forEachObserver([&](TFPropertyObserver* o) { o->onHistogramSelectionChange(selection); });
}

TransferFunctionProperty::TransferFunctionProperty(std::string_view identifier,
                                                   std::string_view displayName, Document help,
                                                   const TransferFunction& value, TFData port,
                                                   InvalidationLevel invalidationLevel,
                                                   PropertySemantics semantics)
    : Property(identifier, displayName, std::move(help), invalidationLevel, std::move(semantics))
    , tf_{"TransferFunction", value}
    , zoomH_("zoomH_", dvec2(0.0, 1.0))
    , zoomV_("zoomV_", dvec2(0.0, 1.0))
    , histogramMode_("showHistogram_", port ? HistogramMode::All : HistogramMode::Off)
    , histogramSelection_("histogramSelection", histogramSelectionAll)
    , lookup_{tf_.value}
    , data_{std::move(port)} {

    tf_.value.addObserver(this);
}

TransferFunctionProperty::TransferFunctionProperty(std::string_view identifier,
                                                   std::string_view displayName,
                                                   const TransferFunction& value, TFData port,
                                                   InvalidationLevel invalidationLevel,
                                                   PropertySemantics semantics)
    : TransferFunctionProperty(identifier, displayName, Document{}, value, std::move(port),
                               invalidationLevel, std::move(semantics)) {}

TransferFunctionProperty::TransferFunctionProperty(std::string_view identifier,
                                                   std::string_view displayName, TFData port,
                                                   InvalidationLevel invalidationLevel,
                                                   PropertySemantics semantics)
    : TransferFunctionProperty(identifier, displayName,
                               TransferFunction({{0.0, vec4(0.0f, 0.0f, 0.0f, 0.0f)},
                                                 {1.0, vec4(1.0f, 1.0f, 1.0f, 1.0f)}}),
                               std::move(port), invalidationLevel, std::move(semantics)) {}

TransferFunctionProperty::TransferFunctionProperty(const TransferFunctionProperty& rhs)
    : Property(rhs)
    , tf_{rhs.tf_}
    , zoomH_(rhs.zoomH_)
    , zoomV_(rhs.zoomV_)
    , histogramMode_(rhs.histogramMode_)
    , histogramSelection_(rhs.histogramSelection_)
    , lookup_{tf_.value}
    , data_{rhs.data_} {

    tf_.value.addObserver(this);
}

TransferFunctionProperty* TransferFunctionProperty::clone() const {
    return new TransferFunctionProperty(*this);
}

TransferFunctionProperty::~TransferFunctionProperty() = default;

TransferFunction& TransferFunctionProperty::get() { return tf_.value; }

const TransferFunction& TransferFunctionProperty::get() const { return tf_.value; }

TransferFunctionProperty& TransferFunctionProperty::setHistogramMode(HistogramMode mode) {
    if (histogramMode_ != mode) {
        histogramMode_ = mode;
        notifyHistogramModeChange(histogramMode_);
    }
    return *this;
}

auto TransferFunctionProperty::getHistogramMode() const -> HistogramMode { return histogramMode_; }

TransferFunctionProperty& TransferFunctionProperty::setHistogramSelection(
    HistogramSelection selection) {
    if (histogramSelection_ != selection) {
        histogramSelection_ = selection;
        notifyHistogramSelectionChange(histogramSelection_);
    }
    return *this;
}

auto TransferFunctionProperty::getHistogramSelection() const -> HistogramSelection {
    return histogramSelection_;
}

TransferFunctionProperty& TransferFunctionProperty::resetToDefaultState() {
    const NetworkLock lock(this);

    bool modified = false;
    modified |= tf_.reset();
    modified |= zoomH_.reset();
    modified |= zoomV_.reset();
    modified |= histogramMode_.reset();
    modified |= histogramSelection_.reset();
    if (modified) this->propertyModified();
    return *this;
}

TransferFunctionProperty& TransferFunctionProperty::setCurrentStateAsDefault() {
    Property::setCurrentStateAsDefault();
    tf_.setAsDefault();
    zoomH_.setAsDefault();
    zoomV_.setAsDefault();
    histogramMode_.setAsDefault();
    histogramSelection_.setAsDefault();
    return *this;
}

TransferFunctionProperty& TransferFunctionProperty::setDefault(const TransferFunction& tf) {
    tf_.defaultValue = tf;
    return *this;
}

void TransferFunctionProperty::serialize(Serializer& s) const {
    Property::serialize(s);

    tf_.serialize(s, this->serializationMode_);
    zoomH_.serialize(s, this->serializationMode_);
    zoomV_.serialize(s, this->serializationMode_);
    histogramMode_.serialize(s, this->serializationMode_);
    histogramSelection_.serialize(s, this->serializationMode_);
}

void TransferFunctionProperty::deserialize(Deserializer& d) {
    Property::deserialize(d);

    bool modified = false;
    modified |= tf_.deserialize(d, this->serializationMode_);
    modified |= zoomH_.deserialize(d, this->serializationMode_);
    modified |= zoomV_.deserialize(d, this->serializationMode_);
    modified |= histogramMode_.deserialize(d, this->serializationMode_);
    modified |= histogramSelection_.deserialize(d, this->serializationMode_);
    if (modified) propertyModified();
}

TransferFunctionProperty& TransferFunctionProperty::setMask(double maskMin, double maskMax) {
    if (maskMax < maskMin) {
        maskMax = maskMin;
    }
    tf_.value.setMask(dvec2{maskMin, maskMax});
    return *this;
}

TransferFunctionProperty& TransferFunctionProperty::clearMask() {
    tf_.value.clearMask();
    return *this;
}

dvec2 TransferFunctionProperty::getMask() const { return tf_.value.getMask(); }

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

TransferFunctionProperty& TransferFunctionProperty::set(const TransferFunction& tf) {
    tf_.value.removeObserver(this);
    if (tf_.update(tf)) propertyModified();
    tf_.value.addObserver(this);
    return *this;
}

const TransferFunction& TransferFunctionProperty::operator*() const { return tf_.value; }
TransferFunction& TransferFunctionProperty::operator*() { return tf_.value; }
const TransferFunction* TransferFunctionProperty::operator->() const { return &tf_.value; }
TransferFunction* TransferFunctionProperty::operator->() { return &tf_.value; }

void TransferFunctionProperty::set(const Property* property) {
    if (const auto* tf = dynamic_cast<const TransferFunctionProperty*>(property)) {
        set(tf);
    } else if (const auto* isoTF = dynamic_cast<const IsoTFProperty*>(property)) {
        set(isoTF);
    }
}

void TransferFunctionProperty::set(const TransferFunctionProperty* p) { set(p->tf_.value); }

void TransferFunctionProperty::set(const IsoTFProperty* p) { set(p->tf_.get()); }

void TransferFunctionProperty::onTFPrimitiveAdded(const TFPrimitiveSet&, TFPrimitive&) {
    propertyModified();
}
void TransferFunctionProperty::onTFPrimitiveRemoved(const TFPrimitiveSet&, TFPrimitive&) {
    propertyModified();
}
void TransferFunctionProperty::onTFPrimitiveChanged(const TFPrimitiveSet&, const TFPrimitive&) {
    propertyModified();
}
void TransferFunctionProperty::onTFTypeChanged(const TFPrimitiveSet&, TFPrimitiveSetType) {
    propertyModified();
}
void TransferFunctionProperty::onTFMaskChanged(const TFPrimitiveSet&, dvec2) { propertyModified(); }

}  // namespace inviwo
