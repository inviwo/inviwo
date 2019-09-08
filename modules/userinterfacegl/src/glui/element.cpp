/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2019 Inviwo Foundation
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
#include <inviwo/core/util/colorconversion.h>
#include <inviwo/core/interaction/events/pickingevent.h>
#include <inviwo/core/interaction/events/mouseevent.h>
#include <inviwo/core/interaction/events/touchevent.h>
#include <modules/fontrendering/textrenderer.h>
#include <modules/opengl/openglutils.h>

#include <numeric>

namespace inviwo {

namespace glui {

Element::Element(const std::string &label, Processor &processor, Renderer &uiRenderer,
                 UIOrientation orientation)
    : action_([]() {})
    , moveAction_([](const dvec2 &) { return false; })
    , hovered_(false)
    , pushed_(false)
    , checked_(false)
    , visible_(true)
    , enabled_(true)
    , boldLabel_(false)
    , labelVisible_(true)
    , labelFontSize_(uiRenderer.getDefaultFontSize())
    , orientation_(orientation)
    , scalingFactor_(1.0)
    , labelDirty_(true)
    , processor_(&processor)
    , uiRenderer_(&uiRenderer)
    , pickingMapper_(&processor, 1, [&](PickingEvent *e) { handlePickingEvent(e); })
    , currentPickingID_(0u) {
    setLabel(label);
}

Element::~Element() = default;

void Element::setLabel(const std::string &str) {
    if (str != labelStr_) {
        labelStr_ = str;

        // update label extent
        if (!labelStr_.empty()) {
            labelExtent_ = ivec2(getCurrentTextRenderer().computeTextSize(labelStr_));
        } else {
            labelExtent_ = ivec2(0, 0);
        }
        labelDirty_ = true;
        processor_->invalidate(InvalidationLevel::InvalidOutput);
    }
}

const std::string &Element::getLabel() const { return labelStr_; }

void Element::setFontSize(int size) {
    if (labelFontSize_ != size) {
        labelFontSize_ = size;
        labelDirty_ = true;
        processor_->invalidate(InvalidationLevel::InvalidOutput);
    }
}

int Element::getFontSize() const { return labelFontSize_; }

void Element::setVisible(bool visible) { visible_ = visible; }

bool Element::isVisible() const { return visible_; }

void Element::setEnabled(bool enable) { enabled_ = enable; }
bool Element::isEnabled() const { return enabled_; }

void Element::setLabelVisible(bool visible) {
    if (labelVisible_ != visible) {
        labelVisible_ = visible;
        labelDirty_ = true;
    }
}

bool Element::isLabelVisible() const { return labelVisible_; }

void Element::setScalingFactor(double factor) {
    if (std::abs(scalingFactor_ - factor) < glm::epsilon<double>()) {
        return;
    }
    scalingFactor_ = factor;
    // update label extent
    if (!labelStr_.empty()) {
        labelExtent_ = ivec2(getCurrentTextRenderer().computeTextSize(labelStr_));
    } else {
        labelExtent_ = ivec2(0, 0);
    }
    // label might need repositioning
    labelDirty_ = true;
}

double Element::getScalingFactor() const { return scalingFactor_; }

void Element::setOrientation(UIOrientation orientation) {
    if (orientation_ == orientation) {
        return;
    }
    orientation_ = orientation;
    // label might need repositioning
    labelDirty_ = true;

    processor_->invalidate(InvalidationLevel::InvalidOutput);
}

UIOrientation Element::getOrientation() const { return orientation_; }

void Element::setLabelBold(bool bold) {
    if (boldLabel_ != bold) {
        boldLabel_ = bold;

        if (!labelStr_.empty()) {
            labelExtent_ = ivec2(getCurrentTextRenderer().computeTextSize(labelStr_));
            labelDirty_ = true;
            processor_->invalidate(InvalidationLevel::InvalidOutput);
        }
    }
}

bool Element::isLabelBold() const { return boldLabel_; }

const ivec2 &Element::getExtent() {
    if (labelDirty_) {
        updateExtent();
    }
    return extent_;
}

bool Element::isDirty() const { return labelDirty_; }

void Element::setWidgetExtent(const ivec2 &extent) {
    if (extent != widgetExtent_) {
        labelDirty_ = true;
        // label might need repositioning
        widgetExtent_ = extent;
        processor_->invalidate(InvalidationLevel::InvalidOutput);
    }
}

const ivec2 &Element::getWidgetExtent() const { return widgetExtent_; }

void Element::setWidgetExtentScaled(const ivec2 &extent) {
    ivec2 newExtent(extent);
    if (std::abs(1.0 - scalingFactor_) > glm::epsilon<double>()) {
        // consider scaling
        newExtent = ivec2(dvec2(extent) / scalingFactor_);
    }
    if (newExtent != widgetExtent_) {
        labelDirty_ = true;
        // label might need repositioning
        widgetExtent_ = newExtent;
        processor_->invalidate(InvalidationLevel::InvalidOutput);
    }
}

ivec2 Element::getWidgetExtentScaled() const {
    if (std::abs(1.0 - scalingFactor_) < glm::epsilon<double>()) {
        // no custom scaling
        return widgetExtent_;
    } else {
        return ivec2(dvec2(widgetExtent_) * scalingFactor_ + 0.5);
    }
}

void Element::render(const ivec2 &origin, const size2_t &canvasDim) {
    utilgl::DepthFuncState depthFunc(GL_ALWAYS);
    utilgl::BlendModeState blendMode(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    auto &uiShader = uiRenderer_->getShader();
    uiShader.activate();

    uiShader.setUniform("outportParameters.dimensions", vec2(canvasDim));
    uiShader.setUniform("outportParameters.reciprocalDimensions", vec2(1.0f) / vec2(canvasDim));

    // set widget colors
    if (enabled_) {
        uiShader.setUniform("uiColor", uiRenderer_->getUIColor());
        uiShader.setUniform("haloColor", uiRenderer_->getHoverColor());
        uiShader.setUniform("uiBorderColor", uiRenderer_->getBorderColor());
    } else {
        uiShader.setUniform("uiColor", uiRenderer_->getDisabledColor());
        // make halo invisible
        uiShader.setUniform("haloColor", vec4(0.0f));
        uiShader.setUniform("uiBorderColor", adjustColor(uiRenderer_->getBorderColor()));
    }

    if (orientation_ == UIOrientation::Vertical) {
        // perform a rotation by 90deg counter-clock wise
        uiShader.setUniform("uiTextureMatrix",
                            mat3(0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f));
    } else {
        uiShader.setUniform("uiTextureMatrix", mat3(1.0f));
    }

    renderWidget(origin, canvasDim);

    uiShader.deactivate();

    renderLabel(origin, canvasDim);
}

void Element::setHoverState(bool enable) {
    if (enabled_) {
        hovered_ = enable;
        if (!enable) {
            // reset the pushed flag when hovering ends
            pushed_ = false;
        }
    }
}

void Element::setPushedState(bool pushed) {
    if (enabled_) {
        pushed_ = pushed;
        pushStateChanged();
    }
}

bool Element::isPushed() const { return pushed_; }

void Element::setChecked(bool checked) {
    if (enabled_) {
        checked_ = checked;
    }
}

bool Element::isChecked() const { return checked_; }

void Element::setAction(const std::function<void()> &action) { action_ = action; }

void Element::setPickingEventAction(std::function<void(PickingEvent *e)> pickingAction) {
    pickingAction_ = pickingAction;
}

void Element::triggerAction() {
    if (enabled_) {
        updateState();
        action_();
    }
}

void Element::setMouseMoveAction(const std::function<bool(const dvec2 &)> &action) {
    moveAction_ = action;
}

bool Element::moveAction(const dvec2 &delta) {
    if (enabled_) {
        return moveAction_(delta);
    }
    return false;
}

Element::UIState Element::uiState() const { return UIState::Normal; }

vec2 Element::marginScale() const { return vec2(1.0f); }

void Element::updateExtent() {
    if (labelDirty_) {
        updateLabelPos();
    }
    if (labelVisible_) {
        extent_ = glm::max(widgetPos_ + getWidgetExtentScaled(), labelPos_ + labelExtent_);
    } else {
        extent_ = widgetPos_ + getWidgetExtentScaled();
    }
}

void Element::updateLabelPos() {
    if (!labelStr_.empty()) {
        // compute label position
        // keep track of the font descent to account for proper centering
        int fontDescent = getCurrentTextRenderer().getBaseLineDescender();
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
        getCurrentTextRenderer().renderToTexture(labelTexture_, labelStr_, black);
    } else {
        labelTexture_ = nullptr;
    }

    labelDirty_ = false;
}

void Element::handlePickingEvent(PickingEvent *e) {
    bool triggerUpdate = false;

    if (e->getState() == PickingState::Started) {
        setHoverState(true);
        currentPickingID_ = e->getPickedId();
        triggerUpdate = true;
    } else if (e->getState() == PickingState::Finished) {
        // Releasing out of screen may not trigger PickingPressState::Release,
        // so ensure that states are reset
        setHoverState(false);
        setPushedState(false);
        currentPickingID_ = 0u;
        triggerUpdate = true;
    }
    if (e->getPressState() != PickingPressState::None) {
        if (e->getPressState() == PickingPressState::Move &&
            e->getPressItems() & PickingPressItem::Primary) {
            auto delta = e->getDeltaPressedPosition();
            triggerUpdate = moveAction(delta * dvec2(e->getCanvasSize()));
        } else if (e->getPressState() == PickingPressState::Press &&
                   e->getPressItem() & PickingPressItem::Primary) {
            // initial activation with button press
            setPushedState(true);
            triggerUpdate = true;
        } else if (e->getPressState() == PickingPressState::Release &&
                   (e->getPressItem() & PickingPressItem::Primary) && isPushed()) {
            // Button is released above the active element
            triggerAction();
            setPushedState(false);
            triggerUpdate = true;
        }
        e->markAsUsed();
    }

    if (pickingAction_) pickingAction_(e);
    if (triggerUpdate) processor_->invalidate(InvalidationLevel::InvalidOutput);
}

vec4 Element::adjustColor(const vec4 &color) {
    const float lumDiff = 0.15f;
    vec3 hsv(color::rgb2hsv(color));
    // darken color
    hsv.z -= lumDiff;
    // reduce saturation
    hsv.y = std::max(0.0f, hsv.x - 0.15f);
    return vec4(color::hsv2rgb(hsv), color.a);
}

TextRenderer &Element::getCurrentTextRenderer() const {
    // set font size first
    auto &textRenderer = uiRenderer_->getTextRenderer(boldLabel_);
    textRenderer.setFontSize(std::max(1, static_cast<int>(scalingFactor_ * labelFontSize_)));
    return textRenderer;
}

void Element::renderLabel(const ivec2 &origin, const size2_t &canvasDim) {
    if (!labelVisible_) {
        return;
    }
    if (labelDirty_) {
        updateLabel();
    }
    if (labelTexture_) {
        utilgl::DepthFuncState depthFunc(GL_ALWAYS);
        utilgl::BlendModeState blending(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
        uiRenderer_->getTextureQuadRenderer().renderToRect(labelTexture_, origin + labelPos_,
                                                           labelExtent_, canvasDim);
    }
}

}  // namespace glui

}  // namespace inviwo
