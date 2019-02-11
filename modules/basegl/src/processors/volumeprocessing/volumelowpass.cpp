/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2019 Inviwo Foundation
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

#include <modules/basegl/processors/volumeprocessing/volumelowpass.h>

#include <inviwo/core/datastructures/volume/volumeram.h>
#include <inviwo/core/datastructures/volume/volumeramprecision.h>
#include <modules/opengl/volume/volumegl.h>
#include <modules/opengl/shader/shader.h>
#include <modules/opengl/shader/shaderutils.h>
#include <modules/base/algorithm/dataminmax.h>

namespace inviwo {

const ProcessorInfo VolumeLowPass::processorInfo_{
    "org.inviwo.VolumeLowPass",  // Class identifier
    "Volume Low Pass",           // Display name
    "Volume Operation",          // Category
    CodeState::Stable,           // Code state
    "GL",                        // Tags
};
const ProcessorInfo VolumeLowPass::getProcessorInfo() const { return processorInfo_; }

VolumeLowPass::VolumeLowPass()
    : VolumeGLProcessor("volume_lowpass.frag")
    , kernelSize_("kernelSize", "Kernel size", 3, 2, 27)
    , useGaussianWeights_("useGaussianWeights", "Use Gaussian Weights")
    , sigma_("sigma", "Sigma", 1.f, 0.001f, 2.f, 0.001f)
    , updateDataRange_("updateDataRange", "Update Data Range", false) {
    addProperty(kernelSize_);
    addProperty(useGaussianWeights_);
    addProperty(updateDataRange_);
    useGaussianWeights_.addProperty(sigma_);
    useGaussianWeights_.getBoolProperty()->setInvalidationLevel(
        InvalidationLevel::InvalidResources);

    sigma_.onChange([&]() {
        int kernelSize90 = static_cast<int>(sigma_.get() * 2 * 1.645f);
        int kernelSize95 = static_cast<int>(sigma_.get() * 2 * 1.960f);
        int kernelSize99 = static_cast<int>(sigma_.get() * 2 * 2.576f);
        // https://de.wikipedia.org/wiki/Normalverteilung
        LogInfo("Optimizal kernelSize for sigma " << sigma_.get() << " is: "
                                                  << "\n\t90%: " << kernelSize90 << "\n\t95%: "
                                                  << kernelSize95 << "\n\t99%: " << kernelSize99);
    });

    setAllPropertiesCurrentStateAsDefault();
}

VolumeLowPass::~VolumeLowPass() {}

void VolumeLowPass::preProcess(TextureUnitContainer &) {
    utilgl::setUniforms(shader_, kernelSize_);

    float sigmaSq2 = 2.0f * sigma_.get() * sigma_.get();
    float a = static_cast<float>(1.0 / (sigmaSq2 * M_PI));

    shader_.setUniform("sigmaSq2", sigmaSq2);
    shader_.setUniform("a", a);
}

void VolumeLowPass::postProcess() {
    if (updateDataRange_.get()) {
        auto minmax = util::volumeMinMax(volume_.get(), IgnoreSpecialValues::Yes);
        auto min = minmax.first.x;
        auto max = minmax.second.x;
        for (size_t i = 1; i < volume_->getDataFormat()->getComponents(); i++) {
            min = std::min(min, minmax.first[i]);
            max = std::max(max, minmax.second[i]);
        }
        volume_->dataMap_.valueRange = volume_->dataMap_.dataRange = dvec2(min, max);

    } else {
        volume_->dataMap_ = inport_.getData()->dataMap_;
    }
}

void VolumeLowPass::initializeResources() {
    if (useGaussianWeights_.isChecked()) {
        shader_.getFragmentShaderObject()->addShaderDefine("GAUSSIAN");
    } else {
        shader_.getFragmentShaderObject()->removeShaderDefine("GAUSSIAN");
    }

    VolumeGLProcessor::initializeResources();
    shader_.build();
}

}  // namespace inviwo
