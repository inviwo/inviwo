/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2021 Inviwo Foundation
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

namespace inviwo {

namespace plot {

const std::string ScatterPlotGL::Properties::classIdentifier =
    "org.inviwo.ScatterPlotGL.Properties";
std::string ScatterPlotGL::Properties::getClassIdentifier() const { return classIdentifier; }

ScatterPlotGL::Properties::Properties(std::string identifier, std::string displayName,
                                      InvalidationLevel invalidationLevel,
                                      PropertySemantics semantics)
    : CompositeProperty(identifier, displayName, invalidationLevel, semantics)
    , useCircle_("useCircle", "Use Circles (else squares)", true)
    , radiusRange_("radius", "Radius", 5, 0, 10, 0.01f)
    , minRadius_("minRadius", "Min Radius", 0.1f, 0, 10, 0.01f)
    , tf_("transferFunction", "Transfer Function",
          TransferFunction({{0.0, vec4(1.0f)}, {1.0, vec4(1.0f)}}))
    , color_("color", "Color", vec4(1, 0, 0, 1), vec4(0), vec4(1), vec4(0.1f),
             InvalidationLevel::InvalidOutput, PropertySemantics::Color)
    , hoverColor_("hoverColor", "Hover Color", vec4(1.0f, 0.906f, 0.612f, 1))
    , selectionColor_("selectionColor", "Selection Color", vec4(1.0f, 0.769f, 0.247f, 1))
    , boxSelectionSettings_("dragRectSettings", "Box Selection/Filtering")
    , margins_("margins", "Margins", 5, 5, 5, 5)
    , axisMargin_("axisMargin", "Axis Margin", 15.0f, 0.0f, 50.0f)

    , borderWidth_("borderWidth", "Border Width", 2, 0, 20)
    , borderColor_("borderColor", "Border Color", vec4(0, 0, 0, 1))
    , hovering_("hovering", "Enable Hovering", true)

    , axisStyle_("axisStyle", "Global Axis Style")
    , xAxis_("xAxis", "X Axis")
    , yAxis_("yAxis", "Y Axis", AxisProperty::Orientation::Vertical) {
    hoverColor_.setSemantics(PropertySemantics::Color);
    selectionColor_.setSemantics(PropertySemantics::Color);
    borderColor_.setSemantics(PropertySemantics::Color);

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
    , hoverColor_(rhs.hoverColor_)
    , selectionColor_(rhs.selectionColor_)
    , boxSelectionSettings_(rhs.boxSelectionSettings_)
    , margins_(rhs.margins_)
    , axisMargin_(rhs.axisMargin_)
    , borderWidth_(rhs.borderWidth_)
    , borderColor_(rhs.borderColor_)
    , hovering_(rhs.hovering_)
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
    , xAxis_(nullptr)
    , yAxis_(nullptr)
    , color_(nullptr)
    , radius_(nullptr)
    , sorting_(nullptr)
    , axisRenderers_({{properties_.xAxis_, properties_.yAxis_}})
    , picking_(processor, 1, [this](PickingEvent* p) { objectPicked(p); })
    , processor_(processor)
    , boxSelectionHandler_(properties_.boxSelectionSettings_, xAxis_, yAxis_,
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
    properties_.hovering_.onChange([this]() {
        if (!properties_.hovering_.get()) {
            highlighted_.clear();
            highlightChangedCallback_.invoke(highlighted_);
        }
    });

    boxSelectionChangedCallBack_ = boxSelectionHandler_.addSelectionChangedCallback(
        [this](const std::vector<bool>& selected, bool append) {
            ensureSelectAndFilterSizes();
            for (auto&& [ind, elem] : util::enumerate(selected)) {
                if (append && elem)
                    selected_[ind] = elem;
                else {
                    selected_[ind] = elem;
                }
            }

            selectedIndicesGLDirty_ = true;
            // selection changed, inform processor
            selectionChangedCallback_.invoke(selected_);
        });
    boxFilteringChangedCallBack_ = boxSelectionHandler_.addFilteringChangedCallback(
        [this](const std::vector<bool>& filtered, bool append) {
            ensureSelectAndFilterSizes();
            for (auto&& [ind, elem] : util::enumerate(filtered)) {
                if (append && elem)
                    filtered_[ind] = elem;
                else {
                    filtered_[ind] = elem;
                }
            }
            filteringDirty_ = true;
            // May filter selected points
            selectedIndicesGLDirty_ = true;
            filteringChangedCallback_.invoke(filtered_);
        });
}

void ScatterPlotGL::plot(Image& dest, IndexBuffer* indices, bool useAxisRanges) {
    utilgl::activateAndClearTarget(dest);
    plot(dest.getDimensions(), indices, useAxisRanges);
    utilgl::deactivateCurrentTarget();
}

void ScatterPlotGL::plot(Image& dest, const Image& src, IndexBuffer* indices, bool useAxisRanges) {
    utilgl::activateTargetAndCopySource(dest, src);
    plot(dest.getDimensions(), indices, useAxisRanges);
    utilgl::deactivateCurrentTarget();
}

void ScatterPlotGL::plot(ImageOutport& dest, IndexBuffer* indices, bool useAxisRanges) {
    utilgl::activateAndClearTarget(dest);
    plot(dest.getDimensions(), indices, useAxisRanges);
    utilgl::deactivateCurrentTarget();
}

void ScatterPlotGL::plot(ImageOutport& dest, ImageInport& src, IndexBuffer* indices,
                         bool useAxisRanges) {
    utilgl::activateTargetAndCopySource(dest, src);
    plot(dest.getDimensions(), indices, useAxisRanges);
    utilgl::deactivateCurrentTarget();
}

void ScatterPlotGL::plot(const ivec2& start, const ivec2& size, IndexBuffer* indices,
                         bool useAxisRanges) {
    utilgl::ViewportState viewport(start.x, start.y, size.x, size.y);
    plot(size, indices, useAxisRanges);
}

void ScatterPlotGL::plot(const size2_t& dims, IndexBuffer* indexBuffer, bool useAxisRanges) {
    ensureSelectAndFilterSizes();
    // adjust all margins by axis margin
    vec4 margins = properties_.margins_.getAsVec4() + properties_.axisMargin_.get();

    shader_.activate();

    vec2 pixelSize = vec2(1) / vec2(dims);

    TextureUnitContainer cont;
    if (useAxisRanges) {
        shader_.setUniform("minmaxX", vec2(properties_.xAxis_.range_.get()));
        shader_.setUniform("minmaxY", vec2(properties_.yAxis_.range_.get()));
    } else {
        shader_.setUniform("minmaxX", minmaxX_);
        shader_.setUniform("minmaxY", minmaxY_);
    }
    shader_.setUniform("borderWidth", properties_.borderWidth_.get());
    shader_.setUniform("borderColor", properties_.borderColor_.get());

    shader_.setUniform("pixelSize", pixelSize);
    shader_.setUniform("dims", ivec2(dims));
    shader_.setUniform("circle", properties_.useCircle_.get() ? 1 : 0);

    auto xbuf = xAxis_->getRepresentation<BufferGL>();
    auto ybuf = yAxis_->getRepresentation<BufferGL>();
    auto xbufObj = xbuf->getBufferObject();
    auto ybufObj = ybuf->getBufferObject();

    if (!boa_) {
        boa_ = std::make_unique<BufferObjectArray>();
    }
    boa_->bind();
    boa_->clear();

    boa_->attachBufferObject(xbufObj.get(), 0, BufferObject::BindingType::ForceFloat);
    boa_->attachBufferObject(ybufObj.get(), 1, BufferObject::BindingType::ForceFloat);

    if (picking_.isEnabled() && pickIds_) {
        // bind picking buffer
        auto pickBuf = pickIds_->getRepresentation<BufferGL>();
        auto pickBufObj = pickBuf->getBufferObject();
        boa_->attachBufferObject(pickBufObj.get(), 4);
    }
    shader_.setUniform("pickingEnabled", picking_.isEnabled());

    if (color_) {
        utilgl::bindAndSetUniforms(shader_, cont, properties_.tf_);
        shader_.setUniform("minmaxC", minmaxC_);
        shader_.setUniform("has_color", 1);

        auto cbuf = color_->getRepresentation<BufferGL>();
        auto cbufObj = cbuf->getBufferObject();
        boa_->attachBufferObject(cbufObj.get(), 2, BufferObject::BindingType::ForceFloat);

    } else {
        shader_.setUniform("has_color", 0);
        shader_.setUniform("default_color", properties_.color_.get());
    }

    float minRadius = properties_.minRadius_.get();
    float maxRadius = properties_.radiusRange_.get();

    if (minRadius > maxRadius) {
        minRadius = maxRadius;
    }
    if (maxRadius < minRadius) {
        maxRadius = minRadius;
    }

    shader_.setUniform("minRadius", minRadius);
    shader_.setUniform("maxRadius", maxRadius);
    shader_.setUniform("margins", margins);

    utilgl::BlendModeState blending(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    utilgl::DepthFuncState depthFunc(GL_LEQUAL);

    if (radius_) {
        shader_.setUniform("minmaxR", minmaxR_);
        shader_.setUniform("has_radius", (minmaxR_.x != minmaxR_.y ? 1 : 0));

        auto rbuf = radius_->getRepresentation<BufferGL>();
        auto rbufObj = rbuf->getBufferObject();
        boa_->attachBufferObject(rbufObj.get(), 3, BufferObject::BindingType::ForceFloat);

    } else {
        shader_.setUniform("has_radius", 0);
    }
    // Will be called if no indexBuffer is specified.
    // (assuming brushing & linking filters, so internal filters should not be applied)
    auto setupInternalFiltering = [this, xbuf]() {
        if (!filteringDirty_) return;

        std::vector<uint32_t> selectedIndices;

        // no indices given, draw all non-filtered data points
        size_t nFiltered = std::count(filtered_.begin(), filtered_.end(), true);
        // std::reduce<size_t>(filtered_.begin(), filtered_.end(), size_t(0), [);
        auto nNotFiltered = xbuf->getSize() - nFiltered;

        if (!indices_) indices_ = std::make_unique<IndexBuffer>(nNotFiltered);
        auto& inds = indices_->getEditableRAMRepresentation()->getDataContainer();
        inds.clear();
        inds.reserve(nNotFiltered);

        if (indexColumn_) {
            auto& indexCol =
                indexColumn_->getTypedBuffer()->getRAMRepresentation()->getDataContainer();
            for (auto [ind, filtered] : util::enumerate(filtered_)) {
                if (!filtered) {
                    inds.push_back(indexCol[ind]);
                }
            }
        } else {
            auto indicesSeq =
                util::make_sequence<unsigned int>(0, static_cast<unsigned int>(xbuf->getSize()), 1);
            std::copy_if(indicesSeq.begin(), indicesSeq.end(), std::back_inserter(inds),
                         [this](auto val) { return !filtered_[val]; });
        }
        filteringDirty_ = false;
    };
    IndexBuffer* indices;
    if (sorting_ || radius_) {
        // prefer sorting column over radius if defined
        std::shared_ptr<const BufferBase>& buffer = sorting_ ? sorting_ : radius_;

        if (indexBuffer) {
            // copy selected indices
            indices_ = std::unique_ptr<IndexBuffer>(indexBuffer->clone());
        } else {
            setupInternalFiltering();
        }
        indices = indices_.get();

        // sort according to radii, larger first
        auto& inds = indices->getEditableRAMRepresentation()->getDataContainer();

        buffer->getRepresentation<BufferRAM>()->dispatch<void, dispatching::filter::Scalars>(
            [&](auto bufferpr) {
                const auto& data = bufferpr->getDataContainer();
                if (sortOrder_ == SortingOrder::Ascending) {
                    std::sort(inds.begin(), inds.end(),
                              [&data](const uint32_t& a, const uint32_t& b) {
                                  return data[a] < data[b];
                              });
                } else {
                    std::sort(inds.begin(), inds.end(),
                              [&data](const uint32_t& a, const uint32_t& b) {
                                  return data[a] > data[b];
                              });
                }
            });
    } else {
        if (indexBuffer) {
            // copy selected indices
            indices = indexBuffer;
        } else {
            setupInternalFiltering();
            indices = indices_.get();
        }
    }

    boa_->bind();
    auto indicesGL = indices->getRepresentation<BufferGL>();
    indicesGL->bind();
    glDrawElements(GL_POINTS, static_cast<uint32_t>(indices->getSize()), indicesGL->getFormatType(),
                   nullptr);
    indicesGL->getBufferObject()->unbind();
    // draw selected and hovered points on top

    if (selectedIndicesGLDirty_ || nSelectedButNotFiltered_ > 0) {
        shader_.setUniform("has_color", 0);
        shader_.setUniform("default_color", properties_.selectionColor_.get());
        if (selectedIndicesGLDirty_) {
            nSelectedButNotFiltered_ = 0;
            std::vector<uint32_t> selectedIndices;
            for (auto [ind, selected, filtered] : util::enumerate(selected_, filtered_)) {
                if (selected && !filtered) {
                    selectedIndices.push_back(static_cast<uint32_t>(ind));
                }
            }

            if (selectedIndicesGL_.getSizeInBytes() <
                static_cast<GLsizeiptr>(selectedIndices.size() * sizeof(uint32_t))) {
                selectedIndicesGL_.setSizeInBytes(selectedIndices.size() * sizeof(uint32_t));
            }
            nSelectedButNotFiltered_ = selectedIndices.size();
            selectedIndicesGL_.upload(selectedIndices.data(),
                                      selectedIndices.size() * sizeof(uint32_t));
            selectedIndicesGLDirty_ = false;
        }
        if (nSelectedButNotFiltered_ > 0) {
            selectedIndicesGL_.bind();
            glDrawElements(GL_POINTS, static_cast<uint32_t>(nSelectedButNotFiltered_),
                           selectedIndicesGL_.getFormatType(), nullptr);
            selectedIndicesGL_.unbind();
        }
    }
    if (!highlighted_.empty()) {
        shader_.setUniform("has_color", 0);
        shader_.setUniform("default_color", properties_.hoverColor_.get());

        if (highlightDirty_) {
            if (highlightIndexGL_.getSizeInBytes() <
                static_cast<GLsizeiptr>(highlighted_.size() * sizeof(uint32_t))) {
                highlightIndexGL_.setSizeInBytes(highlighted_.size() * sizeof(uint32_t));
            }
            // Will both bind and upload
            highlightIndexGL_.upload(static_cast<const void*>(highlighted_.toVector().data()),
                                     sizeof(uint32_t) * highlighted_.size());
            highlightDirty_ = false;
        } else {
            highlightIndexGL_.bind();
        }
        glDrawElements(GL_POINTS, highlighted_.cardinality(), highlightIndexGL_.getFormatType(),
                       nullptr);
        highlightIndexGL_.unbind();
    }

    shader_.deactivate();
    boa_->unbind();

    // Render selection rectangle
    selectionRectRenderer_.render(boxSelectionHandler_.getDragRectangle(), dims);

    renderAxis(dims);
}  // namespace plot

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
        xAxis_ = col->getBuffer();
        minmaxX_ = vec2(col->getRange());
        properties_.xAxis_.setRange(minmaxX_);
    } else {
        xAxis_ = nullptr;
    }
    boxSelectionHandler_.setXAxisData(xAxis_);
}

void ScatterPlotGL::setYAxisData(const Column* col) {
    if (col) {
        yAxis_ = col->getBuffer();
        minmaxY_ = vec2(col->getRange());
        properties_.yAxis_.setRange(minmaxY_);
    } else {
        yAxis_ = nullptr;
    }
    boxSelectionHandler_.setYAxisData(yAxis_);
}

void ScatterPlotGL::setColorData(const Column* col) {
    if (col) {
        color_ = col->getBuffer();
        minmaxC_ = col->getRange();
    } else {
        color_ = nullptr;
    }
    properties_.tf_.setVisible(color_ != nullptr);
    properties_.color_.setVisible(color_ == nullptr);
}

void ScatterPlotGL::setRadiusData(const Column* col) {
    if (col) {
        radius_ = col->getBuffer();
        minmaxR_ = vec2(col->getRange());
    } else {
        radius_ = nullptr;
    }
    properties_.minRadius_.setVisible(radius_ != nullptr);
}

void ScatterPlotGL::setSortingData(const Column* col) {
    if (col) {
        sorting_ = col->getBuffer();
    } else {
        sorting_ = nullptr;
    }
}

void ScatterPlotGL::setIndexColumn(std::shared_ptr<const TemplateColumn<uint32_t>> indexcol) {
    indexColumn_ = indexcol;

    if (indexColumn_) {
        picking_.resize(indexColumn_->getSize());

        std::vector<uint32_t> ids(indexColumn_->getSize());
        for (size_t i = 0; i < ids.size(); ++i) {
            ids[i] = getGlobalPickId(static_cast<uint32_t>(i));
        }
        pickIds_ = util::makeBuffer<uint32_t>(std::move(ids));
    } else {
        pickIds_.reset();
    }
}

void ScatterPlotGL::setSortingOrder(SortingOrder order) { sortOrder_ = order; }

void ScatterPlotGL::setHighlightedIndices(const BitSet& indices) {
    highlighted_ = indices;
    highlightDirty_ = true;
}

void ScatterPlotGL::setSelectedIndices(const BitSet& indices) {
    ensureSelectAndFilterSizes();
    std::fill(selected_.begin(), selected_.end(), false);
    selected_.resize(xAxis_->getSize(), false);
    for (auto i : indices) {
        selected_[i] = true;
    }
    selectedIndicesGLDirty_ = true;
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
    if (p->getHoverState() == PickingHoverState::Move ||
        p->getHoverState() == PickingHoverState::Enter) {

        tooltipCallback_.invoke(p, id);

    } else if (p->getHoverState() == PickingHoverState::Exit) {
        p->setToolTip("");
    }

    if (properties_.hovering_.get()) {
        if (p->getHoverState() == PickingHoverState::Enter ||
            (p->getGlobalPickingId() != p->getPreviousGlobalPickingId())) {
            highlighted_.clear();
            highlighted_.add(id);
            highlightChangedCallback_.invoke(highlighted_);
            highlightDirty_ = true;
        } else if (p->getHoverState() == PickingHoverState::Exit) {
            highlighted_.clear();
            highlightChangedCallback_.invoke(highlighted_);
        }
    }

    if ((p->getPressState() == PickingPressState::Release) &&
        (p->getPressItem() == PickingPressItem::Primary) &&
        (p->getCurrentGlobalPickingId() == p->getPressedGlobalPickingId())) {
        ensureSelectAndFilterSizes();
        selected_[id] = !selected_[id];
        selectedIndicesGLDirty_ = true;

        // selection changed, inform processor
        selectionChangedCallback_.invoke(selected_);
    }
    p->setUsed(true);
}

uint32_t ScatterPlotGL::getGlobalPickId(uint32_t localIndex) const {
    return static_cast<uint32_t>(picking_.getPickingId(localIndex));
}

void ScatterPlotGL::ensureSelectAndFilterSizes() {
    if (xAxis_->getSize() != selected_.size() || xAxis_->getSize() != filtered_.size()) {
        selected_.resize(xAxis_->getSize(), false);
        filtered_.resize(xAxis_->getSize(), false);
        selectedIndicesGLDirty_ = true;
        filteringDirty_ = true;
    }
}

}  // namespace plot

}  // namespace inviwo
