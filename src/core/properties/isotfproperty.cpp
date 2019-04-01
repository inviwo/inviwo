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

#include <inviwo/core/properties/isotfproperty.h>

namespace inviwo {

const std::string IsoTFProperty::classIdentifier = "org.inviwo.IsoTFProperty";
std::string IsoTFProperty::getClassIdentifier() const { return classIdentifier; }

IsoTFProperty::IsoTFProperty(const std::string& identifier, const std::string& displayName,
                             const IsoValueCollection& isovalues, const TransferFunction& tf,
                             VolumeInport* volumeInport, InvalidationLevel invalidationLevel,
                             PropertySemantics semantics)
    : CompositeProperty(identifier, displayName, invalidationLevel, semantics)
    , isovalues_("isovalues", "Iso Values", isovalues, volumeInport)
    , tf_("transferFunction", "Transfer Function", tf, volumeInport) {

    addProperty(isovalues_);
    addProperty(tf_);

    tf_.TFPropertyObservable::addObserver(this);
    isovalues_.TFPropertyObservable::addObserver(this);
}

IsoTFProperty::IsoTFProperty(const std::string& identifier, const std::string& displayName,
                             VolumeInport* volumeInport, InvalidationLevel invalidationLevel,
                             PropertySemantics semantics)
    : IsoTFProperty(identifier, displayName, {},
                    TransferFunction({{0.0, vec4(0.0f)}, {1.0, vec4(1.0f)}}), volumeInport,
                    invalidationLevel, semantics) {}

IsoTFProperty::IsoTFProperty(const IsoTFProperty& rhs)
    : CompositeProperty(rhs), isovalues_(rhs.isovalues_), tf_(rhs.tf_) {

    addProperty(isovalues_);
    addProperty(tf_);

    tf_.TFPropertyObservable::addObserver(this);
    isovalues_.TFPropertyObservable::addObserver(this);
}

IsoTFProperty* IsoTFProperty::clone() const { return new IsoTFProperty(*this); }

std::string IsoTFProperty::getClassIdentifierForWidget() const {
    return IsoTFProperty::classIdentifier;
}

void IsoTFProperty::set(const Property* property) {
    if (const auto isotfprop = dynamic_cast<const CompositeProperty*>(property)) {
        CompositeProperty::set(isotfprop);
    } else if (auto isoprop = dynamic_cast<const IsoValueProperty*>(property)) {
        isovalues_.set(isoprop);
    } else if (auto tfprop = dynamic_cast<const TransferFunctionProperty*>(property)) {
        tf_.set(tfprop);
    }
}

void IsoTFProperty::set(const IsoValueProperty& p) { isovalues_.set(p); }

void IsoTFProperty::set(const TransferFunctionProperty& p) { tf_.set(p); }

void IsoTFProperty::setMask(double maskMin, double maskMax) { tf_.setMask(maskMin, maskMax); }

const dvec2 IsoTFProperty::getMask() const { return tf_.getMask(); }

void IsoTFProperty::clearMask() { tf_.clearMask(); }

void IsoTFProperty::setZoomH(double zoomHMin, double zoomHMax) {
    tf_.setZoomH(zoomHMin, zoomHMax);
    isovalues_.setZoomH(zoomHMin, zoomHMax);
}

const dvec2& IsoTFProperty::getZoomH() const { return tf_.getZoomH(); }

void IsoTFProperty::setZoomV(double zoomVMin, double zoomVMax) {
    tf_.setZoomV(zoomVMin, zoomVMax);
    isovalues_.setZoomV(zoomVMin, zoomVMax);
}

const dvec2& IsoTFProperty::getZoomV() const { return tf_.getZoomV(); }

void IsoTFProperty::setHistogramMode(HistogramMode type) {
    tf_.setHistogramMode(type);
    isovalues_.setHistogramMode(type);
}

HistogramMode IsoTFProperty::getHistogramMode() { return tf_.getHistogramMode(); }

VolumeInport* IsoTFProperty::getVolumeInport() { return tf_.getVolumeInport(); }

void IsoTFProperty::onMaskChange(const dvec2& mask) { notifyMaskChange(mask); }

void IsoTFProperty::onZoomHChange(const dvec2& zoomH) { notifyZoomHChange(zoomH); }

void IsoTFProperty::onZoomVChange(const dvec2& zoomV) { notifyZoomVChange(zoomV); }

void IsoTFProperty::onHistogramModeChange(HistogramMode mode) { notifyHistogramModeChange(mode); }

}  // namespace inviwo
