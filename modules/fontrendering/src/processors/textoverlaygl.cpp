/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2019 Inviwo Foundation
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

#include <modules/fontrendering/processors/textoverlaygl.h>
#include <modules/fontrendering/util/fontutils.h>

#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/datastructures/image/image.h>
#include <modules/opengl/inviwoopengl.h>
#include <modules/opengl/texture/textureutils.h>
#include <modules/opengl/openglutils.h>
#include <modules/opengl/shader/shaderutils.h>
#include <modules/opengl/image/imagegl.h>
#include <inviwo/core/util/assertion.h>
#include <inviwo/core/util/rendercontext.h>

#include <cctype>
#include <locale>

namespace inviwo {

const ProcessorInfo TextOverlayGL::processorInfo_{
    "org.inviwo.TextOverlayGL",  // Class identifier
    "Text Overlay",              // Display name
    "Drawing",                   // Category
    CodeState::Stable,           // Code state
    "GL, Font, Text",            // Tags
};
const ProcessorInfo TextOverlayGL::getProcessorInfo() const { return processorInfo_; }

TextOverlayGL::TextOverlayGL()
    : Processor()
    , inport_("inport")
    , outport_("outport")
    , enable_("enable", "Enabled", true)
    , text_("text", "Text", "Lorem ipsum etc.", InvalidationLevel::InvalidOutput,
            PropertySemantics::TextEditor)
    , color_("color", "Color", vec4(1.0f), vec4(0.0f), vec4(1.0f), vec4(0.01f),
             InvalidationLevel::InvalidOutput, PropertySemantics::Color)
    , font_("font", "Font Settings")
    , position_("position", "Position", vec2(0.0f), vec2(0.0f), vec2(1.0f), vec2(0.01f))
    , offset_("offset", "Offset (Pixel)", ivec2(0), ivec2(-100), ivec2(100))
    , addArgButton_("addArgBtn", "Add String Argument")
    , textRenderer_{[]() {
        // ensure the default context is active when creating the TextRenderer
        RenderContext::getPtr()->activateDefaultRenderContext();
        return TextRenderer{};
    }()}
    , numArgs_(0u) {
    inport_.setOptional(true);

    addPort(inport_);
    addPort(outport_);
    addProperty(enable_);
    addProperty(text_);
    addProperty(color_);
    addProperty(font_);
    addProperty(position_);
    addProperty(offset_);
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

    font_.fontFace_.setSelectedIdentifier("arial");
    font_.fontFace_.setCurrentStateAsDefault();

    font_.fontSize_.set(14);
    font_.fontSize_.setCurrentStateAsDefault();
}

void TextOverlayGL::process() {
    if (!enable_.get()) {
        outport_.setData(inport_.getData());
        return;
    }

    if (font_.fontFace_.isModified()) {
        textRenderer_.setFont(font_.fontFace_.get());
    }

    auto argModified = [this]() {
        auto args = this->getPropertiesByType<StringProperty>(false);
        return util::any_of(args, [this](const auto& p) { return p != &text_ && p->isModified(); });
    };

    // check whether a property was modified
    if (!textObject_.texture || text_.isModified() || color_.isModified() || font_.isModified() ||
        argModified()) {
        updateCache();
    }

    // draw cached overlay on top of the input image
    if (inport_.isReady()) {
        utilgl::activateTargetAndCopySource(outport_, inport_, ImageType::ColorDepth);
    } else {
        utilgl::activateAndClearTarget(outport_, ImageType::ColorDepth);
    }

    utilgl::DepthFuncState depthFunc(GL_ALWAYS);
    utilgl::BlendModeState blending(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    // use integer position for best results
    vec2 size(textObject_.bbox.textExtent);
    vec2 shift = 0.5f * size * (font_.anchorPos_.get() + vec2(1.0f, 1.0f));

    ivec2 pos(position_.get() * vec2(outport_.getDimensions()));
    pos += offset_.get() - ivec2(shift);

    // render texture containing the text onto the current canvas
    textureRenderer_.render(textObject_.texture, pos + textObject_.bbox.glyphsOrigin,
                            outport_.getDimensions());

    utilgl::deactivateCurrentTarget();
}

void TextOverlayGL::deserialize(Deserializer& d) {
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
        } else {
            ++offset;
        }
    }

    if (printWarning) {
        LogWarn("Input text contains more than the allowed " << maxNumArgs_ << " place markers.");
    }
    return str;
}

void TextOverlayGL::updateCache() {
    textRenderer_.setFontSize(font_.fontSize_.get());
    textRenderer_.setLineSpacing(font_.lineSpacing_.get());
    std::string str(getString());
    textObject_ =
        util::createTextTextureObject(textRenderer_, str, color_.get(), textObject_.texture);
}

}  // namespace inviwo
