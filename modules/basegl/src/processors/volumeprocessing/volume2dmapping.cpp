/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2025-2026 Inviwo Foundation
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

#include <modules/basegl/processors/volumeprocessing/volume2dmapping.h>
#include <modules/opengl/shader/shaderutils.h>
#include <modules/opengl/texture/textureutils.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo Volume2DMapping::processorInfo_{
    "org.inviwo.Volume2DMapping",  // Class identifier
    "Volume 2D Mapping",           // Display name
    "Volume Operation",            // Category
    CodeState::Stable,             // Code state
    Tags::GL,                      // Tags
    R"(Maps voxel values of two channels of an input volume to a color-mapped volume using
       a Layer for 2D lookups.)"_unindentHelp,
};

const ProcessorInfo& Volume2DMapping::getProcessorInfo() const { return processorInfo_; }

Volume2DMapping::Volume2DMapping()
    : VolumeGLProcessor{"volume_2dmapping.frag"}
    , tfLookup_{"tfLookup", ""_help}
    , channel_{OptionPropertyInt{"channel1", "Channel 1",
                                 "Selected channel for the first dimension of the TF lookup"_help,
                                 util::enumeratedOptions("Channel", 4)},
               OptionPropertyInt{"channel2", "Channel 2",
                                 "Selected channel for the second dimension of the TF lookup"_help,
                                 util::enumeratedOptions("Channel", 4), 1}} {

    addProperties(channel_[0], channel_[1]);
    addPort(tfLookup_);
    outport_.setHelp(
        "Output volume after applying the 2D transfer function to two input channels."_help);
}

void Volume2DMapping::preProcess(TextureUnitContainer& cont, Shader& shader, VolumeConfig& config) {
    utilgl::bindAndSetUniforms(shader, cont, tfLookup_);
    utilgl::setUniforms(shader, channel_[0], channel_[1]);

    config.format = tfLookup_.getData()->getDataFormat();
    config.swizzleMask = tfLookup_.getData()->getSwizzleMask();
    config.dataRange = tfLookup_.getData()->dataMap.dataRange;
    config.valueRange = tfLookup_.getData()->dataMap.valueRange;
    config.valueAxis = tfLookup_.getData()->dataMap.valueAxis;
}

}  // namespace inviwo
