/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2018 Inviwo Foundation
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

#include <modules/basegl/processors/simplecrosshairoverlay.h>

#include <inviwo/core/interaction/events/mouseevent.h>

#include <modules/opengl/openglutils.h>
#include <modules/opengl/texture/textureutils.h>
#include <modules/opengl/rendering/meshdrawergl.h>

namespace inviwo {

const ProcessorInfo SimpleCrosshairOverlay::processorInfo_{
    "org.inviwo.SimpleCrosshairOverlay", // Class identifier
    "Simple Crosshair Overlay",          // Display name
    "Overlay",                           // Category
    CodeState::Experimental,             // Code state
    Tags::GL,                            // Tags
};
const ProcessorInfo SimpleCrosshairOverlay::getProcessorInfo() const { return processorInfo_; }

SimpleCrosshairOverlay::SimpleCrosshairOverlay()
    : Processor()
    , imageIn_("imageIn")
    , imageOut_("imageOut")
    , angleDiff_(0)
    , cursorAngle_("cursorAngle", "Cursor Angle", 0.0f, 0.0f, glm::two_pi<float>())
    , cursorPos_("cursorPos", "Cursor Position", vec2(0.5f), vec2(0.0f), vec2(1.0f))
    , cursorRadius_("cursorRadius", "Cursor Radius", 0.1f, 0.0f, 1.0f)
    , mouseEvent_("mouseEvent", "Mouse Event", [this](Event* e) { updateMouse(e); },
                  MouseButton::Left, MouseState::Press | MouseState::Move | MouseState::Release)
    , interactionState_ (InteractionState::NONE)
    , color1_("color1", "Color 1", vec4(1), vec4(0), vec4(1))
    , color2_("color2", "Color 2", vec4(1), vec4(0), vec4(1))
    , color3_("color3", "Color 3", vec4(1), vec4(0), vec4(1))
    , thickness1_("thickness1", "Thickness 1", 2u, 0u, 20u)
    , thickness2_("thickness2", "Thickness 2", 2u, 0u, 20u)
    , crosshairMesh_(nullptr)
    , outlineMesh_(nullptr)
    , cursorCenterMesh_(nullptr)
    , shader_("standard.vert", "standard.frag") {

    imageIn_.setOptional(true);
    addPort(imageIn_);
    addPort(imageOut_);

    addProperty(cursorAngle_);
    addProperty(cursorPos_);
    addProperty(cursorRadius_);

	mouseEvent_.setVisible(false);
    addProperty(mouseEvent_);

    color1_.setSemantics(PropertySemantics::Color);
    addProperty(color1_);
    color2_.setSemantics(PropertySemantics::Color);
    addProperty(color2_);
    color3_.setSemantics(PropertySemantics::Color);
    addProperty(color3_);

    addProperty(thickness1_);
    addProperty(thickness2_);
}

void SimpleCrosshairOverlay::process() {

    const auto pxthickness = thickness1_;
    const auto pxthicknessOutline = thickness2_;
    const auto pxdims = imageOut_.getDimensions();

    const auto thickness = 1.f / vec2(pxdims) * static_cast<float>(pxthickness);
    const auto thicknessOutline = 2.f / vec2(pxdims) * static_cast<float>(pxthicknessOutline);
    const auto pos = cursorPos_.get() * 2.0f - 1.0f;

    crosshairMesh_ = std::make_shared<Mesh>(DrawType::Triangles, ConnectivityType::None);
    outlineMesh_ = std::make_shared<Mesh>(DrawType::Triangles, ConnectivityType::None);
    cursorCenterMesh_ = std::make_shared<Mesh>(DrawType::Lines, ConnectivityType::Loop);

    // Create crosshair in NDC with double screen size bars so that endings are never visible
    crosshairMesh_->addBuffer(BufferType::PositionAttrib, util::makeBuffer<vec2>({
        // horizontal
        vec2(4.f, pos.y + thickness.y), vec2(-4.f, pos.y + thickness.y), vec2(-4.f, pos.y - thickness.y), // upper triangle (CCW)
        vec2(4.f, pos.y - thickness.y), vec2(4.f, pos.y + thickness.y), vec2(-4.f, pos.y - thickness.y), // lower triangle
        // vertical
        vec2(pos.x + thickness.x, 4.f), vec2(pos.x - thickness.x, 4.f), vec2(pos.x - thickness.x, -4.f), // left triangle
        vec2(pos.x + thickness.x, -4.f), vec2(pos.x + thickness.x, 4.f), vec2(pos.x - thickness.x, -4.f) // right triangle
        }));
    crosshairMesh_->addBuffer(BufferType::ColorAttrib, util::makeBuffer<vec4>(std::vector<vec4>{
        color1_, color1_, color1_, color1_, color1_, color1_,
            color2_, color2_, color2_, color2_, color2_, color2_
    }));

    // Create viewport outline as useful indicator when having multiple views, e.g. in MPR
    outlineMesh_->addBuffer(BufferType::PositionAttrib, util::makeBuffer<vec2>({
        // top
        vec2(1.f, 1.f), vec2(-1.f, 1.f), vec2(-1.f, 1.f - thicknessOutline.y), // upper triangle (CCW)
        vec2(-1.f, 1.f - thicknessOutline.y), vec2(1.f, 1.f - thicknessOutline.y), vec2(1.f, 1.f), // lower triangle
        // bottom
        vec2(1.f, -1.f), vec2(1.f, -1.f + thicknessOutline.y), vec2(-1.f, -1.f + thicknessOutline.y), // upper triangle
        vec2(-1.f, -1.f), vec2(1.f, -1.f), vec2(-1.f, -1.f + thicknessOutline.y), // lower triangle
        // left
        vec2(-1.f, 1.f), vec2(-1.f + thicknessOutline.x, -1.f), vec2(-1.f, -1.f), // left triangle
        vec2(-1.f + thicknessOutline.x, 1.f), vec2(-1.f + thicknessOutline.x, -1.f), vec2(-1.f, 1.f), // right triangle
        // right
        vec2(1.f - thicknessOutline.x, 1.f), vec2(1.f, -1.f), vec2(1.f - thicknessOutline.x, -1.f), // left triangle
        vec2(1.f, 1.f), vec2(1.f, -1.f), vec2(1.f - thicknessOutline.x, 1.f) // right triangle
        }));
    outlineMesh_->addBuffer(BufferType::ColorAttrib, util::makeBuffer<vec4>(std::vector<vec4>(24, color3_)));

    // Create circle at center of cursor
    const auto canvas_size = vec2(imageOut_.getDimensions());
    const auto aspect_ratio = canvas_size.x / canvas_size.y;
    const size_t num_pts(32);
    auto vertex_buffer_ram = std::make_shared<Vec2BufferRAM>(num_pts);
    auto vertex_buffer = std::make_shared<Buffer<vec2>>(vertex_buffer_ram);
    for (size_t idx = 0; idx < num_pts; ++idx) {
        const auto psi(static_cast<float>(idx) / static_cast<float>(num_pts) * glm::two_pi<float>());
        const vec2 pt(
            cursorRadius_ * glm::cos(psi) * aspect_ratio,
            cursorRadius_ * glm::sin(psi)
        );
        vertex_buffer_ram->set(idx, pt + pos);
    }
    cursorCenterMesh_->addBuffer(BufferType::PositionAttrib, vertex_buffer);
    cursorCenterMesh_->addBuffer(BufferType::ColorAttrib, util::makeBuffer<vec4>(std::vector<vec4>(num_pts, vec4(1.0f)))); // ToDo: change white to property

    // Render mesh over input image and copy to output port
    utilgl::activateTargetAndCopySource(imageOut_, imageIn_, ImageType::ColorDepth);
    shader_.activate();
    float cosA = cos(cursorAngle_), sinA = sin(cursorAngle_);
    mat4 rot(mat2(cosA, -sinA, sinA, cosA));
    mat4 transl1(1); transl1[3] = vec4(-pos, 0, 1);
    mat4 transl2(1); transl2[3] = vec4(pos, 0, 1);
    shader_.setUniform("dataToClip", transl2 * rot * transl1);
    utilgl::DepthFuncState depth(GL_ALWAYS);
    MeshDrawerGL(crosshairMesh_.get()).draw();
    MeshDrawerGL(cursorCenterMesh_.get()).draw();
    shader_.setUniform("dataToClip", mat4(1));
    MeshDrawerGL(outlineMesh_.get()).draw();
    shader_.deactivate();
}

float SimpleCrosshairOverlay::getAngleFromMouse(vec2 mousePos) {

    // get angle in [0..2pi] relative to "12 o'clock"

    const vec2 watchHand = normalize(mousePos * 2.f - 1.f);

    if (watchHand.x > 0)
        return acos(watchHand.y);
    else
        return acos(-watchHand.y) + M_PI;
}

void SimpleCrosshairOverlay::updateMouse(Event* e) {
    const auto mouseEvent = static_cast<MouseEvent*>(e);
    const auto mouseState = mouseEvent->state();
    const auto newMousePos = vec2(mouseEvent->posNormalized());

    if (mouseState == MouseState::Press) { // ### update last mouse position at mouse down ###

        // Here we have actually a press but no move

        if (glm::distance(newMousePos, cursorPos_.get()) < cursorRadius_) { // ### determine if cursor rotation or movement
            interactionState_ = InteractionState::MOVE;
        } else {
            interactionState_ = InteractionState::ROTATE;

            // To prevent snapping angle to mouse, save the diff to the last rotation interaction here
            angleDiff_ = getAngleFromMouse(newMousePos) - cursorAngle_;
        }
    } else if (mouseState == MouseState::Move) {

        // Here we have a move and press at the same time, i.e. drag

        if (interactionState_ == InteractionState::ROTATE) { // ### update angle at mouse move ###

            // set new angle minus diff
            // be sure to stay in [0..2pi] after applying diff
            float a = getAngleFromMouse(newMousePos) - angleDiff_;
            if (a < 0) a = glm::two_pi<float>() + a;
            cursorAngle_ = fmod(a, glm::two_pi<float>());

        } else if (interactionState_ == InteractionState::MOVE) { // ### update position at mouse move ###

            // set new position vec2[0..1]
            cursorPos_ = clamp(newMousePos, vec2(0), vec2(1));

        }
    } else if (mouseState == MouseState::Release) {
        interactionState_ = InteractionState::NONE;
    }
}

}  // namespace inviwo
