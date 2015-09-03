/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2015 Inviwo Foundation
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

#include "volumesource.h"
#include <inviwo/core/resources/resourcemanager.h>
#include <inviwo/core/resources/templateresource.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/util/raiiutils.h>
#include <inviwo/core/io/datareaderfactory.h>
#include <inviwo/core/io/rawvolumereader.h>
#include <inviwo/core/network/processornetwork.h>
#include <inviwo/core/datastructures/volume/volumeram.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <math.h>

namespace inviwo {

ProcessorClassIdentifier(VolumeSource, "org.inviwo.VolumeSource");
ProcessorDisplayName(VolumeSource,  "Volume Source");
ProcessorTags(VolumeSource, Tags::CPU);
ProcessorCategory(VolumeSource, "Data Input");
ProcessorCodeState(VolumeSource, CODE_STATE_STABLE);

VolumeSource::VolumeSource()
    : Processor()
    , outport_("data")
    , file_("filename", "File")
    , reload_("reload", "Reload data")
    , basis_("Basis", "Basis and offset")
    , information_("Information", "Data information")
    , volumeSequence_("Sequence", "Sequence")
    , isDeserializing_(false) {

    file_.setContentType("volume");
    file_.setDisplayName("Volume file");

    file_.onChange([this]() { load(); });
    reload_.onChange([this]() { load(); });

    volumeSequence_.setVisible(false);

    addFileNameFilters();

    addPort(outport_);

    addProperty(file_);
    addProperty(reload_);
    addProperty(information_);
    addProperty(basis_);
    addProperty(volumeSequence_);
}

VolumeSource::~VolumeSource() {}

void VolumeSource::load(bool deserialize) {
    if (isDeserializing_ || file_.get().empty()) return;

    auto rf = DataReaderFactory::getPtr();

    std::unique_ptr<VolumeVector> volumes;

    std::string ext = filesystem::getFileExtension(file_.get());
    if (auto reader = rf->getReaderForTypeAndExtension<VolumeVector>(ext)) {
        try {
            volumes.reset(reader->readMetaData(file_.get()));

            std::swap(volumes, volumes_);
        } catch (DataReaderException const& e) {
            LogProcessorError("Could not load data: " << file_.get() << ", " << e.getMessage());
        }
    } else if (auto reader = rf->getReaderForTypeAndExtension<Volume>(ext)) {
        try {
            std::unique_ptr<Volume> volume(reader->readMetaData(file_.get()));
            volumes = util::make_unique<VolumeVector>();
            volumes->push_back(std::move(volume));

            std::swap(volumes, volumes_);
        } catch (DataReaderException const& e) {
            LogProcessorError("Could not load data: " << file_.get() << ", " << e.getMessage());
        }
    } else {
        LogProcessorError("Could not find a data reader for file: " << file_.get());
    }

    if (volumes_ && !volumes_->empty() && (*volumes_)[0]) {
        basis_.updateForNewVolume(*(*volumes_)[0], deserialize);
        information_.updateForNewVolume(*(*volumes_)[0], deserialize);
        
        volumeSequence_.updateMax(volumes_->size());
        volumeSequence_.setVisible(volumes_->size() > 1);
        invalidate(INVALID_OUTPUT);
    }
}

void VolumeSource::addFileNameFilters() {
    auto rf = DataReaderFactory::getPtr();
    auto extensions = rf->getExtensionsForType<Volume>();
    for (auto& ext : extensions) {
        file_.addNameFilter(ext.description_ + " (*." + ext.extension_ + ")");
    }
    extensions = rf->getExtensionsForType<VolumeVector>();
    for (auto& ext : extensions) {
        file_.addNameFilter(ext.description_ + " (*." + ext.extension_ + ")");
    }
}

void VolumeSource::process() {
    if (!isDeserializing_ && volumes_ && !volumes_->empty()) {
        size_t index = std::min(volumes_->size()-1, static_cast<size_t>(volumeSequence_.selectedSequenceIndex_.get()));
        
        if (!(*volumes_)[index]) return;
        
        basis_.updateVolume(*(*volumes_)[index]);
        information_.updateVolume(*(*volumes_)[index]);

        outport_.setData((*volumes_)[index].get(), false);
    }
}

void VolumeSource::serialize(IvwSerializer& s) const {
    Processor::serialize(s);
}

void VolumeSource::deserialize(IvwDeserializer& d) {
    {
        isDeserializing_ = true;
        Processor::deserialize(d);
        addFileNameFilters();
        isDeserializing_ = false;
    }
    load(true);
}

VolumeBasisProperty::VolumeBasisProperty(std::string identifier, std::string displayName,
                                         InvalidationLevel invalidationLevel,
                                         PropertySemantics semantics)
    : CompositeProperty(identifier, displayName, invalidationLevel, semantics)
    , overRideDefaults_("override", "Override", false)
    , a_("a", "A", vec3(1.0f, 0.0f, 0.0f), vec3(-10.0f), vec3(10.0f))
    , b_("b", "B", vec3(0.0f, 1.0f, 0.0f), vec3(-10.0f), vec3(10.0f))
    , c_("c", "C", vec3(0.0f, 0.0f, 1.0f), vec3(-10.0f), vec3(10.0f))
    , offset_("offset", "Offset", vec3(0.0f), vec3(-10.0f), vec3(10.0f))

    , overrideA_("overrideA", "A", vec3(1.0f, 0.0f, 0.0f), vec3(-10.0f), vec3(10.0f))
    , overrideB_("overrideB", "B", vec3(0.0f, 1.0f, 0.0f), vec3(-10.0f), vec3(10.0f))
    , overrideC_("overrideC", "C", vec3(0.0f, 0.0f, 1.0f), vec3(-10.0f), vec3(10.0f))
    , overrideOffset_("overrideOffset", "Offset", vec3(0.0f), vec3(-10.0f), vec3(10.0f)) {

    a_.setReadOnly(true);
    a_.setSerializationMode(PropertySerializationMode::NONE);
    b_.setReadOnly(true);
    b_.setSerializationMode(PropertySerializationMode::NONE);
    c_.setReadOnly(true);
    c_.setSerializationMode(PropertySerializationMode::NONE);
    offset_.setReadOnly(true);
    offset_.setSerializationMode(PropertySerializationMode::NONE);

    overrideA_.setSerializationMode(PropertySerializationMode::ALL);
    overrideA_.setVisible(false);
    overrideB_.setSerializationMode(PropertySerializationMode::ALL);
    overrideB_.setVisible(false);
    overrideC_.setSerializationMode(PropertySerializationMode::ALL);
    overrideC_.setVisible(false);
    overrideOffset_.setSerializationMode(PropertySerializationMode::ALL);
    overrideOffset_.setVisible(false);

    addProperty(overRideDefaults_);
    addProperty(a_);
    addProperty(b_);
    addProperty(c_);
    addProperty(offset_);
    addProperty(overrideA_);
    addProperty(overrideB_);
    addProperty(overrideC_);
    addProperty(overrideOffset_);
    
    overRideDefaults_.onChange([this](){onOverrideChange();});
}

VolumeBasisProperty::VolumeBasisProperty(const VolumeBasisProperty& rhs)
    : CompositeProperty(rhs)
    , overRideDefaults_(rhs.overRideDefaults_)
    , a_(rhs.a_)
    , b_(rhs.b_)
    , c_(rhs.c_)
    , offset_(rhs.offset_)
    , overrideA_(rhs.overrideB_)
    , overrideB_(rhs.overrideB_)
    , overrideC_(rhs.overrideC_)
    , overrideOffset_(rhs.overrideOffset_) {

    addProperty(overRideDefaults_);
    addProperty(a_);
    addProperty(b_);
    addProperty(c_);
    addProperty(offset_);
    addProperty(overrideA_);
    addProperty(overrideB_);
    addProperty(overrideC_);
    addProperty(overrideOffset_);

    overRideDefaults_.onChange([this]() {onOverrideChange();});
}

void VolumeBasisProperty::updateForNewVolume(const Volume& volume, bool deserialize) {
    // Set basis properties to the values from the new volume
    a_.set(volume.getBasis()[0]);
    b_.set(volume.getBasis()[1]);
    c_.set(volume.getBasis()[2]);
    offset_.set(volume.getOffset());
    a_.setCurrentStateAsDefault();
    b_.setCurrentStateAsDefault();
    c_.setCurrentStateAsDefault();
    offset_.setCurrentStateAsDefault();


    if (deserialize) {
        Property::setStateAsDefault(overrideA_, volume.getBasis()[0]);
        Property::setStateAsDefault(overrideB_, volume.getBasis()[1]);
        Property::setStateAsDefault(overrideC_, volume.getBasis()[2]);
        Property::setStateAsDefault(overrideOffset_, volume.getOffset());
    } else {
        overrideA_.set(volume.getBasis()[0]);
        overrideB_.set(volume.getBasis()[1]);
        overrideC_.set(volume.getBasis()[2]);
        overrideOffset_.set(volume.getOffset());
        overrideA_.setCurrentStateAsDefault();
        overrideB_.setCurrentStateAsDefault();
        overrideC_.setCurrentStateAsDefault();
        overrideOffset_.setCurrentStateAsDefault();
    }
}

void VolumeBasisProperty::onOverrideChange() {
    if (overRideDefaults_) {
        a_.setVisible(false);
        b_.setVisible(false);
        c_.setVisible(false);
        offset_.setVisible(false);

        overrideA_.setVisible(true);
        overrideB_.setVisible(true);
        overrideC_.setVisible(true);
        overrideOffset_.setVisible(true);
    } else {
        overrideA_.setVisible(false);
        overrideB_.setVisible(false);
        overrideC_.setVisible(false);
        overrideOffset_.setVisible(false);

        a_.setVisible(true);
        b_.setVisible(true);
        c_.setVisible(true);
        offset_.setVisible(true);
    }
}

VolumeBasisProperty& VolumeBasisProperty::operator=(const VolumeBasisProperty& that) {
    if (this != &that) {
        CompositeProperty::operator=(that);
        overRideDefaults_ = that.overRideDefaults_;
        a_ = that.a_;
        b_ = that.b_;
        c_ = that.c_;
        offset_ = that.offset_;
        overrideA_ = that.overrideB_;
        overrideB_ = that.overrideB_;
        overrideC_ = that.overrideC_;
        overrideOffset_ = that.overrideOffset_;
    }
    return *this;
}

void inviwo::VolumeBasisProperty::updateVolume(Volume& volume) {
    if (overRideDefaults_) {
        vec4 offset = vec4(overrideOffset_.get(), 1.0f);
        mat3 basis(overrideA_, overrideB_, overrideC_);
        mat4 basisAndOffset(basis);
        basisAndOffset[3] = offset;
        volume.setModelMatrix(basisAndOffset);
    } else {
        vec4 offset = vec4(offset_.get(), 1.0f);
        mat3 basis(a_, b_, c_);
        mat4 basisAndOffset(basis);
        basisAndOffset[3] = offset;
        volume.setModelMatrix(basisAndOffset);
    }
}

VolumeInformationProperty::VolumeInformationProperty(std::string identifier,
                                                     std::string displayName,
                                                     InvalidationLevel invalidationLevel,
                                                     PropertySemantics semantics)
    : CompositeProperty(identifier, displayName, invalidationLevel, semantics)
    , dimensions_("dimensions", "Dimensions")
    , format_("format", "Format", "")
    , dataRange_("dataRange", "Data range", 0., 255.0, -DataFLOAT64::max(), DataFLOAT64::max(), 0.0,
                 0.0, INVALID_OUTPUT, PropertySemantics("Text"))
    , valueRange_("valueRange", "Value range", 0., 255.0, -DataFLOAT64::max(), DataFLOAT64::max(),
                  0.0, 0.0, INVALID_OUTPUT, PropertySemantics("Text"))
    , valueUnit_("valueUnit", "Value unit", "arb. unit.") {


    dimensions_.setReadOnly(true);
    format_.setReadOnly(true);
    dimensions_.setSerializationMode(PropertySerializationMode::NONE);
    format_.setSerializationMode(PropertySerializationMode::NONE);
    dimensions_.setCurrentStateAsDefault();
    format_.setCurrentStateAsDefault();

    dataRange_.setSerializationMode(PropertySerializationMode::ALL);
    valueRange_.setSerializationMode(PropertySerializationMode::ALL);
    valueUnit_.setSerializationMode(PropertySerializationMode::ALL);

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

VolumeInformationProperty& VolumeInformationProperty::operator=(const VolumeInformationProperty& that) {
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
    if (volume.dataMap_.dataRange != dataRange_.get() &&
        volume.hasRepresentation<VolumeRAM>()) {
        auto volumeRAM = volume.getEditableRepresentation<VolumeRAM>();
        if (volumeRAM->hasHistograms()) {
            volumeRAM->getHistograms()->setValid(false);
        }
    }

    volume.dataMap_.dataRange = dataRange_.get();
    volume.dataMap_.valueRange = valueRange_.get();
    volume.dataMap_.valueUnit = valueUnit_.get();
}

SequenceTimerProperty::SequenceTimerProperty(std::string identifier, std::string displayName,
                                             InvalidationLevel invalidationLevel,
                                             PropertySemantics semantics)
    : CompositeProperty(identifier, displayName, invalidationLevel, semantics)
    , selectedSequenceIndex_("selectedSequenceIndex", "Sequence Index", 1, 1, 1, 1, VALID)
    , playSequence_("playSequence", "Play Sequence", false)
    , volumesPerSecond_("volumesPerSecond", "Frame rate", 30, 1, 60, 1, VALID)
    , sequenceTimer_(1000 / volumesPerSecond_.get(), [this]() { onTimerEvent(); }) {

    playSequence_.onChange(this, &SequenceTimerProperty::onPlaySequenceToggled);

    volumesPerSecond_.onChange(
        [this]() { sequenceTimer_.setInterval(1000 / volumesPerSecond_.get()); });
    selectedSequenceIndex_.setSerializationMode(PropertySerializationMode::ALL);
    addProperty(selectedSequenceIndex_);
    addProperty(playSequence_);
    addProperty(volumesPerSecond_);
}

SequenceTimerProperty::SequenceTimerProperty(const SequenceTimerProperty& rhs) 
    : CompositeProperty(rhs)
    , selectedSequenceIndex_(rhs.selectedSequenceIndex_)
    , playSequence_(rhs.playSequence_)
    , volumesPerSecond_(rhs.volumesPerSecond_)
    , sequenceTimer_(1000 / volumesPerSecond_.get(), [this]() { onTimerEvent(); }) {


    volumesPerSecond_.onChange(
        [this]() { sequenceTimer_.setInterval(1000 / volumesPerSecond_.get()); });
    selectedSequenceIndex_.setSerializationMode(PropertySerializationMode::ALL);
    addProperty(selectedSequenceIndex_);
    addProperty(playSequence_);
    addProperty(volumesPerSecond_);

}
SequenceTimerProperty& SequenceTimerProperty::operator=(const SequenceTimerProperty& that) {
    if (this != &that) {
        CompositeProperty::operator=(that);
        selectedSequenceIndex_ = that.selectedSequenceIndex_;
        playSequence_ = that.playSequence_;
        volumesPerSecond_ = that.volumesPerSecond_;    
    }
    return *this;
}

void SequenceTimerProperty::updateMax(size_t max) {
    selectedSequenceIndex_.setMaxValue(static_cast<int>(max));
    selectedSequenceIndex_.set(1);
    selectedSequenceIndex_.setCurrentStateAsDefault();
}

void inviwo::SequenceTimerProperty::onTimerEvent() {
    selectedSequenceIndex_ =
        (selectedSequenceIndex_ < selectedSequenceIndex_.getMaxValue() ? selectedSequenceIndex_ + 1
                                                                       : 1);
}

void inviwo::SequenceTimerProperty::onPlaySequenceToggled() {
    if (selectedSequenceIndex_.getMaxValue() > 1) {
        if (playSequence_.get()) {
            sequenceTimer_.start(1000 / volumesPerSecond_.get());
            selectedSequenceIndex_.setReadOnly(true);
            volumesPerSecond_.setReadOnly(false);
        } else {
            sequenceTimer_.stop();
            selectedSequenceIndex_.setReadOnly(false);
            volumesPerSecond_.setReadOnly(true);
        }
    }
}

}  // namespace
