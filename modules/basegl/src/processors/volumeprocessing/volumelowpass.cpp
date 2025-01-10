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

#include <modules/basegl/processors/volumeprocessing/volumelowpass.h>

#include <inviwo/core/algorithm/markdown.h>                                // for operator""_help
#include <inviwo/core/datastructures/datamapper.h>                         // for DataMapper
#include <inviwo/core/ports/volumeport.h>                                  // for VolumeInport
#include <inviwo/core/processors/processorinfo.h>                          // for ProcessorInfo
#include <inviwo/core/processors/processorstate.h>                         // for CodeState, Cod...
#include <inviwo/core/processors/processortags.h>                          // for Tags, Tags::GL
#include <inviwo/core/properties/boolcompositeproperty.h>                  // for BoolCompositeP...
#include <inviwo/core/properties/boolproperty.h>                           // for BoolProperty
#include <inviwo/core/properties/invalidationlevel.h>                      // for InvalidationLevel
#include <inviwo/core/properties/ordinalproperty.h>                        // for FloatProperty
#include <inviwo/core/util/document.h>                                     // for Document
#include <inviwo/core/util/formats.h>                                      // for DataFormatBase
#include <inviwo/core/util/glmvec.h>                                       // for dvec2
#include <inviwo/core/util/logcentral.h>                                   // for LogCentral
#include <modules/base/algorithm/algorithmoptions.h>                       // for IgnoreSpecialV...
#include <modules/base/algorithm/dataminmax.h>                             // for volumeMinMax
#include <modules/basegl/processors/volumeprocessing/volumeglprocessor.h>  // for VolumeGLProcessor
#include <modules/opengl/shader/shader.h>                                  // for Shader
#include <modules/opengl/shader/shaderobject.h>                            // for ShaderObject
#include <modules/opengl/shader/shaderutils.h>                             // for setUniforms

#include <algorithm>    // for max, min
#include <cmath>        // for M_PI
#include <cstddef>      // for size_t
#include <functional>   // for __base
#include <memory>       // for shared_ptr
#include <ostream>      // for operator<<
#include <string>       // for string, char_t...
#include <string_view>  // for string_view
#include <type_traits>  // for remove_extent_t
#include <utility>      // for pair
#include <numbers>

#include <glm/vec4.hpp>  // for vec, vec<>::(a...

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
    , kernelSize_("kernelSize", "Kernel size", 3, 2, 27)
    , useGaussianWeights_("useGaussianWeights", "Use Gaussian Weights")
    , sigma_("sigma", "Sigma", 1.f, 0.001f, 2.f, 0.001f)
    , updateDataRange_("updateDataRange", "Update Data Range", false) {

    kernelSize_.getHelp() = Document{"Size of the applied low pass filter."};
    useGaussianWeights_.getHelp() = Document{"Toggles between a Gaussian kernel and a box filter."};
    sigma_.getHelp() = Document{"Sigma used by the Gaussian kernel."};
    updateDataRange_.getHelp() =
        Document{"Calculate and assign a new data range for the smoothed volume."};

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
        log::info("Optimal kernelSize for sigma {} is:\n\t90%: {}\n\t95%: {}\n\t99%: {}",
                  sigma_.get(), kernelSize95, kernelSize90, kernelSize99);
    });

    setAllPropertiesCurrentStateAsDefault();
}

VolumeLowPass::~VolumeLowPass() {}

void VolumeLowPass::preProcess(TextureUnitContainer&) {
    utilgl::setUniforms(shader_, kernelSize_);

    float sigmaSq2 = 2.0f * sigma_.get() * sigma_.get();
    float a = 1.0f / (sigmaSq2 * std::numbers::pi_v<float>);

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
        volume_->dataMap.valueRange = volume_->dataMap.dataRange = dvec2(min, max);

    } else {
        volume_->dataMap = inport_.getData()->dataMap;
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
