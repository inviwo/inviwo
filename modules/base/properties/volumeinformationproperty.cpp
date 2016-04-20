/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015 Inviwo Foundation
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

#include "volumeinformationproperty.h"
#include <inviwo/core/datastructures/volume/volumeram.h>
namespace inviwo {

PropertyClassIdentifier(VolumeInformationProperty, "org.inviwo.VolumeInformationProperty");

VolumeInformationProperty::VolumeInformationProperty(std::string identifier,
                                                     std::string displayName,
                                                     InvalidationLevel invalidationLevel,
                                                     PropertySemantics semantics)
    : CompositeProperty(identifier, displayName, invalidationLevel, semantics)
    , dimensions_("dimensions", "Dimensions")
    , format_("format", "Format", "")
    , dataRange_("dataRange", "Data range", 0., 255.0, -DataFloat64::max(), DataFloat64::max(), 0.0,
                 0.0, InvalidationLevel::InvalidOutput, PropertySemantics("Text"))
    , valueRange_("valueRange", "Value range", 0., 255.0, -DataFloat64::max(), DataFloat64::max(),
                  0.0, 0.0, InvalidationLevel::InvalidOutput, PropertySemantics("Text"))
    , valueUnit_("valueUnit", "Value unit", "arb. unit.") {
    dimensions_.setReadOnly(true);
    format_.setReadOnly(true);
    dimensions_.setSerializationMode(PropertySerializationMode::None);
    format_.setSerializationMode(PropertySerializationMode::None);
    dimensions_.setCurrentStateAsDefault();
    format_.setCurrentStateAsDefault();

    dataRange_.setSerializationMode(PropertySerializationMode::All);
    valueRange_.setSerializationMode(PropertySerializationMode::All);
    valueUnit_.setSerializationMode(PropertySerializationMode::All);

    addProperty(dimensions_);
    addProperty(format_);
    addProperty(dataRange_);
    addProperty(valueRange_);
    addProperty(valueUnit_);
}

VolumeInformationProperty::VolumeInformationProperty(const VolumeInformationProperty& rhs)
    : CompositeProperty(rhs)
    , dimensions_(rhs.dimensions_)
    , format_(rhs.format_)
    , dataRange_(rhs.dataRange_)
    , valueRange_(rhs.valueRange_)
    , valueUnit_(rhs.valueUnit_) {
    addProperty(dimensions_);
    addProperty(format_);
    addProperty(dataRange_);
    addProperty(valueRange_);
    addProperty(valueUnit_);
}

VolumeInformationProperty* VolumeInformationProperty::clone() const {
    return new VolumeInformationProperty(*this);
}

void VolumeInformationProperty::updateForNewVolume(const Volume& volume, bool deserialize) {
    std::stringstream ss;
    ss << volume.getDimensions().x << " x " << volume.getDimensions().y << " x "
       << volume.getDimensions().z;

    dimensions_.set(ss.str());
    format_.set(volume.getDataFormat()->getString());
    dimensions_.setCurrentStateAsDefault();
    format_.setCurrentStateAsDefault();
    if (deserialize) {
        Property::setStateAsDefault(dataRange_, volume.dataMap_.dataRange);
        Property::setStateAsDefault(valueRange_, volume.dataMap_.valueRange);
        Property::setStateAsDefault(valueUnit_, volume.dataMap_.valueUnit);
    } else {
        dataRange_.set(volume.dataMap_.dataRange);
        valueRange_.set(volume.dataMap_.valueRange);
        valueUnit_.set(volume.dataMap_.valueUnit);
        dataRange_.setCurrentStateAsDefault();
        valueRange_.setCurrentStateAsDefault();
        valueUnit_.setCurrentStateAsDefault();
    }
}

VolumeInformationProperty& VolumeInformationProperty::operator=(
    const VolumeInformationProperty& that) {
    if (this != &that) {
        CompositeProperty::operator=(that);
        dimensions_ = that.dimensions_;
        format_ = that.format_;
        dataRange_ = that.dataRange_;
        valueRange_ = that.valueRange_;
        valueUnit_ = that.valueUnit_;
    }
    return *this;
}

void inviwo::VolumeInformationProperty::updateVolume(Volume& volume) {
    if (volume.dataMap_.dataRange != dataRange_.get() && volume.hasRepresentation<VolumeRAM>()) {
        auto volumeRAM = volume.getEditableRepresentation<VolumeRAM>();
        if (volumeRAM->hasHistograms()) {
            volumeRAM->getHistograms()->setValid(false);
        }
    }

    volume.dataMap_.dataRange = dataRange_.get();
    volume.dataMap_.valueRange = valueRange_.get();
    volume.dataMap_.valueUnit = valueUnit_.get();
}

}  // namespace
