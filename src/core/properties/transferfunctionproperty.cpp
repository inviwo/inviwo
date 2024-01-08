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

void TFPropertyObserver::onMaskChange(const dvec2&) {}

void TFPropertyObserver::onZoomHChange(const dvec2&) {}

void TFPropertyObserver::onZoomVChange(const dvec2&) {}

void TFPropertyObserver::onHistogramModeChange(HistogramMode) {}

void TFPropertyObserver::onHistogramSelectionChange(HistogramSelection) {}

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

void TFPropertyObservable::notifyHistogramSelectionChange(HistogramSelection selection) {
    forEachObserver([&](TFPropertyObserver* o) { o->onHistogramSelectionChange(selection); });
}

TransferFunctionProperty::TransferFunctionProperty(std::string_view identifier,
                                                   std::string_view displayName, Document help,
                                                   const TransferFunction& value,
                                                   VolumeInport* volumeInport,
                                                   InvalidationLevel invalidationLevel,
                                                   PropertySemantics semantics)
    : Property(identifier, displayName, std::move(help), invalidationLevel, semantics)
    , tf_{"TransferFunction", value}
    , zoomH_("zoomH_", dvec2(0.0, 1.0))
    , zoomV_("zoomV_", dvec2(0.0, 1.0))
    , histogramMode_("showHistogram_", HistogramMode::All)
    , histogramSelection_("histogramSelection", histogramSelectionAll)
    , volumeInport_(volumeInport) {

    tf_.value.addObserver(this);
}

TransferFunctionProperty::TransferFunctionProperty(
    std::string_view identifier, std::string_view displayName, const TransferFunction& value,
    VolumeInport* volumeInport, InvalidationLevel invalidationLevel, PropertySemantics semantics)
    : TransferFunctionProperty(identifier, displayName, Document{}, value, volumeInport,
                               invalidationLevel, semantics) {}

TransferFunctionProperty::TransferFunctionProperty(std::string_view identifier,
                                                   std::string_view displayName,
                                                   VolumeInport* volumeInport,
                                                   InvalidationLevel invalidationLevel,
                                                   PropertySemantics semantics)
    : TransferFunctionProperty(identifier, displayName,
                               TransferFunction({{0.0, vec4(0.0f, 0.0f, 0.0f, 0.0f)},
                                                 {1.0, vec4(1.0f, 1.0f, 1.0f, 1.0f)}}),
                               volumeInport, invalidationLevel, semantics) {}

TransferFunctionProperty::TransferFunctionProperty(const TransferFunctionProperty& rhs)
    : Property(rhs)
    , tf_{rhs.tf_}
    , zoomH_(rhs.zoomH_)
    , zoomV_(rhs.zoomV_)
    , histogramMode_(rhs.histogramMode_)
    , histogramSelection_(rhs.histogramSelection_)
    , volumeInport_(rhs.volumeInport_) {

    tf_.value.addObserver(this);
}

TransferFunctionProperty* TransferFunctionProperty::clone() const {
    return new TransferFunctionProperty(*this);
}

TransferFunctionProperty::~TransferFunctionProperty() { volumeInport_ = nullptr; }

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

VolumeInport* TransferFunctionProperty::getVolumeInport() { return volumeInport_; }

TransferFunctionProperty& TransferFunctionProperty::resetToDefaultState() {
    NetworkLock lock(this);

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

    if (tf_.value.getMaskMin() != maskMax || tf_.value.getMaskMax() != maskMax) {
        tf_.value.setMaskMin(maskMin);
        tf_.value.setMaskMax(maskMax);
        notifyMaskChange(dvec2(maskMin, maskMax));
        propertyModified();
    }
    return *this;
}

TransferFunctionProperty& TransferFunctionProperty::clearMask() {
    auto prevMask = getMask();

    tf_.value.clearMask();
    if (getMask() != prevMask) {
        notifyMaskChange(getMask());
    }
    propertyModified();
    return *this;
}

const dvec2 TransferFunctionProperty::getMask() const {
    return dvec2(tf_.value.getMaskMin(), tf_.value.getMaskMax());
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

TransferFunctionProperty& TransferFunctionProperty::set(const TransferFunction& value) {
    tf_.value.removeObserver(this);
    if (tf_.update(value)) propertyModified();
    tf_.value.addObserver(this);
    return *this;
}

const TransferFunction& TransferFunctionProperty::operator*() const { return tf_.value; }
TransferFunction& TransferFunctionProperty::operator*() { return tf_.value; }
const TransferFunction* TransferFunctionProperty::operator->() const { return &tf_.value; }
TransferFunction* TransferFunctionProperty::operator->() { return &tf_.value; }

void TransferFunctionProperty::set(const Property* property) {
    if (auto tfp = dynamic_cast<const TransferFunctionProperty*>(property)) {
        set(tfp);
    } else if (auto isotf = dynamic_cast<const IsoTFProperty*>(property)) {
        set(isotf);
    }
}

void TransferFunctionProperty::set(const TransferFunctionProperty* p) { set(p->tf_.value); }

void TransferFunctionProperty::set(const IsoTFProperty* p) { set(p->tf_.get()); }

void TransferFunctionProperty::onTFPrimitiveAdded(TFPrimitive&) { propertyModified(); }

void TransferFunctionProperty::onTFPrimitiveRemoved(TFPrimitive&) { propertyModified(); }

void TransferFunctionProperty::onTFPrimitiveChanged(const TFPrimitive&) { propertyModified(); }

void TransferFunctionProperty::onTFTypeChanged(const TFPrimitiveSet&) { propertyModified(); }

}  // namespace inviwo
