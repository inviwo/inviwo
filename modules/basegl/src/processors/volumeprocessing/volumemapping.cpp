/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2025 Inviwo Foundation
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

#include <modules/basegl/processors/volumeprocessing/volumemapping.h>

#include <inviwo/core/processors/processorinfo.h>                          // for ProcessorInfo
#include <inviwo/core/processors/processorstate.h>                         // for CodeState, Cod...
#include <inviwo/core/processors/processortags.h>                          // for Tags, Tags::GL
#include <inviwo/core/properties/transferfunctionproperty.h>               // for TransferFuncti...
#include <modules/basegl/processors/volumeprocessing/volumeglprocessor.h>  // for VolumeGLProcessor
#include <modules/opengl/texture/textureutils.h>                           // for bindAndSetUnif...

#include <string>       // for string
#include <string_view>  // for string_view

namespace inviwo {
class TextureUnitContainer;

const ProcessorInfo VolumeMapping::processorInfo_{
    "org.inviwo.VolumeMapping",  // Class identifier
    "Volume Mapping",            // Display name
    "Volume Operation",          // Category
    CodeState::Stable,           // Code state
    Tags::GL,                    // Tags
    "Maps the voxel values of an input volume to an alpha-only volume "
    "by applying a transfer function."_help};
const ProcessorInfo& VolumeMapping::getProcessorInfo() const { return processorInfo_; }

VolumeMapping::VolumeMapping()
    : VolumeGLProcessor("volume_mapping.frag")
    , tfProperty_("transferFunction", "Transfer function",
                  "Defines the transfer function for mapping voxel values to opacity"_help,
                  TransferFunction({{0.0, vec4(0.0f)}, {1.0, vec4(1.0f)}}), &inport_) {
    addProperty(tfProperty_);

    outport_.setHelp(
        "Output volume containing the alpha channel "
        "after applying the transfer function to the input"_help);
}

VolumeMapping::~VolumeMapping() = default;

void VolumeMapping::preProcess(TextureUnitContainer& cont) {
    utilgl::bindAndSetUniforms(shader_, cont, tfProperty_);
}

}  // namespace inviwo
