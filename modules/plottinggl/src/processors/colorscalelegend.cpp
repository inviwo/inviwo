/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2025 Inviwo Foundation
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

#include <inviwo/core/algorithm/markdown.h>                           // for operator""_help
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
#include <modules/opengl/geometry/meshgl.h>                           // for MeshGL
#include <modules/opengl/rendering/meshdrawergl.h>                    // for MeshDrawerGL::Dra...
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

#include <array>        // for array
#include <cmath>        // for ceil
#include <memory>       // for shared_ptr
#include <type_traits>  // for remove_extent_t

#include <fmt/core.h>    // for basic_string_view, arg
#include <glm/vec2.hpp>  // for operator+, operator-
#include <glm/gtx/vec_swizzle.hpp>

namespace inviwo {
namespace plot {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo ColorScaleLegend::processorInfo_{
    "org.inviwo.ColorScaleLegend",                                             // Class identifier
    "Color Scale Legend",                                                      // Display name
    "Plotting",                                                                // Category
    CodeState::Stable,                                                         // Code state
    Tags::GL | Tag{"Plotting"} | Tag{"Colormap"} | Tag{"TF"} | Tag{"Legend"},  // Tags
    "Displays a color legend axis based on a transfer function. If a volume is connected, its data "
    "ranges are considered. The corresponding values of the colors can be displayed next to the "
    "colors."_help};
const ProcessorInfo& ColorScaleLegend::getProcessorInfo() const { return processorInfo_; }

ColorScaleLegend::ColorScaleLegend()
    : Processor()
    , inport_("inport", "Input image"_help)
    , volumeInport_("volumeInport", "Volume used for data range"_help)
    , outport_("outport", "Output image"_help)
    , enabled_(
          "enabled", "Enabled",
          "The color legend will be rendered if enabled. Otherwise the input image is passed through."_help,
          true)
    , isotfComposite_("isotfComposite", "TF & Isovalues",
                      "The transfer function used in the legend."_help, &volumeInport_)
    , positioning_("positioning", "Positioning & Size")
    , legendPresets_(
          "legendPresets", "Presets",
          "Adjust the placement of the legend on the canvas according to the presets."_help,
          buttons(), InvalidationLevel::Valid)
    , position_("position", "Position",
                "Sets the legend position in screen coordinates [0,1]."_help, vec2(0.5f, 0.0f),
                {vec2(0.0f), ConstraintBehavior::Editable},
                {vec2(1.0f), ConstraintBehavior::Editable})
    , margin_("margin", "Margin (in pixels)",
              util::ordinalCount(10, 100).set(
                  "Sets the legend margin to canvas borders in pixels."_help))
    , legendSize_("legendSize", "Legend Size", "Sets the legend width and height in pixel."_help,
                  vec2(200, 25), {vec2(10, 10), ConstraintBehavior::Editable},
                  {vec2(2000, 500), ConstraintBehavior::Editable})
    , axisStyle_("style", "Style")
    , labelType_{"titleType",
                 "Title Type",
                 {{"string", "Title String", LabelType::String},
                  {"data", "Title From Data", LabelType::Data},
                  {"custom", "Custom Format (example '{n}{u: [}')", LabelType::Custom}},
                 0}
    , title_("title", "Legend Title", "Legend")
    , backgroundStyle_("backgroundStyle", "Background",
                       "Background style when dealing with transparent transfer functions."_help,
                       {{"noBackground", "No background", BackgroundStyle::NoBackground},
                        {"checkerBoard", "Checkerboard", BackgroundStyle::CheckerBoard},
                        {"solid", "Solid", BackgroundStyle::SolidColor},
                        {"ceckerboardOpaque", "Split checkerboard/Opaque",
                         BackgroundStyle::CheckerboardAndOpaque},
                        {"opaque", "Opaque TF (ignore alpha)", BackgroundStyle::Opaque}},
                       0)
    , checkerBoardSize_("checkerBoardSize", "Checkerboard Size",
                        "Size of the checkerboard cells in pixel."_help, 5.0f,
                        {1.0f, ConstraintBehavior::Editable}, {20.0f, ConstraintBehavior::Editable})
    , bgColor_("backgroundColor", "Background Color",
               util::ordinalColor(vec4(0.0f, 0.0f, 0.0f, 1.0f))
                   .set("Color of the solid background."_help))
    , borderWidth_("borderWidth", "Border Width",
                   util::ordinalCount(2, 10).set("Border width of the legend in pixel."_help))
    , isovalues_("isovalues", "Show Isovalues", "Indicate isovalues with small triangles"_help,
                 true)
    , triSize_("triSize", "Triangle Size (pixel)",
               util::ordinalLength(10.0f, 50.0f)
                   .setInc(0.5f)
                   .set("Size of the isovalue indicators in pixel."_help))
    , shader_("img_texturequad.vert", "legend.frag")
    , isoValueShader_("isovaluetri.vert", "isovaluetri.geom", "standard.frag", Shader::Build::No)
    , axis_("axis", "Scale Axis")
    , axisRenderer_(axis_)
    , isovalueMesh_(DrawType::Points, ConnectivityType::None) {

    isovalueMesh_.addBuffer(Mesh::BufferInfo(BufferType::PositionAttrib),
                            util::makeBuffer(std::vector<vec2>{vec2(0.0f)}));
    isovalueMesh_.addIndices(Mesh::MeshInfo{DrawType::Points, ConnectivityType::None},
                             util::makeIndexBuffer({0u}));

    inport_.setOptional(true);
    volumeInport_.setOptional(true);
    addPorts(inport_, volumeInport_, outport_);

    // legend position
    positioning_.addProperties(legendPresets_, margin_, position_, legendSize_);

    // legend style
    isovalues_.addProperty(triSize_);
    isovalues_.setCollapsed(true);

    axisStyle_.insertProperty(0, labelType_);
    axisStyle_.insertProperty(1, title_);
    axisStyle_.addProperties(backgroundStyle_, checkerBoardSize_, bgColor_, borderWidth_,
                             isovalues_);
    checkerBoardSize_.setVisible(false);
    bgColor_.setVisible(false);

    checkerBoardSize_.visibilityDependsOn(backgroundStyle_, [&](auto p) {
        const auto style = p.get();
        return (style == BackgroundStyle::CheckerBoard) ||
               (style == BackgroundStyle::CheckerboardAndOpaque);
    });
    bgColor_.visibilityDependsOn(backgroundStyle_,
                                 [&](auto p) { return p.get() == BackgroundStyle::SolidColor; });

    axisStyle_.registerProperty(axis_);

    addProperties(enabled_, isotfComposite_, positioning_, axisStyle_, axis_);

    // set initial axis parameters
    axis_.width_ = 0;
    axis_.color_.setVisible(false);
    axis_.setCaption(title_.get());
    axis_.captionSettings_.setChecked(true);
    axis_.labelSettings_.font_.fontFace_.set(axis_.captionSettings_.font_.fontFace_.get());
    axis_.majorTicks_.style_.set(plot::TickStyle::Outside);
    axis_.setCurrentStateAsDefault();

    auto updateTitle = [this]() {
        switch (labelType_.get()) {
            case LabelType::String:
                axis_.setCaption(title_.get());
                break;
            case LabelType::Data:
                if (auto volume = volumeInport_.getData()) {
                    axis_.setCaption(fmt::format("{}{: [}", volume->dataMap.valueAxis.name,
                                                 volume->dataMap.valueAxis.unit));
                } else {
                    axis_.setCaption("?");
                }
                break;
            case LabelType::Custom:
                if (auto volume = volumeInport_.getData()) {
                    axis_.setCaption(fmt::format(fmt::runtime(title_.get()),
                                                 fmt::arg("n", volume->dataMap.valueAxis.name),
                                                 fmt::arg("u", volume->dataMap.valueAxis.unit)));
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

    isoValueShader_.onReload([this]() { invalidate(InvalidationLevel::InvalidResources); });
    shader_.onReload([this]() { invalidate(InvalidationLevel::InvalidResources); });
}

std::tuple<ivec2, ivec2, ivec2, ivec2> ColorScaleLegend::getPositions(ivec2 dimensions) const {
    const float ticsWidth = ceil(axis_.majorTicks_.tickWidth_.get());
    const auto borderWidth = borderWidth_.get();

    const auto [position, legendSize] = [&]() {
        const auto initialPos = position_.get();
        const auto size = (axis_.orientation_ == AxisProperty::Orientation::Horizontal)
                              ? legendSize_.get()
                              : glm::yx(legendSize_.get());

        auto lpos = 0.5f * vec2{size + 2 * ivec2{margin_}} +
                    initialPos * vec2{dimensions - size - 2 * ivec2{margin_}};

        return std::make_tuple(lpos, size);
    }();

    // define the legend corners
    const ivec2 bottomLeft = position - vec2{legendSize} * vec2{0.5};
    const ivec2 bottomRight(bottomLeft.x + legendSize.x, bottomLeft.y);
    const ivec2 topLeft(bottomLeft.x, bottomLeft.y + legendSize.y);
    const ivec2 topRight(bottomRight.x, topLeft.y);

    ivec2 axisStart{0};
    ivec2 axisEnd{0};

    if (axis_.getOrientation() == AxisProperty::Orientation::Horizontal) {
        if (axis_.getMirrored()) {
            axisStart = topLeft + ivec2(ticsWidth / 2, 0) + ivec2(-borderWidth, borderWidth);
            axisEnd = topRight - ivec2(ticsWidth / 2, 0) + ivec2(borderWidth);
        } else {
            axisStart = bottomLeft + ivec2(ticsWidth / 2, 0) - ivec2(borderWidth);
            axisEnd = bottomRight - ivec2(ticsWidth / 2, 0) + ivec2(borderWidth, -borderWidth);
        }
    } else {
        if (axis_.getMirrored()) {
            axisStart = bottomLeft + ivec2(0, ticsWidth / 2) - ivec2(borderWidth);
            axisEnd = topLeft - ivec2(0, ticsWidth / 2) + ivec2(-borderWidth, borderWidth);
        } else {
            axisStart = bottomRight + ivec2(0, ticsWidth / 2) + ivec2(borderWidth, -borderWidth);
            axisEnd = topRight - ivec2(0, ticsWidth / 2) + ivec2(borderWidth);
        }
    }
    return {bottomLeft, legendSize, axisStart, axisEnd};
}

void ColorScaleLegend::initializeResources() {
    const auto isovalueCount = isotfComposite_.isovalues_.get().size();
    isoValueShader_.getGeometryShaderObject()->addShaderDefine(
        "MAX_ISOVALUE_COUNT", StrBuffer{"{}", std::max<size_t>(1, isovalueCount)});

    isoValueShader_.build();
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
        axis_.setRange(volumeInport_.getData()->dataMap.valueRange);
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
                        isotfComposite_);
    shader_.setUniform("legendOrientation",
                       (axis_.getOrientation() == AxisProperty::Orientation::Horizontal) ? 0 : 1);

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

        if (!isotfComposite_.isovalues_.get().empty() && isovalues_) {
            isoValueShader_.activate();

            MeshDrawerGL::DrawObject drawer(isovalueMesh_.getRepresentation<MeshGL>(),
                                            isovalueMesh_.getDefaultMeshInfo());

            if (axis_.getOrientation() == AxisSettings::Orientation::Horizontal) {
                isoValueShader_.setUniform("trafo", mat4(1.0f));
                isoValueShader_.setUniform("screenDim", vec2(view.z, view.w));
                isoValueShader_.setUniform("screenDimInv", 1.0f / vec2(view.z, view.w));
            } else {
                isoValueShader_.setUniform(
                    "trafo", glm::rotate(-glm::half_pi<float>(), vec3(0.0f, 0.0f, 1.0f)));
                isoValueShader_.setUniform("screenDim", vec2(view.w, view.z));
                isoValueShader_.setUniform("screenDimInv", 1.0f / vec2(view.w, view.z));
            }
            utilgl::setUniforms(isoValueShader_, isotfComposite_, borderWidth_, axis_.color_,
                                triSize_);
            glDrawArraysInstanced(GL_POINTS, 0, 1,
                                  static_cast<GLsizei>(isotfComposite_.isovalues_.get().size()));
        }
    }

    axisRenderer_.render(dimensions, axisStart, axisEnd);

    utilgl::deactivateCurrentTarget();
}

std::vector<ButtonGroupProperty::Button> ColorScaleLegend::buttons() {
    return {
        {{std::nullopt, ":svgicons/axis-inside-left.svg", "Legend on the left with inside labels",
          [this] { setPlacement(Placement::InsideLeft); }},
         {std::nullopt, ":svgicons/axis-inside-top.svg", "Legend on the top with inside labels",
          [this] { setPlacement(Placement::InsideTop); }},
         {std::nullopt, ":svgicons/axis-inside-right.svg", "Legend on the right with inside labels",
          [this] { setPlacement(Placement::InsideRight); }},
         {std::nullopt, ":svgicons/axis-inside-bottom.svg",
          "Legend on the bottom with inside labels",
          [this] { setPlacement(Placement::InsideBottom); }},
         {std::nullopt, ":svgicons/axis-outside-left.svg", "Legend on the left with outside labels",
          [this] { setPlacement(Placement::OutsideLeft); }},
         {std::nullopt, ":svgicons/axis-outside-top.svg", "Legend on the top with outside labels",
          [this] { setPlacement(Placement::OutsideTop); }},
         {std::nullopt, ":svgicons/axis-outside-right.svg",
          "Legend on the right with outside labels",
          [this] { setPlacement(Placement::OutsideRight); }},
         {std::nullopt, ":svgicons/axis-outside-bottom.svg",
          "Legend on the bottom with outside labels",
          [this] { setPlacement(Placement::OutsideBottom); }}}};
}

void ColorScaleLegend::setPlacement(Placement placement) {

    auto setPositions = [&](const vec2& legendPos, const vec2& anchor) {
        axis_.labelSettings_.font_.anchorPos_.set(anchor);
        axis_.captionSettings_.font_.anchorPos_.set(anchor);
        position_.set(legendPos);
    };

    switch (placement) {
        case Placement::OutsideLeft:
        default:
            axis_.set(AxisProperty::Orientation::Vertical, true);
            setPositions(vec2{0.0f, 0.5f}, vec2{1.0f, 0.0f});
            break;
        case Placement::OutsideTop:
            axis_.set(AxisProperty::Orientation::Horizontal, true);
            setPositions(vec2{0.5f, 1.0f}, vec2{0.0f, -1.0f});
            break;
        case Placement::OutsideRight:
            axis_.set(AxisProperty::Orientation::Vertical, false);
            setPositions(vec2{1.0f, 0.5f}, vec2{-1.0f, 0.0f});
            break;
        case Placement::OutsideBottom:
            axis_.set(AxisProperty::Orientation::Horizontal, false);
            setPositions(vec2{0.5f, 0.0f}, vec2{0.0f, 1.0f});
            break;
        case Placement::InsideLeft:
            axis_.set(AxisProperty::Orientation::Vertical, false);
            setPositions(vec2{0.0f, 0.5f}, vec2{-1.0f, 0.0f});
            break;
        case Placement::InsideTop:
            axis_.set(AxisProperty::Orientation::Horizontal, false);
            setPositions(vec2{0.5f, 1.0f}, vec2{0.0f, 1.0f});
            break;
        case Placement::InsideRight:
            axis_.set(AxisProperty::Orientation::Vertical, true);
            setPositions(vec2{1.0f, 0.5f}, vec2{1.0f, 0.0f});
            break;
        case Placement::InsideBottom:
            axis_.set(AxisProperty::Orientation::Horizontal, true);
            setPositions(vec2{0.5f, 0.0f}, vec2{0.0f, -1.0f});
            break;
    }
}

}  // namespace plot

}  // namespace inviwo
