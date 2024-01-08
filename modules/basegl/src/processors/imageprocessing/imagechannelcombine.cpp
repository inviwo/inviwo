/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2024 Inviwo Foundation
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
    CodeState::Stable,                 // Code state
    Tags::GL,                          // Tags
    "Creates a 4 channel image with the connected inputs, from each of them the channel specified "
    "in its property is used. If the optional alpha channel is not provided, the alpha property "
    "is used to set the alpha value of the entire image."_help};
const ProcessorInfo ImageChannelCombine::getProcessorInfo() const { return processorInfo_; }

namespace {
const std::vector<OptionPropertyIntOption> channelsList = {
    {"r", "Red", 0}, {"g", "Green", 1}, {"b", "Blue", 2}, {"a", "Alpha", 3}};
}
ImageChannelCombine::ImageChannelCombine()
    : Processor()
    , inport_{ImageInport{"inport0", "Input for first channel (red)"_help,
                          OutportDeterminesSize::Yes},
              ImageInport{"inport1", "Input for the second channel (green)"_help,
                          OutportDeterminesSize::Yes},
              ImageInport{"inport2", "Input for the third channel (blue)"_help,
                          OutportDeterminesSize::Yes},
              ImageInport{"inport3", "Input for the fourth channel (alpha, optional)"_help,
                          OutportDeterminesSize::Yes}}
    , outport_("outport", false)
    , channel_{OptionPropertyInt{"redChannel", "Red Channel", channelsList},
               OptionPropertyInt{"greenChannel", "Green Channel", channelsList},
               OptionPropertyInt{"blueChannel", "Blue Channel", channelsList},
               OptionPropertyInt{"alphaChannel", "Alpha Channel", channelsList}}
    , alpha_("alpha", "Alpha",
             util::ordinalLength(1.0f, 1.0f)
                 .set("Alpha value used if there is no input for the fourth channel"_help))
    , shader_("img_channel_combine.frag") {

    outport_.setHelp("Output image with combined channels"_help);
    inport_[3].setOptional(true);
    addPorts(inport_[0], inport_[1], inport_[2], inport_[3], outport_);

    for (auto& prop : channel_) {
        addProperty(prop);
    }
    addProperty(alpha_);

    shader_.onReload([this]() { invalidate(InvalidationLevel::InvalidResources); });
}

void ImageChannelCombine::process() {
    const size2_t dims{inport_.front().getData()->getDimensions()};
    if (std::ranges::any_of(inport_, [dims](auto& p) {
            if (!p.hasData()) {
                return false;
            }
            return p.getData()->getDimensions() != dims;
        })) {
        throw Exception("Image dimensions of all inports need to be identical", IVW_CONTEXT);
    }

    auto&& [type, precision] = [&]() {
        std::vector<const DataFormatBase*> formats;
        for (auto& port : inport_) {
            if (port.hasData()) {
                formats.push_back(port.getData()->getDataFormat());
            }
        }

        return std::make_pair(util::commonNumericType(formats),
                              util::commonFormatPrecision(formats));
    }();

    auto image = std::make_shared<Image>(*inport_.front().getData(), noData,
                                         DataFormatBase::get(type, 4, precision));
    outport_.setData(image);

    utilgl::activateAndClearTarget(outport_, ImageType::ColorDepth);
    shader_.activate();
    TextureUnitContainer units;
    for (auto& port : inport_) {
        if (port.hasData()) {
            utilgl::bindAndSetUniforms(shader_, units, port, ImageType::ColorOnly);
        }
    }
    for (auto& prop : channel_) {
        utilgl::setUniforms(shader_, prop);
    }
    utilgl::setUniforms(shader_, outport_, alpha_);
    shader_.setUniform("use_alpha_texture", inport_.back().hasData());
    utilgl::singleDrawImagePlaneRect();
    shader_.deactivate();
    utilgl::deactivateCurrentTarget();
}

}  // namespace inviwo
