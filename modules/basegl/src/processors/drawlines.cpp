/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2026 Inviwo Foundation
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

#include <modules/basegl/processors/drawlines.h>

#include <inviwo/core/datastructures/buffer/buffer.h>
#include <inviwo/core/datastructures/buffer/bufferram.h>
#include <inviwo/core/datastructures/geometry/geometrytype.h>
#include <inviwo/core/datastructures/geometry/mesh.h>
#include <inviwo/core/datastructures/image/imagetypes.h>
#include <inviwo/core/datastructures/representationconverter.h>
#include <inviwo/core/datastructures/representationconverterfactory.h>
#include <inviwo/core/interaction/events/keyboardevent.h>
#include <inviwo/core/interaction/events/keyboardkeys.h>
#include <inviwo/core/interaction/events/mousebuttons.h>
#include <inviwo/core/interaction/events/mouseevent.h>
#include <inviwo/core/ports/imageport.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/processors/processorinfo.h>
#include <inviwo/core/processors/processorstate.h>
#include <inviwo/core/processors/processortags.h>
#include <inviwo/core/properties/buttonproperty.h>
#include <inviwo/core/properties/eventproperty.h>
#include <inviwo/core/properties/invalidationlevel.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/propertysemantics.h>
#include <inviwo/core/util/glmvec.h>
#include <modules/opengl/inviwoopengl.h>
#include <modules/opengl/openglutils.h>
#include <modules/opengl/rendering/meshdrawergl.h>
#include <modules/opengl/shader/shader.h>
#include <modules/opengl/texture/textureutils.h>

#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>

#include <flags/flags.h>
#include <glm/vec3.hpp>

namespace inviwo {
class Event;

const ProcessorInfo DrawLines::processorInfo_{
    "org.inviwo.DrawLines",  // Class identifier
    "Draw Lines",            // Display name
    "Drawing",               // Category
    CodeState::Stable,       // Code state
    Tags::GL,                // Tags
    R"(Interactive 2D line drawing.
    Hold Ctrl+D and click/move Left Mouse Button to Draw)"_unindentHelp,
};
const ProcessorInfo& DrawLines::getProcessorInfo() const { return processorInfo_; }

DrawLines::DrawLines()
    : Processor()
    , inport_("inport")
    , outport_("outport")
    , lineSize_("lineSize", "Line Size",
                util::ordinalScale(1.f, 10.f).set("Defines size of all lines."_help))
    , lineColor_("lineColor", "Line Color",
                 util::ordinalColor(vec4(1.f)).set("Defines color of all lines."_help))
    , clearButton_("clearButton", "Clear Lines", "Clear all lines."_help,
                   [this]() { clearLines(); })
    , mouseDraw_(
          "mouseDraw", "Draw Line", [this](Event* e) { eventDraw(e); }, MouseButton::Left,
          MouseStates(flags::any), KeyModifier::Control)
    , keyEnableDraw_(
          "keyEnableDraw", "Enable Draw", [this](Event* e) { eventEnableDraw(e); }, IvwKey::D,
          KeyStates(flags::any), KeyModifier::Control)

    , lines_(DrawType::Lines, ConnectivityType::Strip)
    , lineDrawer_(&lines_)
    , lineShader_("img_color.frag") {

    addPort(inport_);
    addPort(outport_);

    addProperties(lineSize_, lineColor_, clearButton_, mouseDraw_, keyEnableDraw_);

    lineShader_.onReload([this]() { invalidate(InvalidationLevel::InvalidResources); });

    lines_.addBuffer(BufferType::PositionAttrib, std::make_shared<Buffer<vec2>>());

    GLint aliasRange[2];
    glGetIntegerv(GL_ALIASED_LINE_WIDTH_RANGE, aliasRange);

    lineSize_.setMinValue(static_cast<float>(aliasRange[0]));
    lineSize_.setMaxValue(static_cast<float>(aliasRange[1]));

    if (aliasRange[0] == aliasRange[1]) lineSize_.setVisible(false);
}

DrawLines::~DrawLines() = default;

void DrawLines::process() {
    utilgl::activateTargetAndCopySource(outport_, inport_, ImageType::ColorOnly);
    {
        utilgl::GlBoolState linesmooth(GL_LINE_SMOOTH, false);

        lineShader_.activate();
        lineShader_.setUniform("color", lineColor_);
        lineDrawer_.draw();
        lineShader_.deactivate();
    }
    utilgl::deactivateCurrentTarget();
}

void DrawLines::addPoint(vec2 p) {
    auto buff =
        static_cast<Vec2BufferRAM*>(lines_.getBuffer(0)->getEditableRepresentation<BufferRAM>());
    buff->add(p);
}

void DrawLines::clearLines() {
    auto buff =
        static_cast<Vec2BufferRAM*>(lines_.getBuffer(0)->getEditableRepresentation<BufferRAM>());

    buff->clear();
}

void DrawLines::eventDraw(Event* event) {
    if (!drawModeEnabled_) return;

    auto mouseEvent = static_cast<MouseEvent*>(event);
    auto line = mouseEvent->ndc();

    addPoint(vec2(line.x, line.y));
    invalidate(InvalidationLevel::InvalidOutput);
}

void DrawLines::eventEnableDraw(Event* event) {
    auto keyEvent = static_cast<KeyboardEvent*>(event);
    drawModeEnabled_ = (keyEvent->state() != KeyState::Release);
}

}  // namespace inviwo
