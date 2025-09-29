/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2025 Inviwo Foundation
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

#include <modules/plottinggl/processors/scatterplotmatrixprocessor.h>

#include "inviwo/core/util/colorbrewer-generated.h"                    // for Category, Category...
#include <inviwo/core/datastructures/bitset.h>                         // for BitSet, BitSet::Bi...
#include <inviwo/core/datastructures/buffer/buffer.h>                  // for IndexBuffer
#include <inviwo/core/datastructures/buffer/bufferramprecision.h>      // for BufferRAMPrecision
#include <inviwo/core/datastructures/transferfunction.h>               // for TransferFunction
#include <inviwo/core/interaction/events/event.h>                      // for Event
#include <inviwo/core/interaction/events/mousebuttons.h>               // for MouseButton, Mouse...
#include <inviwo/core/interaction/events/mouseevent.h>                 // for MouseEvent
#include <inviwo/core/ports/imageport.h>                               // for ImageOutport
#include <inviwo/core/processors/processor.h>                          // for Processor
#include <inviwo/core/processors/processorinfo.h>                      // for ProcessorInfo
#include <inviwo/core/processors/processorstate.h>                     // for CodeState, CodeSta...
#include <inviwo/core/processors/processortags.h>                      // for Tags
#include <inviwo/core/properties/boolproperty.h>                       // for BoolProperty
#include <inviwo/core/properties/compositeproperty.h>                  // for CompositeProperty
#include <inviwo/core/properties/eventproperty.h>                      // for EventProperty::Action
#include <inviwo/core/properties/invalidationlevel.h>                  // for InvalidationLevel
#include <inviwo/core/properties/ordinalproperty.h>                    // for FloatVec4Property
#include <inviwo/core/properties/property.h>                           // for Property
#include <inviwo/core/properties/propertysemantics.h>                  // for PropertySemantics
#include <inviwo/core/properties/selectioncolorproperty.h>             // for SelectionColorProp...
#include <inviwo/core/properties/transferfunctionproperty.h>           // for TransferFunctionPr...
#include <inviwo/core/properties/valuewrapper.h>                       // for PropertySerializat...
#include <inviwo/core/util/colorbrewer.h>                              // for getTransferFunction
#include <inviwo/core/util/exception.h>                                // for Exception
#include <inviwo/core/util/glmvec.h>                                   // for vec2, size2_t, ivec2
#include <inviwo/core/util/sourcecontext.h>                            // for SourceContext
#include <inviwo/core/util/utilities.h>                                // for stripIdentifier
#include <inviwo/core/util/zip.h>                                      // for enumerate, zipIter...
#include <inviwo/core/util/rendercontext.h>                            // for RenderContext
#include <inviwo/dataframe/datastructures/dataframe.h>                 // for DataFrame, DataFra...
#include <inviwo/dataframe/properties/columnoptionproperty.h>          // for ColumnOptionProperty
#include <modules/brushingandlinking/brushingandlinkingmanager.h>      // for BrushingTargetsInv...
#include <modules/brushingandlinking/datastructures/brushingaction.h>  // for BrushingTarget
#include <modules/brushingandlinking/ports/brushingandlinkingports.h>  // for BrushingAndLinking...
#include <modules/fontrendering/properties/fontfaceoptionproperty.h>   // for FontFaceOptionProp...
#include <modules/fontrendering/textrenderer.h>                        // for createTextTexture
#include <modules/fontrendering/util/fontutils.h>                      // for FontType, FontType...
#include <modules/opengl/inviwoopengl.h>                               // for GL_RGBA, GL_FLOAT
#include <modules/opengl/openglutils.h>                                // for BlendModeState
#include <modules/opengl/rendering/texturequadrenderer.h>              // for TextureQuadRenderer
#include <modules/opengl/texture/texture2d.h>                          // for Texture2D
#include <modules/opengl/texture/textureutils.h>                       // for activateAndClearTa...
#include <modules/plotting/utils/statsutils.h>                         // for RegresionResult
#include <modules/plottinggl/plotters/scatterplotgl.h>                 // for ScatterPlotGL, Sca...

#include <functional>   // for __base
#include <iomanip>      // for operator<<, setpre...
#include <ostream>      // for basic_ostream, ope...
#include <string>       // for char_traits, string
#include <string_view>  // for string_view
#include <type_traits>  // for remove_extent_t
#include <utility>      // for move, pair

#include <glm/detail/setup.hpp>  // for size_t
#include <glm/vec2.hpp>          // for vec<>::(anonymous)

namespace inviwo {
class Column;

namespace plot {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo ScatterPlotMatrixProcessor::processorInfo_{
    "org.inviwo.ScatterPlotMatrixProcessor",  // Class identifier
    "Scatter Plot Matrix",                    // Display name
    "Plotting",                               // Category
    CodeState::Stable,                        // Code state
    "GL, Plotting",                           // Tags
    "Creates a scatter plot matrix for a given DataFrame supporting Brushing and Linking."_help};
const ProcessorInfo& ScatterPlotMatrixProcessor::getProcessorInfo() const { return processorInfo_; }

ScatterPlotMatrixProcessor::ScatterPlotMatrixProcessor()
    : Processor()
    , dataFrame_("dataFrame", "Data input for plotting"_help)
    , brushing_("brushing_", "Inport for brushing & linking interactions"_help,
                {{{BrushingTarget::Row},
                  BrushingModification::Filtered,
                  InvalidationLevel::InvalidOutput}})
    , outport_("outport", "Rendered image of the scatter plot matrix"_help)
    , numParams_(0)
    , scatterPlotproperties_("scatterPlotproperties", "Properties")
    , color_("colorCol", "Color column", dataFrame_, ColumnOptionProperty::AddNoneOption::Yes, 3)
    , selectedX_("selectedX", "Select X", dataFrame_, ColumnOptionProperty::AddNoneOption::Yes)
    , selectedY_("selectedY", "Select Y", dataFrame_, ColumnOptionProperty::AddNoneOption::Yes)
    , labels_("labels", "Labels")
    , fontColor_("fontColor", "Font Color", vec4(0, 0, 0, 1))
    , fontFace_("fontFace", "Font Face", font::FontType::Caption)
    , fontSize_("fontSize", "Font size", 20, 0, 144, 1, InvalidationLevel::Valid,
                PropertySemantics("Fontsize"))
    , fontFaceStats_("fontFaceStats", "Font Face (stats)", font::FontType::Label)
    , statsFontSize_("statsFontSize", "Font size (stats)", 14, 0, 144, 1, InvalidationLevel::Valid,
                     PropertySemantics("Fontsize"))
    , showCorrelationValues_("showStatistics", "Show correlation values", true)
    , parameters_("parameters", "Parameters")
    , correlationTF_("correlectionTF", "Correlation TF",
                     colorbrewer::getTransferFunction(colorbrewer::Category::Diverging,
                                                      colorbrewer::Family::RdBu, 9, false))

    , textRenderer_()
    , textureQuadRenderer_()

    , mouseEvent_(
          "mouseEvent", "Mouse Event",
          [&](Event* e) {
              if (auto me = dynamic_cast<MouseEvent*>(e)) {
                  auto p = ivec2(me->posNormalized() * dvec2(static_cast<double>(numParams_)));
                  if (p.x == p.y) {
                      color_.setSelectedValue(visibleIDToColumnID_[p.x]);
                  } else {
                      selectedX_.setSelectedValue(visibleIDToColumnID_[p.x]);
                      selectedY_.setSelectedValue(visibleIDToColumnID_[p.y]);
                  }
              }
          },
          MouseButton::Left, MouseState::Press)

{
    addPort(dataFrame_);
    addPort(brushing_);
    addPort(outport_);

    brushing_.setOptional(true);

    selectedX_.setVisible(false);
    selectedY_.setVisible(false);
    scatterPlotproperties_.showHighlighted_.setVisible(false);

    addProperties(scatterPlotproperties_, color_, selectedX_, selectedY_, labels_, correlationTF_,
                  showCorrelationValues_, parameters_, mouseEvent_);

    color_.onChange([&]() {
        if (dataFrame_.hasData()) {
            auto index = color_.getSelectedValue();
            auto colorCol = index >= 0 ? dataFrame_.getData()->getColumn(index).get() : nullptr;
            for (auto& p : plots_) {
                p->setColorData(colorCol);
            }
            scatterPlotproperties_.tf_.setVisible(colorCol != nullptr);
            scatterPlotproperties_.color_.setVisible(colorCol == nullptr);
        }
    });

    labels_.addProperties(fontColor_, fontFace_, fontSize_, fontFaceStats_, statsFontSize_);
    fontColor_.setSemantics(PropertySemantics::Color);

    auto updateLabels = [&]() { labelsTextures_.clear(); };
    auto updateStatsLabels = [&]() { statsTextures_.clear(); };
    fontFace_.onChange(updateLabels);
    fontSize_.onChange(updateLabels);
    fontColor_.onChange(updateLabels);

    fontColor_.onChange(updateStatsLabels);
    fontFaceStats_.onChange(updateStatsLabels);
    statsFontSize_.onChange(updateStatsLabels);
    correlationTF_.onChange(updateStatsLabels);

    scatterPlotproperties_.onChange([&]() {
        RenderContext::getPtr()->activateDefaultRenderContext();
        for (auto& p : plots_) {
            p->properties_.set(&scatterPlotproperties_);
        }
    });

    dataFrame_.onChange([&]() {
        plots_.clear();
        labelsTextures_.clear();
        statsTextures_.clear();
    });

    parameters_.onChange([&]() {
        RenderContext::getPtr()->activateDefaultRenderContext();
        plots_.clear();
        labelsTextures_.clear();
        statsTextures_.clear();
    });
}

void ScatterPlotMatrixProcessor::process() {
    bool initialSetup = false;
    if (plots_.empty()) {
        createScatterPlots();
        for (auto& p : plots_) {
            p->properties_.set(&scatterPlotproperties_);
        }

        indexToRowMap_ = [&]() {
            auto iCol = dataFrame_.getData()->getIndexColumn();
            auto& indexCol = iCol->getTypedBuffer()->getRAMRepresentation()->getDataContainer();
            std::unordered_map<uint32_t, uint32_t> indexToRow;
            indexToRow.reserve(indexCol.size());
            for (auto&& [row, index] : util::enumerate(indexCol)) {
                indexToRow.try_emplace(index, static_cast<uint32_t>(row));
            }
            return indexToRow;
        }();

        initialSetup = true;
    }
    if (labelsTextures_.empty()) {
        createLabels();
    }
    if (statsTextures_.empty()) {
        createStatsLabels();
    }

    std::unique_ptr<IndexBuffer> indicies = nullptr;
    if (brushing_.isConnected() && (brushing_.isFilteringModified() || initialSetup)) {
        auto transformIdsToRows = [&](const BitSet& b) {
            BitSet rows;
            for (auto id : b) {
                auto it = indexToRowMap_.find(id);
                if (it != indexToRowMap_.end()) {
                    rows.add(it->second);
                }
            }
            return rows;
        };
        filteredIndices_ = transformIdsToRows(brushing_.getFilteredIndices());
        initialSetup = false;
    }

    utilgl::activateAndClearTarget(outport_);
    utilgl::BlendModeState blending(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    const size2_t dims = outport_.getDimensions();
    const size2_t extent = dims / numParams_;

    size_t idx = 0;
    for (size_t i = 0; i < numParams_; i++) {
        size2_t pos{extent.x * i, 0};
        for (size_t j = i + 1; j < numParams_; j++) {
            pos.y = extent.y * j;
            plots_[idx]->setFilteredIndices(filteredIndices_);
            plots_[idx]->plot(pos, extent);

            const ivec2 origin(extent.x * j, extent.y * i);
            textureQuadRenderer_.renderToRect(bgTextures_[idx], origin, extent, dims);

            if (showCorrelationValues_) {
                const vec2 statstextPos = vec2(extent) * vec2((j + 0.5f), i + 0.5f) -
                                          vec2(statsTextures_[i]->getDimensions()) / 2.0f;
                textureQuadRenderer_.render(statsTextures_[idx], ivec2(statstextPos), dims);
            }

            ++idx;
        }

        vec2 textPos = vec2(extent) * vec2(i + 0.5f, i + 0.5f);
        textPos -= vec2(labelsTextures_[i]->getDimensions()) / 2.f;

        textureQuadRenderer_.render(labelsTextures_[i], ivec2(textPos), dims);
    }
    utilgl::deactivateCurrentTarget();

    outport_.getEditableData()->getPickingLayer();
}

void ScatterPlotMatrixProcessor::createScatterPlots() {
    numParams_ = 0;
    if (outport_.hasData()) {
        auto& dataFrame = *dataFrame_.getData();

        auto colorCol = [&]() -> std::shared_ptr<const Column> {
            auto idx = color_.get();
            if (idx == -1) {
                return nullptr;
            } else {
                return dataFrame.getColumn(idx);
            }
        }();

        plots_.clear();

        int a = -1;
        for (auto x = dataFrame.begin(); x != dataFrame.end(); ++x) {
            a++;
            if (!isIncluded(*x)) continue;
            visibleIDToColumnID_[numParams_++] = a;

            for (auto y = x + 1; y != dataFrame.end(); ++y) {
                if (!isIncluded(*y)) continue;
                auto plot = std::make_unique<ScatterPlotGL>();
                plot->properties_.set(&scatterPlotproperties_);
                plot->setXAxis((*x).get());
                plot->setYAxis((*y).get());

                plot->setColorData(colorCol.get());

                plots_.push_back(std::move(plot));
            }
        }
    }
}

void ScatterPlotMatrixProcessor::createStatsLabels() {
    if (outport_.hasData()) {
        statsTextures_.clear();
        bgTextures_.clear();

        textRenderer_.setFont(fontFaceStats_.get());
        textRenderer_.setFontSize(statsFontSize_.get());

        auto& dataFrame = *dataFrame_.getData();
        for (auto x = dataFrame.begin(); x != dataFrame.end(); ++x) {
            if (!isIncluded(*x)) continue;
            for (auto y = x + 1; y != dataFrame.end(); ++y) {
                if (!isIncluded(*y)) continue;
                auto res = statsutil::linearRegresion(*(*x)->getBuffer(), *(*y)->getBuffer());

                std::ostringstream oss;
                oss << std::setprecision(2) << "corr ρ = " << res.corr << std::endl
                    << "r² = " << res.r2;

                auto tex = util::createTextTexture(textRenderer_, oss.str(), fontColor_);
                statsTextures_.push_back(tex);

                auto tex2 = std::make_shared<Texture2D>(size2_t(1, 1), GL_RGBA, GL_RGBA, GL_FLOAT,
                                                        GL_LINEAR);
                auto v = res.r2;
                if (res.corr < 0) {
                    v = -v;
                }
                v += 1;
                v /= 2;

                auto c = correlationTF_.get().sample(v);
                tex2->initialize(&c);

                bgTextures_.push_back(tex2);
            }
        }
    }
}

void ScatterPlotMatrixProcessor::createLabels() {
    if (outport_.hasData()) {
        labelsTextures_.clear();
        textRenderer_.setFont(fontFace_.get());
        textRenderer_.setFontSize(fontSize_.get());

        auto& dataFrame = *dataFrame_.getData();
        for (auto x = dataFrame.begin(); x != dataFrame.end(); ++x) {
            if (!isIncluded(*x)) continue;
            auto tex = util::createTextTexture(textRenderer_, (*x)->getHeader(), fontColor_);
            labelsTextures_.push_back(tex);
        }
    }
}

bool ScatterPlotMatrixProcessor::isIncluded(std::shared_ptr<Column> col) {
    std::string displayName = col->getHeader();
    std::string identifier = util::stripIdentifier(displayName);

    auto prop = parameters_.getPropertyByIdentifier(identifier);
    if (prop) {
        if (auto bp = dynamic_cast<BoolProperty*>(prop)) {
            bp->setSerializationMode(PropertySerializationMode::All);
            return bp->get();
        }
        throw inviwo::Exception("Not a bool property");
    } else {
        auto newProp = new BoolProperty(identifier, displayName, true);
        newProp->setSerializationMode(PropertySerializationMode::All);
        parameters_.addProperty(newProp);
        return true;
    }
}

}  // namespace plot

}  // namespace inviwo
