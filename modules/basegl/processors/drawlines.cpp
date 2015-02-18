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

#include "drawlines.h"
#include <inviwo/core/datastructures/buffer/bufferramprecision.h>
#include <inviwo/core/interaction/events/keyboardevent.h>
#include <inviwo/core/interaction/events/mouseevent.h>
#include <modules/opengl/glwrap/shader.h>
#include <modules/opengl/textureutils.h>

namespace inviwo {

ProcessorClassIdentifier(DrawLines, "org.inviwo.DrawLines");
ProcessorDisplayName(DrawLines, "Draw Lines");
ProcessorTags(DrawLines, Tags::GL);
ProcessorCategory(DrawLines, "Drawing");
ProcessorCodeState(DrawLines, CODE_STATE_STABLE);

DrawLines::DrawLines()
    : CompositeProcessorGL()
    , inport_("inport")
    , outport_("outport")
    , lineSize_("lineSize", "Line Size", 1.f, 1.f, 10.f)
    , lineColor_("lineColor", "Line Color", vec4(1.f))
    , clearButton_("clearButton", "Clear Lines")
    , mouseDraw_("mouseDraw", "Draw Line",
    new MouseEvent(MouseEvent::MOUSE_BUTTON_LEFT, InteractionEvent::MODIFIER_CTRL,
    MouseEvent::MOUSE_STATE_ANY),
    new Action(this, &DrawLines::eventDraw))
    , keyEnableDraw_("keyEnableDraw", "Enable Draw", 
    new KeyboardEvent('D', InteractionEvent::MODIFIER_CTRL, KeyboardEvent::KEY_STATE_ANY),
    new Action(this, &DrawLines::eventEnableDraw))
    , lines_(NULL)
    , lineRenderer_(NULL) 
    , lineShader_(NULL)
{
    addPort(inport_);
    addPort(outport_);

    addProperty(lineSize_);
    lineColor_.setSemantics(PropertySemantics::Color);
    addProperty(lineColor_);
    clearButton_.onChange(this, &DrawLines::clearLines);
    addProperty(clearButton_);

    addProperty(mouseDraw_);
    addProperty(keyEnableDraw_);
}

DrawLines::~DrawLines() {}

void DrawLines::initialize() {
    CompositeProcessorGL::initialize();

    GLint aliasRange[2];
    glGetIntegerv(GL_ALIASED_LINE_WIDTH_RANGE, aliasRange);

    lineSize_.setMinValue(aliasRange[0]);
    lineSize_.setMaxValue(aliasRange[1]);

    if (aliasRange[0] == aliasRange[1])
        lineSize_.setVisible(false);

    lineShader_ = new Shader("img_color.frag");
    lines_ = new Mesh(GeometryEnums::LINES, GeometryEnums::STRIP);
    lines_->addAttribute(new Position2dBuffer());
    lineRenderer_ = new MeshRenderer(lines_);
}

void DrawLines::deinitialize() {
    CompositeProcessorGL::deinitialize();
    delete lineShader_;
    lineShader_ = NULL;
    delete lineRenderer_;
    lineRenderer_ = NULL;
    delete lines_;
    lines_ = NULL;
}

void DrawLines::process() {
    inport_.passOnDataToOutport(&outport_);

    utilgl::activateAndClearTarget(outport_, COLOR_ONLY);
    bool reEnableLineSmooth = false;
    if (glIsEnabled(GL_LINE_SMOOTH)) {
        glDisable(GL_LINE_SMOOTH);
        reEnableLineSmooth = true;
    }
    glLineWidth(static_cast<GLfloat>(lineSize_.get()));
    lineShader_->activate();
    lineShader_->setUniform("color_", lineColor_.get());
    lineRenderer_->render();
    lineShader_->deactivate();
    glLineWidth(1.f);
    if (reEnableLineSmooth)
        glEnable(GL_LINE_SMOOTH);
    utilgl::deactivateCurrentTarget();
    compositePortsToOutport(outport_, COLOR_ONLY, inport_);
}

void DrawLines::addPoint(vec2 p) {
    if (lines_)
        lines_->getAttributes(0)->getEditableRepresentation<Position2dBufferRAM>()->add(p);
}

void DrawLines::clearLines() {
    if (lines_)
        lines_->getAttributes(0)->getEditableRepresentation<Position2dBufferRAM>()->clear();
}

void DrawLines::eventDraw(Event* event){
    if (!drawModeEnabled_)
        return;

    MouseEvent* mouseEvent = static_cast<MouseEvent*>(event);
    vec2 line = mouseEvent->posNormalized();
    line *= 2.f;
    line -= 1.f;
    line.y = -line.y;
    addPoint(line);
    invalidate(INVALID_OUTPUT);
}

void DrawLines::eventEnableDraw(Event* event){
    KeyboardEvent* keyEvent = static_cast<KeyboardEvent*>(event);
    drawModeEnabled_ = (keyEvent->state() != KeyboardEvent::KEY_STATE_RELEASE);
}

}  // namespace
