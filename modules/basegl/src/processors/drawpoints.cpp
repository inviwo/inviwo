/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2019 Inviwo Foundation
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

#include <modules/basegl/processors/drawpoints.h>
#include <inviwo/core/datastructures/buffer/bufferramprecision.h>
#include <inviwo/core/interaction/events/keyboardevent.h>
#include <inviwo/core/interaction/events/mouseevent.h>
#include <modules/opengl/texture/textureutils.h>
#include <modules/opengl/openglutils.h>
#include <modules/opengl/shader/shaderutils.h>

namespace inviwo {

const ProcessorInfo DrawPoints::processorInfo_{
    "org.inviwo.DrawPoints",  // Class identifier
    "Draw Points",            // Display name
    "Drawing",                // Category
    CodeState::Stable,        // Code state
    Tags::GL,                 // Tags
};
const ProcessorInfo DrawPoints::getProcessorInfo() const { return processorInfo_; }

DrawPoints::DrawPoints()
    : Processor()
    , inport_("inputImage")
    , outport_("outputImage")
    , pointSize_("pointSize", "Point Size", 5, 1, 10)
    , pointColor_("pointColor", "Point Color", vec4(1.f))
    , clearButton_("clearButton", "Clear Drawing")
    , mouseDraw_("mouseDraw", "Draw Point", [this](Event* e) { eventDraw(e); }, MouseButton::Left,
                 MouseStates(flags::any), KeyModifier::Control)
    , keyEnableDraw_("keyEnableDraw", "Enable Draw", [this](Event* e) { eventEnableDraw(e); },
                     IvwKey::D, KeyStates(flags::any), KeyModifier::Control)
    , points_(DrawType::Points, ConnectivityType::None)
    , pointDrawer_(&points_)
    , pointShader_("img_color.frag")
    , drawModeEnabled_(false) {
    addPort(inport_);
    addPort(outport_);

    addProperty(pointSize_);
    pointColor_.setSemantics(PropertySemantics::Color);
    addProperty(pointColor_);
    clearButton_.onChange([this]() { clearPoints(); });
    addProperty(clearButton_);

    addProperty(mouseDraw_);
    addProperty(keyEnableDraw_);

    pointShader_.onReload([this]() { invalidate(InvalidationLevel::InvalidResources); });
    points_.addBuffer(BufferType::PositionAttrib, std::make_shared<Buffer<vec2>>());
}

DrawPoints::~DrawPoints() = default;

void DrawPoints::process() {
    utilgl::activateTargetAndCopySource(outport_, inport_, ImageType::ColorOnly);
    {
        utilgl::PointSizeState pointsize(static_cast<GLfloat>(pointSize_));
        pointShader_.activate();
        pointShader_.setUniform("color", pointColor_);
        pointDrawer_.draw();
        pointShader_.deactivate();
    }
    utilgl::deactivateCurrentTarget();
    compositor_.composite(inport_, outport_, ImageType::ColorOnly);
}

void DrawPoints::addPoint(vec2 p) {
    auto buff =
        static_cast<Vec2BufferRAM*>(points_.getBuffer(0)->getEditableRepresentation<BufferRAM>());
    buff->add(p);
}

void DrawPoints::clearPoints() {
    auto buff =
        static_cast<Vec2BufferRAM*>(points_.getBuffer(0)->getEditableRepresentation<BufferRAM>());

    buff->clear();
}

void DrawPoints::eventDraw(Event* event) {
    if (!drawModeEnabled_) return;

    auto mouseEvent = static_cast<MouseEvent*>(event);
    auto point = mouseEvent->ndc();

    addPoint(vec2(point.x, point.y));

    invalidate(InvalidationLevel::InvalidOutput);
}

void DrawPoints::eventEnableDraw(Event* event) {
    KeyboardEvent* keyEvent = static_cast<KeyboardEvent*>(event);
    drawModeEnabled_ = (keyEvent->state() != KeyState::Release);
}

}  // namespace inviwo
