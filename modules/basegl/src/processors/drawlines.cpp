/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2022 Inviwo Foundation
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

#include <inviwo/core/datastructures/buffer/buffer.h>                   // for Buffer, BufferBase
#include <inviwo/core/datastructures/buffer/bufferram.h>                // for Vec2BufferRAM
#include <inviwo/core/datastructures/geometry/geometrytype.h>           // for BufferType, Buffe...
#include <inviwo/core/datastructures/geometry/mesh.h>                   // for Mesh
#include <inviwo/core/datastructures/image/imagetypes.h>                // for ImageType, ImageT...
#include <inviwo/core/datastructures/representationconverter.h>         // for RepresentationCon...
#include <inviwo/core/datastructures/representationconverterfactory.h>  // for RepresentationCon...
#include <inviwo/core/interaction/events/keyboardevent.h>               // for KeyboardEvent
#include <inviwo/core/interaction/events/keyboardkeys.h>                // for KeyModifier, KeyM...
#include <inviwo/core/interaction/events/mousebuttons.h>                // for MouseStates, Mous...
#include <inviwo/core/interaction/events/mouseevent.h>                  // for MouseEvent
#include <inviwo/core/ports/imageport.h>                                // for ImageInport, Imag...
#include <inviwo/core/processors/processor.h>                           // for Processor
#include <inviwo/core/processors/processorinfo.h>                       // for ProcessorInfo
#include <inviwo/core/processors/processorstate.h>                      // for CodeState, CodeSt...
#include <inviwo/core/processors/processortags.h>                       // for Tags, Tags::GL
#include <inviwo/core/properties/buttonproperty.h>                      // for ButtonProperty
#include <inviwo/core/properties/eventproperty.h>                       // for EventProperty
#include <inviwo/core/properties/invalidationlevel.h>                   // for InvalidationLevel
#include <inviwo/core/properties/ordinalproperty.h>                     // for FloatProperty
#include <inviwo/core/properties/propertysemantics.h>                   // for PropertySemantics
#include <inviwo/core/util/glmvec.h>                                    // for vec2, vec4
#include <modules/opengl/inviwoopengl.h>                                // for GLint, glGetIntegerv
#include <modules/opengl/openglutils.h>                                 // for GlBoolState
#include <modules/opengl/rendering/meshdrawergl.h>                      // for MeshDrawerGL
#include <modules/opengl/shader/shader.h>                               // for Shader
#include <modules/opengl/texture/textureutils.h>                        // for activateTargetAnd...

#include <functional>                                                   // for __base
#include <memory>                                                       // for unique_ptr, make_...
#include <string>                                                       // for string
#include <string_view>                                                  // for string_view
#include <unordered_map>                                                // for unordered_map
#include <unordered_set>                                                // for unordered_set

#include <flags/flags.h>                                                // for any
#include <glm/vec3.hpp>                                                 // for vec, vec<>::(anon...

namespace inviwo {
class Event;

const ProcessorInfo DrawLines::processorInfo_{
    "org.inviwo.DrawLines",  // Class identifier
    "Draw Lines",            // Display name
    "Drawing",               // Category
    CodeState::Stable,       // Code state
    Tags::GL,                // Tags
};
const ProcessorInfo DrawLines::getProcessorInfo() const { return processorInfo_; }

DrawLines::DrawLines()
    : Processor()
    , inport_("inport")
    , outport_("outport")
    , lineSize_("lineSize", "Line Size", 1.f, 1.f, 10.f)
    , lineColor_("lineColor", "Line Color", vec4(1.f))
    , clearButton_("clearButton", "Clear Lines")
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

    addProperty(lineSize_);
    lineColor_.setSemantics(PropertySemantics::Color);
    addProperty(lineColor_);
    clearButton_.onChange([this]() { clearLines(); });
    addProperty(clearButton_);

    addProperty(mouseDraw_);
    addProperty(keyEnableDraw_);
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
