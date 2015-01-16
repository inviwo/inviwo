/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 * Version 0.9
 *
 * Copyright (c) 2013-2015 Inviwo Foundation
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

#include "volumesubset.h"
#include <inviwo/core/datastructures/volume/volumeramsubset.h>
#include <glm/gtx/vector_angle.hpp>

namespace inviwo {

ProcessorClassIdentifier(VolumeSubset, "org.inviwo.VolumeSubset");
ProcessorDisplayName(VolumeSubset,  "Volume Subset");
ProcessorTags(VolumeSubset, Tags::CPU);
ProcessorCategory(VolumeSubset, "Volume Operation");
ProcessorCodeState(VolumeSubset, CODE_STATE_STABLE);

VolumeSubset::VolumeSubset() : Processor()
      , inport_("volume.inport")
      , outport_("volume.outport")
      , enabled_("enabled", "Enable Operation", true)
      , adjustBasisAndOffset_("adjustBasisAndOffset", "Adjust Basis and Offset", false)
      , rangeX_("rangeX", "X Slices", 0, 256, 0, 256, 1, 1)
      , rangeY_("rangeY", "Y Slices", 0, 256, 0, 256, 1, 1)
      , rangeZ_("rangeZ", "Z Slices", 0, 256, 0, 256, 1, 1)
{
    addPort(inport_);
    addPort(outport_);
    addProperty(enabled_);
    addProperty(adjustBasisAndOffset_);
    addProperty(rangeX_);
    addProperty(rangeY_);
    addProperty(rangeZ_);
    dims_ = uvec3(1,1,1);
}

VolumeSubset::~VolumeSubset() {}

void VolumeSubset::initialize() {
    Processor::initialize();
}

void VolumeSubset::deinitialize() {
    Processor::deinitialize();
}

void VolumeSubset::process() {
    if (dims_ != inport_.getData()->getDimensions()) {
        dims_ = inport_.getData()->getDimensions();
        rangeX_.setRangeMax(static_cast<int>(dims_.x));
        rangeY_.setRangeMax(static_cast<int>(dims_.y));
        rangeZ_.setRangeMax(static_cast<int>(dims_.z));
    }

    if (enabled_.get()) {
        const VolumeRAM* vol = inport_.getData()->getRepresentation<VolumeRAM>();
        uvec3 dim = uvec3(static_cast<unsigned int>(rangeX_.get().y), static_cast<unsigned int>(rangeY_.get().y),
                          static_cast<unsigned int>(rangeZ_.get().y));
        uvec3 offset = uvec3(static_cast<unsigned int>(rangeX_.get().x), static_cast<unsigned int>(rangeY_.get().x),
                             static_cast<unsigned int>(rangeZ_.get().x));
        dim -= offset;

        if (dim == dims_)
            outport_.setConstData(inport_.getData());
        else {
            Volume* volume = new Volume(VolumeRAMSubSet::apply(vol, dim, offset));
            // pass meta data on
            volume->copyMetaDataFrom(*inport_.getData());
            volume->dataMap_ = inport_.getData()->dataMap_;

            if (adjustBasisAndOffset_.get()) {
                vec3 volOffset = inport_.getData()->getOffset() + vec3(offset) / vec3(dims_);
                mat3 volBasis = inport_.getData()->getBasis();

                vec3 aVec(volBasis[0]);
                vec3 bVec(volBasis[1]);
                vec3 cVec(volBasis[2]);

                float alpha = glm::angle(bVec, cVec);
                float beta = glm::angle(cVec, aVec);
                float gamma = glm::angle(aVec, bVec);

                vec3 volLength(glm::length(aVec), glm::length(bVec), glm::length(cVec));
                // adjust volLength
                volLength *= vec3(dim) / vec3(dims_);

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
    }
    else {
        outport_.setConstData(inport_.getData());
    }
}

} // inviwo namespace
