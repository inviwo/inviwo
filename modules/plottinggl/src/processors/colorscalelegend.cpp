/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2022 Inviwo Foundation
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

#include <inviwo/core/datastructures/datamapper.h>                    // for DataMapper
#include <inviwo/core/datastructures/unitsystem.h>                    // for Axis
#include <inviwo/core/ports/datainport.h>                             // for DataInport
#include <inviwo/core/ports/imageport.h>                              // for ImageInport, ImageO...
#include <inviwo/core/ports/volumeport.h>                             // for VolumeInport
#include <inviwo/core/processors/processor.h>                         // for Processor
#include <inviwo/core/processors/processorinfo.h>                     // for ProcessorInfo
#include <inviwo/core/processors/processorstate.h>                    // for CodeState, CodeStat...
#include <inviwo/core/processors/processortags.h>                     // for Tags
#include <inviwo/core/properties/boolproperty.h>                      // for BoolProperty
#include <inviwo/core/properties/compositeproperty.h>                 // for CompositeProperty
#include <inviwo/core/properties/invalidationlevel.h>                 // for InvalidationLevel
#include <inviwo/core/properties/optionproperty.h>                    // for OptionProperty, Opt...
#include <inviwo/core/properties/ordinalproperty.h>                   // for IntProperty, FloatP...
#include <inviwo/core/properties/propertysemantics.h>                 // for PropertySemantics
#include <inviwo/core/properties/stringproperty.h>                    // for StringProperty
#include <inviwo/core/util/defaultvalues.h>                           // for Defaultvalues
#include <inviwo/core/util/glmvec.h>                                  // for ivec2, vec2, vec4
#include <inviwo/core/util/staticstring.h>                            // for operator+
#include <modules/fontrendering/properties/fontfaceoptionproperty.h>  // for FontFaceOptionProperty
#include <modules/fontrendering/properties/fontproperty.h>            // for FontProperty
#include <modules/opengl/inviwoopengl.h>                              // for GL_ALWAYS, GL_ONE
#include <modules/opengl/openglutils.h>                               // for Activate, BlendMode...
#include <modules/opengl/shader/shader.h>                             // for Shader
#include <modules/opengl/shader/shaderutils.h>                        // for setUniforms
#include <modules/opengl/texture/textureunit.h>                       // for TextureUnitContainer
#include <modules/opengl/texture/textureutils.h>                      // for activateAndClearTarget
#include <modules/plotting/datastructures/axissettings.h>             // for AxisSettings::Orien...
#include <modules/plotting/datastructures/majorticksettings.h>        // for TickStyle, TickStyl...
#include <modules/plotting/properties/axisproperty.h>                 // for AxisProperty
#include <modules/plotting/properties/axisstyleproperty.h>            // for AxisStyleProperty
#include <modules/plotting/properties/plottextproperty.h>             // for PlotTextProperty
#include <modules/plotting/properties/tickproperty.h>                 // for MajorTickProperty
#include <modules/plottinggl/utils/axisrenderer.h>                    // for AxisRenderer

#include <array>                                                      // for array
#include <cmath>                                                      // for ceil
#include <memory>                                                     // for shared_ptr
#include <type_traits>                                                // for remove_extent_t

#include <fmt/core.h>                                                 // for basic_string_view, arg
#include <glm/vec2.hpp>                                               // for operator+, operator-

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
    , enabled_("enabled", "Enabled", true)
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
    , labelType_{"titleType",
                 "Title Type",
                 {{"string", "Title String", LabelType::String},
                  {"data", "Title From Data", LabelType::Data},
                  {"custom", "Custom Format (example '{n}{u: [}')", LabelType::Custom}},
                 0}
    , title_("title", "Legend Title", "Legend")
    , backgroundStyle_("backgroundStyle", "Background",
                       {{"noBackground", "No background", BackgroundStyle::NoBackground},
                        {"checkerBoard", "Checker board", BackgroundStyle::CheckerBoard},
                        {"solid", "Solid", BackgroundStyle::SolidColor}},
                       0)
    , checkerBoardSize_("checkerBoardSize", "Checker Board Size", 5, 1, 20)
    , bgColor_("backgroundColor", "Background Color", vec4(0.0f, 0.0f, 0.0f, 1.0f), vec4(0.0f),
               vec4(1.0f), Defaultvalues<vec4>::getInc(), InvalidationLevel::InvalidOutput,
               PropertySemantics::Color)
    , borderWidth_("borderWidth", "Border Width", 2, 0, 10)
    , shader_("img_texturequad.vert", "legend.frag")
    , axis_("axis", "Scale Axis")
    , axisRenderer_(axis_) {

    shader_.onReload([this]() { invalidate(InvalidationLevel::InvalidResources); });

    addPort(inport_).setOptional(true);
    addPort(outport_);
    addPort(volumeInport_).setOptional(true);

    // legend position
    positioning_.addProperties(legendPlacement_, rotation_, position_, margin_, legendSize_);
    rotation_.visibilityDependsOn(legendPlacement_, [](auto l) { return l.get() == 4; });
    position_.visibilityDependsOn(legendPlacement_, [](auto l) { return l.get() == 4; });
    axis_.flipped_.visibilityDependsOn(legendPlacement_, [](auto l) { return l.get() == 4; });

    // legend style
    axisStyle_.insertProperty(0, labelType_);
    axisStyle_.insertProperty(1, title_);
    axisStyle_.addProperties(backgroundStyle_, checkerBoardSize_, bgColor_, borderWidth_);
    checkerBoardSize_.setVisible(false);
    bgColor_.setVisible(false);

    axisStyle_.registerProperty(axis_);

    addProperties(enabled_, isotfComposite_, positioning_, axisStyle_, axis_);

    // set initial axis parameters
    axis_.width_ = 0;
    axis_.setCaption(title_.get());
    axis_.captionSettings_.setChecked(true);
    axis_.labelSettings_.font_.fontFace_.set(axis_.captionSettings_.font_.fontFace_.get());
    axis_.captionSettings_.offset_.set(20);
    axis_.captionSettings_.font_.anchorPos_.set(vec2{0.0f, 0.0f});
    axis_.majorTicks_.style_.set(plot::TickStyle::Outside);
    axis_.setCurrentStateAsDefault();

    auto updateTitle = [this]() {
        switch (labelType_.get()) {
            case LabelType::String:
                axis_.setCaption(title_.get());
                break;
            case LabelType::Data:
                if (auto volume = volumeInport_.getData()) {
                    axis_.setCaption(fmt::format("{}{: [}", volume->dataMap_.valueAxis.name,
                                                 volume->dataMap_.valueAxis.unit));
                } else {
                    axis_.setCaption("?");
                }
                break;
            case LabelType::Custom:
                if (auto volume = volumeInport_.getData()) {
                    axis_.setCaption(fmt::format(title_.get(),
                                                 fmt::arg("n", volume->dataMap_.valueAxis.name),
                                                 fmt::arg("u", volume->dataMap_.valueAxis.unit)));
                } else {
                    axis_.setCaption("?");
                }
                break;
            default:
                axis_.setCaption(title_.get());
                break;
        }
    };

    volumeInport_.onChange(updateTitle);
    labelType_.onChange(updateTitle);
    title_.onChange(updateTitle);

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

    legendPlacement_.onChange([this]() {
        if (legendPlacement_ != 4) rotation_.set(legendPlacement_.get());
    });
    if (legendPlacement_ != 4) rotation_.set(legendPlacement_.get());

    auto updatePlacement = [&]() {
        axis_.orientation_.set(getAxisOrientation(rotation_.get()));
        axis_.placement_.set(getAxisPlacement(rotation_.get()));

        if (rotation_ == 0 || rotation_ == 3) {  // Top/Left
            axis_.flipped_ = false;
        } else if (rotation_ == 1 || rotation_ == 2) {  // Right/Bottom
            axis_.flipped_ = true;
        }
    };

    rotation_.onChange(updatePlacement);
    updatePlacement();

    checkerBoardSize_.visibilityDependsOn(
        backgroundStyle_, [&](auto p) { return p.get() == BackgroundStyle::CheckerBoard; });
    bgColor_.visibilityDependsOn(backgroundStyle_,
                                 [&](auto p) { return p.get() == BackgroundStyle::SolidColor; });
}  // namespace plot

// this function handles the legend rotation and updates the axis thereafter
std::tuple<ivec2, ivec2, ivec2, ivec2> ColorScaleLegend::getPositions(ivec2 dimensions) const {
    const float ticsWidth = ceil(axis_.majorTicks_.tickWidth_.get());
    const auto borderWidth = borderWidth_.get();

    const auto [position, legendSize] = [&]() {
        const std::array<vec2, 4> pos = {{{0.5f, 1.0f}, {1.0f, 0.5f}, {0.5f, 0.0f}, {0.0f, 0.5f}}};
        const auto initialPos = legendPlacement_ == 4 ? position_.get() : pos[legendPlacement_];
        const auto rotation = legendPlacement_ == 4 ? rotation_.get() : legendPlacement_.get();
        const auto size =
            rotation % 2 == 0 ? legendSize_.get() : ivec2{legendSize_.get().y, legendSize_.get().x};

        auto lpos = 0.5f * vec2{size + 2 * ivec2{margin_}} +
                    initialPos * vec2{dimensions - size - 2 * ivec2{margin_}};

        return std::make_tuple(lpos, size);
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
    if (!enabled_) {
        if (inport_.isReady()) {
            outport_.setData(inport_.getData());
        } else {
            utilgl::activateAndClearTarget(outport_);
        }

        return;
    }
    utilgl::activateTargetAndClearOrCopySource(outport_, inport_);

    // update the legend range if a volume is connected to inport
    if (volumeInport_.isChanged() && volumeInport_.hasData()) {
        axis_.setRange(volumeInport_.getData()->dataMap_.valueRange);
    } else if (!volumeInport_.isConnected()) {
        axis_.setRange(vec2(0, 1));
    }

    const ivec2 dimensions = outport_.getDimensions();
    const auto [bottomLeft, legendSize, axisStart, axisEnd] = getPositions(dimensions);

    utilgl::DepthFuncState depthFunc(GL_ALWAYS);
    utilgl::BlendModeState blending(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    TextureUnitContainer units;
    utilgl::Activate<Shader> activate(&shader_);
    utilgl::bindAndSetUniforms(shader_, units, isotfComposite_);
    utilgl::setUniforms(shader_, axis_.color_, borderWidth_, backgroundStyle_, checkerBoardSize_,
                        rotation_, isotfComposite_);

    if (backgroundStyle_ == BackgroundStyle::NoBackground) {
        shader_.setUniform("backgroundColor", vec4(0.0f));
    } else {
        shader_.setUniform("backgroundColor", bgColor_);
    }

    const ivec4 view{bottomLeft - ivec2{borderWidth_}, legendSize + ivec2{borderWidth_ * 2}};
    shader_.setUniform("viewport", view);
    {
        utilgl::ViewportState viewport(view);
        utilgl::singleDrawImagePlaneRect();
    }

    axisRenderer_.render(dimensions, axisStart, axisEnd);

    utilgl::deactivateCurrentTarget();
}

}  // namespace plot

}  // namespace inviwo
