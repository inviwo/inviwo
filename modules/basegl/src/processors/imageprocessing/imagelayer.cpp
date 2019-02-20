/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2019 Inviwo Foundation
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

#include <modules/basegl/processors/imageprocessing/imagelayer.h>

#include <inviwo/core/datastructures/image/image.h>
#include <modules/opengl/openglutils.h>
#include <modules/opengl/texture/textureutils.h>
#include <modules/opengl/texture/textureunit.h>
#include <modules/opengl/image/imagegl.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo ImageLayer::processorInfo_{
    "org.inviwo.ImageLayer",  // Class identifier
    "Image Layer",            // Display name
    "Image Operation",        // Category
    CodeState::Stable,        // Code state
    "GL, Image, Layer",       // Tags
};
const ProcessorInfo ImageLayer::getProcessorInfo() const { return processorInfo_; }

ImageLayer::ImageLayer()
    : Processor()
    , inport_("inport")
    , outport_("outport")
    , outputLayer_("outputLayer", "Output Layer")
    , shader_("img_identity.vert", "img_copy.frag") {

    addPort(inport_);
    addPort(outport_);

    addProperty(outputLayer_);

    auto populateOptionProperty = [this]() {
        std::vector<OptionPropertyIntOption> options;
        if (inport_.hasData()) {
            for (size_t i = 0; i < inport_.getData()->getNumberOfColorLayers(); ++i) {
                options.emplace_back("color" + std::to_string(i),
                                     "Color Layer " + std::to_string(i + 1), static_cast<int>(i));
            }
            options.emplace_back("depth", "Depth Layer", LayerEnum::Depth);
            options.emplace_back("picking", "Picking Layer", LayerEnum::Picking);
        }
        outputLayer_.replaceOptions(options);
    };

    inport_.onChange(populateOptionProperty);

    shader_.onReload([this]() { invalidate(InvalidationLevel::InvalidResources); });
}

void ImageLayer::process() {
    utilgl::activateAndClearTarget(outport_, ImageType::ColorDepthPicking);

    shader_.activate();
    utilgl::DepthFuncState depthFunc(GL_ALWAYS);

    TextureUnit colorTexUnit;
    TextureUnit pickingTexUnit;
    TextureUnit depthTexUnit;

    int colorUnit = colorTexUnit.getUnitNumber();
    switch (outputLayer_.get()) {
        case LayerEnum::Depth:
            colorUnit = depthTexUnit.getUnitNumber();
            break;
        case LayerEnum::Picking:
            colorUnit = pickingTexUnit.getUnitNumber();
            break;
        default: {
            auto imageGL = inport_.getData()->getRepresentation<ImageGL>();
            if (auto layer = imageGL->getColorLayerGL(outputLayer_.get())) {
                layer->bindTexture(colorTexUnit);
            }
            break;
        }
    }

    utilgl::bindDepthTexture(inport_, depthTexUnit.getEnum());
    utilgl::bindPickingTexture(inport_, pickingTexUnit.getEnum());

    shader_.setUniform("color_", colorUnit);
    shader_.setUniform("depth_", depthTexUnit.getUnitNumber());
    shader_.setUniform("picking_", pickingTexUnit.getUnitNumber());

    utilgl::singleDrawImagePlaneRect();

    shader_.deactivate();
    utilgl::deactivateCurrentTarget();
}

}  // namespace inviwo
