/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017 Inviwo Foundation
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

#include <modules/userinterfacegl/glui/element.h>
#include <modules/userinterfacegl/glui/renderer.h>

#include <inviwo/core/util/exception.h>
#include <inviwo/core/util/moduleutils.h>
#include <modules/fontrendering/textrenderer.h>
#include <modules/opengl/openglutils.h>

#include <numeric>

namespace inviwo {

namespace glui {

Element::Element(ItemType type, const std::string &label, Renderer *uiRenderer)
    : itemType_(type)
    , action_([]() {})
    , moveAction_([](const dvec2&) { return false; })
    , hovered_(false)
    , pushed_(false)
    , checked_(false)
    , visible_(true)
    , labelDirty_(true)
    , uiRenderer_(uiRenderer)
{
    pickingIDs_.resize(1);
    if (!uiRenderer) {
        LogError("Cannot create GL UI element without a valid renderer");
        throw Exception("Cannot create GL UI element without a valid renderer");
    }

    setLabel(label);
}

Element::~Element() = default;

ItemType Element::getType() const { return itemType_; }

void Element::setLabel(const std::string &str) {
    if (str != labelStr_) {
        labelStr_ = str;
        
        // update label extent
        if (!labelStr_.empty()) {
            labelExtent_ = ivec2(uiRenderer_->getTextRenderer().computeTextSize(labelStr_));
        } else {
            labelExtent_ = ivec2(0, 0);
        }
        labelDirty_ = true;
    }
}

const std::string& Element::getLabel() const {
    return labelStr_;
}

bool Element::isDirty() const { return labelDirty_; }

void Element::setVisible(bool visible /*= true*/) {
    visible_ = visible;
}

bool Element::isVisible() const {
    return visible_;
}

const ivec2 &Element::getExtent() { 
    if (labelDirty_) {
        updateExtent();
    }
    return extent_; 
}

int Element::getNumWidgetComponents() const { return static_cast<int>(pickingIDs_.size()); }

int Element::updatePickingIDs(const int startIndex) {
    std::iota(pickingIDs_.begin(), pickingIDs_.end(), startIndex);
    return startIndex + getNumWidgetComponents();
}

bool Element::hasPickingID(int id) {
    return std::find(pickingIDs_.begin(), pickingIDs_.end(), id) != pickingIDs_.end();
}

void Element::render(const ivec2 &origin, const PickingMapper &pickingMapper, const ivec2 &canvasDim) {
    utilgl::DepthFuncState depthFunc(GL_ALWAYS);
    utilgl::BlendModeState blendMode(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    auto &uiShader = uiRenderer_->getShader();
    uiShader.activate();

    uiShader.setUniform("outportParameters.dimensions", vec2(canvasDim));
    uiShader.setUniform("outportParameters.reciprocalDimensions",
                        vec2(1.0f) / vec2(canvasDim));

    uiShader.setUniform("uiColor_", uiRenderer_->getUIColor());
    uiShader.setUniform("haloColor_", uiRenderer_->getHoverColor());

    renderWidget(origin, pickingMapper);

    uiShader.deactivate();
}

void Element::renderLabel(const ivec2 &origin, const size2_t &canvasDim) {
    if (labelDirty_) {
        updateLabel();
    }
    if (labelTexture_) {
        uiRenderer_->getTextureQuadRenderer().renderToRect(labelTexture_, origin + labelPos_,
                                                           labelExtent_, canvasDim);
    }
}

void Element::setHoverState(bool enable) {
    hovered_ = enable;
    if (!enable) {
        // reset the pushed flag when hovering ends
        pushed_ = false;
    }
}

void Element::setPushedState(bool pushed) {
    pushed_ = pushed;
    pushStateChanged();
}

bool Element::isPushed() const {
    return pushed_;
}

void Element::setAction(const std::function<void()> &action) {
    action_ = action;
}

void Element::triggerAction() {
    updateState();
    action_();
}

void Element::setMouseMoveAction(const std::function<bool(const dvec2&)> &action) {
    moveAction_ = action;
}

bool Element::moveAction(const dvec2 &delta) {
    return moveAction_(delta);
}

Element::UIState Element::uiState() const {
    return UIState::Normal;
}

vec2 Element::marginScale() const {
    return vec2(1.0f);
}

void Element::updateExtent() {
    if (labelDirty_) {
        updateLabelPos();
    }
    extent_ = glm::max(widgetPos_ + widgetExtent_, labelPos_ + labelExtent_);
}

void Element::updateLabelPos() {
    if (!labelStr_.empty()) {
        // compute label position
        // keep track of the font descent to account for proper centering
        int fontDescent = uiRenderer_->getTextRenderer().getBaseLineDescent();
        labelPos_ = computeLabelPos(fontDescent);
    } else {
        labelPos_ = ivec2(0, 0);
    }
}

void Element::updateLabel() {
    const vec4 black(0.0f, 0.0f, 0.0f, 1.0f);

    updateLabelPos();

    if (!labelStr_.empty()) {
        size2_t labelSize(labelExtent_);
        if (!labelTexture_ || (labelTexture_->getDimensions() != labelSize)) {
            auto texture = std::make_shared<Texture2D>(labelSize, GL_RGBA, GL_RGBA,
                                                       GL_UNSIGNED_BYTE, GL_LINEAR);
            texture->initialize(nullptr);
            labelTexture_ = texture;
        }
        uiRenderer_->getTextRenderer().renderToTexture(labelTexture_, labelStr_, black);
    } else {
        labelTexture_ = nullptr;
    }

    labelDirty_ = false;
}

} // namespace glui

}  // namespace inviwo
