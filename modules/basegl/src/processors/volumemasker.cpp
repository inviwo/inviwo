/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2023-2025 Inviwo Foundation
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

#include <modules/basegl/processors/volumemasker.h>
#include <modules/opengl/volume/volumeutils.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo VolumeMasker::processorInfo_{"org.inviwo.VolumeMasker",  // Class identifier
                                                 "Volume Masker",            // Display name
                                                 "Volume Operation",         // Category
                                                 CodeState::Experimental,    // Code state
                                                 Tags::GL,                   // Tags
                                                 R"(
Mask `inputVolume` with the second `mask` volume through multiplication. The output volume inherits all 
parameters from the input volume including value/data ranges and transformations. Its contents are set to
```glsl
output = input * mask.r,
```
where the mask volume is assumed to be a single-channel volume.

**Note:** 
Depending on the data ranges of the input volume and the mask volume, the result may lie outside the 
data range of the input.
)"_unindentHelp};

const ProcessorInfo& VolumeMasker::getProcessorInfo() const { return processorInfo_; }

VolumeMasker::VolumeMasker()
    : VolumeGLProcessor{"volumemask.frag"}
    , mask_{"mask", "Volume mask applied to the input volume"_help}
    , useWorldSpace_{"useWorldSpace", "Use World Coordinates",
                     "If enabled, world coordinates are used for texture lookups meaning that "
                     "basis and world transformations of the input volume and the mask are "
                     "considered. Otherwise texture lookups use [0, 1] texture coordinates "
                     "assuming overlapping volumes."_help,
                     false}
    , textureWrap_{"textureWrap", "Apply Texture Wrapping",
                   "Accessing the mask volume considers its texture wrapping settings, if enabled. "
                   "Otherwise texture accesses outside [0,1] automatically result in 0. Only "
                   "affects the masking when using world coordinates."_help,
                   false} {

    addPorts(mask_);
    addProperties(useWorldSpace_, textureWrap_);
}

void VolumeMasker::preProcess(TextureUnitContainer& cont) {
    utilgl::bindAndSetUniforms(shader_, cont, *mask_.getData(), "mask");

    if (useWorldSpace_) {
        shader_.setUniform(
            "texTrafo", mask_.getData()->getCoordinateTransformer().getWorldToDataMatrix() *
                            inport_.getData()->getCoordinateTransformer().getDataToWorldMatrix());
    } else {
        shader_.setUniform("texTrafo", mat4(1.0f));
    }
    shader_.setUniform("textureWrap", textureWrap_);
}

}  // namespace inviwo
