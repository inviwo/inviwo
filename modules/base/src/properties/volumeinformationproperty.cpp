/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015-2019 Inviwo Foundation
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

#include <modules/base/properties/volumeinformationproperty.h>
#include <inviwo/core/datastructures/volume/volumeram.h>
#include <inviwo/core/util/stdextensions.h>

namespace inviwo {

const std::string VolumeInformationProperty::classIdentifier =
    "org.inviwo.VolumeInformationProperty";
std::string VolumeInformationProperty::getClassIdentifier() const { return classIdentifier; }

auto VolumeInformationProperty::props() {
    return std::tie(dimensions_, format_, channels_, numVoxels_);
}

VolumeInformationProperty::VolumeInformationProperty(std::string identifier,
                                                     std::string displayName,
                                                     InvalidationLevel invalidationLevel,
                                                     PropertySemantics semantics)
    : CompositeProperty(identifier, displayName, invalidationLevel, semantics)
    , dimensions_("dimensions", "Dimensions", size3_t(0), size3_t(0),
                  size3_t(std::numeric_limits<size_t>::max()), size3_t(1), InvalidationLevel::Valid,
                  PropertySemantics("Text"))
    , format_("format", "Format", "")
    , channels_("channels", "Channels", 0, 0, std::numeric_limits<size_t>::max(), 1,
                InvalidationLevel::InvalidOutput, PropertySemantics("Text"))
    , numVoxels_("numVoxels", "Number of Voxels", 0, 0, std::numeric_limits<size_t>::max(), 1,
                 InvalidationLevel::Valid, PropertySemantics::Text)
    , dataRange_("dataRange", "Data range", 0., 255.0, -DataFloat64::max(), DataFloat64::max(), 0.0,
                 0.0, InvalidationLevel::InvalidOutput, PropertySemantics("Text"))
    , valueRange_("valueRange", "Value range", 0., 255.0, -DataFloat64::max(), DataFloat64::max(),
                  0.0, 0.0, InvalidationLevel::InvalidOutput, PropertySemantics("Text"))
    , valueUnit_("valueUnit", "Value unit", "arb. unit.") {

    util::for_each_in_tuple(
        [&](auto& e) {
            e.setReadOnly(true);
            e.setSerializationMode(PropertySerializationMode::None);
            e.setCurrentStateAsDefault();
            this->addProperty(e);
        },
        props());

    dataRange_.setSerializationMode(PropertySerializationMode::All);
    valueRange_.setSerializationMode(PropertySerializationMode::All);
    valueUnit_.setSerializationMode(PropertySerializationMode::All);

    addProperty(dataRange_);
    addProperty(valueRange_);
    addProperty(valueUnit_);
}

VolumeInformationProperty::VolumeInformationProperty(const VolumeInformationProperty& rhs)
    : CompositeProperty(rhs)
    , dimensions_(rhs.dimensions_)
    , format_(rhs.format_)
    , channels_(rhs.channels_)
    , numVoxels_(rhs.numVoxels_)
    , dataRange_(rhs.dataRange_)
    , valueRange_(rhs.valueRange_)
    , valueUnit_(rhs.valueUnit_) {

    util::for_each_in_tuple([&](auto& e) { this->addProperty(e); }, props());

    addProperty(dataRange_);
    addProperty(valueRange_);
    addProperty(valueUnit_);
}

VolumeInformationProperty* VolumeInformationProperty::clone() const {
    return new VolumeInformationProperty(*this);
}

void VolumeInformationProperty::updateForNewVolume(const Volume& volume, bool deserialize) {
    const auto dim = volume.getDimensions();

    dimensions_.set(dim);
    format_.set(volume.getDataFormat()->getString());
    channels_.set(volume.getDataFormat()->getComponents());
    numVoxels_.set(dim.x * dim.y * dim.z);

    util::for_each_in_tuple([&](auto& e) { e.setCurrentStateAsDefault(); }, props());

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

}  // namespace inviwo
