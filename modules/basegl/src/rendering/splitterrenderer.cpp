/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2021-2022 Inviwo Foundation
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

namespace inviwo {

SplitterRenderer::SplitterRenderer(Processor* processor)
    : processor_(processor)
    , shader_("splitter.vert", "splitter.geom", "linerenderer.frag")
    , triShader_("splitter.vert", "splittertriangle.geom", "standard.frag")
    , mesh_(DrawType::Points, ConnectivityType::None)
    , pickingMapper_(processor, 1, [&](PickingEvent* e) { handlePickingEvent(e); }) {

    mesh_.addBuffer(Mesh::BufferInfo(BufferType::PositionAttrib),
                    util::makeBuffer(std::vector<vec2>{vec2(0.0f)}));
}

SplitterRenderer::SplitterRenderer(const SplitterRenderer& rhs)
    : processor_(rhs.processor_)
    , shader_(rhs.shader_)
    , triShader_(rhs.triShader_)
    , mesh_(rhs.mesh_)
    , pickingMapper_(processor_, 1, [&](PickingEvent* e) { handlePickingEvent(e); }) {}

void SplitterRenderer::setInvalidateAction(InvalidateCallback callback) { invalidate_ = callback; }

void SplitterRenderer::setDragAction(DragCallback callback) { dragAction_ = callback; }

void SplitterRenderer::render(const SplitterSettings& settings, splitter::Direction direction,
                              const std::vector<float>& pos, size2_t canvasDims) {
    if (!settings.enabled() || pos.empty()) {
        return;
    }

    currentDirection_ = direction;
    const splitter::Style style = settings.getStyle();

    const auto [lineWidth, handleWidth, drawHandle,
                drawBorder] = [&]() -> std::tuple<float, float, bool, bool> {
        const float width = settings.getWidth();
        switch (style) {
            case splitter::Style::Handle:
                return {width / 3.0f, width, true, true};
            case splitter::Style::Divider:
                return {width, width * 0.6f, true, false};
            case splitter::Style::Line:
                return {width, 0.0f, false, false};
            case splitter::Style::Invisible:
                return {0.0f, 0.0f, true, false};
            default:
                return {0.0f, 0.0f, true, false};
        }
    }();

    // create a matrix transforming the splitter position from normalized coords [0,1] to NDC [-1,1]
    // along x
    mat4 posToNDC = []() {
        mat4 m(1.0f);
        m[0][0] = 2.0f;
        m[1][1] = 2.0f;
        m[3] = vec4(-1.0f, -1.0f, 0.0f, 1.0f);
        return m;
    }();

    const vec2 canvasDimsf(canvasDims);
    const int index = (direction == splitter::Direction::Vertical) ? 1 : 0;
    const float len = settings.getLength() / canvasDimsf[index];

    // scale matrix so that handle spans n pixel
    mat4 mHandle(posToNDC);
    mHandle[1][1] *= len;
    mHandle[3][1] = -len;

    mat4 mTri(posToNDC);
    mTri[3][1] = 0.0f;  // no vertical translation for triangle indicators

    if (direction == splitter::Direction::Horizontal) {
        // rotate by 90 degree for horizontal split
        mat4 rotMat = glm::rotate(-glm::half_pi<float>(), vec3(0.0f, 0.0f, 1.0f));
        posToNDC = rotMat * posToNDC;
        mHandle = rotMat * mHandle;
        mTri = rotMat * mTri;
    }

    if (auto count = pos.size(); maxSplittersInShader_ != count) {
        shader_.getGeometryShaderObject()->addShaderDefine("NUM_SPLITTERS", std::to_string(count));
        shader_.getGeometryShaderObject()->addShaderDefine("NUM_OUT_VERTICES",
                                                           std::to_string(4 * count));
        shader_.build();
        maxSplittersInShader_ = count;
    }
    if (pickingMapper_.getSize() < maxSplittersInShader_) {
        pickingMapper_.resize(maxSplittersInShader_);
    }

    utilgl::DepthFuncState depthFunc(GL_ALWAYS);
    utilgl::BlendModeState blending(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    MeshDrawerGL::DrawObject drawer(mesh_.getRepresentation<MeshGL>(), mesh_.getDefaultMeshInfo());

    shader_.activate();

    shader_.setUniform("positions", pos.size(), pos.data());

    shader_.setUniform("screenDim", canvasDimsf);
    // draw picking buffer
    shader_.setUniform("pickingEnabled", true);
    shader_.setUniform("pickId", static_cast<uint32_t>(pickingMapper_.getPickingId(0)));
    shader_.setUniform("lineWidth", settings.getWidth());
    shader_.setUniform("trafo", posToNDC);
    drawer.draw();

    if (style != splitter::Style::Invisible) {
        // draw background line
        const vec4 lineColor =
            (style == splitter::Style::Line) ? settings.getColor() : settings.getBackgroundColor();

        shader_.setUniform("pickingEnabled", false);
        shader_.setUniform("color", lineColor);
        shader_.setUniform("lineWidth", lineWidth);
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
            shader_.setUniform("color", settings.getColor());
            drawer.draw();
        }
    }
    shader_.deactivate();

    if ((hoveredSplitter_ > -1) && (settings.getTriangleSize() > 0) &&
        (static_cast<int>(pos.size()) > hoveredSplitter_)) {
        triShader_.activate();
        triShader_.setUniform("position", pos[hoveredSplitter_]);
        triShader_.setUniform("trafo", mTri);
        triShader_.setUniform("triangleSize", settings.getTriangleSize());
        triShader_.setUniform("color", settings.getHoverColor());
        if (direction == splitter::Direction::Horizontal) {
            triShader_.setUniform("screenDimInv", 1.0f / vec2(canvasDimsf.y, canvasDimsf.x));
        } else {
            triShader_.setUniform("screenDimInv", 1.0f / canvasDimsf);
        }
        drawer.draw();
        triShader_.deactivate();
    }
}

void SplitterRenderer::handlePickingEvent(PickingEvent* e) {
    bool triggerUpdate = false;
    const int pickId = static_cast<int>(e->getPickedId());

    auto moveAction = [&](dvec2 screenPos) {
        if (!dragAction_) return;

        // screenPos in normalized screen coords [0,1]
        if (currentDirection_ == splitter::Direction::Vertical) {
            dragAction_(static_cast<float>(screenPos.x), pickId);
        } else {
            // invert vertical position
            screenPos.y = 1.0 - screenPos.y;
            dragAction_(static_cast<float>(screenPos.y), pickId);
        }
    };

    if (auto mouseEvent = e->getEventAs<MouseEvent>()) {
        const bool leftMouseBtn = (mouseEvent->button() == MouseButton::Left);
        const bool mousePress = (mouseEvent->state() == MouseState::Press);
        const bool mouseMove = (mouseEvent->state() == MouseState::Move);

        if (e->getState() == PickingState::Started) {
            hoveredSplitter_ = pickId;

            const auto cursor = (currentDirection_ == splitter::Direction::Vertical)
                                    ? MouseCursor::SplitH
                                    : MouseCursor::SplitV;
            mouseEvent->setMouseCursor(cursor);
            triggerUpdate = true;
        } else if (e->getState() == PickingState::Finished) {
            // end hover
            hoveredSplitter_ = -1;
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
    } else if (auto touchEvent = e->getEventAs<TouchEvent>()) {
        if (touchEvent->touchPoints().size() == 1) {
            // allow interaction only for a single touch point
            const auto& touchPoint = touchEvent->touchPoints().front();

            if (touchPoint.state() == TouchState::Started) {
                hoveredSplitter_ = pickId;
                triggerUpdate = true;
            } else if (touchPoint.state() == TouchState::Finished) {
                hoveredSplitter_ = -1;
                triggerUpdate = true;
            } else if (touchPoint.state() == TouchState::Updated) {
                moveAction(touchPoint.pos());
                triggerUpdate = true;
                e->markAsUsed();
            }
        }
    }
    if (triggerUpdate && invalidate_) {
        invalidate_();
    }
}

}  // namespace inviwo
