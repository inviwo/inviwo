/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2015 Inviwo Foundation
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

#include "volumesubsample.h"
#include <modules/base/algorithm/volume/volumeramsubsample.h>
#include <glm/gtx/vector_angle.hpp>

namespace inviwo {

ProcessorClassIdentifier(VolumeSubsample, "org.inviwo.VolumeSubsample");
ProcessorDisplayName(VolumeSubsample,  "Volume Subsample");
ProcessorTags(VolumeSubsample, Tags::CPU);
ProcessorCategory(VolumeSubsample, "Volume Operation");
ProcessorCodeState(VolumeSubsample, CODE_STATE_EXPERIMENTAL);

VolumeSubsample::VolumeSubsample() : Processor()
      , inport_("volume.inport")
      , outport_("volume.outport")
      , enabled_("enabled", "Enable Operation", true)
      , adjustBasisAndOffset_("adjustBasisAndOffset", "Adjust Basis and Offset", false)
      , subSampleFactor_("subSampleFacor", "Factor")
{
    addPort(inport_);
    addPort(outport_);
    addProperty(enabled_);
    addProperty(adjustBasisAndOffset_);

    subSampleFactor_.addOption("none", "None", 0);
    subSampleFactor_.addOption("half", "Half", 2);
    subSampleFactor_.setSelectedIdentifier("half");
    subSampleFactor_.setCurrentStateAsDefault();
    addProperty(subSampleFactor_);
}

VolumeSubsample::~VolumeSubsample() {}

void VolumeSubsample::initialize() {
    Processor::initialize();
}

void VolumeSubsample::deinitialize() {
    Processor::deinitialize();
}

void VolumeSubsample::process() {
    if (enabled_.get() && subSampleFactor_.get() > 0) {
        const VolumeRAM* vol = inport_.getData()->getRepresentation<VolumeRAM>();

        Volume* volume = nullptr;

        if(subSampleFactor_.get() == 2)
            volume = new Volume(VolumeRAMSubSample::apply(vol, VolumeRAMSubSample::FACTOR::HALF));
        
        if(!volume) {
            outport_.setConstData(inport_.getData());
            return;
        }

        size3_t dim = volume->getDimensions();
        size3_t offset = size3_t(0);

        // pass meta data on
        volume->copyMetaDataFrom(*inport_.getData());
        volume->dataMap_.dataRange = inport_.getData()->dataMap_.dataRange;

        if (adjustBasisAndOffset_.get()) {
            vec3 volOffset = inport_.getData()->getOffset() + vec3(offset) / vec3(inport_.getData()->getDimensions());
            mat3 volBasis = inport_.getData()->getBasis();

            vec3 aVec(volBasis[0]);
            vec3 bVec(volBasis[1]);
            vec3 cVec(volBasis[2]);

            float alpha = glm::angle(bVec, cVec);
            float beta = glm::angle(cVec, aVec);
            float gamma = glm::angle(aVec, bVec);

            vec3 volLength(glm::length(aVec), glm::length(bVec), glm::length(cVec));
            // adjust volLength
            volLength *= vec3(dim) / vec3(inport_.getData()->getDimensions());

            float a = volLength.x;
            float b = volLength.y;
            float c = volLength.z;

            float v = std::sqrt(1 - std::cos(alpha)*std::cos(alpha) - std::cos(beta)*std::cos(beta) - std::cos(gamma)*std::cos(gamma)
                - 2 * std::cos(alpha)*std::cos(beta)*std::cos(gamma));
            mat4 newBasisAndOffset(
                a, b*std::cos(gamma), c*std::cos(beta), volOffset[0],
                0.0f, b*std::sin(gamma), c*(std::cos(alpha) - std::cos(beta)*std::cos(gamma)) / std::sin(gamma), volOffset[1],
                0.0f, 0.0f, c*v / std::sin(gamma), volOffset[2],
                0.0f, 0.0f, 0.0f, 1.0f
                );
            volume->setModelMatrix(glm::transpose(newBasisAndOffset));
        }
        else {
            // copy basis and offset
            volume->setModelMatrix(inport_.getData()->getModelMatrix());
        }
        outport_.setData(volume);
    }
    else {
        outport_.setConstData(inport_.getData());
    }
}

} // inviwo namespace
