/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018 Inviwo Foundation
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

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo ColorScaleLegend::processorInfo_{
    "org.inviwo.ColorScaleLegend",  // Class identifier
    "Color Scale Legend",           // Display name
    "Drawing",                      // Category
    CodeState::Experimental,        // Code state
    Tags::GL,                       // Tags
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

    rotation_.onChange([&]() { setLegendRotation(); });

    title_.onChange([&]() { axis_.caption_.title_.set(title_.get()); });

    fontSize_.onChange([&]() {
        // the caption should be bigger than labels
        axis_.caption_.font_.fontSize_.set(fontSize_.get() + 2);
        axis_.labels_.font_.fontSize_.set(fontSize_.get());
    });

    color_.onChange([&]() {
        axis_.color_.set(color_.get());
        axis_.caption_.color_.set(color_.get());
        axis_.labels_.color_.set(color_.get());
        axis_.ticks_.majorTicks_.color_.set(color_.get());
        axis_.ticks_.minorTicks_.color_.set(color_.get());
    });

    legendPlacement_.onChange([&]() {
        if (legendPlacement_.get() == 4) {
            rotation_.setVisible(true);
            position_.setVisible(true);
        } else {
            rotation_.setVisible(false);
            position_.setVisible(false);
        }
    });

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

void ColorScaleLegend::initializeResources() {
    // set initial axis parameters
    axis_.width_ = 0;
    axis_.caption_.title_.set(title_.get());
    axis_.caption_.setChecked(true);
    axis_.labels_.font_.fontFace_.set(axis_.caption_.font_.fontFace_.get());
    axis_.caption_.offset_.set(8);
    fontSize_.propertyModified();

    shader_.build();
}

vec2 ColorScaleLegend::getRealSize() {
    vec2 size = legendSize_.get();
    if (rotation_.get() % 2 == 0) return size;
    return vec2(size.y, size.x);
}

// this function handles the legend rotation and updates the axis thereafter
void ColorScaleLegend::setAxisPosition() {
    float ticsWidth = ceil(axis_.ticks_.majorTicks_.tickWidth_.get());
    auto borderWidth = borderWidth_.get();

    switch (rotation_.get()) {
        default:
        case 0:  // 0 degrees rotation (top)
            axisStart_ = bottomLeft_ + ivec2(ticsWidth / 2, 0) - ivec2(borderWidth);
            axisEnd_ = bottomRight_ - ivec2(ticsWidth / 2, 0) + ivec2(borderWidth, -borderWidth);
            axis_.orientation_.set(plot::AxisProperty::Orientation::Horizontal);
            axis_.placement_.set(plot::AxisProperty::Placement::Outside);
            break;
        case 1:  // 90 degrees rotation (right)
            axisStart_ = bottomLeft_ + ivec2(0, ticsWidth / 2) - ivec2(borderWidth);
            axisEnd_ = topLeft_ - ivec2(0, ticsWidth / 2) + ivec2(-borderWidth, borderWidth);
            axis_.orientation_.set(plot::AxisProperty::Orientation::Vertical);
            axis_.placement_.set(plot::AxisProperty::Placement::Outside);
            break;
        case 2:  // 180 degrees rotation (bottom)
            axisStart_ = topLeft_ + ivec2(ticsWidth / 2, 0) + ivec2(-borderWidth, borderWidth);
            axisEnd_ = topRight_ - ivec2(ticsWidth / 2, 0) + ivec2(borderWidth);
            axis_.orientation_.set(plot::AxisProperty::Orientation::Horizontal);
            axis_.placement_.set(plot::AxisProperty::Placement::Inside);
            break;
        case 3:  // 270 degrees rotation (left)
            axisStart_ = bottomRight_ + ivec2(0, ticsWidth / 2) + ivec2(borderWidth, -borderWidth);
            axisEnd_ = topRight_ - ivec2(0, ticsWidth / 2) + ivec2(borderWidth);
            axis_.orientation_.set(plot::AxisProperty::Orientation::Vertical);
            axis_.placement_.set(plot::AxisProperty::Placement::Inside);
            break;
    }
}

// set the boundaries of the position so that the legend is always inside the output canvas
void ColorScaleLegend::updatePositionBoundaries() {
    auto legendSize = getRealSize();
    auto dim = outport_.getDimensions();
    vec2 normalizedMin(((float)legendSize.x / 2) / dim.x, ((float)legendSize.y / 2) / dim.y);
    vec2 normalizedMax(1.0 - normalizedMin.x, 1.0 - normalizedMin.y);
    vec2 normalizedMargin((float)margin_.get() / dim.x, (float)margin_.get() / dim.y);

    position_.setMinValue(
        vec2(normalizedMin.x + normalizedMargin.x, normalizedMin.y + normalizedMargin.y));
    position_.setMaxValue(
        vec2(normalizedMax.x - normalizedMargin.x, normalizedMax.y - normalizedMargin.y));
}

void ColorScaleLegend::setLegendPosition() {
    switch (legendPlacement_.get()) {
        default:
            break;
        case 0:
            position_.set(vec2(0.5, 1));
            break;
        case 1:
            position_.set(vec2(1, 0.5));
            break;
        case 2:
            position_.set(vec2(0.5, 0));
            break;
        case 3:
            position_.set(vec2(0, 0.5));
            break;
    }
}

void ColorScaleLegend::setLegendRotation() {
    if (legendPlacement_.get() != 4) rotation_.set(legendPlacement_.get());

    // update the legend size boundaries
    if (rotation_.get() % 2 == 0) {
        auto maxLength = outport_.getDimensions().x - margin_.get() * 2;
        legendSize_.setMaxValue(vec2(maxLength, legendSize_.getMaxValue().y));
    } else {
        auto maxLength = outport_.getDimensions().x - margin_.get() * 2;
        legendSize_.setMaxValue(vec2(maxLength, legendSize_.getMaxValue().y));
    }
}

void ColorScaleLegend::process() {

    // draw cached overlay on top of the input image
    if (inport_.isReady()) {
        utilgl::activateTargetAndCopySource(outport_, inport_);
    } else {
        utilgl::activateAndClearTarget(outport_);
    }

    setLegendRotation();
    updatePositionBoundaries();
    setLegendPosition();

    ivec2 dimensions = outport_.getDimensions();
    vec2 position = position_.get();
    ivec2 legendSize = getRealSize();
    auto borderWidth = borderWidth_.get();

    // define the legend corners
    bottomLeft_ = vec2(position.x * dimensions.x - (legendSize.x / 2.0),
                       position.y * dimensions.y - (legendSize.y / 2.0));
    bottomRight_ = vec2(bottomLeft_.x + legendSize.x, bottomLeft_.y);
    topLeft_ = vec2(bottomLeft_.x, bottomLeft_.y + legendSize.y);
    topRight_ = vec2(bottomRight_.x, topLeft_.y);

    // update the legend range if a volume is connected to inport
    if (volumeInport_.isChanged() && volumeInport_.isConnected()) {
        axis_.setRange(volumeInport_.getData()->dataMap_.dataRange);
    } else if (!volumeInport_.isConnected()) {
        axis_.setRange(vec2(0, 1));
    }
    setAxisPosition();
    axisRenderer_.render(dimensions, axisStart_, axisEnd_);

    utilgl::DepthFuncState depthFunc(GL_ALWAYS);
    utilgl::BlendModeState blending(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    TextureUnit colorUnit;
    shader_.activate();
    shader_.setUniform("color_", colorUnit.getUnitNumber());
    shader_.setUniform("dimensions_", dimensions);
    shader_.setUniform("position_", position);
    shader_.setUniform("legendSize_", legendSize);
    shader_.setUniform("borderColor_", color_.get());
    shader_.setUniform("backgroundAlpha_", (int)backgroundStyle_.get());
    shader_.setUniform("checkerBoardSize_", (int)checkerBoardSize_.get());
    shader_.setUniform("rotationTF_", rotation_.get());

    utilgl::bindTexture(isotfComposite_.tf_, colorUnit);

    utilgl::ViewportState viewport(bottomLeft_.x - borderWidth, bottomLeft_.y - borderWidth,
                                   legendSize.x + (borderWidth * 2),
                                   legendSize.y + (borderWidth * 2));

    utilgl::singleDrawImagePlaneRect();

    shader_.deactivate();

    TextureUnit::setZeroUnit();
    utilgl::deactivateCurrentTarget();
}

}  // namespace inviwo
