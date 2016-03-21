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

#include "volumemerger.h"
#include <modules/opengl/volume/volumeutils.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo VolumeMerger::processorInfo_{
    "org.inviwo.VolumeMerger",      // Class identifier
    "Volume Merger",                // Display name
    "Volume Operation",              // Category
    CodeState::Experimental,  // Code state
    Tags::None,               // Tags
};
const ProcessorInfo VolumeMerger::getProcessorInfo() const {
    return processorInfo_;
}

VolumeMerger::VolumeMerger()
    : VolumeGLProcessor("volumemerger.frag"), vol2_("volume2"), vol3_("volume3"), vol4_("volume4") {
    
    addPort(vol2_);
    addPort(vol3_);
    addPort(vol4_);
    
    vol2_.setOptional(true);
    vol3_.setOptional(true);
    vol4_.setOptional(true);

    auto changeFormat = [this]() {
        bool a = vol2_.isConnected();
        bool b = vol3_.isConnected();
        bool c = vol4_.isConnected();

        int numVolumes = 1;
        if (a ) {
            numVolumes++;
            shader_.getFragmentShaderObject()->addShaderDefine("HAS_VOL2");
        }
        else {
            shader_.getFragmentShaderObject()->removeShaderDefine("HAS_VOL2");
        }

        if (b) {
            numVolumes++;
            shader_.getFragmentShaderObject()->addShaderDefine("HAS_VOL3");
        }
        else {
            shader_.getFragmentShaderObject()->removeShaderDefine("HAS_VOL3");
        }        
        
        if (c) {
            numVolumes++;
            shader_.getFragmentShaderObject()->addShaderDefine("HAS_VOL4");
        }
        else {
            shader_.getFragmentShaderObject()->removeShaderDefine("HAS_VOL4");
        }

        shader_.build();

        auto inDF = inport_.getData()->getDataFormat();
        dataFormat_ = DataFormatBase::get(inDF->getNumericType(), numVolumes, inDF->getSize() * 8);

        internalInvalid_ = true;


        


    };

    inport_.onChange(changeFormat);
    vol2_.onChange(changeFormat);
    vol3_.onChange(changeFormat);
    vol4_.onChange(changeFormat);
}
    
void VolumeMerger::preProcess(TextureUnitContainer &cont)
{
    bool a = vol2_.isConnected();
    bool b = vol3_.isConnected();
    bool c = vol4_.isConnected();

    if (a) {
        utilgl::bindAndSetUniforms(shader_, cont, *vol2_.getData(), "vol2");
    }
    if (b) {
        utilgl::bindAndSetUniforms(shader_, cont, *vol3_.getData(), "vol3");
    }
    if (c) {
        utilgl::bindAndSetUniforms(shader_, cont, *vol4_.getData(), "vol4");
    }
}


} // namespace
