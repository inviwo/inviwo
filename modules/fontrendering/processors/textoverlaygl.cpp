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

ProcessorClassIdentifier(TextOverlayGL, "org.inviwo.TextOverlayGL");
ProcessorDisplayName(TextOverlayGL, "Text Overlay");
ProcessorTags(TextOverlayGL, Tags::GL);
ProcessorCategory(TextOverlayGL, "Drawing");
ProcessorCodeState(TextOverlayGL, CODE_STATE_STABLE);

TextOverlayGL::TextOverlayGL()
    : Processor()
    , inport_("inport")
    , outport_("outport")
    , text_("Text", "Text", "Lorem ipsum etc.", INVALID_OUTPUT,
            PropertySemantics::TextEditor)
    , color_("color_", "Color", vec4(1.0f), vec4(0.0f), vec4(1.0f), vec4(0.01f),
                  INVALID_OUTPUT, PropertySemantics::Color)
    , fontSize_("Font size", "Font size")
    , fontPos_("Position", "Position", vec2(0.0f), vec2(0.0f), vec2(1.0f), vec2(0.01f))
    , anchorPos_("Anchor", "Anchor", vec2(-1.0f), vec2(-1.0f), vec2(1.0f), vec2(0.01f))
    , textRenderer_() {

    addPort(inport_);
    addPort(outport_);
    addProperty(text_);
    addProperty(color_);
    addProperty(fontPos_);
    addProperty(anchorPos_);
    addProperty(fontSize_);
    fontSize_.addOption("10", "10", 10);
    fontSize_.addOption("12", "12", 12);
    fontSize_.addOption("18", "18", 18);
    fontSize_.addOption("24", "24", 24);
    fontSize_.addOption("36", "36", 36);
    fontSize_.addOption("48", "48", 48);
    fontSize_.addOption("60", "60", 60);
    fontSize_.addOption("72", "72", 72);
    fontSize_.setSelectedIndex(3);
    fontSize_.setCurrentStateAsDefault();
}


void TextOverlayGL::process() {
    utilgl::activateTargetAndCopySource(outport_, inport_, COLOR_DEPTH);

    glDepthFunc(GL_ALWAYS);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    vec2 scale(2.f / vec2(outport_.getData()->getDimensions()));

    int fontSize = fontSize_.getSelectedValue();
    textRenderer_.setFontSize(fontSize);
    float xpos_ = fontPos_.get().x * outport_.getData()->getDimensions().x;
    float ypos_ = fontPos_.get().y * outport_.getData()->getDimensions().y + float(fontSize);

    vec2 size = textRenderer_.computeTextSize(text_.get().c_str(), scale);
    vec2 shift = 0.5f * size * (anchorPos_.get() + vec2(1.0f, 1.0f));
    textRenderer_.render(text_.get().c_str(), -1 + xpos_ * scale.x - shift.x,
                          1 - ypos_ * scale.y + shift.y, scale, color_.get());

    glDisable(GL_BLEND);
    glDepthFunc(GL_LESS);
    utilgl::deactivateCurrentTarget();
}

}  // namespace
