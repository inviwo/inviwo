/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2022 Inviwo Foundation
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

#include <modules/plottinggl/plotters/scatterplotgl.h>
#include <modules/opengl/buffer/buffergl.h>
#include <modules/opengl/buffer/bufferobject.h>
#include <modules/opengl/buffer/bufferobjectarray.h>
#include <modules/opengl/shader/shaderutils.h>
#include <modules/opengl/texture/textureutils.h>
#include <modules/opengl/openglutils.h>

#include <inviwo/core/processors/processor.h>
#include <inviwo/core/interaction/events/pickingevent.h>
#include <inviwo/core/interaction/events/mouseevent.h>
#include <inviwo/core/interaction/events/touchevent.h>
#include <inviwo/core/datastructures/buffer/buffer.h>
#include <inviwo/core/datastructures/buffer/bufferram.h>
#include <inviwo/core/datastructures/geometry/basicmesh.h>
#include <inviwo/core/properties/cameraproperty.h>
#include <inviwo/core/util/colorconversion.h>
#include <inviwo/core/util/zip.h>
#include <modules/opengl/buffer/bufferobjectarray.h>

#include <algorithm>

namespace inviwo {

namespace plot {

const std::string ScatterPlotGL::Properties::classIdentifier =
    "org.inviwo.ScatterPlotGL.Properties";
std::string ScatterPlotGL::Properties::getClassIdentifier() const { return classIdentifier; }

ScatterPlotGL::Properties::Properties(std::string_view identifier, std::string_view displayName,
                                      InvalidationLevel invalidationLevel,
                                      PropertySemantics semantics)
    : CompositeProperty(identifier, displayName, invalidationLevel, semantics)
    , useCircle_("useCircle", "Use Circles (else squares)", true)
    , radiusRange_("radius", "Radius", 5, 0, 10, 0.01f)
    , minRadius_("minRadius", "Min Radius", 0.1f, 0, 10, 0.01f)
    , tf_("transferFunction", "Transfer Function",
          TransferFunction({{0.0, vec4(1.0f)}, {1.0, vec4(1.0f)}}))
    , color_("defaultColor", "Color", util::ordinalColor(1.0f, 0.0f, 0.0f, 1.0f))

    , showHighlighted_("showHighlighted", "Show Highlighted", true, vec3(1.0f, 0.906f, 0.612f))
    , showSelected_("showSelected", "Show Selected", true, vec3(1.0f, 0.769f, 0.247f))
    , showFiltered_("showFiltered", "Show Filtered", false, vec3(0.5f, 0.5f, 0.5f))

    , tooltip_("tooltip", "Show Tooltip", true)

    , boxSelectionSettings_("dragRectSettings", "Box Selection/Filtering")
    , margins_("margins", "Margins", 5, 5, 5, 5)
    , axisMargin_("axisMargin", "Axis Margin", 15.0f, 0.0f, 50.0f)

    , borderWidth_("borderWidth", "Border Width", 2, 0, 20)
    , borderColor_("borderColor", "Border Color", util::ordinalColor(0.0f, 0.0f, 0.0f, 1.0f))

    , axisStyle_("axisStyle", "Global Axis Style")
    , xAxis_("xAxis", "X Axis")
    , yAxis_("yAxis", "Y Axis", AxisProperty::Orientation::Vertical) {

    util::for_each_in_tuple([&](auto& e) { this->addProperty(e); }, props());

    axisStyle_.registerProperties(xAxis_, yAxis_);

    yAxis_.flipped_.set(true);

    color_.setVisible(true);
    tf_.setVisible(!color_.getVisible());
    minRadius_.setVisible(false);

    tf_.setCurrentStateAsDefault();
}

ScatterPlotGL::Properties::Properties(const ScatterPlotGL::Properties& rhs)
    : CompositeProperty(rhs)
    , useCircle_(rhs.useCircle_)
    , radiusRange_(rhs.radiusRange_)
    , minRadius_(rhs.minRadius_)
    , tf_(rhs.tf_)
    , color_(rhs.color_)
    , showHighlighted_(rhs.showHighlighted_)
    , showSelected_(rhs.showSelected_)
    , showFiltered_(rhs.showFiltered_)
    , tooltip_(rhs.tooltip_)
    , boxSelectionSettings_(rhs.boxSelectionSettings_)
    , margins_(rhs.margins_)
    , axisMargin_(rhs.axisMargin_)
    , borderWidth_(rhs.borderWidth_)
    , borderColor_(rhs.borderColor_)
    , axisStyle_(rhs.axisStyle_)
    , xAxis_(rhs.xAxis_)
    , yAxis_(rhs.yAxis_) {

    util::for_each_in_tuple([&](auto& e) { this->addProperty(e); }, props());
    axisStyle_.unregisterAll();
    axisStyle_.registerProperties(xAxis_, yAxis_);
}

ScatterPlotGL::Properties* ScatterPlotGL::Properties::clone() const {
    return new Properties(*this);
}

ScatterPlotGL::ScatterPlotGL(Processor* processor)
    : properties_("scatterplot", "Scatterplot")
    , shader_("scatterplot.vert", "scatterplot.geom", "scatterplot.frag")
    , axisRenderers_({{properties_.xAxis_, properties_.yAxis_}})
    , picking_(processor, 1, [this](PickingEvent* p) { objectPicked(p); })
    , partitionDirty_(true)
    , processor_(processor)
    , boxSelectionHandler_(properties_.boxSelectionSettings_, points_.xCoord, points_.yCoord,
                           [&](dvec2 p, const size2_t& dims) {
                               const dvec4 margins =
                                   properties_.margins_.getAsVec4() +
                                   properties_.axisMargin_.get();  // top, right, bottom, left

                               dvec2 bottomLeft{margins.w, margins.z};
                               dvec2 topRight{static_cast<double>(dims.x) - margins.y,
                                              static_cast<double>(dims.y) - margins.x};
                               if (bottomLeft.x > topRight.x) {
                                   std::swap(bottomLeft.x, topRight.x);
                               }
                               if (bottomLeft.y > topRight.y) {
                                   std::swap(bottomLeft.y, topRight.y);
                               }

                               // clamp position to plotting area
                               p = glm::clamp(p, bottomLeft, topRight);
                               const dvec2 pNormalized = (p - bottomLeft) / (topRight - bottomLeft);

                               const dvec2 rangeX = properties_.xAxis_.range_.get();
                               const dvec2 rangeY = properties_.yAxis_.range_.get();
                               const dvec2 extent{rangeX.y - rangeX.x, rangeY.y - rangeY.x};

                               return dvec2{pNormalized * extent + dvec2{rangeX.x, rangeY.x}};
                           })
    , selectionRectRenderer_(properties_.boxSelectionSettings_) {

    if (processor_) {
        shader_.onReload([this]() { processor_->invalidate(InvalidationLevel::InvalidOutput); });
    }
    properties_.showHighlighted_.onChange([this]() {
        if (!properties_.showHighlighted_) {
            highlighted_.clear();
            highlightChangedCallback_.invoke(highlighted_);
        }
    });
    properties_.xAxis_.useDataRange_.onChange([&]() { properties_.xAxis_.setRange(minmaxX_); });
    properties_.yAxis_.useDataRange_.onChange([&]() { properties_.yAxis_.setRange(minmaxY_); });

    boxSelectionChangedCallback_ = boxSelectionHandler_.addSelectionChangedCallback(
        [this](const std::vector<bool>& selected, bool append) {
            BitSet b(selected);
            // prevent selection of filtered indices
            b -= filtered_;
            if (append) {
                b &= selected_;
            }
            selectionChangedCallback_.invoke(b);
        });
    boxFilteringChangedCallback_ = boxSelectionHandler_.addFilteringChangedCallback(
        [this](const std::vector<bool>& filtered, bool append) {
            BitSet b(filtered);
            if (append) {
                b &= filtered_;
            }
            filteringChangedCallback_.invoke(b);
        });
}

void ScatterPlotGL::plot(Image& dest, bool useAxisRanges) {
    utilgl::activateAndClearTarget(dest);
    plot(dest.getDimensions(), useAxisRanges);
    utilgl::deactivateCurrentTarget();
}

void ScatterPlotGL::plot(Image& dest, const Image& src, bool useAxisRanges) {
    utilgl::activateTargetAndCopySource(dest, src);
    plot(dest.getDimensions(), useAxisRanges);
    utilgl::deactivateCurrentTarget();
}

void ScatterPlotGL::plot(ImageOutport& dest, bool useAxisRanges) {
    utilgl::activateAndClearTarget(dest);
    plot(dest.getDimensions(), useAxisRanges);
    utilgl::deactivateCurrentTarget();
}

void ScatterPlotGL::plot(ImageOutport& dest, ImageInport& src, bool useAxisRanges) {
    utilgl::activateTargetAndCopySource(dest, src);
    plot(dest.getDimensions(), useAxisRanges);
    utilgl::deactivateCurrentTarget();
}

void ScatterPlotGL::plot(const ivec2& start, const ivec2& size, bool useAxisRanges) {
    utilgl::ViewportState viewport(start.x, start.y, size.x, size.y);
    plot(size, useAxisRanges);
}

void ScatterPlotGL::setXAxisLabel(const std::string& label) {
    properties_.xAxis_.setCaption(label);
}

void ScatterPlotGL::setYAxisLabel(const std::string& label) {
    properties_.yAxis_.setCaption(label);
}

void ScatterPlotGL::setXAxis(const Column* col) {
    setXAxisData(col);
    setXAxisLabel(fmt::format("{}{: [}", col->getHeader(), col->getUnit()));
}

void ScatterPlotGL::setYAxis(const Column* col) {
    setYAxisData(col);
    setYAxisLabel(fmt::format("{}{: [}", col->getHeader(), col->getUnit()));
}

void ScatterPlotGL::setXAxisData(const Column* col) {
    if (col) {
        points_.xCoord = col->getBuffer();
        minmaxX_ = vec2(col->getRange());
        properties_.xAxis_.setRange(minmaxX_);
    } else {
        points_.xCoord = nullptr;
    }
    partitionDirty_ = true;
    boxSelectionHandler_.setXAxisData(points_.xCoord);
}

void ScatterPlotGL::setYAxisData(const Column* col) {
    if (col) {
        points_.yCoord = col->getBuffer();
        minmaxY_ = vec2(col->getRange());
        properties_.yAxis_.setRange(minmaxY_);
    } else {
        points_.yCoord = nullptr;
    }
    partitionDirty_ = true;
    boxSelectionHandler_.setYAxisData(points_.yCoord);
}

void ScatterPlotGL::setColorData(const Column* col) {
    if (col) {
        points_.color = col->getBuffer();
        minmaxC_ = col->getRange();
    } else {
        points_.color = nullptr;
    }
    partitionDirty_ = true;
    properties_.tf_.setVisible(points_.color != nullptr);
    properties_.color_.setVisible(points_.color == nullptr);
}

void ScatterPlotGL::setRadiusData(const Column* col) {
    if (col) {
        points_.radius = col->getBuffer();
        minmaxR_ = vec2(col->getRange());
    } else {
        points_.radius = nullptr;
    }
    partitionDirty_ = true;
    properties_.minRadius_.setVisible(points_.radius != nullptr);
}

void ScatterPlotGL::setSortingData(const Column* col) {
    if (col) {
        points_.sorting = col->getBuffer();
    } else {
        points_.sorting = nullptr;
    }
    partitionDirty_ = true;
}

void ScatterPlotGL::setIndexColumn(std::shared_ptr<const TemplateColumn<uint32_t>> indexcol) {
    indexColumn_ = indexcol;

    if (indexColumn_) {
        const auto numValues = static_cast<std::uint32_t>(indexColumn_->getSize());
        picking_.resize(numValues);
        points_.pickIds.getEditableRAMRepresentation()->getDataContainer() =
            util::transform(util::sequence<std::uint32_t>(0, numValues, 1),
                            [&](auto i) { return getGlobalPickId(i); });
    } else {
        points_.pickIds.getEditableRAMRepresentation()->getDataContainer().clear();
    }
}

void ScatterPlotGL::setSortingOrder(SortingOrder order) {
    if (sortOrder_ != order) {
        sortOrder_ = order;
        partitionDirty_ = true;
    }
}

void ScatterPlotGL::setIndices(const BitSet& filtered, const BitSet& selected,
                               const BitSet& highlighted) {
    partitionDirty_ |= filtered_.set(filtered);
    partitionDirty_ |= selected_.set(selected);
    partitionDirty_ |= highlighted_.set(highlighted);
}

void ScatterPlotGL::setFilteredIndices(const BitSet& indices) {
    partitionDirty_ |= filtered_.set(indices);
}

void ScatterPlotGL::setSelectedIndices(const BitSet& indices) {
    partitionDirty_ |= selected_.set(indices);
}

void ScatterPlotGL::setHighlightedIndices(const BitSet& indices) {
    partitionDirty_ |= highlighted_.set(indices);
}

auto ScatterPlotGL::addToolTipCallback(std::function<ToolTipFunc> callback)
    -> ToolTipCallbackHandle {
    return tooltipCallback_.add(callback);
}

auto ScatterPlotGL::addHighlightChangedCallback(std::function<HighlightFunc> callback)
    -> HighlightCallbackHandle {
    return highlightChangedCallback_.add(callback);
}

auto ScatterPlotGL::addSelectionChangedCallback(std::function<SelectionFunc> callback)
    -> SelectionCallbackHandle {
    return selectionChangedCallback_.add(callback);
}

auto ScatterPlotGL::addFilteringChangedCallback(std::function<SelectionFunc> callback)
    -> SelectionCallbackHandle {
    return filteringChangedCallback_.add(callback);
}

void ScatterPlotGL::invokeEvent(Event* event) {
    boxSelectionHandler_.invokeEvent(event);
    if (event->hasBeenUsed()) {
        if (processor_) {
            processor_->invalidate(InvalidationLevel::InvalidOutput);
        }
    }
}

void ScatterPlotGL::plot(const size2_t& dims, bool useAxisRanges) {
    if (partitionDirty_) {
        partitionData();
    }

    if (!points_.indices.empty()) {
        utilgl::BlendModeState blending(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
        utilgl::DepthFuncState depthFunc(GL_LEQUAL);

        TextureUnitContainer cont;
        shader_.activate();
        setShaderUniforms(cont, dims, useAxisRanges);

        points_.boa.bind();
        attachVertexAttributes();

        auto indicesGL = points_.indices.getRepresentation<BufferGL>();
        indicesGL->bind();

        // filtered, regular, selected, highlighted
        std::array<vec4, 4> secondaryColor = {
            properties_.showFiltered_.getColor(), properties_.color_.get(),
            properties_.showSelected_.getColor(), properties_.showHighlighted_.getColor()};
        std::array<float, 4> mixColor = {properties_.showFiltered_.getMixIntensity(), 0.0f,
                                         properties_.showSelected_.getMixIntensity(),
                                         properties_.showHighlighted_.getMixIntensity()};
        std::array<float, 4> mixAlpha = {1.0, 0.0f, 1.0f, 1.0f};
        std::array<bool, 4> enabled = {properties_.showFiltered_, true, properties_.showSelected_,
                                       properties_.showHighlighted_};

        for (size_t i = properties_.showFiltered_ ? 0 : 1; i < points_.offsets.size() - 1; ++i) {
            if (!enabled[i]) continue;
            auto begin = points_.offsets[i];
            auto end = points_.offsets[i + 1];
            if (end == begin) continue;

            shader_.setUniform("secondaryColor.color", secondaryColor[i]);
            shader_.setUniform("secondaryColor.colorMixIn", mixColor[i]);
            shader_.setUniform("secondaryColor.alphaMixIn", mixAlpha[i]);

            glDrawElements(GL_POINTS, static_cast<uint32_t>(end - begin),
                           indicesGL->getFormatType(),
                           reinterpret_cast<const GLvoid*>(begin * sizeof(std::uint32_t)));
        }

        indicesGL->unbind();
        points_.boa.unbind();
        shader_.deactivate();
    }

    selectionRectRenderer_.render(boxSelectionHandler_.getDragRectangle(), dims);
    renderAxis(dims);
}

void ScatterPlotGL::attachVertexAttributes() {
    auto attachAttrib = [&](auto buffer, GLuint loc,
                            BufferObject::BindingType bindingType =
                                BufferObject::BindingType::ForceFloat) {
        if (buffer) {
            points_.boa.attachBufferObjectEnforce(
                buffer->template getRepresentation<BufferGL>()->getBufferObject().get(), loc,
                bindingType);
        } else {
            points_.boa.detachBufferObject(loc);
        }
    };

    attachAttrib(points_.xCoord, 0);
    attachAttrib(points_.yCoord, 1);
    attachAttrib(points_.color, 2);
    attachAttrib(points_.radius, 3);

    const bool picking = picking_.isEnabled() && !points_.pickIds.empty();
    attachAttrib(picking ? &points_.pickIds : nullptr, 4, BufferObject::BindingType::Native);
}

void ScatterPlotGL::setShaderUniforms(TextureUnitContainer& cont, const size2_t& dims,
                                      bool useAxisRanges) {
    shader_.setUniform("pixelSize", vec2(1.0f) / vec2(dims));
    shader_.setUniform("dims", ivec2(dims));
    shader_.setUniform("circle", properties_.useCircle_.get() ? 1 : 0);
    shader_.setUniform("borderWidth", properties_.borderWidth_.get());
    shader_.setUniform("borderColor", properties_.borderColor_.get());
    shader_.setUniform("pickingEnabled", picking_.isEnabled());

    // adjust all margins by axis margin
    shader_.setUniform("margins", properties_.margins_.getAsVec4() + properties_.axisMargin_.get());
    shader_.setUniform("minmaxX", useAxisRanges ? vec2(properties_.xAxis_.range_.get()) : minmaxX_);
    shader_.setUniform("minmaxY", useAxisRanges ? vec2(properties_.yAxis_.range_.get()) : minmaxY_);

    shader_.setUniform("default_color", properties_.color_.get());
    shader_.setUniform("has_color", static_cast<int>(points_.color != nullptr));
    if (points_.color) {
        utilgl::bindAndSetUniforms(shader_, cont, properties_.tf_);
        shader_.setUniform("minmaxC", minmaxC_);
    }
    shader_.setUniform("has_radius", (points_.radius && (minmaxR_.x != minmaxR_.y)) ? 1 : 0);
    shader_.setUniform("minRadius",
                       std::min<float>(properties_.minRadius_, properties_.radiusRange_));
    shader_.setUniform("maxRadius", properties_.radiusRange_.get());
    if (points_.radius) {
        shader_.setUniform("minmaxR", minmaxR_);
    }
}

void ScatterPlotGL::renderAxis(const size2_t& dims) {
    const size2_t lowerLeft(properties_.margins_.getLeft(), properties_.margins_.getBottom());
    const size2_t upperRight(dims.x - 1 - properties_.margins_.getRight(),
                             dims.y - 1 - properties_.margins_.getTop());

    const auto padding = properties_.axisMargin_.get();

    // draw horizontal axis
    axisRenderers_[0].render(dims, lowerLeft + size2_t(padding, 0),
                             size2_t(upperRight.x - padding, lowerLeft.y));
    // draw vertical axis
    axisRenderers_[1].render(dims, lowerLeft + size2_t(0, padding),
                             size2_t(lowerLeft.x, upperRight.y - padding));
}

void ScatterPlotGL::objectPicked(PickingEvent* p) {
    const uint32_t id = static_cast<uint32_t>(p->getPickedId());

    // Show tooltip for current item
    if (properties_.tooltip_ && (p->getHoverState() == PickingHoverState::Move ||
                                 p->getHoverState() == PickingHoverState::Enter)) {
        tooltipCallback_.invoke(p, id);
    } else if (p->getHoverState() == PickingHoverState::Exit) {
        // unset tooltip at all times in case one was set prior disabling tooltips
        p->setToolTip("");
    }

    if (properties_.showHighlighted_) {
        if (p->getHoverState() == PickingHoverState::Enter ||
            (p->getGlobalPickingId() != p->getPreviousGlobalPickingId())) {
            highlightChangedCallback_.invoke(BitSet(id));
        } else if (p->getHoverState() == PickingHoverState::Exit) {
            highlightChangedCallback_.invoke(BitSet());
        }
    }

    if ((p->getPressState() == PickingPressState::Release) &&
        (p->getPressItem() == PickingPressItem::Primary) && !p->getMovedSincePressed()) {
        if (p->modifiers().contains(KeyModifier::Control)) {
            // additive selection: add/remove current id
            BitSet selected(selected_);
            selected.flip(id);
            selectionChangedCallback_.invoke(selected);
        } else {
            selectionChangedCallback_.invoke(BitSet(id));
        }
        // reset box selection handler since the data point was selected here
        boxSelectionHandler_.reset();
        p->setUsed(true);
    }
}

uint32_t ScatterPlotGL::getGlobalPickId(uint32_t localIndex) const {
    return static_cast<uint32_t>(picking_.getPickingId(localIndex));
}

void ScatterPlotGL::partitionData() {
    const auto numDataPoints = static_cast<std::uint32_t>(points_.xCoord->getSize());

    auto& indices = points_.indices.getEditableRAMRepresentation()->getDataContainer();
    if (indices.size() != numDataPoints) {
        auto seq = util::make_sequence<std::uint32_t>(0, numDataPoints);
        indices.assign(seq.begin(), seq.end());
    }

    const auto lastFilteredIt = std::partition(indices.begin(), indices.end(),
                                               [&](auto ind) { return filtered_.contains(ind); });
    const auto lastRegularIt = std::partition(lastFilteredIt, indices.end(), [&](auto ind) {
        return !selected_.contains(ind) && !highlighted_.contains(ind);
    });
    const auto lastSelectedIt = std::partition(lastRegularIt, indices.end(), [&](auto ind) {
        return selected_.contains(ind) && !highlighted_.contains(ind);
    });

    points_.offsets[0] = 0;
    points_.offsets[1] = static_cast<std::uint32_t>(std::distance(indices.begin(), lastFilteredIt));
    points_.offsets[1] = static_cast<std::uint32_t>(std::distance(indices.begin(), lastFilteredIt));
    points_.offsets[2] = static_cast<std::uint32_t>(std::distance(indices.begin(), lastRegularIt));
    points_.offsets[3] = static_cast<std::uint32_t>(std::distance(indices.begin(), lastSelectedIt));
    points_.offsets[4] = numDataPoints;

    // Sort each partition to determine draw order. Prefer sorting buffer over radius for sorting.
    if (const auto& sortingBuf = points_.sorting ? points_.sorting : points_.radius; sortingBuf) {
        sortingBuf->getRepresentation<BufferRAM>()->dispatch<void, dispatching::filter::Scalars>(
            [&](const auto brprecision) {
                const auto& data = brprecision->getDataContainer();

                auto ascending = [&data](std::uint32_t a, std::uint32_t b) {
                    return data[a] < data[b];
                };
                auto descending = [&data](std::uint32_t a, std::uint32_t b) {
                    return data[a] > data[b];
                };
                for (size_t i = 0; i < points_.offsets.size() - 1; ++i) {
                    const auto begin = points_.offsets[i];
                    const auto end = points_.offsets[i + 1];
                    if (sortOrder_ == SortingOrder::Ascending) {
                        std::sort(indices.begin() + begin, indices.begin() + end, ascending);
                    } else {
                        std::sort(indices.begin() + begin, indices.begin() + end, descending);
                    }
                }
            });
    }

    partitionDirty_ = false;
}

}  // namespace plot

}  // namespace inviwo
