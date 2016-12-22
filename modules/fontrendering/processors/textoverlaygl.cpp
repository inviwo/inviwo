/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2016 Inviwo Foundation
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

#include "textoverlaygl.h"
#include <modules/fontrendering/util/fontutils.h>

#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/datastructures/image/image.h>
#include <inviwo/core/util/filesystem.h>
#include <modules/opengl/inviwoopengl.h>
#include <modules/opengl/texture/textureutils.h>
#include <modules/opengl/openglutils.h>
#include <modules/opengl/shader/shaderutils.h>
#include <modules/opengl/image/imagegl.h>
#include <inviwo/core/util/assertion.h>

#include <cctype>
#include <locale>

namespace inviwo {

const ProcessorInfo TextOverlayGL::processorInfo_{
    "org.inviwo.TextOverlayGL",  // Class identifier
    "Text Overlay",              // Display name
    "Drawing",                   // Category
    CodeState::Stable,           // Code state
    Tags::GL,                    // Tags
};
const ProcessorInfo TextOverlayGL::getProcessorInfo() const {
    return processorInfo_;
}

TextOverlayGL::TextOverlayGL()
    : Processor()
    , inport_("inport")
    , outport_("outport")
    , enable_("enable","Enabled",true)
    , text_("Text", "Text", "Lorem ipsum etc.", InvalidationLevel::InvalidOutput,
            PropertySemantics::TextEditor)
    , color_("color_", "Color", vec4(1.0f), vec4(0.0f), vec4(1.0f), vec4(0.01f),
                  InvalidationLevel::InvalidOutput, PropertySemantics::Color)
    , fontFace_("fontFace", "Font Face")
    , fontSize_("fontSize", "Font size")
    , fontPos_("Position", "Position", vec2(0.0f), vec2(0.0f), vec2(1.0f), vec2(0.01f))
    , anchorPos_("Anchor", "Anchor", vec2(-1.0f), vec2(-1.0f), vec2(1.0f), vec2(0.01f))
    , addArgButton_("addArgBtn", "Add String Argument")
    , numArgs_(0u)
{
    addPort(inport_);
    addPort(outport_);
    addProperty(enable_);
    addProperty(text_);
    addProperty(color_);
    addProperty(fontFace_);
    addProperty(fontPos_);
    addProperty(anchorPos_);
    addProperty(fontSize_);
    addProperty(addArgButton_);

    addArgButton_.onChange([this]() {
        if (numArgs_ >= maxNumArgs_) {
            addArgButton_.setReadOnly(numArgs_ >= maxNumArgs_);
            return;
        }
        ++numArgs_;
        std::string num = std::to_string(numArgs_);
        auto property = new StringProperty(std::string("arg") + num, "Arg " + num);
        property->setSerializationMode(PropertySerializationMode::All);
        addProperty(property, true);
    });
    
    auto fonts = util::getAvailableFonts();

    for (auto font : fonts) {
        auto identifier = filesystem::getFileNameWithoutExtension(font.second);
        // use the file name w/o extension as identifier
        fontFace_.addOption(identifier, font.first, font.second);
    }
    fontFace_.setSelectedIdentifier("arial");
    fontFace_.setCurrentStateAsDefault();

    // set up different font sizes
    std::vector<int> fontSizes ={ 8, 10, 11, 12, 14, 16, 20, 24, 28, 36, 48, 60, 72, 96 };
    for (auto size : fontSizes) {
        std::string str = std::to_string(size);
        fontSize_.addOption(str, str, size);
    }
    fontSize_.setSelectedIndex(4);
    fontSize_.setCurrentStateAsDefault();
}

void TextOverlayGL::process() {
    if (!enable_.get()) {
        outport_.setData(inport_.getData());
        return;
    }
    
    if (fontFace_.isModified()) {
        textRenderer_.setFont(fontFace_.get());
    }

    // check whether a property was modified
    if (!cacheTexture_ ||
        util::any_of(getProperties(), [](const auto& p) { return p->isModified(); })) {
        updateCache();
    }

    // draw cached overlay on top of the input image
    utilgl::activateTargetAndCopySource(outport_, inport_, ImageType::ColorDepthPicking);
    utilgl::DepthFuncState depthFunc(GL_ALWAYS);
    utilgl::BlendModeState blending(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // use integer position for best results
    vec2 size(cacheTexture_->getDimensions());
    vec2 shift = 0.5f * size * (anchorPos_.get() + vec2(1.0f, 1.0f));

    ivec2 pos(fontPos_.get() * vec2(outport_.getDimensions()));
    pos -= ivec2(shift);
    // render texture containing the text onto the current canvas
    textureRenderer_.render(cacheTexture_, pos, outport_.getDimensions());

    utilgl::deactivateCurrentTarget();
}

void TextOverlayGL::deserialize(Deserializer & d) {
    Processor::deserialize(d);
    // update the number of place markers properties using the total number of string properties in
    // this processor. Note that this number is one element larger.
    auto args = this->getPropertiesByType<StringProperty>(false);
    numArgs_ = args.size() - 1;
    
    // only maxNumArgs_ are supported, disable button if more exist
    addArgButton_.setReadOnly(numArgs_ > maxNumArgs_);
}

std::string TextOverlayGL::getString() const {
    std::string str = text_.get();
    // replace all occurrences of place markers with the corresponding args
    auto args = this->getPropertiesByType<StringProperty>(false);
    // remove default text string property
    util::erase_remove_if(args, [this](const auto& p) { return p == &text_; });
    ivwAssert(numArgs_ == args.size(),
              "TextOverlayGL: number arguments not matching internal count");

    // parse string for all "%" and try to extract the number following the percent sign
    bool printWarning = false;

    std::string matchStr("%");
    for (std::size_t offset = str.find(matchStr, 0u); offset != std::string::npos;
         offset = str.find(matchStr, offset)) {
        // extract number substring,
        // read 3 characters to ensure that the number only has at most 2 digits
        std::string numStr = str.substr(offset + 1, 3);
        if (std::isdigit(numStr[0])) {
            std::size_t numDigits = 0;
            // extract number and reduce it by one since the %args start with 1
            // std::stoul will not throw an invalid argument exception since we made sure, it is a
            // number (std::isdigit above)
            std::size_t argNum = std::stoul(numStr, &numDigits) - 1;
            if (argNum <= numArgs_) {
                // make textual replacement ("%" and number of digits)
                str.replace(offset, numDigits + 1, args[argNum]->get());
                offset += args[argNum]->get().size();
            } else {
                if (numDigits > 2) printWarning = true;
                offset += 1 + numDigits;
            }
        }
    }

    if (printWarning) {
        LogWarn("Input text contains more than the allowed " << maxNumArgs_ << " place markers.");
    }
    return str;
}

void TextOverlayGL::updateCache() {
    textRenderer_.setFontSize(fontSize_.getSelectedValue());
    std::string str(getString());

    size2_t labelSize(textRenderer_.computeTextSize(str));
    if (!cacheTexture_ ||
        (cacheTexture_->getDimensions() != labelSize)) {
        auto texture = std::make_shared<Texture2D>(labelSize, GL_RGBA, GL_RGBA,
                                                   GL_UNSIGNED_BYTE, GL_LINEAR);
        texture->initialize(nullptr);
        cacheTexture_ = texture;
    }
    textRenderer_.renderToTexture(cacheTexture_, str, color_.get());
}

}  // namespace
