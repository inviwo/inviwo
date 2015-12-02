/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
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
#include <modules/base/algorithm/volume/volumeramsubset.h>
#include <inviwo/core/network/networklock.h>
#include <glm/gtx/vector_angle.hpp>

namespace inviwo {

const ProcessorInfo VolumeSubset::processorInfo_{
    "org.inviwo.VolumeSubset",  // Class identifier
    "Volume Subset",            // Display name
    "Volume Operation",         // Category
    CodeState::Stable,          // Code state
    Tags::CPU,                  // Tags
};
const ProcessorInfo VolumeSubset::getProcessorInfo() const {
    return processorInfo_;
}

VolumeSubset::VolumeSubset() : Processor()
      , inport_("volume.inport")
      , outport_("volume.outport")
      , enabled_("enabled", "Enable Operation", true)
      , adjustBasisAndOffset_("adjustBasisAndOffset", "Adjust Basis and Offset", true)
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
    dims_ = size3_t(1,1,1);

    // Since the ranges depend on the input volume dimensions, we make sure to always
    // serialize them so we can do a proper renormalization when we load new data.
    rangeX_.setSerializationMode(PropertySerializationMode::ALL);
    rangeY_.setSerializationMode(PropertySerializationMode::ALL);
    rangeZ_.setSerializationMode(PropertySerializationMode::ALL);

    inport_.onChange(this, &VolumeSubset::onVolumeChange);
}

VolumeSubset::~VolumeSubset() {}

void VolumeSubset::process() {
    if (enabled_.get()) {
        const VolumeRAM* vol = inport_.getData()->getRepresentation<VolumeRAM>();
        size3_t dim = size3_t(static_cast<unsigned int>(rangeX_.get().y),
                          static_cast<unsigned int>(rangeY_.get().y),
                          static_cast<unsigned int>(rangeZ_.get().y));
        size3_t offset = size3_t(static_cast<unsigned int>(rangeX_.get().x),
                             static_cast<unsigned int>(rangeY_.get().x),
                             static_cast<unsigned int>(rangeZ_.get().x));
        dim -= offset;

        if (dim == dims_)
            outport_.setData(inport_.getData());
        else {
            Volume* volume = new Volume(VolumeRAMSubSet::apply(vol, dim, offset));
            // pass meta data on
            volume->copyMetaDataFrom(*inport_.getData());
            volume->dataMap_ = inport_.getData()->dataMap_;

            if (adjustBasisAndOffset_.get()) {
                vec3 volOffset = inport_.getData()->getOffset();
                mat3 volBasis = inport_.getData()->getBasis();

                const vec3 newOffset =
                    volOffset + volBasis * (static_cast<vec3>(offset) / static_cast<vec3>(dims_));

                mat3 newBasis = volBasis;
                vec3 dimRatio = (static_cast<vec3>(dim) / static_cast<vec3>(dims_));
                newBasis[0] *= dimRatio[0];
                newBasis[1] *= dimRatio[1];
                newBasis[2] *= dimRatio[2];

                volume->setBasis(newBasis);
                volume->setOffset(newOffset);

            } else {
                // copy basis and offset
                volume->setModelMatrix(inport_.getData()->getModelMatrix());
            }
            outport_.setData(volume);
        }
    } else {
        outport_.setData(inport_.getData());
    }
}

void VolumeSubset::onVolumeChange() {
    NetworkLock lock(this);
    
    // Update to the new dimensions.
    dims_ = inport_.getData()->getDimensions();

    rangeX_.setRangeNormalized(ivec2(0, dims_.x));
    rangeY_.setRangeNormalized(ivec2(0, dims_.y));
    rangeZ_.setRangeNormalized(ivec2(0, dims_.z));

    // set the new dimensions to default if we were to press reset
    rangeX_.setCurrentStateAsDefault();
    rangeY_.setCurrentStateAsDefault();
    rangeZ_.setCurrentStateAsDefault();
}

} // inviwo namespace

