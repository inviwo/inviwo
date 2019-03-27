/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2019 Inviwo Foundation
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

#include <modules/plottinggl/processors/parallelcoordinates/parallelcoordinates.h>
#include <modules/plottinggl/processors/parallelcoordinates/pcpaxissettings.h>
#include <modules/plottinggl/plottingglmodule.h>

#include <inviwo/core/datastructures/image/layer.h>
#include <inviwo/core/datastructures/image/imageram.h>
#include <inviwo/core/interaction/events/mouseevent.h>
#include <inviwo/core/interaction/events/pickingevent.h>
#include <inviwo/core/rendering/meshdrawerfactory.h>
#include <inviwo/core/util/imagesampler.h>
#include <inviwo/core/util/rendercontext.h>
#include <inviwo/core/properties/boolproperty.h>

#include <modules/base/algorithm/dataminmax.h>

#include <modules/opengl/image/imagegl.h>
#include <modules/opengl/openglutils.h>
#include <modules/opengl/rendering/meshdrawergl.h>
#include <modules/opengl/shader/shaderutils.h>
#include <modules/opengl/texture/textureutils.h>
#include <modules/opengl/buffer/buffergl.h>
#include <inviwo/core/io/datareaderfactory.h>

#include <modules/plotting/utils/statsutils.h>
#include <modules/plotting/datastructures/dataframeutil.h>
#include <inviwo/core/util/utilities.h>
#include <inviwo/core/util/zip.h>

namespace inviwo {

namespace plot {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo ParallelCoordinates::processorInfo_{
    "org.inviwo.ParallelCoordinates",  // Class identifier
    "Parallel Coordinates",            // Display name
    "Plotting",                        // Category
    CodeState::Stable,                 // Code state
    "GL, Plotting",                    // Tags
};
const ProcessorInfo ParallelCoordinates::getProcessorInfo() const { return processorInfo_; }

ParallelCoordinates::ParallelCoordinates()
    : Processor()
    , dataFrame_{"dataFrame"}
    , brushingAndLinking_{"brushingAndLinking"}
    , outport_{"outport"}

    , axisProperties_{"axisProps_", "Axis"}
    , selectedColorAxis_{"selectedColorAxis", "Selected Color Axis", dataFrame_, false, 1}

    , tf_{"tf", "Line Color",
          TransferFunction{
              {{0.0, vec4{1, 0, 0, 1}}, {0.5, vec4{1, 1, 0, 1}}, {1.0, vec4{0, 1, 0, 1}}}}}

    , lineSettings_{"lines", "Line Settings"}
    , blendMode_("blendMode", "Blend Mode",
                 {{"additive", "Additive", BlendMode::Additive},
                  {"subractive", "Subractive", BlendMode::Sutractive},
                  {"regular", "Regular", BlendMode::Regular},
                  {"noblend", "None", BlendMode::None}},
                 2)
    , falllofPower_("falllofPower", "Falloff Power", 1.0f, 0.01f, 3.f, 0.01f)
    , lineWidth_("lineWidth", "Line Width", 7.0f, 1.0f, 20.0f)
    , selectedLineWidth_("selectedLineWidth", "Selected Line Width", 10.0f, 1.0f, 20.0f)
    , showFiltered_("showFiltered", "Show Filtered", false)
    , filterColor_("filterColor", "Filter Color", vec3(.2f, .2f, .2f), vec3(0.0f), vec3(1.0f),
                   vec4(0.01f), InvalidationLevel::InvalidOutput, PropertySemantics::Color)
    , filterAlpha_("filterAlpha", "Filter Alpha", 0.75f)
    , filterIntensity_("filterIntensity", "Filter Mixing", 0.7f, 0.01f, 1.0f, 0.001f)

    , captionSettings_("captions", "Caption Settings", "Montserrat-Regular", 24, 0.0f,
                       vec2{0.0f, -1.0f})
    , captionPosition_("position", "Position",
                       {{"none", "None", LabelPosition::None},
                        {"above", "Above", LabelPosition::Above},
                        {"below", "Below", LabelPosition::Below}},
                       1)
    , captionOffset_("offset", "Offset", 15.0f, -50.0f, 50.0f, 0.1f)
    , captionColor_("color", "Color", vec4(.0, .0f, .0f, 1.0f), vec4(0.0f), vec4(1.0f), vec4(0.01f),
                    InvalidationLevel::InvalidOutput, PropertySemantics::Color)

    , labelSettings_("labels", "Label Settings", "Montserrat-Regular", 20, 0.0f, vec2{-1.0f, 0.0f})
    , showLabels_("show", "Display min/max", true)
    , labelOffset_("offset", "Offset", 15.0f, -50.0, 50.0f)
    , labelFormat_("format", "Format", "%.4f")
    , labelColor_("color", "Color", vec4(.0, .0f, .0f, 1), vec4(0.0f), vec4(1.0f), vec4(0.01f),
                  InvalidationLevel::InvalidOutput, PropertySemantics::Color)

    , axesSettings_("axesSettings", "Axes Settings")
    , axisSize_("axisSize", "Size", 4.0f, 0.0f, 50.0f, 0.01f)
    , axisColor_("axisColor", "Color", vec4(.3f, .3f, .3f, 1), vec4(0.0f), vec4(1.0f), vec4(0.01f),
                 InvalidationLevel::InvalidOutput, PropertySemantics::Color)
    , axisHoverColor_("axisHoverColor", "Hover Color", vec4(.8f, .8f, .8f, 1), vec4(0.0f),
                      vec4(1.0f), vec4(0.01f), InvalidationLevel::InvalidOutput,
                      PropertySemantics::Color)
    , axisSelectedColor_("axisSelectedColor", "Selected Color", vec4(.8f, .2f, .2f, 1), vec4(0.0f),
                         vec4(1.0f), vec4(0.01f), InvalidationLevel::InvalidOutput,
                         PropertySemantics::Color)
    , handleSize_("handleSize", "Handle Size", 20.0f, 15.0f, 100.0f)
    , handleColor_("handleColor", "Handle Color (Not filtering)", vec4(.4f, .4f, .4f, 1),
                   vec4(0.0f), vec4(1.0f), vec4(0.01f), InvalidationLevel::InvalidOutput,
                   PropertySemantics::Color)
    , handleFilteredColor_("handleFilteredColor", "Handle Color (When filtering)",
                           vec4(.6f, .6f, .6f, 1), vec4(0.0f), vec4(1.0f), vec4(0.01f),
                           InvalidationLevel::InvalidOutput, PropertySemantics::Color)

    , margins_("margins", "Margins", 0, 0, 0, 0)
    , autoMargins_("autoMargins", "Auto Margins", true)
    , resetHandlePositions_("resetHandlePositions", "Reset Handle Positions")

    , linePicking_(this, 1, [&](PickingEvent* p) { linePicked(p); })
    , axisPicking_(this, 1,
                   [&](PickingEvent* p) { axisPicked(p, p->getPickedId(), PickType::Axis); })
    , lineShader_("pcp_lines.vert", "pcp_lines.geom", "pcp_lines.frag", false)
    , lines_{}
    , brushingDirty_(true)  // needs to be true after deserialization
{
    addPort(dataFrame_);
    addPort(brushingAndLinking_);
    addPort(outport_);

    addProperty(axisProperties_);
    addProperty(selectedColorAxis_);
    addProperty(tf_);

    addProperty(lineSettings_);
    lineSettings_.addProperty(blendMode_);
    lineSettings_.addProperty(falllofPower_);
    lineSettings_.addProperty(lineWidth_);
    lineSettings_.addProperty(selectedLineWidth_);
    lineSettings_.addProperty(showFiltered_);
    lineSettings_.addProperty(filterColor_);
    lineSettings_.addProperty(filterAlpha_);
    lineSettings_.addProperty(filterIntensity_);
    lineSettings_.setCollapsed(true);

    addProperty(captionSettings_);
    captionSettings_.insertProperty(0, captionPosition_);
    captionSettings_.addProperty(captionOffset_);
    captionSettings_.addProperty(captionColor_);
    captionPosition_.onChange([&]() {
        auto pos = captionSettings_.anchorPos_.get();
        auto offset = captionOffset_.get();
        if (captionPosition_ == ParallelCoordinates::LabelPosition::Above) {
            pos.y = -glm::abs(pos.y);
            offset = glm::abs(offset);
        } else if (captionPosition_ == ParallelCoordinates::LabelPosition::Below) {
            pos.y = glm::abs(pos.y);
            offset = -glm::abs(offset);
        }

        captionSettings_.anchorPos_.set(pos);
        captionOffset_.set(offset);
    });
    captionSettings_.setCollapsed(true);

    addProperty(labelSettings_);
    labelSettings_.insertProperty(0, showLabels_);
    labelSettings_.addProperty(labelOffset_);
    labelSettings_.addProperty(labelFormat_);
    labelSettings_.addProperty(labelColor_);
    labelSettings_.setCollapsed(true);

    addProperty(axesSettings_);
    axesSettings_.addProperty(axisSize_);
    axesSettings_.addProperty(axisColor_);
    axesSettings_.addProperty(axisHoverColor_);
    axesSettings_.addProperty(axisSelectedColor_);
    axesSettings_.addProperty(handleSize_);
    axesSettings_.addProperty(handleColor_);
    axesSettings_.addProperty(handleFilteredColor_);
    axesSettings_.setCollapsed(true);

    addProperty(margins_);
    margins_.insertProperty(0, autoMargins_);
    const auto makeMarginReadonly = [&]() {
        margins_.left_.setReadOnly(autoMargins_);
        margins_.bottom_.setReadOnly(autoMargins_);
        margins_.right_.setReadOnly(autoMargins_);
        margins_.top_.setReadOnly(autoMargins_);
    };
    makeMarginReadonly();
    autoMargins_.onChange(makeMarginReadonly);
    margins_.setCollapsed(true);

    addProperty(resetHandlePositions_);

    captionSettings_.lineSpacing_.setVisible(false);
    labelSettings_.lineSpacing_.setVisible(false);

    {
        auto vs = lineShader_.getVertexShaderObject();
        vs->clearInDeclarations();
        vs->addInDeclaration("in_Vertex", buffertraits::PositionsBuffer1D::bi().location, "float");
        vs->addInDeclaration("in_Picking", buffertraits::PickingBuffer::bi().location, "uint");
        vs->addInDeclaration("in_ScalarMeta", buffertraits::ScalarMetaBuffer::bi().location,
                             "float");
        vs->addShaderDefine("NUMBER_OF_AXIS", toString(1));

        lineShader_.onReload([&]() { this->invalidate(InvalidationLevel::InvalidOutput); });
        lineShader_.build();
    }

    dataFrame_.onChange([&]() { createOrUpdateProperties(); });

    resetHandlePositions_.onChange([&]() {
        for (auto& axis : axes_) {
            axis.pcp->moveHandle(true, std::numeric_limits<double>::max());
            axis.pcp->moveHandle(false, std::numeric_limits<double>::lowest());
        }
    });

    setAllPropertiesCurrentStateAsDefault();
}

void ParallelCoordinates::autoAdjustMargins() {
    if (enabledAxes_.empty() || isDragging_) return;

    const auto dim = outport_.getDimensions();

    auto llMargin = margins_.getLowerLeftMargin();
    auto urMargin = margins_.getUpperRightMargin();

    do {
        std::pair<vec2, vec2> bRect = {vec2{dim} * 0.5f, vec2{dim} * 0.5f};
        for (auto& axis : axes_) {
            if (axis.pcp->isChecked()) {
                const auto ap = axisPos(axis.pcp->columnId());
                const auto axisBRect = axis.axisRender->boundingRect(ap.first, ap.second);
                bRect.first = glm::min(bRect.first, axisBRect.first);
                bRect.second = glm::max(bRect.second, axisBRect.second);
            }
        }

        const float padding = 5.0f;
        const auto rect = margins_.getRect(vec2{dim} - 1.0f);
        llMargin = rect.first - glm::floor(bRect.first) + vec2{padding};
        urMargin = glm::ceil(bRect.second) - rect.second + vec2{padding};

        margins_.setLowerLeftMargin(llMargin);
        margins_.setUpperRightMargin(urMargin);

    } while (margins_.getLowerLeftMargin() != llMargin ||
             margins_.getUpperRightMargin() != urMargin);
}

ParallelCoordinates::~ParallelCoordinates() = default;

void ParallelCoordinates::process() {
    const auto dims = outport_.getDimensions();

    enabledAxesModified_ |= [&]() {
        std::vector<ColumnAxis*> enabledAxes{enabledAxes_};
        util::erase_remove_if(enabledAxes, [](auto axis) { return !axis->pcp->isChecked(); });
        for (auto& axis : axes_) {
            if (axis.pcp->isChecked() && !util::contains(enabledAxes, &axis)) {
                enabledAxes.push_back(&axis);
            }
        }
        const auto modified = enabledAxes != enabledAxes_;
        enabledAxes_.swap(enabledAxes);
        return modified;
    }();

    if (brushingDirty_) updateBrushing();
    if (selectedColorAxis_.isModified() || dataFrame_.isChanged()) {
        buildLineMesh();
    } else if (enabledAxesModified_) {
        buildLineIndices();
    } else if (brushingAndLinking_.isChanged() || axisProperties_.isModified()) {
        partitionLines();
    }
    if (autoMargins_ && (enabledAxesModified_ || captionSettings_.isModified() ||
                         labelSettings_.isModified() || axesSettings_.isModified())) {
        autoAdjustMargins();
    }

    const vec4 backgroundColor(blendMode_.get() == BlendMode::Sutractive ? 1.0f : 0.0f);
    utilgl::ClearColor clearColor(backgroundColor);
    utilgl::activateAndClearTarget(outport_, ImageType::ColorPicking);
    utilgl::GlBoolState depthTest(GL_DEPTH_TEST, false);

    drawLines(dims);
    drawAxis(dims);
    drawHandles(dims);

    utilgl::deactivateCurrentTarget();
}

void ParallelCoordinates::createOrUpdateProperties() {
    axes_.clear();
    for (auto& p : axisProperties_.getProperties()) {
        p->setVisible(false);
    }

    if (!dataFrame_.hasData()) return;
    auto data = dataFrame_.getData();
    if (!data->getNumberOfRows()) return;
    if (!data->getNumberOfColumns()) return;

    axisPicking_.resize(data->getNumberOfColumns());
    for (size_t i = 0; i < data->getNumberOfColumns(); i++) {
        auto c = data->getColumn(i);
        std::string displayName = c->getHeader();
        std::string identifier = util::stripIdentifier(displayName);
        // Create axis for filtering
        auto prop = [&]() -> PCPAxisSettings* {
            if (auto p = axisProperties_.getPropertyByIdentifier(identifier)) {
                if (auto pcasp = dynamic_cast<PCPAxisSettings*>(p)) {
                    return pcasp;
                }
                axisProperties_.removeProperty(identifier);
            }
            auto newProp = std::make_unique<PCPAxisSettings>(identifier, displayName, axes_.size());
            auto ptr = newProp.get();
            axisProperties_.addProperty(newProp.release());
            return ptr;
        }();
        prop->setParallelCoordinates(this);
        prop->setColumnId(axes_.size());
        prop->setVisible(true);
        prop->updateFromColumn(c);

        // Create axis for rendering
        auto renderer = std::make_unique<AxisRenderer>(*prop);
        renderer->setAxisPickingId(axisPicking_.getPickingId(i));

        auto slider = std::make_unique<glui::DoubleMinMaxPropertyWidget>(
            prop->range, *this, sliderWidgetRenderer_, ivec2{100, handleSize_.get()},
            glui::UIOrientation::Vertical);
        slider->setLabelVisible(false);
        slider->setShowGroove(false);

        slider->setPickingEventAction([this, id = axes_.size()](PickingEvent* e) {
            axisPicked(e, id, static_cast<PickType>(e->getPickedId() + 1));
        });

        axes_.push_back({prop, std::move(renderer), std::move(slider)});
    }
}

void ParallelCoordinates::buildLineMesh() {
    auto& mesh = lines_.mesh;

    for (auto& item : mesh.getBuffers()) {
        item.second->getEditableRepresentation<BufferRAM>()->clear();
    }

    const auto numberOfAxis = axes_.size();
    const auto numberOfLines = dataFrame_.getData()->getNumberOfRows();

    mesh.reserveSizeInVertexBuffer(numberOfAxis * numberOfLines);
    linePicking_.resize(numberOfLines);

    const auto metaAxisId = selectedColorAxis_.get();
    const auto metaAxes = axes_[glm::clamp(metaAxisId, 0, static_cast<int>(axes_.size()) - 1)].pcp;

    for (size_t i = 0; i < numberOfLines; i++) {
        const auto meta = static_cast<float>(metaAxes->getNormalizedAt(i));
        const auto picking = static_cast<uint32_t>(linePicking_.getPickingId(i));
        for (auto& axis : axes_) {
            mesh.addVertex(static_cast<float>(axis.pcp->getNormalizedAt(i)), picking, meta);
        }
    }

    lineShader_.getVertexShaderObject()->addShaderDefine("NUMBER_OF_AXIS", toString(numberOfAxis));
    lineShader_.build();

    buildLineIndices();
}

void ParallelCoordinates::buildLineIndices() {
    enabledAxesModified_ = false;
    const auto numberOfAxis = axes_.size();
    const auto numberOfEnabledAxis = enabledAxes_.size();
    const auto numberOfLines = dataFrame_.getData()->getNumberOfRows();

    lines_.sizes.resize(numberOfLines, static_cast<int>(numberOfEnabledAxis));
    if (!lines_.sizes.empty() && lines_.sizes.front() != static_cast<int>(numberOfEnabledAxis)) {
        std::fill(lines_.sizes.begin(), lines_.sizes.end(),
                  static_cast<GLsizei>(numberOfEnabledAxis));
    }

    auto& indices = lines_.indices.getEditableRAMRepresentation()->getDataContainer();

    indices.clear();
    indices.reserve(numberOfEnabledAxis * numberOfLines);
    for (size_t i = 0; i < numberOfLines; i++) {
        for (auto axis : enabledAxes_) {
            indices.push_back(static_cast<uint32_t>(i * numberOfAxis + axis->pcp->columnId()));
        }
    }

    lines_.starts.clear();
    if (!indices.empty()) {
        lines_.starts.reserve(numberOfLines);
        for (size_t i = 0; i < numberOfLines; i++) {
            lines_.starts.push_back(Lines::indexToOffset(i, numberOfEnabledAxis));
        }
    }

    buildAxisPositions();
    partitionLines();
}

void ParallelCoordinates::buildAxisPositions() {
    const auto numberOfAxis = axes_.size();
    const auto numberOfEnabledAxis = enabledAxes_.size();

    lines_.axisPositions.resize(numberOfAxis, 0.0f);
    for (auto&& item : util::enumerate(enabledAxes_)) {
        lines_.axisPositions[item.second()->pcp->columnId()] =
            item.first() / float(numberOfEnabledAxis - 1);
    }
}

void ParallelCoordinates::partitionLines() {
    const auto numberOfEnabledAxis = enabledAxes_.size();

    const auto iCol = dataFrame_.getData()->getIndexColumn();
    const auto& indexCol = iCol->getTypedBuffer()->getRAMRepresentation()->getDataContainer();

    const auto lastFilteredIt =
        std::partition(lines_.starts.begin(), lines_.starts.end(), [&](auto ind) {
            return brushingAndLinking_.isFiltered(
                indexCol[Lines::offsetToIndex(ind, numberOfEnabledAxis)]);
        });
    const auto lastRegularIt = std::partition(lastFilteredIt, lines_.starts.end(), [&](auto ind) {
        return !brushingAndLinking_.isSelected(
            indexCol[Lines::offsetToIndex(ind, numberOfEnabledAxis)]);
    });

    lines_.offsets[0] = 0;
    lines_.offsets[1] = std::distance(lines_.starts.begin(), lastFilteredIt);
    lines_.offsets[2] = std::distance(lines_.starts.begin(), lastRegularIt);
    lines_.offsets[3] = lines_.starts.size();
}

void ParallelCoordinates::drawAxis(size2_t size) {
    for (auto& axis : axes_) {
        if (!axis.pcp->isChecked()) continue;
        const auto ap = axisPos(axis.pcp->columnId());
        axis.axisRender->render(size, ap.first, ap.second);
    }
}

void ParallelCoordinates::drawHandles(size2_t size) {
    sliderWidgetRenderer_.setHoverColor(axisHoverColor_);

    for (auto& axis : axes_) {
        if (!axis.pcp->isChecked()) continue;

        const auto i = axis.pcp->columnId();
        const auto ap = axisPos(i);

        // Need to call setWidgetExtent twice, since getHandleWidth needs the extent width and
        // we need the handle width for the extent hight.
        axis.sliderWidget->setWidgetExtent(ivec2{handleSize_.get(), ap.second.y - ap.first.y});
        const auto handleWidth = axis.sliderWidget->getHandleWidth();
        axis.sliderWidget->setWidgetExtent(
            ivec2{handleSize_.get(), ap.second.y - ap.first.y + handleWidth});

        if (brushingAndLinking_.isColumnSelected(i)) {
            sliderWidgetRenderer_.setUIColor(axisSelectedColor_);
        } else if (axis.pcp->isFiltering()) {
            sliderWidgetRenderer_.setUIColor(handleFilteredColor_);
        } else {
            sliderWidgetRenderer_.setUIColor(handleColor_);
        }

        axis.sliderWidget->render(ivec2{ap.first} - ivec2{handleSize_.get(), handleWidth} / 2,
                                  size);
    }
}

void ParallelCoordinates::drawLines(size2_t size) {
    lineShader_.activate();

    auto state = [&]() {
        switch (blendMode_.get()) {
            case BlendMode::Additive:
                return std::make_tuple(
                    utilgl::GlBoolState(GL_DEPTH_TEST, false), utilgl::GlBoolState(GL_BLEND, true),
                    utilgl::BlendModeEquationState(GL_SRC_ALPHA, GL_ONE, GL_FUNC_ADD));
            case BlendMode::Sutractive:
                return std::make_tuple(
                    utilgl::GlBoolState(GL_DEPTH_TEST, false), utilgl::GlBoolState(GL_BLEND, true),
                    utilgl::BlendModeEquationState(GL_SRC_ALPHA, GL_ONE, GL_FUNC_REVERSE_SUBTRACT));
            case BlendMode::Regular:
                return std::make_tuple(utilgl::GlBoolState(GL_DEPTH_TEST, false),
                                       utilgl::GlBoolState(GL_BLEND, true),
                                       utilgl::BlendModeEquationState(
                                           GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_FUNC_ADD));
            case BlendMode::None:
            default:
                return std::make_tuple(
                    utilgl::GlBoolState(GL_DEPTH_TEST, false), utilgl::GlBoolState(GL_BLEND, false),
                    utilgl::BlendModeEquationState(GL_NONE, GL_NONE, GL_FUNC_ADD));
        };
    }();

    // Draw lines

    TextureUnitContainer unit;
    utilgl::bindAndSetUniforms(lineShader_, unit, tf_);

    bool enableBlending =
        (blendMode_.get() == BlendMode::Additive || blendMode_.get() == BlendMode::Sutractive ||
         blendMode_.get() == BlendMode::Regular);
    // pcp_common.glsl
    lineShader_.setUniform("spacing", margins_.getAsVec4());
    lineShader_.setUniform("dims", ivec2(size));
    // pcp_lines.vert
    lineShader_.setUniform("axisPositions", lines_.axisPositions.size(),
                           lines_.axisPositions.data());
    // pcp_lines.geom
    // lineWidth;

    // pcp_lines.frag
    lineShader_.setUniform("additiveBlend", enableBlending);
    lineShader_.setUniform("subtractiveBelnding", blendMode_.get() == BlendMode::Sutractive);
    lineShader_.setUniform("fallofPower", falllofPower_.get());
    lineShader_.setUniform("color", vec4{filterColor_.get(), filterAlpha_.get()});
    lineShader_.setUniform("filterIntensity", filterIntensity_.get());

    {
        auto meshGL = lines_.mesh.getRepresentation<MeshGL>();
        utilgl::Enable<MeshGL> enable{meshGL};
        lines_.indices.getRepresentation<BufferGL>()->bind();

        std::array<float, 3> width = {lineWidth_, lineWidth_, selectedLineWidth_};
        std::array<float, 3> mixColor = {filterIntensity_, 0.0f, 0.0f};
        std::array<float, 3> mixAlpha = {1.0, 0.0f, 0.0f};

        for (size_t i = showFiltered_ ? 0 : 1; i < lines_.offsets.size() - 1; ++i) {
            auto begin = lines_.offsets[i];
            auto end = lines_.offsets[i + 1];
            if (end == begin) continue;

            lineShader_.setUniform("lineWidth", width[i]);
            lineShader_.setUniform("mixColor", mixColor[i]);
            lineShader_.setUniform("mixAlpha", mixAlpha[i]);

            glMultiDrawElements(
                GL_LINE_STRIP, lines_.sizes.data() + begin, GL_UNSIGNED_INT,
                reinterpret_cast<const GLvoid* const*>(lines_.starts.data() + begin),
                static_cast<GLsizei>(end - begin));
        }

        if (hoveredLine_ != -1 && !brushingAndLinking_.isFiltered(hoveredLine_)) {
            lineShader_.setUniform("fallofPower", 0.5f * falllofPower_.get());

            glDrawElements(GL_LINE_STRIP, lines_.sizes[hoveredLine_], GL_UNSIGNED_INT,
                           reinterpret_cast<GLvoid*>(
                               Lines::indexToOffset(hoveredLine_, lines_.sizes[hoveredLine_])));
        }
    }
    lineShader_.deactivate();
}

void ParallelCoordinates::linePicked(PickingEvent* p) {
    if (auto df = dataFrame_.getData()) {
        // Show tooltip about current line
        if (p->getHoverState() == PickingHoverState::Move ||
            p->getHoverState() == PickingHoverState::Enter) {
            p->setToolTip(dataframeutil::createToolTipForRow(*df, p->getPickedId()));
            hoveredLine_ = static_cast<int>(p->getPickedId());
            invalidate(InvalidationLevel::InvalidOutput);

        } else {
            p->setToolTip("");
            hoveredLine_ = -1;
            invalidate(InvalidationLevel::InvalidOutput);
        }
    }

    if (p->getState() == PickingState::Updated && p->getPressState() == PickingPressState::Press &&
        p->getPressItem() == PickingPressItem::Primary) {

        auto iCol = dataFrame_.getData()->getIndexColumn();
        auto& indexCol = iCol->getTypedBuffer()->getRAMRepresentation()->getDataContainer();

        auto id = p->getPickedId();

        auto selection = brushingAndLinking_.getSelectedIndices();
        if (brushingAndLinking_.isSelected(indexCol[id])) {
            selection.erase(indexCol[id]);
        } else {
            selection.insert(indexCol[id]);
        }
        brushingAndLinking_.sendSelectionEvent(selection);

        p->markAsUsed();
        invalidate(InvalidationLevel::InvalidOutput);
    }
}

void ParallelCoordinates::axisPicked(PickingEvent* p, size_t pickedID, PickType pt) {

    if (p->getHoverState() == PickingHoverState::Enter) {
        hoveredAxis_ = static_cast<int>(pickedID);
        invalidate(InvalidationLevel::InvalidOutput);
    } else if (p->getHoverState() == PickingHoverState::Exit) {
        hoveredAxis_ = -1;
        invalidate(InvalidationLevel::InvalidOutput);
    }

    if (p->getPressState() == PickingPressState::Release &&
        p->getPressItem() == PickingPressItem::Primary &&
        p->getPressedGlobalPickingId() == p->getCurrentGlobalPickingId() &&
        glm::length2(p->getDeltaPressedPosition()) < 0.01 && pt != PickType::Lower &&
        pt != PickType::Upper) {

        auto selection = brushingAndLinking_.getSelectedColumns();
        if (brushingAndLinking_.isColumnSelected(pickedID)) {
            selection.erase(pickedID);
        } else {
            selection.insert(pickedID);
        }
        brushingAndLinking_.sendColumnSelectionEvent(selection);

        p->markAsUsed();
        invalidate(InvalidationLevel::InvalidOutput);
    }

    const auto swap = [&](size_t a, size_t b) {
        if (a < enabledAxes_.size() || b < enabledAxes_.size()) {
            std::swap(enabledAxes_[a], enabledAxes_[b]);
            p->markAsUsed();
            buildAxisPositions();
            enabledAxesModified_ = true;
            invalidate(InvalidationLevel::InvalidOutput);
        }
    };

    if (p->getPressState() == PickingPressState::Move &&
        p->getPressItems().count(PickingPressItem::Primary) && pt != PickType::Lower &&
        pt != PickType::Upper) {
        isDragging_ = true;

        auto delta = glm::abs(p->getDeltaPressedPosition());

        if (delta.x > delta.y) {
            const auto it = util::find(enabledAxes_, &axes_[pickedID]);
            if (it != enabledAxes_.end()) {
                const auto id = static_cast<size_t>(std::distance(enabledAxes_.begin(), it));
                if (id > 0 && pickedID > 0 &&
                    p->getPosition().x * p->getCanvasSize().x <
                        static_cast<float>(
                            axisPos(enabledAxes_[id - 1]->pcp->columnId()).first.x)) {
                    swap(id, id - 1);
                } else if (id + 1 < enabledAxes_.size() && pickedID + 1 < axes_.size() &&
                           p->getPosition().x * p->getCanvasSize().x >
                               static_cast<float>(
                                   axisPos(enabledAxes_[id + 1]->pcp->columnId()).first.x)) {
                    swap(id, id + 1);
                } else if (pickedID < axes_.size()) {
                    const auto rect = margins_.getRect(outport_.getDimensions());
                    lines_.axisPositions[pickedID] =
                        glm::clamp(float(p->getPosition().x * p->getCanvasSize().x - rect.first.x) /
                                       (rect.second.x - rect.first.x),
                                   0.0f, 1.0f);
                    invalidate(InvalidationLevel::InvalidOutput);
                }
            }
        } else {
            buildAxisPositions();
            invalidate(InvalidationLevel::InvalidOutput);
        }
    }
    if (p->getPressState() == PickingPressState::Release &&
        p->getPressItem() == PickingPressItem::Primary) {
        buildAxisPositions();
        isDragging_ = false;
        invalidate(InvalidationLevel::InvalidOutput);
    }
}

void ParallelCoordinates::updateBrushing(PCPAxisSettings&) { updateBrushing(); }

void ParallelCoordinates::updateBrushing() {
    brushingDirty_ = false;

    auto iCol = dataFrame_.getData()->getIndexColumn();
    auto& indexCol = iCol->getTypedBuffer()->getRAMRepresentation()->getDataContainer();

    const auto nRows = indexCol.size();

    std::vector<bool> brushed(nRows, false);

    for (auto& axis : axes_) {
        auto& brushedAxis = axis.pcp->getBrushed();
        for (size_t i = 0; i < nRows; ++i) {
            brushed[i] = brushed[i] || brushedAxis[i];
        }
    }

    std::unordered_set<size_t> brushedID;
    for (size_t i = 0; i < nRows; ++i) {
        if (brushed[i]) brushedID.insert(indexCol[i]);
    }
    brushingAndLinking_.sendFilterEvent(brushedID);
}

std::pair<size2_t, size2_t> ParallelCoordinates::axisPos(size_t columnId) const {
    const auto dim = outport_.getDimensions();
    const auto rect = margins_.getRect(vec2{dim} - 1.0f);
    const size2_t lowerLeft(rect.first);
    const size2_t upperRight(rect.second);
    
    const auto dx = columnId < lines_.axisPositions.size() ? lines_.axisPositions[columnId] : 0.0f;
    const auto x = static_cast<size_t>(dx * (upperRight.x - lowerLeft.x));
    const auto startPos = lowerLeft + size2_t(x, 0);
    const auto endPos = size2_t(lowerLeft.x + x, upperRight.y);

    return {startPos, endPos};
}

}  // namespace plot

}  // namespace inviwo