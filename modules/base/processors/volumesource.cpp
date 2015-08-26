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
#include <inviwo/core/io/datareaderfactory.h>
#include <inviwo/core/io/rawvolumereader.h>
#include <inviwo/core/network/processornetwork.h>
#include <inviwo/core/datastructures/volume/volumeram.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <glm/gtx/vector_angle.hpp>
#include <math.h>

namespace inviwo {

ProcessorClassIdentifier(VolumeSource, "org.inviwo.VolumeSource");
ProcessorDisplayName(VolumeSource,  "Volume Source");
ProcessorTags(VolumeSource, Tags::CPU);
ProcessorCategory(VolumeSource, "Data Input");
ProcessorCodeState(VolumeSource, CODE_STATE_STABLE);

VolumeSource::VolumeSource()
    : DataSource<Volume, VolumeOutport>()
    , basis_("Basis", "Basis and offset")
    , information_("Information", "Data information")
    , volumeSequence_("Sequence", "Sequence")
    , dataRange_("dataRange", "Data range", 0., 255.0, -DataFLOAT64::max(), DataFLOAT64::max(), 0.0,
                 0.0, INVALID_OUTPUT, PropertySemantics("Text"))
    , valueRange_("valueRange", "Value range", 0., 255.0, -DataFLOAT64::max(), DataFLOAT64::max(),
                  0.0, 0.0, INVALID_OUTPUT, PropertySemantics("Text"))
    , valueUnit_("valueUnit", "Value unit", "arb. unit.")
    , overRideDefaults_("override", "Override", false)
    , a_("a", "A", vec3(1.0f, 0.0f, 0.0f), vec3(-10.0f), vec3(10.0f))
    , b_("b", "B", vec3(0.0f, 1.0f, 0.0f), vec3(-10.0f), vec3(10.0f))
    , c_("c", "C", vec3(0.0f, 0.0f, 1.0f), vec3(-10.0f), vec3(10.0f))
    , offset_("offset", "Offset", vec3(0.0f), vec3(-10.0f), vec3(10.0f))

    , overrideA_("overrideA", "A", vec3(1.0f, 0.0f, 0.0f), vec3(-10.0f), vec3(10.0f))
    , overrideB_("overrideB", "B", vec3(0.0f, 1.0f, 0.0f), vec3(-10.0f), vec3(10.0f))
    , overrideC_("overrideC", "C", vec3(0.0f, 0.0f, 1.0f), vec3(-10.0f), vec3(10.0f))
    , overrideOffset_("overrideOffset", "Offset", vec3(0.0f), vec3(-10.0f), vec3(10.0f))

    , dimensions_("dimensions", "Dimensions")
    , format_("format", "Format", "")

    , selectedSequenceIndex_("selectedSequenceIndex", "Sequence Index", 1, 1, 1, 1, VALID)
    , playSequence_("playSequence", "Play Sequence", false)
    , volumesPerSecond_("volumesPerSecond", "Frame rate", 30, 1, 60, 1, VALID)

    , sequenceTimer_(1000 / volumesPerSecond_.get(), [this](){onSequenceTimerEvent();}) {

    DataSource<Volume, VolumeOutport>::file_.setContentType("volume");
    DataSource<Volume, VolumeOutport>::file_.setDisplayName("Volume file");

    dimensions_.setReadOnly(true);
    format_.setReadOnly(true);
    dimensions_.setCurrentStateAsDefault();
    format_.setCurrentStateAsDefault();
    dataRange_.setSerializationMode(PropertySerializationMode::ALL);
    valueRange_.setSerializationMode(PropertySerializationMode::ALL);
    valueUnit_.setSerializationMode(PropertySerializationMode::ALL);
    
    information_.addProperty(dimensions_);
    information_.addProperty(format_);
    information_.addProperty(dataRange_);
    information_.addProperty(valueRange_);
    information_.addProperty(valueUnit_);
    addProperty(information_);

    a_.setReadOnly(true);
    a_.setSerializationMode(PropertySerializationMode::ALL);
    b_.setReadOnly(true);
    b_.setSerializationMode(PropertySerializationMode::ALL);
    c_.setReadOnly(true);
    c_.setSerializationMode(PropertySerializationMode::ALL);
    offset_.setReadOnly(true);
    offset_.setSerializationMode(PropertySerializationMode::ALL);
    
    overrideA_.setSerializationMode(PropertySerializationMode::ALL);
    overrideA_.setVisible(false);
    overrideB_.setSerializationMode(PropertySerializationMode::ALL);
    overrideB_.setVisible(false);
    overrideC_.setSerializationMode(PropertySerializationMode::ALL);
    overrideC_.setVisible(false);
    overrideOffset_.setSerializationMode(PropertySerializationMode::ALL);
    overrideOffset_.setVisible(false);
    
    overRideDefaults_.onChange(this, &VolumeSource::onOverrideChange);
    basis_.addProperty(overRideDefaults_);
    basis_.addProperty(a_);
    basis_.addProperty(b_);
    basis_.addProperty(c_);
    basis_.addProperty(offset_);
    basis_.addProperty(overrideA_);
    basis_.addProperty(overrideB_);
    basis_.addProperty(overrideC_);
    basis_.addProperty(overrideOffset_);
    addProperty(basis_);
        
    playSequence_.onChange(this, &VolumeSource::onPlaySequenceToggled);
    selectedSequenceIndex_.onChange(this, &VolumeSource::onSequenceIndexChanged);

    volumesPerSecond_.onChange(
        [this]() { sequenceTimer_.setInterval(1000 / volumesPerSecond_.get()); });

    selectedSequenceIndex_.setSerializationMode(PropertySerializationMode::ALL);
    volumeSequence_.addProperty(selectedSequenceIndex_);
    volumeSequence_.addProperty(playSequence_);
    volumeSequence_.addProperty(volumesPerSecond_);
    volumeSequence_.setVisible(false);
    addProperty(volumeSequence_);
}

VolumeSource::~VolumeSource() {}

void VolumeSource::onOverrideChange() {
    if (this->isDeserializing()) return;
    
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

void VolumeSource::dataDeserialized(Volume* volume) {
    // We are de-serializing a workspace, so here we
    // mainly need to make sure that the defaults are correct.
    
    NetworkLock lock;
    
    setStateAsDefault(dataRange_, volume->dataMap_.dataRange);
    setStateAsDefault(valueRange_, volume->dataMap_.valueRange);
    setStateAsDefault(valueUnit_, volume->dataMap_.valueUnit);
    setStateAsDefault(a_, volume->getBasis()[0]);
    setStateAsDefault(b_, volume->getBasis()[1]);
    setStateAsDefault(c_, volume->getBasis()[2]);
    setStateAsDefault(offset_, volume->getOffset());
    
    setStateAsDefault(overrideA_, volume->getBasis()[0]);
    setStateAsDefault(overrideB_, volume->getBasis()[1]);
    setStateAsDefault(overrideC_, volume->getBasis()[2]);
    setStateAsDefault(overrideOffset_, volume->getOffset());
    
    std::stringstream ss;
    ss << volume->getDimensions().x << " x "
       << volume->getDimensions().y << " x "
       << volume->getDimensions().z;

    DataSequence<Volume>* volumeSequence = dynamic_cast<DataSequence<Volume>*>(volume);
    if(volumeSequence){
        ss << " x " << volumeSequence->getNumSequences();
        volumeSequence_.setVisible(true);
        selectedSequenceIndex_.setMaxValue(static_cast<int>(volumeSequence->getNumSequences()));
        setStateAsDefault(selectedSequenceIndex_, 1);
        onPlaySequenceToggled();
    } else {
        volumeSequence_.setVisible(false);
    }
    dimensions_.set(ss.str());
    format_.set(volume->getDataFormat()->getString());
    dimensions_.setCurrentStateAsDefault();
    format_.setCurrentStateAsDefault();
    
    invalidateOutput();
}


void VolumeSource::dataLoaded(Volume* volume) {
    // Here we have loaded a new volume we need to make sure all
    // properties have valid values and correct new defaults.

    NetworkLock lock;
    
    // Set the data range from the volume
    dataRange_.set(volume->dataMap_.dataRange);
    valueRange_.set(volume->dataMap_.valueRange);
    valueUnit_.set(volume->dataMap_.valueUnit);
    dataRange_.setCurrentStateAsDefault();
    valueRange_.setCurrentStateAsDefault();
    valueUnit_.setCurrentStateAsDefault();
    
    // Set basis properties to the values from the new volume
    a_.set(volume->getBasis()[0]);
    b_.set(volume->getBasis()[1]);
    c_.set(volume->getBasis()[2]);
    offset_.set(volume->getOffset());
    a_.setCurrentStateAsDefault();
    b_.setCurrentStateAsDefault();
    c_.setCurrentStateAsDefault();
    offset_.setCurrentStateAsDefault();
    
    overrideA_.set(volume->getBasis()[0]);
    overrideB_.set(volume->getBasis()[1]);
    overrideC_.set(volume->getBasis()[2]);
    overrideOffset_.set(volume->getOffset());
    overrideA_.setCurrentStateAsDefault();
    overrideB_.setCurrentStateAsDefault();
    overrideC_.setCurrentStateAsDefault();
    overrideOffset_.setCurrentStateAsDefault();
    
    // Display the format and dimensions, read only.
    std::stringstream ss;
    ss << volume->getDimensions().x << " x "
       << volume->getDimensions().y << " x "
       << volume->getDimensions().z;

    DataSequence<Volume>* volumeSequence = dynamic_cast<DataSequence<Volume>*>(volume);
    if(volumeSequence){
        ss << " x " << volumeSequence->getNumSequences();
        volumeSequence_.setVisible(true);
        selectedSequenceIndex_.setMaxValue(static_cast<int>(volumeSequence->getNumSequences()));
        selectedSequenceIndex_.set(1);
        selectedSequenceIndex_.setCurrentStateAsDefault();
        onPlaySequenceToggled();
    } else {
        volumeSequence_.setVisible(false);
    }

    dimensions_.set(ss.str());
    format_.set(volume->getDataFormat()->getString());
    dimensions_.setCurrentStateAsDefault();
    format_.setCurrentStateAsDefault();

    invalidateOutput();
}

void VolumeSource::onPlaySequenceToggled() {
    if (port_.hasDataSequence()) {
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

void VolumeSource::onSequenceIndexChanged() {
    if (port_.hasDataSequence()) {
        DataSequence<Volume>* volumeSequence = static_cast<DataSequence<Volume>*>(loadedData_);
        volumeSequence->setCurrentIndex(selectedSequenceIndex_.get() - 1);
        invalidateOutput();
    }
}

void VolumeSource::onSequenceTimerEvent() {
    if (port_.hasDataSequence()) {
        selectedSequenceIndex_ = (selectedSequenceIndex_ < selectedSequenceIndex_.getMaxValue()
                                      ? selectedSequenceIndex_ + 1
                                      : 1);
    }
}

void VolumeSource::process() {
    if (this->isDeserializing()) return;

    if (loadedData_) {
        if (overRideDefaults_) {
            vec4 offset = vec4(overrideOffset_.get(), 1.0f);
            mat3 basis(overrideA_, overrideB_, overrideC_);
            mat4 basisAndOffset(basis);
            basisAndOffset[3] = offset;
            loadedData_->setModelMatrix(basisAndOffset);
        } else {
            vec4 offset = vec4(offset_.get(), 1.0f);
            mat3 basis(a_, b_, c_);
            mat4 basisAndOffset(basis);
            basisAndOffset[3] = offset;
            loadedData_->setModelMatrix(basisAndOffset);
        }

        if (loadedData_->dataMap_.dataRange != dataRange_.get() &&
            loadedData_->hasRepresentation<VolumeRAM>()) {
            VolumeRAM* volumeRAM = loadedData_->getEditableRepresentation<VolumeRAM>();
            if (volumeRAM->hasHistograms()) {
                volumeRAM->getHistograms()->setValid(false);
            }
        }

        loadedData_->dataMap_.dataRange = dataRange_.get();
        loadedData_->dataMap_.valueRange = valueRange_.get();
        loadedData_->dataMap_.valueUnit = valueUnit_.get();
    }
}

void VolumeSource::serialize(IvwSerializer& s) const {
    DataSource<Volume, VolumeOutport>::serialize(s);
}

void VolumeSource::deserialize(IvwDeserializer& d) {
    // This function will deseialize all properties, then call load(), which will call dataLoaded()
    DataSource<Volume, VolumeOutport>::deserialize(d);
    onOverrideChange();
}

}  // namespace
