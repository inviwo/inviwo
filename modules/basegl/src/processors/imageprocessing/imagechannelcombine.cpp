/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2023 Inviwo Foundation
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

#include <modules/basegl/processors/imageprocessing/imagechannelcombine.h>

#include <inviwo/core/datastructures/image/image.h>       // for Image
#include <inviwo/core/datastructures/image/imagetypes.h>  // for ImageType, ImageType::ColorOnly
#include <inviwo/core/ports/imageport.h>                  // for ImageInport, ImageOutport
#include <inviwo/core/processors/processor.h>             // for Processor
#include <inviwo/core/processors/processorinfo.h>         // for ProcessorInfo
#include <inviwo/core/processors/processorstate.h>        // for CodeState, CodeState::Experimental
#include <inviwo/core/processors/processortags.h>         // for Tags, Tags::GL
#include <inviwo/core/properties/invalidationlevel.h>     // for InvalidationLevel, Invalidation...
#include <inviwo/core/properties/optionproperty.h>        // for OptionPropertyOption
#include <inviwo/core/util/exception.h>                   // for Exception
#include <inviwo/core/util/formats.h>                     // for DataFormatBase, NumericType
#include <inviwo/core/util/glmvec.h>                      // for size2_t
#include <inviwo/core/util/sourcecontext.h>               // for IVW_CONTEXT
#include <modules/opengl/shader/shader.h>                 // for Shader
#include <modules/opengl/shader/shaderutils.h>            // for setUniforms
#include <modules/opengl/texture/textureunit.h>           // for TextureUnitContainer
#include <modules/opengl/texture/textureutils.h>          // for bindAndSetUniforms, activateAnd...

#include <algorithm>    // for max
#include <functional>   // for __base
#include <memory>       // for shared_ptr, make_shared
#include <string>       // for string
#include <string_view>  // for string_view
#include <type_traits>  // for remove_extent_t

#include <glm/vec2.hpp>  // for operator!=

namespace inviwo {

const ProcessorInfo ImageChannelCombine::processorInfo_{
    "org.inviwo.ImageChannelCombine",  // Class identifier
    "Image Channel Combine",           // Display name
    "Image Operation",                 // Category
    CodeState::Experimental,           // Code state
    Tags::GL,                          // Tags
};
const ProcessorInfo ImageChannelCombine::getProcessorInfo() const { return processorInfo_; }

ImageChannelCombine::ImageChannelCombine()
    : Processor()
    , inport0_("inport0", true)
    , inport1_("inport1", true)
    , inport2_("inport2", true)
    , inport3_("inport3", true)
    , outport_("outport", false)
    , rChannelSrc_("redChannel", "Red Channel",
                   {{"r", "Red", 0}, {"g", "Green", 1}, {"b", "Blue", 2}, {"a", "Alpha", 3}})
    , gChannelSrc_("greenChannel", "Green Channel",
                   {{"r", "Red", 0}, {"g", "Green", 1}, {"b", "Blue", 2}, {"a", "Alpha", 3}})
    , bChannelSrc_("blueChannel", "Blue Channel",
                   {{"r", "Red", 0}, {"g", "Green", 1}, {"b", "Blue", 2}, {"a", "Alpha", 3}})
    , aChannelSrc_("alphaChannel", "Alpha Channel",
                   {{"r", "Red", 0}, {"g", "Green", 1}, {"b", "Blue", 2}, {"a", "Alpha", 3}})
    , alpha_("alpha", "Alpha", 1.0f, 0.0f, 1.0f, 0.001f)
    , shader_("img_channel_combine.frag") {
    shader_.onReload([this]() { invalidate(InvalidationLevel::InvalidResources); });

    addPort(inport0_);
    addPort(inport1_);
    addPort(inport2_);
    inport3_.setOptional(true);
    addPort(inport3_);

    addPort(outport_);

    addProperties(rChannelSrc_, gChannelSrc_, bChannelSrc_, aChannelSrc_, alpha_);
}

void ImageChannelCombine::process() {
    auto isSame = [](auto a, auto b, auto c) {
        if (a != b) return false;
        if (a != c) return false;
        return true;
    };
    auto sourceImage0 = inport0_.getData();
    auto sourceImage1 = inport1_.getData();
    auto sourceImage2 = inport2_.getData();
    auto sourceImage3 = inport3_.getData();

    if (isSame(sourceImage0->getDimensions(), sourceImage1->getDimensions(),
               sourceImage2->getDimensions()) &&
        (inport3_.isConnected() && inport3_.hasData() &&
         sourceImage3->getDimensions() != sourceImage0->getDimensions())) {
        throw Exception("Image dimensions of all inports need to be identical", IVW_CONTEXT);
    }

    const auto dimensions = inport0_.getData()->getDimensions();

    auto&& [type, precision] = [&]() {
        NumericType type0 = inport0_.getData()->getDataFormat()->getNumericType();
        NumericType type1 = inport1_.getData()->getDataFormat()->getNumericType();
        NumericType type2 = inport2_.getData()->getDataFormat()->getNumericType();
        NumericType type;
        if (type0 == type1 && type0 == type2) {
            type = type0;
        } else if (type0 == NumericType::Float || type1 == NumericType::Float ||
                   type2 == NumericType::Float) {
            type = NumericType::Float;
        } else if (type0 == NumericType::SignedInteger || type1 == NumericType::SignedInteger ||
                   type2 == NumericType::SignedInteger) {
            type = NumericType::SignedInteger;
        } else {
            type = NumericType::UnsignedInteger;
        }

        size_t prec0 = inport0_.getData()->getDataFormat()->getPrecision();
        size_t prec1 = inport1_.getData()->getDataFormat()->getPrecision();
        size_t prec2 = inport2_.getData()->getDataFormat()->getPrecision();
        size_t precision = std::max({prec0, prec1, prec2});

        return std::make_pair(type, precision);
    }();

    auto image =
        std::make_shared<Image>(*sourceImage0, noData, DataFormatBase::get(type, 4, precision));
    outport_.setData(image);

    utilgl::activateAndClearTarget(outport_, ImageType::ColorDepth);
    shader_.activate();
    TextureUnitContainer units;
    utilgl::bindAndSetUniforms(shader_, units, inport0_, ImageType::ColorOnly);
    utilgl::bindAndSetUniforms(shader_, units, inport1_, ImageType::ColorOnly);
    utilgl::bindAndSetUniforms(shader_, units, inport2_, ImageType::ColorOnly);
    if (inport3_.hasData()) {
        utilgl::bindAndSetUniforms(shader_, units, inport3_, ImageType::ColorOnly);
    }
    utilgl::setUniforms(shader_, outport_, rChannelSrc_, gChannelSrc_, bChannelSrc_, aChannelSrc_,
                        alpha_);
    shader_.setUniform("use_alpha_texture", inport3_.hasData());
    utilgl::singleDrawImagePlaneRect();
    shader_.deactivate();
    utilgl::deactivateCurrentTarget();
}

}  // namespace inviwo
