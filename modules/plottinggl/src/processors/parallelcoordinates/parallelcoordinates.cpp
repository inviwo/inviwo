/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2026 Inviwo Foundation
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

#include <inviwo/core/datastructures/bitset.h>
#include <inviwo/core/datastructures/buffer/buffer.h>
#include <inviwo/core/datastructures/buffer/bufferram.h>
#include <inviwo/core/datastructures/buffer/bufferramprecision.h>
#include <inviwo/core/datastructures/geometry/mesh.h>
#include <inviwo/core/datastructures/geometry/typedmesh.h>
#include <inviwo/core/interaction/events/pickingevent.h>
#include <inviwo/core/interaction/pickingmapper.h>
#include <inviwo/core/interaction/pickingstate.h>
#include <inviwo/core/io/serialization/deserializer.h>
#include <inviwo/core/io/serialization/serializer.h>
#include <inviwo/core/ports/datainport.h>
#include <inviwo/core/ports/imageport.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/processors/processorinfo.h>
#include <inviwo/core/processors/processorstate.h>
#include <inviwo/core/properties/boolcompositeproperty.h>
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/properties/compositeproperty.h>
#include <inviwo/core/properties/invalidationlevel.h>
#include <inviwo/core/properties/minmaxproperty.h>
#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/property.h>
#include <inviwo/core/properties/propertysemantics.h>
#include <inviwo/core/properties/marginproperty.h>
#include <inviwo/core/util/glmvec.h>
#include <inviwo/core/util/stdextensions.h>
#include <inviwo/core/util/utilities.h>
#include <inviwo/core/util/zip.h>
#include <inviwo/dataframe/properties/columnoptionproperty.h>
#include <inviwo/dataframe/properties/dataframecolormapproperty.h>
#include <inviwo/dataframe/util/dataframeutil.h>
#include <inviwo/dataframe/util/selectionutil.h>
#include <modules/brushingandlinking/datastructures/brushingaction.h>
#include <modules/brushingandlinking/ports/brushingandlinkingports.h>
#include <modules/fontrendering/properties/fontproperty.h>
#include <modules/fontrendering/util/fontutils.h>
#include <modules/opengl/buffer/buffergl.h>
#include <modules/opengl/geometry/meshgl.h>
#include <modules/opengl/openglutils.h>
#include <modules/opengl/shader/shader.h>
#include <modules/opengl/shader/shaderobject.h>
#include <modules/opengl/texture/textureunit.h>
#include <modules/opengl/texture/textureutils.h>
#include <modules/opengl/openglcapabilities.h>
#include <modules/plottinggl/processors/parallelcoordinates/pcpaxissettings.h>
#include <modules/plottinggl/utils/axisrenderer.h>
#include <modules/userinterfacegl/glui/element.h>
#include <modules/userinterfacegl/glui/renderer.h>
#include <modules/userinterfacegl/glui/widgets/doubleminmaxpropertywidget.h>

#include <algorithm>
#include <cstdint>
#include <iterator>
#include <limits>
#include <tuple>
#include <unordered_set>

#include <glm/common.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/vec2.hpp>


namespace inviwo::plot {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo ParallelCoordinates::processorInfo_{
    "org.inviwo.ParallelCoordinates",  // Class identifier
    "Parallel Coordinates",            // Display name
    "Plotting",                        // Category
    CodeState::Stable,                 // Code state
    "GL, Plotting",                    // Tags
    "This processor plots a given DataFrame using a Parallel Coordinate Plot."_help};

const ProcessorInfo& ParallelCoordinates::getProcessorInfo() const { return processorInfo_; }

ParallelCoordinates::ParallelCoordinates()
    : Processor()
    , dataFrame_{"dataFrame", "Data input for plotting"_help}
    , brushingAndLinking_{"brushingAndLinking", "Port for brushing & linking interactions"_help}
    , imageInport_("imageInport", "Background image (optional)"_help)
    , outport_{"outport", "Rendered image of the parallel coordinate plot"_help}

    , axisProperties_{"axisProps_", "Axis"}
    , colormap_("colormap", "Colormap", dataFrame_)
    , axisSelection_("axisSelction", "Axis Selection",
                     {{"single", "Single", AxisSelection::Single},
                      {"multiple", "Multiple", AxisSelection::Multiple},
                      {"none", "None", AxisSelection::None}},
                     1)
    , lineSettings_{"lines", "Line Settings"}
    , blendMode_("blendMode", "Blend Mode",
                 {{"additive", "Additive", BlendMode::Additive},
                  {"subtractive", "Subtractive", BlendMode::Subtractive},
                  {"regular", "Regular", BlendMode::Regular},
                  {"noblend", "None", BlendMode::None}},
                 2)
    , falloffPower_("falloffPower", "Falloff Power", 1.0f, 0.01f, 3.f, 0.01f)
    , lineWidth_("lineWidth", "Line Width", 7.0f, 1.0f, 20.0f)
    , selectedLine_("selectedLine", "Selected Line")
    , selectedLineWidth_("selectedLineWidth", "Line Width", 10.0f, 1.0f, 20.0f)
    , selectedLineColorOverride_("selectedLineColorOverride", "Override Line Color", true)
    , selectedLineColor_("selectedLineColor", "Color", vec4(1.0f, 0.769f, 0.247f, 1.0f), vec4(0.0f),
                         vec4(1.0f))
    , showFiltered_("showFiltered", "Show Filtered", false)
    , filterColor_("filterColor", "Filter Color", vec3(.2f, .2f, .2f), vec3(0.0f), vec3(1.0f),
                   vec4(0.01f), InvalidationLevel::InvalidOutput, PropertySemantics::Color)
    , filterAlpha_("filterAlpha", "Filter Alpha", 0.75f)
    , filterIntensity_("filterIntensity", "Filter Mixing", 0.7f, 0.01f, 1.0f, 0.001f)

    , captionSettings_("captions", "Caption Settings", font::FontType::Caption, 24, 0.0f,
                       vec2{0.0f, -1.0f})
    , captionPosition_("position", "Position",
                       {{"none", "None", LabelPosition::None},
                        {"above", "Above", LabelPosition::Above},
                        {"below", "Below", LabelPosition::Below}},
                       1)
    , captionOffset_("offset", "Offset", 15.0f, -50.0f, 50.0f, 0.1f)
    , captionColor_("color", "Color", vec4(.0, .0f, .0f, 1.0f), vec4(0.0f), vec4(1.0f), vec4(0.01f),
                    InvalidationLevel::InvalidOutput, PropertySemantics::Color)

    , labelSettings_("labels", "Label Settings", font::FontType::Label, 20, 0.0f, vec2{-1.0f, 0.0f})
    , showLabels_("show", "Display min/max", true)
    , labelOffset_("offset", "Offset", 15.0f, -50.0, 50.0f)
    , labelFormat_("format", "Format", defaultLabelFormat)
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
    , handlesVisible_("handlesVisible", "Handles Visible", true)
    , handleSize_("handleSize", "Handle Size", 20.0f, 15.0f, 100.0f)
    , handleColor_("handleColor", "Handle Color (Not filtering)", vec4(.4f, .4f, .4f, 1),
                   vec4(0.0f), vec4(1.0f), vec4(0.01f), InvalidationLevel::InvalidOutput,
                   PropertySemantics::Color)
    , handleFilteredColor_("handleFilteredColor", "Handle Color (When filtering)",
                           vec4(.6f, .6f, .6f, 1), vec4(0.0f), vec4(1.0f), vec4(0.01f),
                           InvalidationLevel::InvalidOutput, PropertySemantics::Color)

    , margins_("margins", "Margins")
    , includeLabelsInMargin_("includeLabelsInMargins", "Include labels", true)
    , resetHandlePositions_("resetHandlePositions", "Reset Handle Positions")
    , boxSelectionProperty_{"boxSelection", "Box Selection/Filtering"}
    , boxSelectionMode_{"boxSelectionMode",
                        "Selection Mode",
                        {{"lines", "Lines", BoxSelectionMode::Lines},
                         {"dataValues", "Data Values (on Columns)", BoxSelectionMode::DataValues}}}

    , boxSelectionRenderer_{}
    , boxSelection_{boxSelectionProperty_,
                    [&](dvec2 screenPos, const size2_t& /*dims*/) { return screenPos; },
                    MouseButton::Left}

    , linePicking_(this, 1, [&](PickingEvent* p) { linePicked(p); })
    , axisPicking_(this, 1,
                   [&](PickingEvent* p) {
                       axisPicked(p, static_cast<std::uint32_t>(p->getPickedId()), PickType::Axis);
                   })
    , lineShader_("pcp_lines.vert", "pcp_lines.geom", "pcp_lines.frag", Shader::Build::No)
    , lines_{}
    , marginsInternal_(0.0f, 0.0f)
    , brushingDirty_{true}  // needs to be true after deserialization
    , rangesDirty_{true} {
    addPort(dataFrame_);
    addPort(brushingAndLinking_);
    addPort(imageInport_);
    addPort(outport_);

    imageInport_.setOptional(true);

    addProperties(axisProperties_, colormap_, axisSelection_, boxSelectionProperty_);
    boxSelectionProperty_.insertProperty(0, boxSelectionMode_);
    boxSelectionProperty_.mode_.setVisible(false);
    boxSelectionProperty_.setCollapsed(true);

    selectedLineColor_.setSemantics(PropertySemantics::Color);
    selectedLineColorOverride_.addProperty(selectedLineColor_);
    selectedLine_.addProperties(selectedLineWidth_, selectedLineColorOverride_);
    selectedLine_.setCollapsed(true);

    addProperty(lineSettings_);
    lineSettings_.addProperties(blendMode_, falloffPower_, lineWidth_, selectedLine_, showFiltered_,
                                filterColor_, filterAlpha_, filterIntensity_);
    lineSettings_.setCollapsed(true);

    addProperty(captionSettings_);
    captionSettings_.insertProperty(0, captionPosition_);
    captionSettings_.addProperties(captionOffset_, captionColor_);
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
    labelSettings_.addProperties(labelOffset_, labelFormat_, labelColor_);
    labelSettings_.setCollapsed(true);

    addProperty(axesSettings_);
    axesSettings_.addProperty(axisSize_);
    axesSettings_.addProperties(axisColor_, axisHoverColor_, axisSelectedColor_, handlesVisible_,
                                handleSize_, handleColor_, handleFilteredColor_);
    axesSettings_.setCollapsed(true);

    addProperty(margins_);
    margins_.insertProperty(0, includeLabelsInMargin_);
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
        vs->addShaderDefine("NUMBER_OF_AXIS", fmt::to_string(1));

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

    util::for_each_in_tuple([&](auto& e) { boxSelectionProperty_.addProperty(e); },
                            boxSelection_.properties());
    boxSelectionCallback_ = boxSelection_.addEventCallback(
        [this](AxisRangeEventState state, AxisRangeInteraction interaction,
               AxisRangeInteractionMode mode, std::optional<std::array<dvec2, 2>> rect) {
            boxSelection(state, interaction, mode, rect);
        });

    setAllPropertiesCurrentStateAsDefault();
}

void ParallelCoordinates::adjustMargins() {
    if (enabledAxes_.empty()) return;

    if (includeLabelsInMargin_) {
        const auto dim = outport_.getDimensions();

        vec2 llMargin(0);
        vec2 urMargin(0);

        marginsInternal_.first = margins_.getLowerLeftMargin();
        marginsInternal_.second = margins_.getUpperRightMargin();

        std::pair<vec2, vec2> bRect = {vec2{dim} * 0.5f, vec2{dim} * 0.5f};
        for (auto& axis : axes_) {
            if (axis.pcp->isChecked()) {
                const auto ap = axisPos(axis.pcp->columnId());
                axis.pcp->update(axis.axisRender->getData());
                const auto axisBRect = axis.axisRender->boundingRect(ap.first, ap.second);
                bRect.first = glm::min(bRect.first, axisBRect.first);
                bRect.second = glm::max(bRect.second, axisBRect.second);
            }
        }

        const auto rect = getDisplayRect(vec2{dim} - 1.0f);
        llMargin = rect.first - glm::floor(bRect.first);
        urMargin = glm::ceil(bRect.second) - rect.second;

        marginsInternal_.first = llMargin + margins_.getLowerLeftMargin();
        marginsInternal_.second = urMargin + margins_.getUpperRightMargin();
    } else {
        marginsInternal_.first = margins_.getLowerLeftMargin();
        marginsInternal_.second = margins_.getUpperRightMargin();
    }
}

ParallelCoordinates::~ParallelCoordinates() = default;

void ParallelCoordinates::process() {
    const utilgl::ClearColor clearColor{vec4{0.0}};
    utilgl::activateTargetAndClearOrCopySource(outport_, imageInport_);

    if (axes_.empty()) {
        // Nothing render, just draw the background
        utilgl::deactivateCurrentTarget();
        return;
    }
    const auto dims = outport_.getDimensions();

    enabledAxesModified_ |= [&]() {
        std::vector<size_t> enabledAxes{enabledAxes_};
        std::erase_if(enabledAxes,
                      [&](auto id) { return id >= axes_.size() || !axes_[id].pcp->isChecked(); });
        for (const auto& axis : axes_) {
            if (axis.pcp->isChecked() && !util::contains(enabledAxes, axis.pcp->columnId())) {
                enabledAxes.push_back(axis.pcp->columnId());
            }
        }
        const auto modified = enabledAxes != enabledAxes_;
        enabledAxes_.swap(enabledAxes);
        return modified;
    }();

    if (brushingDirty_) updateBrushing();
    if (rangesDirty_ || colormap_.isModified() || dataFrame_.isChanged()) {
        buildLineMesh();
    } else if (enabledAxesModified_) {
        buildLineIndices();
    } else if (brushingAndLinking_.isChanged() || axisProperties_.isModified()) {
        partitionLines();
    }
    if ((!isDragging_ || enabledAxesModified_) &&
        (margins_.isModified() || includeLabelsInMargin_.isModified() || enabledAxesModified_ ||
         captionSettings_.isModified() || labelSettings_.isModified() ||
         axesSettings_.isModified())) {
        adjustMargins();
    }

    const utilgl::GlBoolState depthTest(GL_DEPTH_TEST, false);

    drawLines(dims);
    drawAxis(dims);
    drawHandles(dims);

    plot::BoxSelectionData sel;
    boxSelectionProperty_.update(sel);
    boxSelectionRenderer_.render(boxSelection_.getDragRectangle(), dims, sel);

    utilgl::deactivateCurrentTarget();

    enabledAxesModified_ = false;
}

void ParallelCoordinates::createOrUpdateProperties() {
    axes_.clear();

    std::unordered_set<Property*> previousProperties(axisProperties_.getProperties().begin(),
                                                     axisProperties_.getProperties().end());

    if (!dataFrame_.hasData()) return;
    auto data = dataFrame_.getData();
    if (!data->getNumberOfRows()) return;
    if (!data->getNumberOfColumns()) return;

    const auto nColumns = data->getNumberOfColumns();

    updating_ = true;
    axisPicking_.resize(nColumns);
    lines_.axisFlipped.resize(nColumns);

    for (auto [columnIndex, column] : util::enumerate(*data)) {
        const auto& displayName = column->getHeader();
        const auto identifier = util::stripIdentifier(displayName);

        // Create axis for filtering
        auto prop = [&, id = columnIndex]() -> PCPAxisSettings* {
            if (auto p = axisProperties_.getPropertyByIdentifier(identifier)) {
                if (auto pcasp = dynamic_cast<PCPAxisSettings*>(p)) {
                    return pcasp;
                }
                axisProperties_.removeProperty(identifier);
            }

            auto prop = std::make_unique<PCPAxisSettings>(identifier, displayName, id);
            auto ptr = prop.get();
            axisProperties_.addProperty(std::move(prop));
            return ptr;
        }();

        previousProperties.erase(prop);
        prop->setColumnId(static_cast<uint32_t>(columnIndex));
        prop->setVisible(true);

        // Create axis for rendering
        auto renderer = std::make_unique<AxisRenderer>();
        renderer->setAxisPickingId(axisPicking_.getPickingId(columnIndex));

        auto slider = std::make_unique<glui::DoubleMinMaxPropertyWidget>(
            prop->range, *this, sliderWidgetRenderer_, ivec2{100, handleSize_.get()},
            glui::UIOrientation::Vertical);
        slider->setLabelVisible(false);
        slider->setShowGroove(false);
        slider->setFlipped(prop->invertRange);

        slider->setPickingEventAction(
            [this, columnId = static_cast<uint32_t>(columnIndex)](PickingEvent* e) {
                axisPicked(e, columnId, static_cast<PickType>(e->getPickedId() + 1));
            });

        prop->invertRange.onChange([this, id = columnIndex, s = slider.get(), prop]() {
            s->setFlipped(prop->invertRange);
            lines_.axisFlipped[id] = static_cast<int>(prop->invertRange);
        });
        // initialize corresponding flipped flag for the line shader
        lines_.axisFlipped[axes_.size()] = static_cast<int>(prop->invertRange);

        axes_.push_back({prop, std::move(renderer), std::move(slider)});
    }

    for (auto& axis : axes_) {
        axis.pcp->update(data);
        axis.pcp->setParallelCoordinates(this);
    }

    // Remove properties for axes that doesn't exist in the current dataframe
    for (auto prop : previousProperties) {
        axisProperties_.removeProperty(prop);
    }

    updating_ = false;
    updateBrushing();
}

void ParallelCoordinates::buildLineMesh() {
    rangesDirty_ = false;
    auto& mesh = lines_.mesh;

    for (auto& item : mesh.getBuffers()) {
        item.second->getEditableRepresentation<BufferRAM>()->clear();
    }

    const auto numberOfAxis = axes_.size();
    const auto numberOfLines = dataFrame_.getData()->getNumberOfRows();

    mesh.reserveSizeInVertexBuffer(numberOfAxis * numberOfLines);
    linePicking_.resize(numberOfLines);

    const auto metaAxisId = colormap_.selectedColorAxis.get();
    const auto metaAxes = axes_[glm::clamp(metaAxisId, 0, static_cast<int>(axes_.size()) - 1)].pcp;

    for (size_t i = 0; i < numberOfLines; i++) {
        const auto meta = static_cast<float>(metaAxes->getNormalizedAt(i));
        const auto picking = static_cast<uint32_t>(linePicking_.getPickingId(i));
        for (auto& axis : axes_) {
            mesh.addVertex(static_cast<float>(axis.pcp->getNormalizedAt(i)), picking, meta);
        }
    }

    lineShader_.getVertexShaderObject()->addShaderDefine("NUMBER_OF_AXIS",
                                                         fmt::to_string(numberOfAxis));
    lineShader_.build();

    buildLineIndices();
}

void ParallelCoordinates::buildLineIndices() {
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
        for (auto id : enabledAxes_) {
            indices.push_back(static_cast<uint32_t>(i * numberOfAxis + id));
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
        lines_.axisPositions[item.second()] = item.first() / float(numberOfEnabledAxis - 1);
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
        const auto idx = indexCol[Lines::offsetToIndex(ind, numberOfEnabledAxis)];
        return !brushingAndLinking_.isSelected(idx) && !brushingAndLinking_.isHighlighted(idx);
    });
    const auto lastSelectedIt = std::partition(lastRegularIt, lines_.starts.end(), [&](auto ind) {
        const auto idx = indexCol[Lines::offsetToIndex(ind, numberOfEnabledAxis)];
        return brushingAndLinking_.isSelected(idx);
    });

    lines_.offsets[0] = 0;
    lines_.offsets[1] = std::distance(lines_.starts.begin(), lastFilteredIt);
    lines_.offsets[2] = std::distance(lines_.starts.begin(), lastRegularIt);
    lines_.offsets[3] = std::distance(lines_.starts.begin(), lastSelectedIt);
    lines_.offsets[4] = lines_.starts.size();
}

void ParallelCoordinates::drawAxis(size2_t size) {
    for (auto& axis : axes_) {
        if (!axis.pcp->isChecked()) continue;
        auto ap = axisPos(axis.pcp->columnId());
        if (axis.pcp->dataModified()) axis.pcp->update(axis.axisRender->getData());
        if (axis.pcp->invertRange) std::swap(ap.first, ap.second);
        axis.axisRender->render(size, ap.first, ap.second);
    }
}

void ParallelCoordinates::drawHandles(size2_t size) {
    if (!handlesVisible_) return;

    sliderWidgetRenderer_.setHoverColor(axisHoverColor_);

    for (auto& axis : axes_) {
        if (!axis.pcp->isChecked()) continue;

        const auto ap = axisPos(axis.pcp->columnId());

        // Need to call setWidgetExtent twice, since getHandleWidth needs the extent width and
        // we need the handle width for the extent height.
        axis.sliderWidget->setWidgetExtent(ivec2{handleSize_.get(), ap.second.y - ap.first.y});
        const auto handleWidth = axis.sliderWidget->getHandleWidth();
        axis.sliderWidget->setWidgetExtent(
            ivec2{handleSize_.get(), ap.second.y - ap.first.y + handleWidth});

        if (brushingAndLinking_.isSelected(axis.pcp->columnId(), BrushingTarget::Column)) {
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
                return std::make_tuple(utilgl::GlBoolState(GL_DEPTH_TEST, false),
                                       utilgl::GlBoolState(GL_BLEND, true),
                                       utilgl::BlendModeEquationState(GL_ONE, GL_ONE, GL_FUNC_ADD));
            case BlendMode::Subtractive:
                return std::make_tuple(
                    utilgl::GlBoolState(GL_DEPTH_TEST, false), utilgl::GlBoolState(GL_BLEND, true),
                    utilgl::BlendModeEquationState(GL_ONE, GL_ONE, GL_FUNC_REVERSE_SUBTRACT));
            case BlendMode::Regular:
                return std::make_tuple(
                    utilgl::GlBoolState(GL_DEPTH_TEST, false), utilgl::GlBoolState(GL_BLEND, true),
                    utilgl::BlendModeEquationState(GL_ONE, GL_ONE_MINUS_SRC_ALPHA, GL_FUNC_ADD));
            case BlendMode::None:
            default:
                return std::make_tuple(
                    utilgl::GlBoolState(GL_DEPTH_TEST, false), utilgl::GlBoolState(GL_BLEND, false),
                    utilgl::BlendModeEquationState(GL_NONE, GL_NONE, GL_FUNC_ADD));
        };
    }();

    // Disable blending for picking buffer when using additive or subtractive blending
    // Note: requires OpenGL 4.0 or the ARB_draw_buffers_blend extension
    if ((blendMode_ == BlendMode::Additive || blendMode_ == BlendMode::Subtractive) &&
        outport_.getData()->getPickingLayer()) {
        // The picking buffer, if existing, is attached to GL_DRAW_BUFFER1
        // see ImageGL::reattachAllLayers()
        const GLint pickingBuf = 1;
        if (OpenGLCapabilities::getOpenGLVersion() >= 400) {
            glBlendFunci(pickingBuf, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glBlendEquationSeparatei(pickingBuf, GL_FUNC_ADD, GL_FUNC_ADD);
        } else if (OpenGLCapabilities::isExtensionSupported("ARB_draw_buffers_blend")) {
            glBlendFunciARB(pickingBuf, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glBlendEquationSeparateiARB(pickingBuf, GL_FUNC_ADD, GL_FUNC_ADD);
        }
    }

    // Draw lines

    TextureUnitContainer unit;
    utilgl::bindAndSetUniforms(lineShader_, unit, colormap_.tf);

    const bool enableBlending =
        (blendMode_.get() == BlendMode::Additive || blendMode_.get() == BlendMode::Subtractive ||
         blendMode_.get() == BlendMode::Regular);
    // pcp_common.glsl
    lineShader_.setUniform("spacing", vec4(marginsInternal_.second.y, marginsInternal_.second.x,
                                           marginsInternal_.first.y, marginsInternal_.first.x));
    lineShader_.setUniform("dims", ivec2(size));
    // pcp_lines.vert
    lineShader_.setUniform("axisPositions", lines_.axisPositions);
    lineShader_.setUniform("axisFlipped", lines_.axisFlipped);

    // pcp_lines.frag
    lineShader_.setUniform("additiveBlend", enableBlending);
    lineShader_.setUniform("subtractiveBlending", blendMode_.get() == BlendMode::Subtractive);
    lineShader_.setUniform("color", vec4{filterColor_.get(), filterAlpha_.get()});
    lineShader_.setUniform("selectColor", selectedLineColor_.get());

    {
        const auto* meshGL = lines_.mesh.getRepresentation<MeshGL>();
        const utilgl::Enable<MeshGL> enable{meshGL};
        lines_.indices.getRepresentation<BufferGL>()->bind();

        std::array<float, 4> width = {lineWidth_, lineWidth_, selectedLineWidth_,
                                      selectedLineWidth_};
        std::array<float, 4> mixColor = {filterIntensity_, 0.0f, 0.0f, 0.0f};
        std::array<float, 4> mixSelection = {
            0.0f, 0.0f, selectedLineColorOverride_.isChecked() ? 1.0f : 0.0f, 0.8f};
        std::array<float, 4> mixAlpha = {1.0, 0.0f, 0.0f, 0.0f};
        std::array<float, 4> mixFallOfPower = {falloffPower_.get(), falloffPower_.get(),
                                               falloffPower_.get(), 0.5f * falloffPower_.get()};

        for (size_t i = showFiltered_ ? 0 : 1; i < lines_.offsets.size() - 1; ++i) {
            auto begin = lines_.offsets[i];
            auto end = lines_.offsets[i + 1];
            if (end == begin) continue;

            lineShader_.setUniform("lineWidth", width[i]);
            lineShader_.setUniform("mixColor", mixColor[i]);
            lineShader_.setUniform("mixAlpha", mixAlpha[i]);
            lineShader_.setUniform("mixSelection", mixSelection[i]);
            lineShader_.setUniform("fallofPower", mixFallOfPower[i]);

            glMultiDrawElements(
                GL_LINE_STRIP, lines_.sizes.data() + begin, GL_UNSIGNED_INT,
                reinterpret_cast<const GLvoid* const*>(lines_.starts.data() + begin),
                static_cast<GLsizei>(end - begin));
        }
    }
    lineShader_.deactivate();
}

void ParallelCoordinates::linePicked(PickingEvent* p) {
    if (auto df = dataFrame_.getData()) {
        // Show tooltip about current line
        if (p->getHoverState() == PickingHoverState::Move ||
            p->getHoverState() == PickingHoverState::Enter) {
            p->setToolTip(dataframe::createToolTipForRow(*df, p->getPickedId()));
            brushingAndLinking_.highlight({static_cast<uint32_t>(p->getPickedId())});
            invalidate(InvalidationLevel::InvalidOutput);
        } else {
            p->setToolTip("");
            brushingAndLinking_.highlight({});
            invalidate(InvalidationLevel::InvalidOutput);
        }
    }

    if (p->getState() == PickingState::Updated && p->getPressState() == PickingPressState::Press &&
        p->getPressItem() == PickingPressItem::Primary) {

        auto iCol = dataFrame_.getData()->getIndexColumn();
        const auto& indexCol = iCol->getTypedBuffer()->getRAMRepresentation()->getDataContainer();

        auto id = p->getPickedId();

        auto selection = brushingAndLinking_.getSelectedIndices();
        selection.flip(indexCol[id]);
        brushingAndLinking_.select(selection);

        p->markAsUsed();
        invalidate(InvalidationLevel::InvalidOutput);
    }
}

void ParallelCoordinates::axisPicked(PickingEvent* p, uint32_t columnId, PickType pt) {

    auto showValuesAsToolTip = [this, p](size_t id, PickType pt) {
        if (axes_[id].pcp->catCol_) return;

        if (pt == PickType::Upper) {
            p->setToolTip(fmt::to_string(axes_[id].pcp->range.getEnd()));
        } else if (pt == PickType::Lower) {
            p->setToolTip(fmt::to_string(axes_[id].pcp->range.getStart()));
        }
    };

    if (p->getHoverState() == PickingHoverState::Enter) {
        hoveredAxis_ = static_cast<int>(columnId);
        showValuesAsToolTip(columnId, pt);
        invalidate(InvalidationLevel::InvalidOutput);
    } else if (p->getHoverState() == PickingHoverState::Exit) {
        hoveredAxis_ = -1;
        p->setToolTip("");
        invalidate(InvalidationLevel::InvalidOutput);
    }

    if (p->getPressState() == PickingPressState::Release &&
        p->getPressItem() == PickingPressItem::Primary &&
        p->getPressedGlobalPickingId() == p->getCurrentGlobalPickingId() &&
        glm::length2(p->getDeltaPressedPosition()) < 0.01 && pt != PickType::Lower &&
        pt != PickType::Upper && !isDragging_) {

        auto selection = brushingAndLinking_.getSelectedIndices(BrushingTarget::Column);
        if (brushingAndLinking_.isSelected(columnId, BrushingTarget::Column)) {
            selection.remove(columnId);
        } else if (axisSelection_.get() == AxisSelection::Multiple) {
            selection.add(columnId);
        } else if (axisSelection_.get() == AxisSelection::Single) {
            selection.clear();
            selection.add(columnId);
        }
        brushingAndLinking_.select(selection, BrushingTarget::Column);

        p->markAsUsed();
        invalidate(InvalidationLevel::InvalidOutput);
    }
    if (p->getPressState() == PickingPressState::DoubleClick &&
        p->getPressItem() == PickingPressItem::Primary) {

        axes_[columnId].pcp->invertRange.set(!axes_[columnId].pcp->invertRange);
        // undo spurious axis selection caused by the single click event prior to the double click
        auto selection = brushingAndLinking_.getSelectedIndices(BrushingTarget::Column);
        selection.flip(columnId);
        brushingAndLinking_.select(selection, BrushingTarget::Column);

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
        p->getPressItems().contains(PickingPressItem::Primary) && pt != PickType::Lower &&
        pt != PickType::Upper) {
        isDragging_ = true;

        auto delta = glm::abs(p->getDeltaPressedPosition());

        if (delta.x > delta.y) {
            const auto it = util::find(enabledAxes_, columnId);
            if (it != enabledAxes_.end()) {
                const auto id = static_cast<size_t>(std::distance(enabledAxes_.begin(), it));
                if (id > 0 && p->getPosition().x * p->getCanvasSize().x <
                                  static_cast<float>(axisPos(enabledAxes_[id - 1]).first.x)) {
                    swap(id, id - 1);
                } else if (id + 1 < enabledAxes_.size() &&
                           p->getPosition().x * p->getCanvasSize().x >
                               static_cast<float>(axisPos(enabledAxes_[id + 1]).first.x)) {
                    swap(id, id + 1);
                } else if (columnId < axes_.size()) {
                    const auto rect = getDisplayRect(outport_.getDimensions());
                    lines_.axisPositions[columnId] =
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

void ParallelCoordinates::updateAxisRange(PCPAxisSettings&) { rangesDirty_ = true; }

void ParallelCoordinates::updateBrushing(PCPAxisSettings&) { updateBrushing(); }

void ParallelCoordinates::updateBrushing() {
    if (updating_) return;

    brushingDirty_ = false;

    auto iCol = dataFrame_.getData()->getIndexColumn();
    const auto& indexCol = iCol->getTypedBuffer()->getRAMRepresentation()->getDataContainer();

    auto brushAxis = [&](const std::vector<bool>& filtered, std::string_view axisName) {
        BitSet b;
        for (size_t i = 0; i < filtered.size(); ++i) {
            if (filtered[i]) {
                b.add(indexCol[i]);
            }
        }
        brushingAndLinking_.filter(axisName, b);
    };

    for (const auto& axis : axes_) {
        brushAxis(axis.pcp->getBrushed(), axis.pcp->caption_);
    }
}

std::pair<size2_t, size2_t> ParallelCoordinates::axisPos(size_t columnId) const {
    const auto dim = outport_.getDimensions();
    const auto rect = getDisplayRect(vec2{dim} - 1.0f);
    const size2_t lowerLeft(rect.first);
    const size2_t upperRight(rect.second);

    const auto dx = columnId < lines_.axisPositions.size() ? lines_.axisPositions[columnId] : 0.0f;
    const auto x = static_cast<size_t>(dx * (rect.second.x - rect.first.x));
    const auto startPos = lowerLeft + size2_t(x, 0);
    const auto endPos = size2_t(lowerLeft.x + x, upperRight.y);

    return {startPos, endPos};
}

std::pair<vec2, vec2> ParallelCoordinates::getDisplayRect(vec2 size) const {
    return {marginsInternal_.first, size - marginsInternal_.second};
}

void ParallelCoordinates::serialize(Serializer& s) const {
    Processor::serialize(s);
    s.serialize("enabledAxes", enabledAxes_, "axis");
}

void ParallelCoordinates::deserialize(Deserializer& d) {
    Processor::deserialize(d);
    const decltype(enabledAxes_) old{enabledAxes_};
    d.deserialize("enabledAxes", enabledAxes_, "axis");
    enabledAxesModified_ |= old != enabledAxes_;
}

void ParallelCoordinates::boxSelection(AxisRangeEventState state, AxisRangeInteraction interaction,
                                       AxisRangeInteractionMode mode,
                                       std::optional<std::array<dvec2, 2>> screenRect) {

    if (state == AxisRangeEventState::Started) {
        if (interaction == AxisRangeInteraction::Selection) {
            initialBrushingIndices_ = brushingAndLinking_.getSelectedIndices();
        }
    } else if (state == AxisRangeEventState::Finished) {
        initialBrushingIndices_.clear();
        if (mode == AxisRangeInteractionMode::Clear) {
            if (interaction == AxisRangeInteraction::Selection) {
                brushingAndLinking_.select(BitSet{});
                invalidate(InvalidationLevel::InvalidOutput);
            }
        } else {
            // trigger redrawing
            invalidate(InvalidationLevel::InvalidOutput);
        }
    } else if (state == AxisRangeEventState::Updated && screenRect) {
        auto r = screenRect.value_or(std::array<dvec2, 2>{{{}, {}}});
        if (r[0].x > r[1].x) {
            std::swap(r[0].x, r[1].x);
        }
        if (r[0].y > r[1].y) {
            std::swap(r[0].y, r[1].y);
        }

        if (interaction == AxisRangeInteraction::Selection) {
            BitSet b;
            switch (boxSelectionMode_) {
                using enum BoxSelectionMode;
                case DataValues:
                    b = columnDataValueIntersections(r);
                    break;
                case Lines:
                    b = lineIntersections(r);
                    break;
                default:
                    break;
            }
            b -= brushingAndLinking_.getFilteredIndices();
            if (mode == AxisRangeInteractionMode::Append) {
                b |= initialBrushingIndices_;
            }
            brushingAndLinking_.select(b);
            invalidate(InvalidationLevel::InvalidOutput);
        }
    }
}

namespace {

bool lineRectangleOverlap(const std::array<dvec2, 2>& screenRect, dvec2 lineStart, dvec2 lineEnd) {
    const std::array corners{
        screenRect[0],
        dvec2{screenRect[1].x, screenRect[0].y},
        screenRect[1],
        dvec2{screenRect[0].x, screenRect[1].y},
    };

    const dvec2 dir{glm::normalize(lineEnd - lineStart)};
    const dvec2 pDir{-dir.y, dir.x};
    const double pDist{glm::dot(pDir, lineStart)};

    const dvec4 lineDistances{
        glm::dot(pDir, corners[0]) - pDist, glm::dot(pDir, corners[1]) - pDist,
        glm::dot(pDir, corners[2]) - pDist, glm::dot(pDir, corners[3]) - pDist};
    // if distances have opposite signs, the infinite line overlaps with the rectangle
    return std::signbit(glm::compMin(lineDistances)) != std::signbit(glm::compMax(lineDistances));
}

double rangeConversion(double v, dvec2 sourceRange, dvec2 destRange) {
    return (v - sourceRange.x) / (sourceRange.y - sourceRange.x) * (destRange.y - destRange.x) +
           destRange.x;
}

}  // namespace

BitSet ParallelCoordinates::lineIntersections(const std::array<dvec2, 2>& screenRect) const {
    auto axisPosd = [this](auto columnId) {
        auto [lower, upper] = axisPos(columnId);
        return std::pair<dvec2, dvec2>{lower, upper};
    };

    const auto data = dataFrame_.getData();
    auto iCol = dataFrame_.getData()->getIndexColumn();
    const auto& indexCol = iCol->getTypedBuffer()->getRAMRepresentation()->getDataContainer();

    BitSet b;
    for (const auto& [left, right] :
         std::views::zip(enabledAxes_, enabledAxes_ | std::views::drop(1))) {

        const auto leftColumnId = axes_[left].pcp->columnId();
        const auto rightColumnId = axes_[right].pcp->columnId();

        auto [leftLower, leftUpper] = axisPosd(leftColumnId);
        auto [rightLower, rightUpper] = axisPosd(rightColumnId);

        if (std::min(leftLower.x, rightLower.x) <= screenRect[1].x &&
            std::max(leftLower.x, rightLower.x) >= screenRect[0].x) {
            auto leftColumn = data->getColumn(leftColumnId);
            auto rightColumn = data->getColumn(rightColumnId);
            // adjust selection rectangle to fit in between the two columns
            const std::array rect{
                dvec2{std::max(std::min(leftLower.x, rightLower.x), screenRect[0].x),
                      screenRect[0].y},
                dvec2{std::min(std::max(leftLower.x, rightLower.x), screenRect[1].x),
                      screenRect[1].y}};

            auto leftAxisRange{leftColumn->getRange()};
            if (axes_[left].pcp->invertRange) std::swap(leftAxisRange.x, leftAxisRange.y);
            auto rightAxisRange{rightColumn->getRange()};
            if (axes_[right].pcp->invertRange) std::swap(rightAxisRange.x, rightAxisRange.y);

            for (size_t i = 0; i < data->getNumberOfRows(); ++i) {
                const dvec2 lineStart{leftLower.x,
                                      rangeConversion(leftColumn->getAsDouble(i), leftAxisRange,
                                                      dvec2{leftLower.y, leftUpper.y})};
                const dvec2 lineEnd{rightLower.x,
                                    rangeConversion(rightColumn->getAsDouble(i), rightAxisRange,
                                                    dvec2{rightLower.y, rightUpper.y})};

                if (lineRectangleOverlap(rect, lineStart, lineEnd)) {
                    b.add(indexCol[i]);
                }
            }
        }
    }
    return b;
}

BitSet ParallelCoordinates::columnDataValueIntersections(
    const std::array<dvec2, 2>& screenRect) const {
    const auto data = dataFrame_.getData();
    BitSet b;
    for (const auto& axis : axes_) {
        if (!axis.pcp->isChecked()) continue;
        auto [lower, upper] = axisPos(axis.pcp->columnId());
        const dvec2 lowerd{lower};
        const dvec2 upperd{upper};

        if (screenRect[0].x <= lowerd.x && lowerd.x <= screenRect[1].x) {
            dvec2 axisRange{axis.pcp->getRange()};
            if (axis.pcp->invertRange) std::swap(axisRange.x, axisRange.y);
            const double min =
                rangeConversion(screenRect[0].y, dvec2{lowerd.y, upperd.y}, axisRange);
            const double max =
                rangeConversion(screenRect[1].y, dvec2{lowerd.y, upperd.y}, axisRange);
            b |= util::rangeSelection(data->getColumn(axis.pcp->columnId())->getBuffer().get(), min,
                                      max);
        }
    }
    return b;
}

}  // namespace inviwo::plot
