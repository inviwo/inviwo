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

#include <modules/userinterfacegl/glui/widgets/rangeslider.h>
#include <modules/userinterfacegl/glui/renderer.h>

#include <inviwo/core/util/moduleutils.h>
#include <modules/opengl/texture/textureunit.h>
#include <modules/opengl/texture/texture2darray.h>
#include <modules/opengl/shader/shader.h>
#include <modules/opengl/shader/shaderutils.h>
#include <modules/opengl/rendering/meshdrawergl.h>

#include <vector>

namespace inviwo {

namespace glui {

const std::string RangeSlider::classIdentifier = "org.inviwo.glui.RangeSlider";
std::string RangeSlider::getClassIdentifier() const { return classIdentifier; }

RangeSlider::RangeSlider(const std::string &label, const ivec2 &value, int minValue, int maxValue,
                         int minSeparation, Processor &processor, Renderer &uiRenderer,
                         const ivec2 &extent, UIOrientation orientation)
    : Element(label, processor, uiRenderer, orientation)
    , value_(value)
    , min_(minValue)
    , max_(maxValue)
    , minSeparation_(minSeparation)
    , prevValue_(0) {
    widgetExtent_ = extent;
    moveAction_ = [this](const dvec2 &delta) {
        if (!enabled_) {
            return false;
        }
        auto calcNewValue = [this, delta](int prev) {
            return static_cast<int>(
                round(prev + convertDeltaToSlider(delta) * static_cast<double>(max_ - min_)));
        };

        const ivec2 currentValue = value_;
        switch (currentPickingID_) {
            case 0:  // lower bound
                setStart(calcNewValue(getPreviousValue().x));
                break;
            case 1:  // upper bound
                setEnd(calcNewValue(getPreviousValue().y));
                break;
            case 2:  // center (adjust both, lower and upper bound)
            {
                int moveby = calcNewValue(getPreviousValue().x) - getPreviousValue().x;
                // ensure not to move middle part further than min/max values
                if (moveby > 0) {
                    moveby = std::min(moveby, max_ - getPreviousValue().y);
                } else {
                    moveby = -std::min(-moveby, getPreviousValue().x - min_);
                }
                value_ = getPreviousValue() + moveby;
                break;
            }
            default:
                break;
        }
        return (value_ != currentValue);  // trigger update if changed
    };

    // adjust size of picking mapper to fit 3 pieces (min pos, max pos, and center part)
    pickingMapper_.resize(3);

    const auto texSourcePath = module::getModulePath("UserInterfaceGL", ModulePath::Images);

    const std::vector<std::string> sliderFiles = {
        "rangesliderhandle-normal.png", "rangesliderhandle-pressed.png",
        "rangesliderhandle-checked.png", "rangesliderhandle-halo.png",
        "rangesliderhandle-border.png"};
    uiTextures_ = uiRenderer_->createUITextures("RangeSlider", sliderFiles, texSourcePath);

    const std::vector<std::string> centerFiles = {
        "rangeslidercenter-normal.png", "rangeslidercenter-pressed.png",
        "rangeslidercenter-checked.png", "rangeslidercenter-halo.png",
        "rangeslidercenter-border.png"};
    centerTextures_ =
        uiRenderer_->createUITextures("RangeSliderConnector", centerFiles, texSourcePath);

    const std::vector<std::string> sliderGrooveFiles = {
        "slidergroove-normal.png", "slidergroove-pressed.png", "slidergroove-checked.png",
        "slidergroove-halo.png", "slidergroove-border.png"};
    grooveTextures_ =
        uiRenderer_->createUITextures("RangeSliderGroove", sliderGrooveFiles, texSourcePath);

    // normal, pressed, checked, corresponding halo (3x) and border (3x)
    uiTextureMap_ = {{0, 1, 2, 3, 3, 3, 4, 4, 4}};
}

void RangeSlider::set(const ivec2 &value, int minValue, int maxValue, int minSeparation) {
    value_ = value;
    min_ = minValue;
    max_ = maxValue;
    minSeparation_ = minSeparation;
}

void RangeSlider::set(const ivec2 &value) { value_ = value; }

const ivec2 &RangeSlider::get() const { return value_; }

int RangeSlider::getMinValue() const { return min_; }

int RangeSlider::getMaxValue() const { return max_; }

void RangeSlider::setMinSeparation(int sep) {
    minSeparation_ = sep;
    if (minSeparation_ > value_.y - value_.x) {
        // update upper range bound
        setEnd(value_.x + minSeparation_);
    }
}

int RangeSlider::getMinSeparation() const { return minSeparation_; }

void RangeSlider::setStart(int start) {
    start = std::max(start, min_);
    if (value_.x != start) {
        value_.x = start;
        if (value_.x > max_ - minSeparation_) {
            value_.x = max_ - minSeparation_;
        }
        if (value_.y - value_.x < minSeparation_) {
            value_.y = value_.x + minSeparation_;
        }
    }
}

void RangeSlider::setEnd(int stop) {
    stop = std::min(stop, max_);
    if (value_.y != stop) {
        value_.y = stop;
        if (value_.y < min_ + minSeparation_) {
            value_.y = min_ + minSeparation_;
        }
        if (value_.y - value_.x < minSeparation_) {
            value_.x = value_.y - minSeparation_;
        }
    }
}

void RangeSlider::renderWidget(const ivec2 &origin, const size2_t &) {
    TextureUnit texUnit;
    texUnit.activate();

    // bind textures
    auto &uiShader = uiRenderer_->getShader();
    uiShader.setUniform("arrayTexSampler", texUnit.getUnitNumber());
    uiShader.setUniform("arrayTexMap", 9, uiTextureMap_.data());

    const ivec2 extent(getWidgetExtentScaled());

    // render groove first
    if (showGroove_) {
        grooveTextures_->bind();
        uiShader.setUniform("origin", vec2(origin + widgetPos_));
        uiShader.setUniform("extent", vec2(extent));

        uiShader.setUniform("pickingColor", vec3(0.0f));
        uiShader.setUniform("uiState", ivec2(0, (hovered_ ? 1 : 0)));
        uiShader.setUniform("marginScale",
                            vec2(grooveTextures_->getDimensions()) / vec2(widgetExtent_));

        uiRenderer_->getMeshDrawer()->draw();
    }

    const auto sliderPos = getSliderPos();
    // render center part next
    {
        centerTextures_->bind();

        uiShader.setUniform("pickingColor", pickingMapper_.getColor(2));
        // ensure the center is only hovered if picking ID matches as well
        uiShader.setUniform("uiState", ivec2((pushed_ && (currentPickingID_ == 2) ? 1 : 0),
                                             (hovered_ && (currentPickingID_ == 2) ? 1 : 0)));

        vec2 centerPos;
        vec2 centerExtent;
        vec2 margin(1.0f);
        const double handleWidth = getHandleWidth();
        if (orientation_ == UIOrientation::Horizontal) {
            // offset left position of center piece by half the size of a handle
            centerPos = vec2(sliderPos.x + handleWidth / 2.0, 0);
            // extent of center piece corresponds to difference between the two handles
            centerExtent = vec2(sliderPos.y - sliderPos.x, extent.y);
            margin.x = uiTextures_->getDimensions().x / static_cast<float>(widgetExtent_.x);
            margin.y = centerTextures_->getDimensions().y / static_cast<float>(widgetExtent_.y);
        } else {
            // offset left position of center piece by half the size of a handle
            centerPos = vec2(0, sliderPos.x + handleWidth / 2.0);
            // extent of center piece corresponds to difference between the two handles
            centerExtent = vec2(extent.x, sliderPos.y - sliderPos.x);
            margin.x = centerTextures_->getDimensions().y / static_cast<float>(widgetExtent_.x);
            margin.y = uiTextures_->getDimensions().x / static_cast<float>(widgetExtent_.y);
        }
        uiShader.setUniform("marginScale", margin);
        uiShader.setUniform("origin", vec2(origin + widgetPos_) + centerPos);
        uiShader.setUniform("extent", centerExtent);

        uiRenderer_->getMeshDrawer()->draw();
    }

    // render both slider handles, adjust margin scale
    {
        uiTextures_->bind();

        vec2 dims(uiTextures_->getDimensions());
        const float aspectRatio = dims.x / dims.y;
        const float roundness = 0.8f;  // make them appear slightly square

        vec2 positionMask;
        if (orientation_ == UIOrientation::Horizontal) {
            positionMask = vec2(1, 0);
            uiShader.setUniform("extent", vec2(extent.y * aspectRatio, extent.y));
            uiShader.setUniform("marginScale", vec2(roundness));
        } else {
            positionMask = vec2(0, 1);
            uiShader.setUniform("extent", vec2(extent.x, extent.x * aspectRatio));
            uiShader.setUniform("marginScale", vec2(roundness));
        }

        auto drawHandle = [&uiShader, origin, positionMask, this](float pos, size_t id) {
            uiShader.setUniform("origin", vec2(origin + widgetPos_) + vec2(pos) * positionMask);
            // set up picking color
            uiShader.setUniform("pickingColor", pickingMapper_.getColor(id));
            uiShader.setUniform("uiState", ivec2((pushed_ && (currentPickingID_ == id) ? 1 : 0),
                                                 (hovered_ && (currentPickingID_ == id) ? 1 : 0)));
            // render quad
            uiRenderer_->getMeshDrawer()->draw();
        };

        // first handle
        drawHandle(sliderPos.x, flipped_ ? 1 : 0);
        // second handle
        drawHandle(sliderPos.y, flipped_ ? 0 : 1);
    }
}

const ivec2 &RangeSlider::getPreviousValue() const { return prevValue_; }

ivec2 RangeSlider::computeLabelPos(int descent) const {
    const int labelSpacing = 5;

    if (glm::all(glm::greaterThan(labelExtent_, ivec2(0)))) {
        const ivec2 extent(getWidgetExtentScaled());
        vec2 labelSize(labelExtent_);
        labelSize.y -= descent;
        ivec2 labelOrigin(extent.x + labelSpacing, extent.y / 2);
        // compute offset for vertical alignment in the center
        vec2 labelOffset(0.0f, -labelSize.y * 0.5f);

        return ivec2(labelOrigin + ivec2(labelOffset + 0.5f));
    }
    return ivec2(0);
}

Element::UIState RangeSlider::uiState() const {
    return (pushed_ ? UIState::Pressed : UIState::Normal);
}

vec2 RangeSlider::marginScale() const {
    if (uiTextures_) {
        // use unscaled widgetExtent_ here so that corners are also scaled along with the widget
        return (vec2(uiTextures_->getDimensions()) / vec2(widgetExtent_));
    }
    return vec2(1.0f);
}

void RangeSlider::pushStateChanged() { prevValue_ = value_; }

double RangeSlider::getHandleWidth() const {
    const dvec2 ext(getWidgetExtentScaled());
    // need to consider potentially different texture size
    const double ratio = static_cast<double>(uiTextures_->getDimensions().x) /
                         static_cast<double>(uiTextures_->getDimensions().y);
    return ratio * (orientation_ == UIOrientation::Horizontal ? ext.y : ext.x);
}

void RangeSlider::setShowGroove(bool show) { showGroove_ = show; }

bool RangeSlider::getShowGroove() const { return showGroove_; }

void RangeSlider::setFlipped(bool flipped) { flipped_ = flipped; }

bool RangeSlider::getFlipped() const { return flipped_; }

vec2 RangeSlider::getSliderPos() const {
    const dvec2 ext(getWidgetExtentScaled());
    // account for the width of the handle
    const double handleWidth = getHandleWidth();
    const double sliderLength =
        (orientation_ == UIOrientation::Vertical ? ext.y - handleWidth : ext.x - handleWidth);

    if (flipped_) {
        auto calcPos = [this, sliderLength](int pos) {
            return (max_ - glm::clamp(pos, min_, max_)) / static_cast<double>(max_ - min_) *
                   sliderLength;
        };

        return vec2(calcPos(value_.y), calcPos(value_.x));
    } else {
        auto calcPos = [this, sliderLength](int pos) {
            return (glm::clamp(pos, min_, max_) - min_) / static_cast<double>(max_ - min_) *
                   sliderLength;
        };

        return vec2(calcPos(value_.x), calcPos(value_.y));
    }
}

double RangeSlider::convertDeltaToSlider(dvec2 delta) const {
    const auto ext = getWidgetExtentScaled();
    // account for the width of the handle
    const double handleWidth = uiTextures_->getDimensions().x * scalingFactor_;
    if (flipped_) {
        delta *= -1.0;
    }
    if (orientation_ == UIOrientation::Vertical) {
        return delta.y / static_cast<double>(ext.y - handleWidth);
    } else {
        return delta.x / static_cast<double>(ext.x - handleWidth);
    }
}

}  // namespace glui

}  // namespace inviwo
