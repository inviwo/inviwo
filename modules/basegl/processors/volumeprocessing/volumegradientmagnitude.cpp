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

#include <modules/opengl/glwrap/shader.h>
#include "volumegradientmagnitude.h"

namespace inviwo {
ProcessorClassIdentifier(VolumeGradientMagnitude, "org.inviwo.VolumeGradientMagnitude");
ProcessorDisplayName(VolumeGradientMagnitude, "Volume Gradient Magnitude");
ProcessorTags(VolumeGradientMagnitude, Tags::GL);
ProcessorCategory(VolumeGradientMagnitude, "Volume Operation");
ProcessorCodeState(VolumeGradientMagnitude, CODE_STATE_STABLE);

VolumeGradientMagnitude::VolumeGradientMagnitude()
    : VolumeGLProcessor("volumegradientmagnitude.frag")
    , channel_("channel", "Render Channel")
{
    this->dataFormat_ = DataFLOAT32::get();

    channel_.addOption("Channel 1", "Channel 1", 0);
    channel_.setCurrentStateAsDefault();
    
    addProperty(channel_);
}

VolumeGradientMagnitude::~VolumeGradientMagnitude() {}

void VolumeGradientMagnitude::preProcess() {
    shader_->setUniform("channel_", channel_.getSelectedValue());
}

void VolumeGradientMagnitude::postProcess() {
    outport_.getData()->dataMap_.dataRange = dvec2(0, 1);
}

void VolumeGradientMagnitude::afterInportChanged() {
    if (inport_.hasData()){
        int channels = inport_.getData()->getDataFormat()->getComponents();

        if(channels == static_cast<int>(channel_.size()))
            return;

        channel_.clearOptions();
        for (int i = 0; i < channels; i++) {
            std::stringstream ss;
            ss << "Channel " << i;
            channel_.addOption(ss.str() , ss.str(), i);
        }
        channel_.setCurrentStateAsDefault();
    }
}

}  // namespace
