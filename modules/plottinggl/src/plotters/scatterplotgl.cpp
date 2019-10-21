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

#include <modules/plottinggl/plotters/scatterplotgl.h>
#include <modules/opengl/buffer/buffergl.h>
#include <modules/opengl/buffer/bufferobject.h>
#include <modules/opengl/buffer/bufferobjectarray.h>
#include <modules/opengl/texture/textureutils.h>
#include <modules/opengl/openglutils.h>

#include <inviwo/core/processors/processor.h>
#include <inviwo/core/interaction/events/pickingevent.h>
#include <inviwo/core/interaction/events/mouseevent.h>
#include <inviwo/core/interaction/events/touchevent.h>
#include <inviwo/core/datastructures/buffer/buffer.h>
#include <inviwo/core/datastructures/buffer/bufferram.h>
#include <inviwo/core/util/colorconversion.h>
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
    , axisRenderers_({{properties_.xAxis_, properties_.yAxis_}})
    , picking_(processor, 1, [this](PickingEvent* p) { objectPicked(p); })
    , processor_(processor) {
    if (processor_) {
        shader_.onReload([this]() { processor_->invalidate(InvalidationLevel::InvalidOutput); });
    }
    properties_.hovering_.onChange([this]() {
        if (!properties_.hovering_.get()) {
            hoverIndex_ = std::nullopt;
        }
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

    IndexBuffer* indices;
    if (radius_) {
        if (indexBuffer) {
            // copy selected indices
            indices_ = std::unique_ptr<IndexBuffer>(indexBuffer->clone());
        } else {
            // no indices given, draw all data points
            if (!indices_) indices_ = std::make_unique<IndexBuffer>(xbuf->getSize());
            auto& inds = indices_->getEditableRAMRepresentation()->getDataContainer();
            inds.resize(xbuf->getSize());
            std::iota(inds.begin(), inds.end(), 0);
        }
        indices = indices_.get();

        // sort according to radii, larger first
        auto& inds = indices->getEditableRAMRepresentation()->getDataContainer();

        radius_->getRepresentation<BufferRAM>()->dispatch<void, dispatching::filter::Scalars>(
            [&inds](auto bufferpr) {
                auto& radii = bufferpr->getDataContainer();
                std::sort(inds.begin(), inds.end(), [&radii](const uint32_t& a, const uint32_t& b) {
                    return radii[a] > radii[b];
                });
            });
    } else {
        if (indexBuffer) {
            // copy selected indices
            indices = indexBuffer;
        } else {
            // no indices given, draw all data points
            if (!indices_) indices_ = std::make_unique<IndexBuffer>(xbuf->getSize());
            auto& inds = indices_->getEditableRAMRepresentation()->getDataContainer();
            inds.resize(xbuf->getSize());
            std::iota(inds.begin(), inds.end(), 0);
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
    if (!selectedIndices_.empty()) {
        shader_.setUniform("has_color", 0);
        shader_.setUniform("default_color", properties_.selectionColor_.get());

        std::vector<uint32_t> selected;
        selected.reserve(selectedIndices_.size());
        std::transform(selectedIndices_.begin(), selectedIndices_.end(),
                       std::back_inserter(selected),
                       [](size_t i) { return static_cast<uint32_t>(i); });

        glDrawElements(GL_POINTS, static_cast<uint32_t>(selected.size()), GL_UNSIGNED_INT,
                       selected.data());
    }
    if (hoverIndex_) {
        shader_.setUniform("has_color", 0);
        shader_.setUniform("default_color", properties_.hoverColor_.get());
        glDrawElements(GL_POINTS, 1, GL_UNSIGNED_INT, &hoverIndex_.value());
    }

    shader_.deactivate();
    boa_->unbind();

    renderAxis(dims);
}

void ScatterPlotGL::setXAxisLabel(const std::string& label) {
    properties_.xAxis_.setCaption(label);
}

void ScatterPlotGL::setYAxisLabel(const std::string& label) {
    properties_.yAxis_.setCaption(label);
}

void ScatterPlotGL::setXAxis(std::shared_ptr<const Column> col) {
    setXAxisLabel(col->getHeader());
    setXAxisData(col->getBuffer());
}

void ScatterPlotGL::setYAxis(std::shared_ptr<const Column> col) {
    setYAxisLabel(col->getHeader());
    setYAxisData(col->getBuffer());
}

void ScatterPlotGL::setXAxisData(std::shared_ptr<const BufferBase> buffer) {
    xAxis_ = buffer;
    if (buffer) {
        auto minmax = util::bufferMinMax(buffer.get(), IgnoreSpecialValues::Yes);
        minmaxX_.x = static_cast<float>(minmax.first.x);
        minmaxX_.y = static_cast<float>(minmax.second.x);

        properties_.xAxis_.setRange(minmaxX_);
    }
}

void ScatterPlotGL::setYAxisData(std::shared_ptr<const BufferBase> buffer) {
    yAxis_ = buffer;
    if (buffer) {
        auto minmax = util::bufferMinMax(buffer.get(), IgnoreSpecialValues::Yes);
        minmaxY_.x = static_cast<float>(minmax.first.x);
        minmaxY_.y = static_cast<float>(minmax.second.x);

        properties_.yAxis_.setRange(minmaxY_);
    }
}

void ScatterPlotGL::setColorData(std::shared_ptr<const BufferBase> buffer) {
    color_ = buffer;
    if (buffer) {
        auto minmax = util::bufferMinMax(buffer.get(), IgnoreSpecialValues::Yes);
        minmaxC_.x = static_cast<float>(minmax.first.x);
        minmaxC_.y = static_cast<float>(minmax.second.x);
    }
    properties_.tf_.setVisible(buffer != nullptr);
    properties_.color_.setVisible(buffer == nullptr);
}

void ScatterPlotGL::setRadiusData(std::shared_ptr<const BufferBase> buffer) {
    radius_ = buffer;
    if (buffer) {
        auto minmax = util::bufferMinMax(buffer.get(), IgnoreSpecialValues::Yes);
        minmaxR_.x = static_cast<float>(minmax.first.x);
        minmaxR_.y = static_cast<float>(minmax.second.x);
    }
    properties_.minRadius_.setVisible(buffer != nullptr);
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

void ScatterPlotGL::setSelectedIndices(const std::unordered_set<size_t>& indices) {
    selectedIndices_ = indices;
}

auto ScatterPlotGL::addToolTipCallback(std::function<ToolTipFunc> callback)
    -> ToolTipCallbackHandle {
    return tooltipCallback_.add(callback);
}

auto ScatterPlotGL::addSelectionChangedCallback(std::function<SelectionFunc> callback)
    -> SelectionCallbackHandle {
    return selectionChangedCallback_.add(callback);
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
    auto idToDataFrameIndex = [this](uint32_t id) -> std::optional<uint32_t> {
        if (!indexColumn_) {
            return std::nullopt;
        }
        auto& indexCol = indexColumn_->getTypedBuffer()->getRAMRepresentation()->getDataContainer();
        auto it = util::find(indexCol, static_cast<uint32_t>(id));
        if (it != indexCol.end()) {
            return *it;
        } else {
            return std::nullopt;
        }
    };

    const uint32_t id = static_cast<uint32_t>(p->getPickedId());
    auto rowIndex = idToDataFrameIndex(id);

    // Show tooltip for current item
    if (rowIndex) {
        if (p->getHoverState() == PickingHoverState::Move ||
            p->getHoverState() == PickingHoverState::Enter) {

            tooltipCallback_.invoke(p, rowIndex.value());

        } else if (p->getHoverState() == PickingHoverState::Exit) {
            p->setToolTip("");
        }
    }

    if (properties_.hovering_.get()) {
        if (p->getHoverState() == PickingHoverState::Enter) {
            hoverIndex_ = id;
            if (processor_) {
                processor_->invalidate(InvalidationLevel::InvalidOutput);
            }
        } else if (p->getHoverState() == PickingHoverState::Exit) {
            hoverIndex_ = std::nullopt;
            if (processor_) {
                processor_->invalidate(InvalidationLevel::InvalidOutput);
            }
        }
    }

    if ((p->getPressState() == PickingPressState::Release) &&
        (p->getPressItem() == PickingPressItem::Primary) &&
        (p->getCurrentGlobalPickingId() == p->getPressedGlobalPickingId())) {
        if (selectedIndices_.count(id)) {
            selectedIndices_.erase(id);
        } else {
            selectedIndices_.insert(id);
        }
        // selection changed, inform processor
        selectionChangedCallback_.invoke(selectedIndices_);
    }
}

uint32_t ScatterPlotGL::getGlobalPickId(uint32_t localIndex) const {
    return static_cast<uint32_t>(picking_.getPickingId(localIndex));
}

}  // namespace plot

}  // namespace inviwo
