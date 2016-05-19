/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2015 Inviwo Foundation
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
#include <inviwo/core/common/inviwoapplication.h>
#include <modules/opengl/inviwoopengl.h>
#include <modules/opengl/texture/textureutils.h>

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
    , fontSize_("Font size", "Font size")
    , fontPos_("Position", "Position", vec2(0.0f), vec2(0.0f), vec2(1.0f), vec2(0.01f))
    , anchorPos_("Anchor", "Anchor", vec2(-1.0f), vec2(-1.0f), vec2(1.0f), vec2(0.01f))
    , textRenderer_() {

    addPort(inport_);
    addPort(outport_);
    addProperty(enable_);
    addProperty(text_);
    addProperty(color_);
    addProperty(fontPos_);
    addProperty(anchorPos_);
    addProperty(fontSize_);

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

    utilgl::activateTargetAndCopySource(outport_, inport_, ImageType::ColorDepth);

    glDepthFunc(GL_ALWAYS);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    vec2 scale(2.f / vec2(outport_.getData()->getDimensions()));

    int fontSize = fontSize_.getSelectedValue();
    textRenderer_.setFontSize(fontSize);

    // use integer position for best results
    ivec2 pos(fontPos_.get() * vec2(outport_.getDimensions()));
    pos.y += fontSize;
    
    vec2 size = textRenderer_.computeTextSize(text_.get().c_str(), scale);
    vec2 shift = 0.5f * size * (anchorPos_.get() + vec2(1.0f, 1.0f));
    textRenderer_.render(text_.get().c_str(), -1 + pos.x * scale.x - shift.x,
                          1 - pos.y * scale.y + shift.y, scale, color_.get());

    glDisable(GL_BLEND);
    glDepthFunc(GL_LESS);
    utilgl::deactivateCurrentTarget();
}

}  // namespace

