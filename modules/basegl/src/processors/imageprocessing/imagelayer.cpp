/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2026 Inviwo Foundation
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

#include <inviwo/core/datastructures/image/imagetypes.h>
#include <inviwo/core/ports/imageport.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/processors/processorinfo.h>
#include <inviwo/core/processors/processorstate.h>
#include <inviwo/core/processors/processortags.h>
#include <inviwo/core/properties/invalidationlevel.h>
#include <inviwo/core/properties/optionproperty.h>
#include <modules/opengl/image/imagegl.h>
#include <modules/opengl/image/layergl.h>
#include <modules/opengl/inviwoopengl.h>
#include <modules/opengl/openglutils.h>
#include <modules/opengl/shader/shader.h>
#include <modules/opengl/texture/textureunit.h>
#include <modules/opengl/texture/textureutils.h>

#include <cstddef>
#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo ImageLayer::processorInfo_{
    "org.inviwo.ImageLayer",  // Class identifier
    "Image Layer",            // Display name
    "Image Operation",        // Category
    CodeState::Stable,        // Code state
    "GL, Image, Layer",       // Tags
    R"(Outputs a specific layer of the source image as color image. 
    Depth and picking layers are forwarded unmodified.)"_unindentHelp,
};
const ProcessorInfo& ImageLayer::getProcessorInfo() const { return processorInfo_; }

ImageLayer::ImageLayer()
    : Processor()
    , inport_("inport", "input image"_help)
    , outport_("outport", "output image containing the selected input layer as color layer"_help)
    , outputLayer_{"outputLayer",
                   "Output Layer",
                   "Determines which layer of the input image is used as new color layer"_help,
                   {{"color0", "Color Layer 1", 0},
                    {"color1", "Color Layer 2", 1},
                    {"color2", "Color Layer 3", 2},
                    {"color3", "Color Layer 4", 3},
                    {"color4", "Color Layer 5", 4},
                    {"color5", "Color Layer 6", 5},
                    {"color6", "Color Layer 7", 6},
                    {"depth", "Depth Layer", LayerEnum::Depth},
                    {"picking", "Picking Layer", LayerEnum::Picking}}}
    , shader_("img_identity.vert", "img_copy.frag") {

    addPorts(inport_, outport_);
    addProperty(outputLayer_);

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
            const auto nColorLayers = inport_.getData()->getNumberOfColorLayers();
            if (outputLayer_.get() >= static_cast<int>(nColorLayers)) {
                throw Exception(SourceContext{}, "Selected color layer '{}' is out of bounds '{}'",
                                outputLayer_.get(), nColorLayers);
            }
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
