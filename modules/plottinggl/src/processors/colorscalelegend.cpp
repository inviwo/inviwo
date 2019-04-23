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
    , style_("style", "Style")
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
    , legendSize_("legendSize", "Legend Size", vec2(200, 25), vec2(50, 10), vec2(400, 50))
    , title_("title", "Legend Title", "Legend Title")
    , color_("color", "Color", vec4(0, 0, 0, 1))
    , fontSize_("fontSize", "Font Size", 14, 8, 36)
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

    addProperty(isotfComposite_);

    // legend position
    positioning_.addProperty(legendPlacement_);
    positioning_.addProperty(rotation_);
    rotation_.setVisible(false);
    positioning_.addProperty(position_);
    position_.setVisible(false);
    positioning_.addProperty(margin_);
    positioning_.addProperty(legendSize_);
    addProperty(positioning_);

    // legend style
    style_.addProperty(title_);
    style_.addProperty(color_);
    color_.setSemantics(PropertySemantics::Color);
    style_.addProperty(fontSize_);
    style_.addProperty(backgroundStyle_);
    style_.addProperty(checkerBoardSize_);
    style_.addProperty(borderWidth_);
    checkerBoardSize_.setVisible(false);
    addProperty(style_);

    addProperty(axis_);

    rotation_.onChange([&]() { updateLegendState(); });

    title_.onChange([&]() { axis_.setCaption(title_.get()); });

    fontSize_.onChange([&]() {
        // the caption should be bigger than labels
        axis_.captionSettings_.font_.fontSize_.set(fontSize_.get() + 2);
        axis_.labelSettings_.font_.fontSize_.set(fontSize_.get());
    });

    color_.onChange([&]() {
        axis_.color_.set(color_.get());
        axis_.captionSettings_.color_.set(color_.get());
        axis_.labelSettings_.color_.set(color_.get());
        axis_.majorTicks_.color_.set(color_.get());
        axis_.minorTicks_.color_.set(color_.get());
    });
    // set initial axis parameters
    axis_.width_ = 0;
    axis_.setCaption(title_.get());
    axis_.captionSettings_.setChecked(true);
    axis_.labelSettings_.font_.fontFace_.set(axis_.captionSettings_.font_.fontFace_.get());
    axis_.captionSettings_.offset_.set(20);
    axis_.captionSettings_.font_.anchorPos_.set(vec2{0.0f, 0.0f});
    axis_.setCurrentStateAsDefault();

    fontSize_.propertyModified();

    legendPlacement_.onChange([&]() { updateLegendState(); });

    backgroundStyle_.onChange([&]() {
        switch (backgroundStyle_.get()) {
            default:
            case BackgroundStyle::NoBackground:
                checkerBoardSize_.setVisible(false);
                break;
            case BackgroundStyle::CheckerBoard:
                checkerBoardSize_.setVisible(true);
                break;
        }
    });
}

void ColorScaleLegend::initializeResources() { shader_.build(); }

vec2 ColorScaleLegend::getRealSize() {
    vec2 size = legendSize_.get();
    if (rotation_.get() % 2 == 0) return size;
    return vec2(size.y, size.x);
}

// this function handles the legend rotation and updates the axis thereafter
std::pair<ivec2, ivec2> ColorScaleLegend::getAxisPosition() {
    const float ticsWidth = ceil(axis_.majorTicks_.tickWidth_.get());
    const auto borderWidth = borderWidth_.get();

    const ivec2 dimensions = outport_.getDimensions();
    const vec2 position = position_.get();
    const ivec2 legendSize = getRealSize();

    // define the legend corners
    const ivec2 bottomLeft = position * vec2{dimensions} - vec2{legendSize} * vec2{0.5};
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
            axis_.orientation_.set(plot::AxisProperty::Orientation::Horizontal);
            axis_.placement_.set(plot::AxisProperty::Placement::Outside);
            break;
        case 1:  // 90 degrees rotation (right)
            axisStart = bottomLeft + ivec2(0, ticsWidth / 2) - ivec2(borderWidth);
            axisEnd = topLeft - ivec2(0, ticsWidth / 2) + ivec2(-borderWidth, borderWidth);
            axis_.orientation_.set(plot::AxisProperty::Orientation::Vertical);
            axis_.placement_.set(plot::AxisProperty::Placement::Outside);
            break;
        case 2:  // 180 degrees rotation (bottom)
            axisStart = topLeft + ivec2(ticsWidth / 2, 0) + ivec2(-borderWidth, borderWidth);
            axisEnd = topRight - ivec2(ticsWidth / 2, 0) + ivec2(borderWidth);
            axis_.orientation_.set(plot::AxisProperty::Orientation::Horizontal);
            axis_.placement_.set(plot::AxisProperty::Placement::Inside);
            break;
        case 3:  // 270 degrees rotation (left)
            axisStart = bottomRight + ivec2(0, ticsWidth / 2) + ivec2(borderWidth, -borderWidth);
            axisEnd = topRight - ivec2(0, ticsWidth / 2) + ivec2(borderWidth);
            axis_.orientation_.set(plot::AxisProperty::Orientation::Vertical);
            axis_.placement_.set(plot::AxisProperty::Placement::Inside);
            break;
    }
    return {axisStart, axisEnd};
}

void ColorScaleLegend::updateLegendState() {
    const vec2 dim{outport_.getDimensions()};

    const std::array<vec2, 4> pos = {{{0.5f, 1.0f}, {1.0f, 0.5f}, {0.5f, 0.0f}, {0.0f, 0.5f}}};
    if (legendPlacement_.get() == 4) {
        rotation_.setVisible(true);
        position_.setVisible(true);
    } else {
        rotation_.setVisible(false);
        position_.setVisible(false);
        position_.set(pos[legendPlacement_]);
        rotation_.set(legendPlacement_.get());
    }

    if (rotation_ % 2 == 0) {
        auto maxLength = dim.x - margin_ * 2;
        legendSize_.setMaxValue(vec2(maxLength, legendSize_.getMaxValue().y));
    } else {
        auto maxLength = dim.y - margin_ * 2;
        legendSize_.setMaxValue(vec2(maxLength, legendSize_.getMaxValue().y));
    }

    const vec2 legendSize{getRealSize()};
    const vec2 normalizedMin{legendSize / dim / 2.0f};
    const vec2 normalizedMax{vec2{1.0} - normalizedMin};
    const vec2 normalizedMargin{vec2{static_cast<float>(margin_)} / dim};

    position_.setMinValue(normalizedMin + normalizedMargin);
    position_.setMaxValue(normalizedMax - normalizedMargin);

    // update the legend range if a volume is connected to inport
    if (volumeInport_.isChanged() && volumeInport_.hasData()) {
        axis_.setRange(volumeInport_.getData()->dataMap_.valueRange);
    } else if (!volumeInport_.isConnected()) {
        axis_.setRange(vec2(0, 1));
    }
}

void ColorScaleLegend::process() {
    // draw cached overlay on top of the input image
    if (inport_.isReady()) {
        utilgl::activateTargetAndCopySource(outport_, inport_);
    } else {
        utilgl::activateAndClearTarget(outport_);
    }

    updateLegendState();

    const auto axisPos = getAxisPosition();
    const ivec2 dimensions = outport_.getDimensions();
    axisRenderer_.render(dimensions, axisPos.first, axisPos.second);

    utilgl::DepthFuncState depthFunc(GL_ALWAYS);
    utilgl::BlendModeState blending(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    TextureUnitContainer units;
    utilgl::Activate<Shader> activate(&shader_);
    utilgl::bindAndSetUniforms(shader_, units, isotfComposite_);
    utilgl::setUniforms(shader_, position_, color_, borderWidth_, backgroundStyle_,
                        checkerBoardSize_, rotation_, isotfComposite_);

    const ivec2 legendSize = getRealSize();
    const ivec2 bottomLeft = position_.get() * vec2{dimensions} - vec2{legendSize} * vec2{0.5};
    const ivec4 view{bottomLeft - ivec2{borderWidth_}, legendSize + ivec2{borderWidth_ * 2}};
    shader_.setUniform("viewport", view);
    utilgl::ViewportState viewport(view);

    utilgl::singleDrawImagePlaneRect();
    utilgl::deactivateCurrentTarget();
}
}  // namespace plot
}  // namespace inviwo
