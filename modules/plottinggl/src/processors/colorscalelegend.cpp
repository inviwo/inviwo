/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2019 Inviwo Foundation
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

#include <modules/plottinggl/processors/colorscalelegend.h>
#include <modules/opengl/texture/textureutils.h>
#include <modules/opengl/shader/shaderutils.h>

namespace inviwo {
namespace plot {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo ColorScaleLegend::processorInfo_{
    "org.inviwo.ColorScaleLegend",  // Class identifier
    "Color Scale Legend",           // Display name
    "Plotting",                     // Category
    CodeState::Stable,              // Code state
    "GL, Plotting, TF"              // Tags
};
const ProcessorInfo ColorScaleLegend::getProcessorInfo() const { return processorInfo_; }

ColorScaleLegend::ColorScaleLegend()
    : Processor()
    , inport_("inport")
    , outport_("outport")
    , volumeInport_("volumeInport")
    , isotfComposite_("isotfComposite", "TF & Isovalues")
    , positioning_("positioning", "Positioning & Size")
    , legendPlacement_("legendPlacement", "Legend Placement",
                       {{"top", "Top", 0},
                        {"right", "Right", 1},
                        {"bottom", "Bottom", 2},
                        {"left", "Left", 3},
                        {"custom", "Custom", 4}},
                       1)
    , rotation_("legendRotation", "Legend Rotation",
                {{"degree0", "0 degrees", 0},
                 {"degree90", "90 degrees", 1},
                 {"degree180", "180 degrees", 2},
                 {"degree270", "270 degrees", 3}},
                1)
    , position_("position", "Position", vec2(0.5f), vec2(0.0f), vec2(1.0f))
    , margin_("margin", "Margin (in pixels)", 25, 0, 100)
    , legendSize_("legendSize", "Legend Size", vec2(200, 25), vec2(10, 10), vec2(2000, 500))
    , axisStyle_("style", "Style")
    , title_("title", "Legend Title", "Legend")
    , backgroundStyle_("backgroundStyle", "Background",
                       {{"noBackground", "No background", BackgroundStyle::NoBackground},
                        {"checkerBoard", "Checker board", BackgroundStyle::CheckerBoard}},
                       0)
    , checkerBoardSize_("checkerBoardSize", "Checker Board Size", 5, 1, 20)
    , borderWidth_("borderWidth", "Border Width", 2, 0, 10)
    , shader_("img_texturequad.vert", "legend.frag")
    , axis_("axis", "Scale Axis")
    , axisRenderer_(axis_) {

    shader_.onReload([this]() { invalidate(InvalidationLevel::InvalidResources); });

    inport_.setOptional(true);
    volumeInport_.setOptional(true);
    addPort(inport_);
    addPort(outport_);
    addPort(volumeInport_);

    // legend position
    positioning_.addProperties(legendPlacement_, rotation_, position_, margin_, legendSize_);
    rotation_.visibilityDependsOn(legendPlacement_, [](auto l) { return l.get() == 4; });
    position_.visibilityDependsOn(legendPlacement_, [](auto l) { return l.get() == 4; });
    axis_.flipped_.visibilityDependsOn(legendPlacement_, [](auto l) { return l.get() == 4; });

    // legend style
    axisStyle_.insertProperty(0, title_);
    axisStyle_.addProperties(backgroundStyle_, checkerBoardSize_, borderWidth_);
    checkerBoardSize_.setVisible(false);

    axisStyle_.registerProperty(axis_);

    addProperties(isotfComposite_, positioning_, axisStyle_, axis_);

    // set initial axis parameters
    axis_.width_ = 0;
    axis_.setCaption(title_.get());
    axis_.captionSettings_.setChecked(true);
    axis_.labelSettings_.font_.fontFace_.set(axis_.captionSettings_.font_.fontFace_.get());
    axis_.captionSettings_.offset_.set(20);
    axis_.captionSettings_.font_.anchorPos_.set(vec2{0.0f, 0.0f});
    axis_.setCurrentStateAsDefault();

    title_.onChange([&]() { axis_.setCaption(title_.get()); });

    const auto getAxisOrientation = [](int i) {
        switch (i) {
            default:
            case 0:  // 0 degrees rotation (top)
                return AxisProperty::Orientation::Horizontal;
            case 1:  // 90 degrees rotation (right)
                return AxisProperty::Orientation::Vertical;
            case 2:  // 180 degrees rotation (bottom)
                return AxisProperty::Orientation::Horizontal;
            case 3:  // 270 degrees rotation (left)
                return AxisProperty::Orientation::Vertical;
        }
    };
    const auto getAxisPlacement = [](int i) {
        switch (i) {
            default:
            case 0:  // 0 degrees rotation (top)
                return AxisProperty::Placement::Outside;
            case 1:  // 90 degrees rotation (right)
                return AxisProperty::Placement::Outside;
            case 2:  // 180 degrees rotation (bottom)
                return AxisProperty::Placement::Inside;
            case 3:  // 270 degrees rotation (left)
                return AxisProperty::Placement::Inside;
        }
    };

    rotation_.onChange([=]() {
        axis_.orientation_.set(getAxisOrientation(rotation_.get()));
        axis_.placement_.set(getAxisPlacement(rotation_.get()));

        if (rotation_ == 0 || rotation_ == 3) {  // Top/Left
            axis_.flipped_ = false;
        } else if (rotation_ == 1 || rotation_ == 2) {  // Right/Bottom
            axis_.flipped_ = true;
        }
    });

    legendPlacement_.onChange([this]() {
        if (legendPlacement_ != 4) rotation_.set(legendPlacement_.get());
    });

    checkerBoardSize_.visibilityDependsOn(
        backgroundStyle_, [&](auto p) { return p.get() == BackgroundStyle::CheckerBoard; });
}

// this function handles the legend rotation and updates the axis thereafter
std::tuple<ivec2, ivec2, ivec2, ivec2> ColorScaleLegend::getPositions(ivec2 dimensions) const {
    const float ticsWidth = ceil(axis_.majorTicks_.tickWidth_.get());
    const auto borderWidth = borderWidth_.get();

    const auto [position, rotation, legendSize] = [&]() {
        const std::array<vec2, 4> pos = {{{0.5f, 1.0f}, {1.0f, 0.5f}, {0.5f, 0.0f}, {0.0f, 0.5f}}};
        const auto initialPos = legendPlacement_ == 4 ? position_.get() : pos[legendPlacement_];
        const auto rotation = legendPlacement_ == 4 ? rotation_.get() : legendPlacement_.get();
        const auto size =
            rotation % 2 == 0 ? legendSize_.get() : ivec2{legendSize_.get().y, legendSize_.get().x};

        auto lpos = 0.5f * vec2{size + 2 * ivec2{margin_}} +
                    initialPos * vec2{dimensions - size - 2 * ivec2{margin_}};

        return std::make_tuple(lpos, rotation, size);
    }();

    // define the legend corners
    const ivec2 bottomLeft = position - vec2{legendSize} * vec2{0.5};
    const ivec2 bottomRight = vec2(bottomLeft.x + legendSize.x, bottomLeft.y);
    const ivec2 topLeft = vec2(bottomLeft.x, bottomLeft.y + legendSize.y);
    const ivec2 topRight = vec2(bottomRight.x, topLeft.y);

    ivec2 axisStart{0};
    ivec2 axisEnd{0};

    switch (rotation_.get()) {
        default:
        case 0:  // 0 degrees rotation (top)
            axisStart = bottomLeft + ivec2(ticsWidth / 2, 0) - ivec2(borderWidth);
            axisEnd = bottomRight - ivec2(ticsWidth / 2, 0) + ivec2(borderWidth, -borderWidth);
            break;
        case 1:  // 90 degrees rotation (right)
            axisStart = bottomLeft + ivec2(0, ticsWidth / 2) - ivec2(borderWidth);
            axisEnd = topLeft - ivec2(0, ticsWidth / 2) + ivec2(-borderWidth, borderWidth);
            break;
        case 2:  // 180 degrees rotation (bottom)
            axisStart = topLeft + ivec2(ticsWidth / 2, 0) + ivec2(-borderWidth, borderWidth);
            axisEnd = topRight - ivec2(ticsWidth / 2, 0) + ivec2(borderWidth);
            break;
        case 3:  // 270 degrees rotation (left)
            axisStart = bottomRight + ivec2(0, ticsWidth / 2) + ivec2(borderWidth, -borderWidth);
            axisEnd = topRight - ivec2(0, ticsWidth / 2) + ivec2(borderWidth);
            break;
    }
    return {bottomLeft, legendSize, axisStart, axisEnd};
}

void ColorScaleLegend::process() {
    // draw cached overlay on top of the input image
    if (inport_.isReady()) {
        utilgl::activateTargetAndCopySource(outport_, inport_);
    } else {
        utilgl::activateAndClearTarget(outport_);
    }

    // update the legend range if a volume is connected to inport
    if (volumeInport_.isChanged() && volumeInport_.hasData()) {
        axis_.setRange(volumeInport_.getData()->dataMap_.valueRange);
    } else if (!volumeInport_.isConnected()) {
        axis_.setRange(vec2(0, 1));
    }

    const ivec2 dimensions = outport_.getDimensions();
    const auto [bottomLeft, legendSize, axisStart, axisEnd] = getPositions(dimensions);
    axisRenderer_.render(dimensions, axisStart, axisEnd);

    utilgl::DepthFuncState depthFunc(GL_ALWAYS);
    utilgl::BlendModeState blending(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    TextureUnitContainer units;
    utilgl::Activate<Shader> activate(&shader_);
    utilgl::bindAndSetUniforms(shader_, units, isotfComposite_);
    utilgl::setUniforms(shader_, axis_.color_, borderWidth_, backgroundStyle_, checkerBoardSize_,
                        rotation_, isotfComposite_);

    const ivec4 view{bottomLeft - ivec2{borderWidth_}, legendSize + ivec2{borderWidth_ * 2}};
    shader_.setUniform("viewport", view);
    utilgl::ViewportState viewport(view);

    utilgl::singleDrawImagePlaneRect();
    utilgl::deactivateCurrentTarget();
}
}  // namespace plot
}  // namespace inviwo
