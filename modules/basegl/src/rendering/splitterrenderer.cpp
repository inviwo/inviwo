/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2021 Inviwo Foundation
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

#include <modules/basegl/rendering/splitterrenderer.h>

#include <modules/opengl/openglutils.h>
#include <modules/opengl/sharedopenglresources.h>
#include <modules/opengl/texture/textureutils.h>
#include <modules/opengl/texture/textureunit.h>
#include <modules/opengl/image/imagegl.h>
#include <modules/opengl/rendering/meshdrawergl.h>
#include <modules/opengl/shader/shaderutils.h>

#include <inviwo/core/interaction/events/pickingevent.h>
#include <inviwo/core/interaction/events/mouseevent.h>
#include <inviwo/core/interaction/events/touchevent.h>
#include <inviwo/core/datastructures/geometry/mesh.h>

namespace inviwo {

SplitterRenderer::SplitterRenderer(const SplitterSettings& settings, Processor* processor)
    : settings_(settings)
    , processor_(processor)
    , shader_("splitter.vert", "splitter.geom", "linerenderer.frag")
    , triShader_("splitter.vert", "splittertriangle.geom", "standard.frag")
    , pickingMapper_(processor, 1, [&](PickingEvent* e) { handlePickingEvent(e); }) {

    mesh_ = std::make_shared<Mesh>(DrawType::Points, ConnectivityType::None);
    mesh_->addBuffer(Mesh::BufferInfo(BufferType::PositionAttrib),
                     util::makeBuffer(std::vector<vec2>{vec2(0.0f)}));
}

SplitterRenderer::SplitterRenderer(const SplitterRenderer& rhs)
    : settings_(rhs.settings_)
    , processor_(rhs.processor_)
    , shader_("splitter.vert", "linerenderer.geom", "linerenderer.frag")
    , triShader_("standard.vert", "standard.frag")
    , pickingMapper_(processor_, 1, [&](PickingEvent* e) { handlePickingEvent(e); }) {

    mesh_ = std::make_shared<Mesh>(DrawType::Points, ConnectivityType::None);
    mesh_->addBuffer(Mesh::BufferInfo(BufferType::PositionAttrib),
                     util::makeBuffer(std::vector<vec2>{vec2(0.0f)}));
}

void SplitterRenderer::setDragAction(Callback callback) { dragAction_ = callback; }

void SplitterRenderer::render(splitter::Direction direction, float pos, size2_t canvasDims) {
    if (!settings_.get().enabled()) {
        return;
    }

    currentDirection_ = direction;
    const splitter::Style style = settings_.get().getStyle();

    float lineWidth = 0.0f;
    float handleWidth = 0.0f;
    bool drawHandle = true;
    bool drawBorder = false;

    const float width = settings_.get().getWidth();

    switch (style) {
        case splitter::Style::Handle:
            lineWidth = width / 3.0f;
            handleWidth = width;
            drawBorder = true;
            break;
        case splitter::Style::Divider:
            lineWidth = width;
            handleWidth = width * 0.6f;
            break;
        case splitter::Style::Line:
            lineWidth = width;
            drawHandle = false;
            break;
        case splitter::Style::Invisible:
            break;
        default:
            break;
    }

    mat4 m(1.0f);
    m[0][0] = 2.0f;
    m[1][1] = 2.0f;
    m[3] = vec4(pos * 2.0f - 1.0f, -1.0f, 0.0f, 1.0f);

    const vec2 canvasDimsf(canvasDims);
    const int index = (direction == splitter::Direction::Vertical) ? 1 : 0;
    const float len = settings_.get().getLength() / canvasDimsf[index];

    // scale matrix so that handle spans n pixel
    mat4 mHandle(m);
    mHandle[1][1] *= len;
    mHandle[3][1] = -len;

    mat4 mTri(m);
    mTri[3][1] = 0.0f;  // no vertical translation for triangle indicators

    if (direction == splitter::Direction::Horizontal) {
        // rotate by 90 degree for horizontal split
        mat4 rotMat = glm::rotate(-glm::half_pi<float>(), vec3(0.0f, 0.0f, 1.0f));
        m = rotMat * m;
        mHandle = rotMat * mHandle;
        mTri = rotMat * mTri;
    }

    utilgl::DepthFuncState depthFunc(GL_ALWAYS);
    utilgl::BlendModeState blending(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    MeshDrawerGL::DrawObject drawer(mesh_->getRepresentation<MeshGL>(),
                                    mesh_->getDefaultMeshInfo());

    shader_.activate();

    shader_.setUniform("screenDim", canvasDimsf);
    // draw picking buffer
    shader_.setUniform("pickingEnabled", true);
    shader_.setUniform("pickId", static_cast<uint32_t>(pickingMapper_.getPickingId(0)));
    shader_.setUniform("lineWidth", width);
    shader_.setUniform("trafo", m);
    drawer.draw();

    if (style != splitter::Style::Invisible) {
        // draw background line
        const vec4 lineColor = (style == splitter::Style::Line)
                                   ? settings_.get().getColor()
                                   : settings_.get().getBackgroundColor();

        shader_.setUniform("pickingEnabled", false);
        shader_.setUniform("color", lineColor);
        shader_.setUniform("lineWidth", lineWidth);
        shader_.setUniform("trafo", m);
        drawer.draw();

        if (drawHandle) {
            shader_.setUniform("trafo", mHandle);
            // draw handle border
            if (drawBorder) {
                shader_.setUniform("lineWidth", handleWidth + 4.0f);
                drawer.draw();
            }
            // draw handle
            shader_.setUniform("lineWidth", handleWidth);
            shader_.setUniform("color", settings_.get().getColor());
            drawer.draw();
        }
    }
    shader_.deactivate();

    if (hover_) {
        triShader_.activate();
        triShader_.setUniform("trafo", mTri);
        triShader_.setUniform("triangleSize", settings_.get().getTriangleSize());
        triShader_.setUniform("color", settings_.get().getHoverColor());
        triShader_.setUniform("screenDimInv", 1.0f / canvasDimsf);
        drawer.draw();
        triShader_.deactivate();
    }
}

void SplitterRenderer::handlePickingEvent(PickingEvent* e) {
    bool triggerUpdate = false;

    auto moveAction = [this](dvec2 screenPos) {
        if (!dragAction_) return;

        // screenPos in normalized screen coords [0,1]
        if (currentDirection_ == splitter::Direction::Vertical) {
            dragAction_(static_cast<float>(screenPos.x));
        } else {
            // invert vertical position
            screenPos.y = 1.0 - screenPos.y;
            dragAction_(static_cast<float>(screenPos.y));
        }
    };

    if (e->getEvent()->hash() == MouseEvent::chash()) {
        auto mouseEvent = e->getEventAs<MouseEvent>();

        const bool leftMouseBtn = (mouseEvent->button() == MouseButton::Left);
        const bool mousePress = (mouseEvent->state() == MouseState::Press);
        const bool mouseMove = (mouseEvent->state() == MouseState::Move);

        if (e->getState() == PickingState::Started) {
            // start hover
            hover_ = true;

            const auto cursor = (currentDirection_ == splitter::Direction::Vertical)
                                    ? MouseCursor::SplitH
                                    : MouseCursor::SplitV;
            mouseEvent->setMouseCursor(cursor);
            triggerUpdate = true;
        } else if (e->getState() == PickingState::Finished) {
            // end hover
            hover_ = false;
            mouseEvent->setMouseCursor(MouseCursor::Arrow);
            triggerUpdate = true;
        } else if (e->getState() == PickingState::Updated) {
            if (mouseMove && (mouseEvent->buttonState() & MouseButton::Left) == MouseButton::Left) {
                moveAction(e->getPosition());
                triggerUpdate = true;
                e->markAsUsed();
            } else if (leftMouseBtn) {
                if (mousePress) {
                    triggerUpdate = true;
                }
                e->markAsUsed();
            }
        }
    } else if (e->getEvent()->hash() == TouchEvent::chash()) {
        auto touchEvent = e->getEventAs<TouchEvent>();

        if (touchEvent->touchPoints().size() == 1) {
            // allow interaction only for a single touch point
            const auto& touchPoint = touchEvent->touchPoints().front();

            if (touchPoint.state() == TouchState::Started) {
                // initial activation since touch event began
                // start hover
                hover_ = true;
                triggerUpdate = true;
            } else if (touchPoint.state() == TouchState::Finished) {
                // end hover
                hover_ = false;
                triggerUpdate = true;
            } else if (touchPoint.state() == TouchState::Updated) {
                moveAction(touchPoint.pos());
                triggerUpdate = true;
                e->markAsUsed();
            }
        }
    }
    if (triggerUpdate) {
        processor_->invalidate(InvalidationLevel::InvalidOutput);
    }
}

}  // namespace inviwo
