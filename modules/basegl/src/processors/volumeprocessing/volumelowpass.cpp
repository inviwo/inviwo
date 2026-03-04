/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2026 Inviwo Foundation
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

#include <inviwo/core/algorithm/markdown.h>
#include <inviwo/core/datastructures/datamapper.h>
#include <inviwo/core/ports/volumeport.h>
#include <inviwo/core/processors/processorinfo.h>
#include <inviwo/core/processors/processorstate.h>
#include <inviwo/core/processors/processortags.h>
#include <inviwo/core/properties/boolcompositeproperty.h>
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/properties/invalidationlevel.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/util/document.h>
#include <inviwo/core/util/formats.h>
#include <inviwo/core/util/glmvec.h>
#include <inviwo/core/util/logcentral.h>
#include <modules/base/algorithm/algorithmoptions.h>
#include <modules/base/algorithm/dataminmax.h>
#include <modules/basegl/processors/volumeprocessing/volumeglprocessor.h>
#include <modules/opengl/shader/shader.h>
#include <modules/opengl/shader/shaderobject.h>
#include <modules/opengl/shader/shaderutils.h>

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <functional>
#include <memory>
#include <ostream>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <numbers>

#include <glm/vec4.hpp>

namespace inviwo {
class TextureUnitContainer;

const ProcessorInfo VolumeLowPass::processorInfo_{
    "org.inviwo.VolumeLowPass",  // Class identifier
    "Volume Low Pass",           // Display name
    "Volume Operation",          // Category
    CodeState::Stable,           // Code state
    Tags::GL,                    // Tags
    "Applies a low pass filter on the input volume."_help};
const ProcessorInfo& VolumeLowPass::getProcessorInfo() const { return processorInfo_; }

VolumeLowPass::VolumeLowPass()
    : VolumeGLProcessor("volume_lowpass.frag")
    , kernelSize_("kernelSize", "Kernel size", "Size of the applied low pass filter."_help, 3,
                  {2, ConstraintBehavior::Immutable}, {27, ConstraintBehavior::Immutable})
    , useGaussianWeights_("useGaussianWeights", "Use Gaussian Weights",
                          "Toggles between a Gaussian kernel and a box filter."_help)
    , sigma_("sigma", "Sigma",
             util::ordinalScale(1.0f, 2.0f).set("Sigma used by the Gaussian kernel."_help)) {

    addProperties(kernelSize_, useGaussianWeights_, calculateDataRange_, dataRange_);
    useGaussianWeights_.addProperty(sigma_);
    useGaussianWeights_.getBoolProperty()->setInvalidationLevel(
        InvalidationLevel::InvalidResources);

    sigma_.onChange([&]() {
        int kernelSize90 = static_cast<int>(sigma_.get() * 2 * 1.645f);
        int kernelSize95 = static_cast<int>(sigma_.get() * 2 * 1.960f);
        int kernelSize99 = static_cast<int>(sigma_.get() * 2 * 2.576f);
        // https://de.wikipedia.org/wiki/Normalverteilung
        log::info("Optimal kernelSize for sigma {} is:\n\t90%: {}\n\t95%: {}\n\t99%: {}",
                  sigma_.get(), kernelSize95, kernelSize90, kernelSize99);
    });

    setAllPropertiesCurrentStateAsDefault();
}

VolumeLowPass::~VolumeLowPass() = default;

void VolumeLowPass::preProcess([[maybe_unused]] TextureUnitContainer& cont, Shader& shader,
                               [[maybe_unused]] VolumeConfig& config) {
    utilgl::setUniforms(shader, kernelSize_);

    float sigmaSq2 = 2.0f * sigma_.get() * sigma_.get();
    float a = 1.0f / (sigmaSq2 * std::numbers::pi_v<float>);

    shader.setUniform("sigmaSq2", sigmaSq2);
    shader.setUniform("a", a);
}

void VolumeLowPass::initializeShader(Shader& shader) {
    if (useGaussianWeights_.isChecked()) {
        shader.getFragmentShaderObject()->addShaderDefine("GAUSSIAN");
    } else {
        shader.getFragmentShaderObject()->removeShaderDefine("GAUSSIAN");
    }
}

}  // namespace inviwo
