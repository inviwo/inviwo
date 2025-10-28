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

#include <modules/basegl/processors/volumeprocessing/volumediff.h>

#include <inviwo/core/ports/volumeport.h>                                  // for VolumeInport
#include <inviwo/core/processors/processorinfo.h>                          // for ProcessorInfo
#include <inviwo/core/processors/processorstate.h>                         // for CodeState, Cod...
#include <inviwo/core/processors/processortags.h>                          // for Tags, Tags::GL
#include <modules/basegl/processors/volumeprocessing/volumeglprocessor.h>  // for VolumeGLProcessor
#include <modules/opengl/volume/volumeutils.h>                             // for bindAndSetUnif...

#include <memory>       // for shared_ptr
#include <string>       // for string
#include <string_view>  // for string_view

namespace inviwo {
class TextureUnitContainer;

const ProcessorInfo VolumeDiff::processorInfo_{
    "org.inviwo.VolumeDiff",  // Class identifier
    "Volume Difference",      // Display name
    "Volume Operation",       // Category
    CodeState::Stable,        // Code state
    Tags::GL,                 // Tags
    "Computes the difference between two volumes "
    "by subtracting the second volume from the first one."_help,
};

const ProcessorInfo& VolumeDiff::getProcessorInfo() const { return processorInfo_; }

VolumeDiff::VolumeDiff()
    : VolumeGLProcessor("volume_difference.frag"), vol2_("volume2", "Input volume 2"_help) {
    addPort(vol2_);

    addProperties(calculateDataRange_, dataRange_);

    inport_->setHelp("Input volume 2"_help);
    outport_.setHelp(
        "Difference volume corresponding to <tt>(volume 1 - volume 2 + 1.0) / 2.0</tt>"_help);
}

void VolumeDiff::preProcess(TextureUnitContainer& cont, Shader& shader,
                            [[maybe_unused]] VolumeConfig& config) {
    utilgl::bindAndSetUniforms(shader, cont, *vol2_.getData(), "volume2");
}

}  // namespace inviwo
