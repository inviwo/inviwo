/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2019 Inviwo Foundation
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

#include <modules/postprocessing/processors/depthdarkening.h>
#include <modules/opengl/texture/textureutils.h>
#include <inviwo/core/datastructures/image/image.h>
#include <inviwo/core/datastructures/image/layer.h>
#include <inviwo/core/datastructures/image/layerram.h>
#include <inviwo/core/datastructures/image/layerramprecision.h>
#include <modules/opengl/texture/textureutils.h>
#include <modules/opengl/shader/shaderutils.h>
#include <modules/opengl/image/layergl.h>
#include <modules/opengl/texture/texture2d.h>
#include <modules/basegl/processors/imageprocessing/imagelowpass.h>

#include <modules/basegl/algorithm/imageconvolution.h>

#include <modules/base/algorithm/dataminmax.h>
#include <numeric>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo DepthDarkening::processorInfo_{
    "org.inviwo.DepthDarkening",  // Class identifier
    "Depth Darkening",            // Display name
    "Postprocessing",             // Category
    CodeState::Experimental,      // Code state
    Tags::None,                   // Tags
};
const ProcessorInfo DepthDarkening::getProcessorInfo() const { return processorInfo_; }

DepthDarkening::DepthDarkening()
    : Processor()
    , inport_("inport_")
    , outport_("outport")

    , sigma_("sigma", "Spread (Sigma/Gaussian Width)", 2.0f, 0.1f, 100.0f)
    , lambda_("lambda", "Amount (Lambda)", 10.0f, 0.0f, 200.0f)

    , cam_("camera", "Camera")

    , depthDarkeningShader_("depthdarkening.frag")
    , linearizeDepthShader_("depthlinearize.frag", false)

    , convolution_([&]() { this->invalidate(InvalidationLevel::InvalidOutput); })

{
    addPort(inport_);
    addPort(outport_);

    addProperty(sigma_);
    addProperty(lambda_);

    addProperty(cam_);

    linearizeDepthShader_.getFragmentShaderObject()->addShaderDefine("NORMALIZE");
    linearizeDepthShader_.build();

    depthDarkeningShader_.onReload([&]() { this->invalidate(InvalidationLevel::InvalidOutput); });
    linearizeDepthShader_.onReload([&]() { this->invalidate(InvalidationLevel::InvalidOutput); });
}

void DepthDarkening::process() {

    // Pass 1 - Linearize (and normalize) depth

    if (!linearDepth_ || linearDepth_->getDimensions() != outport_.getData()->getDimensions()) {
        linearDepth_ =
            std::make_shared<Image>(outport_.getData()->getDimensions(), DataFloat32::get());
    }

    {

        auto layer = static_cast<const LayerRAMPrecision<float>*>(
            inport_.getData()->getDepthLayer()->getRepresentation<LayerRAM>());
        const float* data = layer->getDataTyped();

        auto size = layer->getDimensions().x * layer->getDimensions().y;

        vec2 minmax(1, 0);
        minmax =
            std::accumulate(data, data + size, minmax, [](const vec2& mm, const float& v) -> vec2 {
                if (v < std::numeric_limits<float>::epsilon() ||
                    v >= 1 - std::numeric_limits<float>::epsilon())
                    return mm;
                return {glm::min(mm.x, v), glm::max(mm.y, v)};
            });

        float z_n = cam_.getNearPlaneDist();
        float z_f = cam_.getFarPlaneDist();
        TextureUnit inportTexture0;
        utilgl::bindDepthTexture(inport_, inportTexture0.getEnum());
        utilgl::activateTarget(*linearDepth_);
        linearizeDepthShader_.activate();
        linearizeDepthShader_.setUniform("inputTexture", inportTexture0.getUnitNumber());
        linearizeDepthShader_.setUniform("clipInfo", vec4(z_n * z_f, z_n - z_f, z_f, 1));
        linearizeDepthShader_.setUniform("minD", minmax.x);
        linearizeDepthShader_.setUniform("maxD", minmax.y);

        utilgl::singleDrawImagePlaneRect();

        linearizeDepthShader_.deactivate();
        utilgl::deactivateCurrentTarget();
        utilgl::unbindDepthTexture(inport_);
    }

    // Pass 2 - Blur depth
    auto blurredDepth = convolution_.gaussianLowpass(*linearDepth_->getColorLayer(), sigma_);

    // Pass 3 - DD
    utilgl::activateTargetAndCopySource(outport_, inport_, ImageType::ColorOnly);
    depthDarkeningShader_.activate();

    utilgl::setUniforms(depthDarkeningShader_, sigma_, lambda_, cam_);
    depthDarkeningShader_.setUniform("reciprocalDimensions",
                                     vec2(1) / vec2(outport_.getDimensions()));

    TextureUnitContainer cont;
    utilgl::bindAndSetUniforms(
        depthDarkeningShader_, cont,
        *blurredDepth->getColorLayer()->getRepresentation<LayerGL>()->getTexture(), "blurredDepth");
    utilgl::bindAndSetUniforms(
        depthDarkeningShader_, cont,
        *inport_.getData()->getColorLayer()->getRepresentation<LayerGL>()->getTexture(), "image");
    utilgl::bindAndSetUniforms(
        depthDarkeningShader_, cont,
        *linearDepth_->getColorLayer()->getRepresentation<LayerGL>()->getTexture(), "depth");

    utilgl::singleDrawImagePlaneRect();

    depthDarkeningShader_.deactivate();
    utilgl::deactivateCurrentTarget();
}

}  // namespace inviwo
