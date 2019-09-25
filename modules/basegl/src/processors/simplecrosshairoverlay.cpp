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
    "org.inviwo.SimpleCrosshairOverlay",  // Class identifier
    "Simple Crosshair Overlay",           // Display name
    "Overlay",                            // Category
    CodeState::Experimental,              // Code state
    Tags::GL,                             // Tags
};
const ProcessorInfo SimpleCrosshairOverlay::getProcessorInfo() const { return processorInfo_; }

SimpleCrosshairOverlay::SimpleCrosshairOverlay()
    : Processor()
    , imageIn_("imageIn")
    , imageOut_("imageOut")
    , cursorAngle_("cursorAngle", "Cursor Angle", 0.0f, 0.0f, glm::two_pi<float>())
    , cursorPos_("cursorPos", "Cursor Position", vec2(0.5f), vec2(0.0f), vec2(1.0f))
    , cursorRadius_("cursorRadius", "Cursor Radius", 0.1f, 0.0f, 1.0f)
    , showCenterIndicator_("showCenterIndicator", "Show Cursor Center", true)
    , mouseEvent_("mouseEvent", "Mouse Event", [this](Event* e) { updateMouse(e); },
                  MouseButton::Left, MouseState::Press | MouseState::Move | MouseState::Release)
    , interactionState_(InteractionState::NONE)
    , lastMousePos_("lastMousePos", "Last Mouse Position", vec2(0.0f), vec2(-10.0f), vec2(10.0f))
    , color1_("color1", "Horizontal Axis Color", vec4(1), vec4(0), vec4(1))
    , color2_("color2", "Vertical Axis Color", vec4(1), vec4(0), vec4(1))
    , color3_("color3", "Border Color", vec4(1), vec4(0), vec4(1))
    , color4_("color4", "Center Indicator Color", vec4(1), vec4(0), vec4(1))
    , color5_("color5", "Slab Color", vec4(1.f, 1.f, 1.f, .5f), vec4(0), vec4(1))
    , thickness1_("thickness1", "Crosshair Thickness", 2u, 0u, 20u)
    , thickness2_("thickness2", "Border Thickness", 2u, 0u, 20u)
    , slab0Offset0_("slab0Offset0", "Slab H Offset 0", -0.01f, -1.0f, 1.0f, 0.001f)
    , slab0Offset1_("slab0Offset1", "Slab H Offset 1", 0.01f, -1.0f, 1.0f, 0.001f)
    , slab1Offset0_("slab1Offset0", "Slab V Offset 0", -0.01f, -1.0f, 1.0f, 0.001f)
    , slab1Offset1_("slab1Offset1", "Slab V Offset 1", 0.01f, -1.0f, 1.0f, 0.001f)
    , showSlabs_("showSlabs", "Show Slabs", true)
    , crosshairMesh_(nullptr)
    , slabMesh_(nullptr)
    , outlineMesh_(nullptr)
    , cursorCenterMesh_(nullptr)
    , shader_("standard.vert", "standard.frag") {

    imageIn_.setOptional(true);
    addPort(imageIn_);
    addPort(imageOut_);

    addProperty(cursorAngle_);
    addProperty(cursorPos_);
    addProperty(cursorRadius_);
    addProperty(showCenterIndicator_);

    mouseEvent_.setVisible(false);
    addProperty(mouseEvent_);
    addProperty(lastMousePos_);

    color1_.setSemantics(PropertySemantics::Color);
    addProperty(color1_);
    color2_.setSemantics(PropertySemantics::Color);
    addProperty(color2_);
    color3_.setSemantics(PropertySemantics::Color);
    addProperty(color3_);
    color4_.setSemantics(PropertySemantics::Color);
    addProperty(color4_);
    color5_.setSemantics(PropertySemantics::Color);
    addProperty(color5_);

    addProperty(thickness1_);
    addProperty(thickness2_);

    addProperties(slab0Offset0_, slab0Offset1_, slab1Offset0_, slab1Offset1_, showSlabs_);
}

void SimpleCrosshairOverlay::process() {

    const auto& pxthicknessCrosshair = thickness1_;
    const auto& pxthicknessOutline = thickness2_;
    const auto canvas_dimensions = vec2(imageOut_.getDimensions());
    const auto aspect_ratio = canvas_dimensions.x / canvas_dimensions.y;

    const float bar_length(100.0f);
    const auto thicknessCrosshair =
        1.0f / canvas_dimensions.x * static_cast<float>(pxthicknessCrosshair);
    const auto thicknessOutline = 2.0f / canvas_dimensions * static_cast<float>(pxthicknessOutline);
    const auto pos = cursorPos_.get() * 2.0f - 1.0f;

    crosshairMesh_ = std::make_shared<Mesh>(DrawType::Triangles, ConnectivityType::None);
    slabMesh_ = std::make_shared<Mesh>(DrawType::Triangles, ConnectivityType::None);
    outlineMesh_ = std::make_shared<Mesh>(DrawType::Triangles, ConnectivityType::None);
    cursorCenterMesh_ = std::make_shared<Mesh>(DrawType::Lines, ConnectivityType::Loop);

    // Create crosshair in NDC with double screen size bars so that endings are never visible
    crosshairMesh_->addBuffer(
        BufferType::PositionAttrib,
        util::makeBuffer<vec2>({
            // horizontal
            vec2(bar_length, pos.y + thicknessCrosshair),
            vec2(-bar_length, pos.y + thicknessCrosshair),
            vec2(-bar_length, pos.y - thicknessCrosshair),  // upper triangle (CCW)
            vec2(bar_length, pos.y - thicknessCrosshair),
            vec2(bar_length, pos.y + thicknessCrosshair),
            vec2(-bar_length, pos.y - thicknessCrosshair),  // lower triangle
            // vertical
            vec2(pos.x + thicknessCrosshair, bar_length),
            vec2(pos.x - thicknessCrosshair, bar_length),
            vec2(pos.x - thicknessCrosshair, -bar_length),  // left triangle
            vec2(pos.x + thicknessCrosshair, -bar_length),
            vec2(pos.x + thicknessCrosshair, bar_length),
            vec2(pos.x - thicknessCrosshair, -bar_length)  // right triangle
        }));
    crosshairMesh_->addBuffer(BufferType::ColorAttrib,
                              util::makeBuffer<vec4>(std::vector<vec4>{
                                  color1_, color1_, color1_, color1_, color1_, color1_, color2_,
                                  color2_, color2_, color2_, color2_, color2_}));

    // Create background for crosshair to show slab thickness (useful in MPR)
    slabMesh_->addBuffer(
        BufferType::PositionAttrib,
        util::makeBuffer<vec2>({
            // background horizontal
            vec2(bar_length, pos.y + slab0Offset0_), vec2(-bar_length, pos.y + slab0Offset0_),
            vec2(-bar_length, pos.y + slab0Offset1_),  // upper triangle
            vec2(bar_length, pos.y + slab0Offset1_), vec2(bar_length, pos.y + slab0Offset0_),
            vec2(-bar_length, pos.y + slab0Offset1_),  // lower triangle
            // background vertical
            vec2(pos.x + slab1Offset1_, bar_length), vec2(pos.x + slab1Offset0_, bar_length),
            vec2(pos.x + slab1Offset0_, -bar_length),  // left triangle
            vec2(pos.x + slab1Offset1_, -bar_length), vec2(pos.x + slab1Offset1_, bar_length),
            vec2(pos.x + slab1Offset0_, -bar_length)  // right triangle
        }));
    slabMesh_->addBuffer(BufferType::ColorAttrib,
                         util::makeBuffer<vec4>(std::vector<vec4>{
                             color5_, color5_, color5_, color5_, color5_, color5_, color5_, color5_,
                             color5_, color5_, color5_, color5_}));

    // Create viewport outline as useful indicator when having multiple views, e.g. in MPR
    outlineMesh_->addBuffer(
        BufferType::PositionAttrib,
        util::makeBuffer<vec2>({
            // top
            vec2(1.f, 1.f), vec2(-1.f, 1.f),
            vec2(-1.f, 1.f - thicknessOutline.y),  // upper triangle (CCW)
            vec2(-1.f, 1.f - thicknessOutline.y), vec2(1.f, 1.f - thicknessOutline.y),
            vec2(1.f, 1.f),  // lower triangle
            // bottom
            vec2(1.f, -1.f), vec2(1.f, -1.f + thicknessOutline.y),
            vec2(-1.f, -1.f + thicknessOutline.y),  // upper triangle
            vec2(-1.f, -1.f), vec2(1.f, -1.f),
            vec2(-1.f, -1.f + thicknessOutline.y),  // lower triangle
            // left
            vec2(-1.f, 1.f), vec2(-1.f + thicknessOutline.x, -1.f),
            vec2(-1.f, -1.f),  // left triangle
            vec2(-1.f + thicknessOutline.x, 1.f), vec2(-1.f + thicknessOutline.x, -1.f),
            vec2(-1.f, 1.f),  // right triangle
            // right
            vec2(1.f - thicknessOutline.x, 1.f), vec2(1.f, -1.f),
            vec2(1.f - thicknessOutline.x, -1.f),                                 // left triangle
            vec2(1.f, 1.f), vec2(1.f, -1.f), vec2(1.f - thicknessOutline.x, 1.f)  // right triangle
        }));
    outlineMesh_->addBuffer(BufferType::ColorAttrib,
                            util::makeBuffer<vec4>(std::vector<vec4>(24, color3_)));

    // Create circle at center of cursor
    const size_t num_pts(32);
    auto vertex_buffer_ram = std::make_shared<Vec2BufferRAM>(num_pts);
    auto vertex_buffer = std::make_shared<Buffer<vec2>>(vertex_buffer_ram);
    for (size_t idx = 0; idx < num_pts; ++idx) {
        const auto psi(static_cast<float>(idx) / static_cast<float>(num_pts) *
                       glm::two_pi<float>());
        const vec2 pt(cursorRadius_ * glm::cos(psi), cursorRadius_ * glm::sin(psi));
        vertex_buffer_ram->set(idx, pt + pos);
    }
    cursorCenterMesh_->addBuffer(BufferType::PositionAttrib, vertex_buffer);
    cursorCenterMesh_->addBuffer(BufferType::ColorAttrib,
                                 util::makeBuffer<vec4>(std::vector<vec4>(num_pts, color4_.get())));

    // Render mesh over input image and copy to output port

    utilgl::activateTargetAndCopySource(imageOut_, imageIn_, ImageType::ColorDepth);
    utilgl::DepthFuncState depth(GL_ALWAYS);
    utilgl::BlendModeState blendModeStateGL(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    shader_.activate();

    const mat4 scale(mat2(1, 0, 0, aspect_ratio));
    const auto cosA = cos(cursorAngle_), sinA = sin(cursorAngle_);
    const mat4 rot(mat2(cosA, sinA, -sinA, cosA));
    mat4 transl1(1);
    transl1[3] = vec4(-pos, 0, 1);
    mat4 transl2(1);
    transl2[3] = vec4(pos, 0, 1);

    shader_.setUniform("dataToClip", transl2 * scale * rot * transl1);
    if (showSlabs_.get()) {
        MeshDrawerGL(slabMesh_.get()).draw();
    }
    MeshDrawerGL(crosshairMesh_.get()).draw();
    if (showCenterIndicator_.get()) {
        MeshDrawerGL(cursorCenterMesh_.get()).draw();
    }
    shader_.setUniform("dataToClip", mat4(1));
    MeshDrawerGL(outlineMesh_.get()).draw();

    shader_.deactivate();
}

void SimpleCrosshairOverlay::updateMouse(Event* e) {
    const auto mouseEvent = static_cast<MouseEvent*>(e);
    const auto mouseState = mouseEvent->state();
    const auto newMousePos = vec2(mouseEvent->posNormalized());

    if (mouseState == MouseState::Press) {  // ### update last mouse position at mouse down ###
        lastMousePos_ = newMousePos;

        if (glm::distance(newMousePos, cursorPos_.get()) <
            cursorRadius_) {  // ### determine if cursor rotation or movement
            interactionState_ = InteractionState::MOVE;
        } else {
            interactionState_ = InteractionState::ROTATE;
        }
    } else if (mouseState == MouseState::Move) {
        if (interactionState_ == InteractionState::ROTATE) {  // ### update angle at mouse move ###
            // angle between cursor center to old and new mouse pos
            const auto dirOld = glm::normalize(lastMousePos_.get() - cursorPos_.get());
            const auto dirNew = glm::normalize(newMousePos - cursorPos_.get());
            const auto angleDiff = glm::acos(glm::dot(dirOld, dirNew));

            // determine direction of rotation
            const float angleSign =
                (glm::cross(vec3(dirOld, 0.0f), vec3(dirNew, 0.0f)).z > 0) ? 1.0f : -1.0f;

            // update angle in temporary variable
            auto tmp = cursorAngle_.get();
            tmp += angleSign * angleDiff;

            // fix angle between 0 and 2pi
            while (tmp > glm::two_pi<float>()) {
                tmp -= glm::two_pi<float>();
            }
            while (tmp < 0.0f) {
                tmp += glm::two_pi<float>();
            }

            // set new angle [0..2pi]
            cursorAngle_ = tmp;

            // update regular mouse pos while rotating
            lastMousePos_ = newMousePos;
        } else if (interactionState_ ==
                   InteractionState::MOVE) {  // ### update position at mouse move ###
            const auto newMousePosClamped = glm::clamp(newMousePos, vec2(0.0f), vec2(1.0f));
            const auto diff = newMousePosClamped - lastMousePos_.get();
            const auto newCursorPos = glm::clamp(cursorPos_.get() + diff, vec2(0.0f), vec2(1.0f));

            // set new position vec2[0..1]
            cursorPos_ = newCursorPos;

            // update clamped new mouse pos while dragging
            lastMousePos_ = newMousePosClamped;
        }
    } else if (mouseState == MouseState::Release) {
        interactionState_ = InteractionState::NONE;
        lastMousePos_ = newMousePos;
    }
}

}  // namespace inviwo
