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

#include "volumelowpass.h"

#include <inviwo/core/datastructures/volume/volumeram.h>
#include <inviwo/core/datastructures/volume/volumeramprecision.h>
#include <modules/opengl/volume/volumegl.h>
#include <modules/opengl/shader/shader.h>
#include <modules/opengl/shader/shaderutils.h>

namespace inviwo {

ProcessorClassIdentifier(VolumeLowPass, "org.inviwo.VolumeLowPass");
ProcessorDisplayName(VolumeLowPass, "Volume Low Pass");
ProcessorTags(VolumeLowPass, "GL");
ProcessorCategory(VolumeLowPass, "Volume Operation");
ProcessorCodeState(VolumeLowPass, CODE_STATE_EXPERIMENTAL);

VolumeLowPass::VolumeLowPass()
    : VolumeGLProcessor("volume_lowpass.frag")
    , kernelSize_("kernelSize", "Kernel size", 3, 2, 27)
    , useGaussianWeights_("useGaussianWeights", "Use Gaussian Weights")
    , sigma_("sigma", "Sigma", 1.f, 0.001f, 2.f, 0.001f) {
    addProperty(kernelSize_);
    addProperty(useGaussianWeights_);
    useGaussianWeights_.addProperty(sigma_);
    useGaussianWeights_.getBoolProperty()->setInvalidationLevel(INVALID_RESOURCES);

    setAllPropertiesCurrentStateAsDefault();
}

VolumeLowPass::~VolumeLowPass() {}

void VolumeLowPass::preProcess(TextureUnitContainer &cont) {
    utilgl::setUniforms(shader_, kernelSize_); 
    shader_.setUniform("inv2Sigma", 1.0f / (sigma_.get() * 2.0f));
}

void VolumeLowPass::initializeResources() {
    VolumeGLProcessor::initializeResources();

    if (useGaussianWeights_.isChecked()) {
        shader_.getFragmentShaderObject()->addShaderDefine("GAUSSIAN");
    } else {
        shader_.getFragmentShaderObject()->removeShaderDefine("GAUSSIAN");
    }

    shader_.build();
}

}  // namespace
