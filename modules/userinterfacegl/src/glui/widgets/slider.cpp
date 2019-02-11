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

#include <modules/userinterfacegl/glui/widgets/slider.h>
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

const std::string Slider::classIdentifier = "org.inviwo.glui.Slider";
std::string Slider::getClassIdentifier() const { return classIdentifier; }

Slider::Slider(const std::string &label, int value, int minValue, int maxValue,
               Processor &processor, Renderer &uiRenderer, const ivec2 &extent,
               UIOrientation orientation)
    : Element(label, processor, uiRenderer, orientation)
    , value_(value)
    , min_(minValue)
    , max_(maxValue)
    , prevValue_(0) {
    widgetExtent_ = extent;
    moveAction_ = [this](const dvec2 &delta) {
        if (!enabled_) {
            return false;
        }
        // delta in pixel (screen coords),
        // need to scale from graphical representation to slider
        const int newVal =
            glm::clamp(static_cast<int>(round(prevValue_ + convertDeltaToSlider(delta) *
                                                               static_cast<double>(max_ - min_))),
                       min_, max_);
        bool triggerUpdate = (value_ != newVal);
        value_ = newVal;
        return triggerUpdate;
    };

    const auto texSourcePath = module::getModulePath("UserInterfaceGL", ModulePath::Images);

    const std::vector<std::string> sliderFiles = {
        "sliderhandle-normal.png", "sliderhandle-pressed.png", "sliderhandle-checked.png",
        "sliderhandle-halo.png", "sliderhandle-border.png"};
    uiTextures_ = uiRenderer_->createUITextures("Slider", sliderFiles, texSourcePath);

    const std::vector<std::string> sliderGrooveFiles = {
        "slidergroove-normal.png", "slidergroove-pressed.png", "slidergroove-checked.png",
        "slidergroove-halo.png", "slidergroove-border.png"};
    grooveTextures_ =
        uiRenderer_->createUITextures("SliderGroove", sliderGrooveFiles, texSourcePath);

    // normal, pressed, checked, corresponding halo (3x) and border (3x)
    uiTextureMap_ = {{0, 1, 2, 3, 3, 3, 4, 4, 4}};
}  // namespace glui

void Slider::set(int value, int minValue, int maxValue) {
    value_ = value;
    min_ = minValue;
    max_ = maxValue;
}

void Slider::set(int value) { value_ = value; }

int Slider::get() const { return value_; }

int Slider::getMinValue() const { return min_; }

int Slider::getMaxValue() const { return max_; }

void Slider::renderWidget(const ivec2 &origin, const size2_t &) {
    TextureUnit texUnit;
    texUnit.activate();

    // bind textures
    auto &uiShader = uiRenderer_->getShader();
    uiShader.setUniform("arrayTexSampler", texUnit.getUnitNumber());
    uiShader.setUniform("arrayTexMap", 9, uiTextureMap_.data());

    const vec2 extent(getWidgetExtentScaled());

    // render groove first
    {
        grooveTextures_->bind();
        uiShader.setUniform("origin", vec2(origin + widgetPos_));
        uiShader.setUniform("extent", vec2(extent));

        uiShader.setUniform("pickingColor", vec3(0.0f));
        uiShader.setUniform("uiState", ivec2(0, (hovered_ ? 1 : 0)));
        uiShader.setUniform("marginScale",
                            vec2(grooveTextures_->getDimensions()) / vec2(widgetExtent_));

        uiRenderer_->getMeshDrawer()->draw();
    }

    // render slider, adjust margin scale
    {
        uiTextures_->bind();

        const auto sliderPos = getSliderPos();
        if (orientation_ == UIOrientation::Horizontal) {
            uiShader.setUniform("origin", vec2(origin + widgetPos_) + vec2(sliderPos, 0));
            uiShader.setUniform("extent", vec2(extent.y));
            uiShader.setUniform("marginScale", vec2(marginScale().y));
        } else {
            uiShader.setUniform("origin", vec2(origin + widgetPos_) + vec2(0, sliderPos));
            uiShader.setUniform("extent", vec2(extent.x));
            uiShader.setUniform("marginScale", vec2(marginScale().x));
        }

        // set up picking color
        uiShader.setUniform("pickingColor", pickingMapper_.getColor(0));
        uiShader.setUniform("uiState", ivec2(uiState(), (hovered_ ? 1 : 0)));

        // render quad
        uiRenderer_->getMeshDrawer()->draw();
    }
}

int Slider::getPreviousValue() const { return prevValue_; }

ivec2 Slider::computeLabelPos(int descent) const {
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

Element::UIState Slider::uiState() const { return (pushed_ ? UIState::Pressed : UIState::Normal); }

vec2 Slider::marginScale() const {
    if (uiTextures_) {
        // use unscaled widgetExtent_ here so that corners are also scaled along with the widget
        return (vec2(uiTextures_->getDimensions()) / vec2(widgetExtent_));
    }
    return vec2(1.0f);
}

void Slider::pushStateChanged() { prevValue_ = value_; }

double Slider::getHandleWidth() const {
    const dvec2 ext(getWidgetExtentScaled());
    // need to consider potentially different texture size
    const double ratio = static_cast<double>(uiTextures_->getDimensions().x) /
                         static_cast<double>(uiTextures_->getDimensions().y);
    return ratio * (orientation_ == UIOrientation::Horizontal ? ext.y : ext.x);
}

double Slider::getSliderPos() const {
    const dvec2 ext(getWidgetExtentScaled());
    // account for the width of the handle
    const double handleWidth = getHandleWidth();
    const double sliderLength =
        (orientation_ == UIOrientation::Vertical ? ext.y - handleWidth : ext.x - handleWidth);

    return (glm::clamp(value_, min_, max_) - min_) / static_cast<double>(max_ - min_) *
           sliderLength;
}

double Slider::convertDeltaToSlider(const dvec2 &delta) const {
    const auto ext = getWidgetExtentScaled();
    // account for the width of the handle
    const double handleWidth = uiTextures_->getDimensions().x * scalingFactor_;
    if (orientation_ == UIOrientation::Vertical) {
        return delta.y / static_cast<double>(ext.y - handleWidth);
    } else {
        return delta.x / static_cast<double>(ext.x - handleWidth);
    }
}

}  // namespace glui

}  // namespace inviwo
