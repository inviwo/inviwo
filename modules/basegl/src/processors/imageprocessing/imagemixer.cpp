/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2024 Inviwo Foundation
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

#include <modules/basegl/processors/imageprocessing/imagemixer.h>

#include <inviwo/core/datastructures/image/image.h>       // for Image
#include <inviwo/core/datastructures/image/imagetypes.h>  // for ImageType, ImageType::ColorDept...
#include <inviwo/core/ports/imageport.h>                  // for ImageInport, ImageOutport
#include <inviwo/core/processors/processor.h>             // for Processor
#include <inviwo/core/processors/processorinfo.h>         // for ProcessorInfo
#include <inviwo/core/processors/processorstate.h>        // for CodeState, CodeState::Stable
#include <inviwo/core/processors/processortags.h>         // for Tags, Tags::GL
#include <inviwo/core/properties/boolproperty.h>          // for BoolProperty
#include <inviwo/core/properties/invalidationlevel.h>     // for InvalidationLevel, Invalidation...
#include <inviwo/core/properties/optionproperty.h>        // for OptionPropertyInt
#include <inviwo/core/properties/ordinalproperty.h>       // for FloatProperty
#include <inviwo/core/util/formats.h>                     // for DataFormatBase, NumericType
#include <modules/opengl/shader/shader.h>                 // for Shader, Shader::Build
#include <modules/opengl/shader/shaderobject.h>           // for ShaderObject
#include <modules/opengl/shader/shaderutils.h>            // for setUniforms
#include <modules/opengl/texture/textureunit.h>           // for TextureUnitContainer
#include <modules/opengl/texture/textureutils.h>          // for bindAndSetUniforms, activateAnd...

#include <algorithm>    // for max
#include <functional>   // for __base
#include <memory>       // for shared_ptr, make_shared, shared...
#include <string>       // for string
#include <string_view>  // for string_view
#include <type_traits>  // for remove_extent_t

namespace inviwo {

const ProcessorInfo ImageMixer::processorInfo_{
    "org.inviwo.ImageMixer",  // Class identifier
    "Image Mixer",            // Display name
    "Image Operation",        // Category
    CodeState::Stable,        // Code state
    Tags::GL,                 // Tags
};
const ProcessorInfo& ImageMixer::getProcessorInfo() const { return processorInfo_; }

ImageMixer::ImageMixer()
    : Processor()
    , inport0_("inport0")
    , inport1_("inport1")
    , outport_("outport")
    , blendingMode_("blendMode", "Blend Mode", InvalidationLevel::InvalidResources)
    , weight_("weight", "Weight", 0.5f, 0.0f, 1.0f)
    , clamp_("clamp", "Clamp values to zero and one", false, InvalidationLevel::InvalidResources)
    , shader_("img_mix.frag", Shader::Build::No) {

    shader_.onReload([this]() { invalidate(InvalidationLevel::InvalidResources); });

    addPort(inport0_);
    addPort(inport1_);
    addPort(outport_);

    blendingMode_.addOption("mix", "Mix", BlendModes::Mix);
    blendingMode_.addOption("over", "Over", BlendModes::Over);
    blendingMode_.addOption("multiply", "Multiply", BlendModes::Multiply);
    blendingMode_.addOption("screen", "Screen", BlendModes::Screen);
    blendingMode_.addOption("overlay", "Overlay", BlendModes::Overlay);
    blendingMode_.addOption("hardlight", "Hard Light", BlendModes::HardLight);
    blendingMode_.addOption("divide", "Divide", BlendModes::Divide);
    blendingMode_.addOption("addition", "Addition", BlendModes::Addition);
    blendingMode_.addOption("subtraction", "Subtraction", BlendModes::Subtraction);
    blendingMode_.addOption("difference", "Difference", BlendModes::Difference);
    blendingMode_.addOption("darkenonly", "DarkenOnly (min)", BlendModes::DarkenOnly);
    blendingMode_.addOption("brightenonly", "BrightenOnly (max)", BlendModes::BrightenOnly);
    blendingMode_.setSelectedValue(BlendModes::Mix);
    blendingMode_.setCurrentStateAsDefault();

    addProperty(blendingMode_);
    addProperty(weight_);
    addProperty(clamp_);

    blendingMode_.onChange([&]() { weight_.setVisible(blendingMode_.get() == BlendModes::Mix); });
}

ImageMixer::~ImageMixer() {}

void ImageMixer::process() {
    if (inport0_.isChanged() || inport1_.isChanged()) {
        auto format0 = inport0_.getData()->getDataFormat();
        auto format1 = inport1_.getData()->getDataFormat();

        // combine format0 and format1, preferring the larger type with respect to
        // precision (size in bit), number of components, and float over unsigned over signed
        auto precision0 = format0->getPrecision();
        auto precision1 = format1->getPrecision();
        auto nf0 = format0->getNumericType();
        auto nf1 = format1->getNumericType();

        NumericType numericType;
        if ((nf0 == NumericType::Float) || (nf1 == NumericType::Float)) {
            numericType = NumericType::Float;
        } else if ((nf0 == NumericType::UnsignedInteger) || (nf1 == NumericType::UnsignedInteger)) {
            numericType = NumericType::UnsignedInteger;
        } else {
            numericType = NumericType::SignedInteger;
        }

        auto format = DataFormatBase::get(
            numericType, std::max(format0->getComponents(), format1->getComponents()),
            std::max(precision0, precision1));
        if (format != outport_.getData()->getDataFormat()) {
            auto dimensions = outport_.getData()->getDimensions();
            auto img = std::make_shared<Image>(dimensions, format);
            img->copyMetaDataFrom(*inport0_.getData());
            outport_.setData(img);
        }
    }

    utilgl::activateAndClearTarget(outport_);
    shader_.activate();
    TextureUnitContainer units;
    utilgl::bindAndSetUniforms(shader_, units, inport0_, ImageType::ColorDepthPicking);
    utilgl::bindAndSetUniforms(shader_, units, inport1_, ImageType::ColorDepthPicking);
    utilgl::setUniforms(shader_, outport_, weight_);
    utilgl::singleDrawImagePlaneRect();
    shader_.deactivate();
    utilgl::deactivateCurrentTarget();
}

void ImageMixer::initializeResources() {
    // compositing defines
    std::string compositingKey = "COLOR_BLENDING(colorA, colorB)";
    std::string compositingValue = "";

    switch (blendingMode_.get()) {
        case BlendModes::Over:  //<! f(a,b) = b, b over a, regular front-to-back blending
            compositingValue = "over(colorA, colorB)";
            break;
        case BlendModes::Multiply:  //!< f(a,b) = a * b
            compositingValue = "multiply(colorA, colorB)";
            break;
        case BlendModes::Screen:  //!< f(a,b) = 1 - (1 - a) * (1 - b)
            compositingValue = "screen(colorA, colorB)";
            break;
        case BlendModes::Overlay:  //!< f(a,b) = 2 * a *b, if a < 0.5,   f(a,b) = 1 - 2(1 - a)(1 -
                                   // b), otherwise (combination of Multiply and Screen)
            compositingValue = "overlay(colorA, colorB)";
            break;
        case BlendModes::HardLight:  //!< Overlay where a and b are swapped
            compositingValue = "overlay(colorB, colorA)";
            break;
        case BlendModes::Divide:  //!< f(a,b) = a/b
            compositingValue = "divide(colorA, colorB)";
            break;
        case BlendModes::Addition:  //!< f(a,b) = a + b, clamped to [0,1]
            compositingValue = "addition(colorA, colorB)";
            break;
        case BlendModes::Subtraction:  //!< f(a,b) = a - b, clamped to [0,1]
            compositingValue = "subtraction(colorA, colorB)";
            break;
        case BlendModes::Difference:  //!< f(a,b) = |a - b|
            compositingValue = "difference(colorA, colorB)";
            break;
        case BlendModes::DarkenOnly:  //!< f(a,b) = min(a, b), per component
            compositingValue = "darkenOnly(colorA, colorB)";
            break;
        case BlendModes::BrightenOnly:  //!< f(a,b) = max(a, b), per component
            compositingValue = "brightenOnly(colorA, colorB)";
            break;
        case BlendModes::Mix:  //!< f(a,b) = a * (1 - alpha) + b * alpha
        default:
            compositingValue = "colorMix(colorA,colorB)";
            break;
    }

    if (clamp_) {
        shader_.getFragmentShaderObject()->addShaderDefine("CLAMP_VALUES");
    } else {
        shader_.getFragmentShaderObject()->removeShaderDefine("CLAMP_VALUES");
    }

    shader_.getFragmentShaderObject()->addShaderDefine(compositingKey, compositingValue);
    shader_.build();
}

}  // namespace inviwo
