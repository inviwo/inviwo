/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015-2018 Inviwo Foundation
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

#include <modules/basegl/processors/volumeprocessing/volumebinary.h>
#include <modules/opengl/shader/shaderutils.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo VolumeBinary::processorInfo_{
    "org.inviwo.VolumeBinary",  // Class identifier
    "Volume Binary",            // Display name
    "Volume Operation",         // Category
    CodeState::Stable,          // Code state
    Tags::None,                 // Tags
};
const ProcessorInfo VolumeBinary::getProcessorInfo() const { return processorInfo_; }

VolumeBinary::VolumeBinary()
    : VolumeGLProcessor("volume_binary.frag", false)
    , threshold_("threshold", "Threshold", 0.5)
    , op_("operator", "Operator", InvalidationLevel::InvalidResources) {
    addProperty(threshold_);
    addProperty(op_);

    op_.addOption("greaterthen", ">", Operator::GreaterThen);
    op_.addOption("greaterthenorequal", ">=", Operator::GreaterThenOrEqual);
    op_.addOption("lessthen", "<", Operator::LessThen);
    op_.addOption("lessthenorequal", "<=", Operator::LessThenOrEqual);
    op_.addOption("equal", "==", Operator::Equal);
    op_.addOption("notequal", "!=", Operator::NotEqual);

    op_.setCurrentStateAsDefault();
    this->dataFormat_ = DataUInt8::get();
}

void VolumeBinary::preProcess(TextureUnitContainer &) { utilgl::setUniforms(shader_, threshold_); }

void VolumeBinary::postProcess() { volume_->dataMap_.dataRange = vec2(0, 255); }

void VolumeBinary::initializeResources() {

    shader_.getFragmentShaderObject()->addShaderDefine("OP", op_.getSelectedDisplayName());
    shader_.build();
}

}  // namespace inviwo
