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

#include <modules/plottinggl/processors/scatterplotmatrixprocessor.h>
#include <modules/plottinggl/plotters/scatterplotgl.h>

#include <modules/plotting/utils/statsutils.h>
#include <inviwo/core/interaction/events/mouseevent.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/util/zip.h>
#include <inviwo/core/util/utilities.h>
#include <inviwo/core/util/stdextensions.h>
#include <modules/fontrendering/util/fontutils.h>

namespace inviwo {

namespace plot {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo ScatterPlotMatrixProcessor::processorInfo_{
    "org.inviwo.ScatterPlotMatrixProcessor",  // Class identifier
    "Scatter Plot Matrix",                    // Display name
    "Plotting",                               // Category
    CodeState::Experimental,                  // Code state
    "GL, Plotting",                           // Tags
};
const ProcessorInfo ScatterPlotMatrixProcessor::getProcessorInfo() const { return processorInfo_; }

ScatterPlotMatrixProcessor::ScatterPlotMatrixProcessor()
    : Processor()
    , dataFrame_("dataFrame")
    , brushing_("brushing_")
    , outport_("outport")
    , numParams_(0)
    , scatterPlotproperties_("scatterPlotproperties", "Properties")
    , color_("colorCol", "Color column", dataFrame_, true, 3)
    , selectedX_("selectedX", "Select X", dataFrame_, true)
    , selectedY_("selectedY", "Select Y", dataFrame_, true)
    , labels_("labels", "Labels")
    , fontColor_("fontColor", "Font Color", vec4(0, 0, 0, 1))
    , fontFace_("fontFace", "Font Face")
    , fontSize_("fontSize", "Font size", 20, 0, 144, 1)
    , fontFaceStats_("fontFaceStats", "Font Face (stats)")
    , statsFontSize_("statsFontSize", "Font size (stats)", 14, 0, 144, 1)
    , showCorrelationValues_("showStatistics", "Show correlation values", true)
    , parameters_("parameters", "Parameters")
    , correlectionTF_("correlectionTF", "Correlation TF")

    , textRenderer_()
    , textureQuadRenderer_()

    , mouseEvent_("mouseEvent", "Mouse Event",
                  [&](Event *e) {
                      if (auto me = dynamic_cast<MouseEvent *>(e)) {
                          auto p =
                              ivec2(me->posNormalized() * dvec2(static_cast<double>(numParams_)));
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
    scatterPlotproperties_.hoverColor_.setVisible(false);
    scatterPlotproperties_.hovering_.setVisible(false);

    addProperty(scatterPlotproperties_);
    addProperty(color_);
    addProperty(selectedX_);
    addProperty(selectedY_);
    addProperty(labels_);
    addProperty(correlectionTF_);
    addProperty(showCorrelationValues_);
    addProperty(parameters_);

    addProperty(mouseEvent_);

    color_.onChange([&]() {
        auto buf = color_.getBuffer();
        for (auto &p : plots_) {
            p->setColorData(buf);
        }
        scatterPlotproperties_.tf_.setVisible(buf != nullptr);
        scatterPlotproperties_.color_.setVisible(buf == nullptr);
    });

    correlectionTF_.get().clear();
    correlectionTF_.get().add(0.0, vec4(1, 0, 0, 1));
    correlectionTF_.get().add(0.5, vec4(1, 1, 1, 1));
    correlectionTF_.get().add(1.0, vec4(0, 0, 1, 1));
    correlectionTF_.setCurrentStateAsDefault();

    for (auto font : util::getAvailableFonts()) {
        auto name = filesystem::getFileNameWithoutExtension(font.second);
        // use the file name w/o extension as identifier
        fontFace_.addOption(name, font.first, font.second);
        fontFaceStats_.addOption(name, font.first, font.second);
    }
    fontFace_.setSelectedIdentifier("OpenSans-Semibold");
    fontFace_.setCurrentStateAsDefault();
    fontFaceStats_.setSelectedIdentifier("OpenSans-Regular");
    fontFaceStats_.setCurrentStateAsDefault();

    fontSize_.setSemantics(PropertySemantics("Fontsize"));
    statsFontSize_.setSemantics(PropertySemantics("Fontsize"));

    labels_.addProperty(fontColor_);
    labels_.addProperty(fontFace_);
    labels_.addProperty(fontSize_);
    labels_.addProperty(fontFaceStats_);
    labels_.addProperty(statsFontSize_);
    fontColor_.setSemantics(PropertySemantics::Color);

    auto updateLabels = [&]() { labelsTextures_.clear(); };
    auto updateStatsLabels = [&]() { statsTextures_.clear(); };
    fontFace_.onChange(updateLabels);
    fontSize_.onChange(updateLabels);
    fontColor_.onChange(updateLabels);

    fontColor_.onChange(updateStatsLabels);
    statsFontSize_.onChange(updateStatsLabels);
    correlectionTF_.onChange(updateStatsLabels);

    scatterPlotproperties_.onChange([&]() {
        for (auto &p : plots_) {
            p->properties_.set(&scatterPlotproperties_);
        }
    });

    dataFrame_.onChange([&]() {
        plots_.clear();
        labelsTextures_.clear();
        statsTextures_.clear();
    });

    parameters_.onChange([&]() {
        plots_.clear();
        labelsTextures_.clear();
        statsTextures_.clear();
    });
}

template <typename T>
struct RangeIterator : public std::iterator<std::forward_iterator_tag, T, T, const T *, T> {
    T i;

    RangeIterator(T i = 0) : i(i) {}

    bool operator==(RangeIterator b) { return i == b.i; }
    bool operator!=(RangeIterator b) { return i != b.i; }
    RangeIterator operator++() {
        ++i;
        return *this;
    }
    RangeIterator operator++(int) {
        auto tmp = *this;
        ++i;
        return tmp;
    }

    T operator*() const { return i; }
};

void ScatterPlotMatrixProcessor::process() {
    if (plots_.empty()) {
        createScatterPlots();
        for (auto &p : plots_) {
            p->properties_.set(&scatterPlotproperties_);
        }
    }
    if (labelsTextures_.empty()) {
        createLabels();
    }
    if (statsTextures_.empty()) {
        createStatsLabels();
    }

    std::unique_ptr<IndexBuffer> indicies = nullptr;
    if (brushing_.isConnected()) {
        auto dataframe = dataFrame_.getData();
        auto dfSize = dataframe->getNumberOfRows();

        auto iCol = dataframe->getIndexColumn();
        auto &indexCol = iCol->getTypedBuffer()->getRAMRepresentation()->getDataContainer();

        auto brushedIndicies = brushing_.getFilteredIndices();
        indicies = std::make_unique<IndexBuffer>();
        auto &vec = indicies->getEditableRAMRepresentation()->getDataContainer();
        vec.reserve(dfSize - brushedIndicies.size());

        auto seq = util::sequence<uint32_t>(0, static_cast<uint32_t>(dfSize), 1);
        std::copy_if(seq.begin(), seq.end(), std::back_inserter(vec),
                     [&](const auto &id) { return !brushing_.isFiltered(indexCol[id]); });
    }

    utilgl::activateAndClearTarget(outport_);

    auto dims = outport_.getDimensions();
    auto size = dims / numParams_;

    utilgl::BlendModeState blending(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    size2_t pos;
    size_t idx = 0;
    for (size_t i = 0; i < numParams_; i++) {
        pos.x = size.x * i;
        for (size_t j = i + 1; j < numParams_; j++) {
            pos.y = size.y * j;
            plots_[idx]->plot(pos, size, indicies.get());

            vec2 statstextPos = vec2(size) * vec2((j + 0.5f), i + 0.5f);
            statstextPos -= vec2(statsTextures_[i]->getDimensions()) / 2.f;

            ivec2 pos2(size.x * j, size.y * i);

            textureQuadRenderer_.renderToRect(bgTextures_[idx], pos2, size, dims);

            if (showCorrelationValues_) {
                textureQuadRenderer_.render(statsTextures_[idx], ivec2(statstextPos), dims);
            }

            idx++;
        }

        vec2 textPos = vec2(size) * vec2(i + 0.5f, i + 0.5f);
        textPos -= vec2(labelsTextures_[i]->getDimensions()) / 2.f;

        textureQuadRenderer_.render(labelsTextures_[i], ivec2(textPos), dims);
    }
    utilgl::deactivateCurrentTarget();

    outport_.getEditableData()->getPickingLayer();
}

void ScatterPlotMatrixProcessor::createScatterPlots() {
    numParams_ = 0;
    if (outport_.hasData()) {
        auto &dataFrame = *dataFrame_.getData();

        auto buffer = [&]() -> std::shared_ptr<const BufferBase> {
            auto idx = color_.get();
            if (idx == -1) {
                return nullptr;
            } else {
                return dataFrame.getColumn(idx)->getBuffer();
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
                plot->setXAxis((*x));
                plot->setYAxis((*y));

                plot->setColorData(buffer);

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

        auto &dataFrame = *dataFrame_.getData();
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

                auto c = correlectionTF_.get().sample(v);
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

        auto &dataFrame = *dataFrame_.getData();
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
        if (auto bp = dynamic_cast<BoolProperty *>(prop)) {
            bp->setSerializationMode(PropertySerializationMode::All);
            return bp->get();
        }
        throw inviwo::Exception("Not a bool property", IVW_CONTEXT);
    } else {
        auto newProp = new BoolProperty(identifier, displayName, true);
        newProp->setSerializationMode(PropertySerializationMode::All);
        parameters_.addProperty(newProp);
        return true;
    }
}

}  // namespace plot

}  // namespace inviwo
