/*********************************************************************************
 *
 *  Inviwo - Interactive Visualization Workshop
 *
 *  Copyright (c) 2018-2019 Inviwo Foundation
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *
 *  1. Redistributions of source code must retain the above copyright notice, this
 *  list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright notice,
 *  this list of conditions and the following disclaimer in the documentation
 *  and/or other materials provided with the distribution.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 *  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 *  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 *  ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 *  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 *  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *********************************************************************************/

#include <modules/plottinggl/plotters/persistencediagramplotgl.h>
#include <modules/opengl/buffer/buffergl.h>
#include <modules/opengl/buffer/bufferobject.h>
#include <modules/opengl/texture/textureutils.h>
#include <modules/opengl/openglutils.h>
#include <modules/opengl/shader/shaderutils.h>
#include <modules/opengl/geometry/meshgl.h>
#include <modules/opengl/volume/volumeutils.h>
#include <modules/base/algorithm/cohensutherland.h>

#include <inviwo/core/processors/processor.h>
#include <inviwo/core/datastructures/buffer/buffer.h>
#include <inviwo/core/datastructures/buffer/bufferram.h>
#include <inviwo/core/util/zip.h>
#include <inviwo/core/interaction/events/pickingevent.h>
#include <inviwo/core/interaction/events/mouseevent.h>
#include <inviwo/core/interaction/events/touchevent.h>
#include <inviwo/core/datastructures/geometry/typedmesh.h>

#include <tuple>

namespace inviwo {

namespace plot {

const std::string PersistenceDiagramPlotGL::Properties::classIdentifier =
    "org.inviwo.PersistenceDiagramPlotGL.Properties";
std::string PersistenceDiagramPlotGL::Properties::getClassIdentifier() const {
    return classIdentifier;
}

PersistenceDiagramPlotGL::Properties::Properties(std::string identifier, std::string displayName,
                                                 InvalidationLevel invalidationLevel,
                                                 PropertySemantics semantics)
    : CompositeProperty(identifier, displayName, invalidationLevel, semantics)
    , showPoints_("showPoints", "Show Points", true)
    , radius_("radius", "Point Radius", 5, 0, 10, 0.01f)
    , lineWidth_("lineWidth", "Line Width", 2, 0, 20, 0.01f)
    , lineWidthDiagonal_("lineWidthDiagonal", "Line Width Diagonal", 1.5, 0, 20, 0.01f)
    , pointColor_("pointColor", "Point Color", vec4(vec3(65, 122, 155) / 255.0f, 1.0f), vec4(0),
                  vec4(1), vec4(0.1f), InvalidationLevel::InvalidOutput, PropertySemantics::Color)
    , lineColor_("lineColor", "Line Color", vec4(vec3(34, 96, 150) / 255.0f, 1.0f), vec4(0),
                 vec4(1), vec4(0.1f), InvalidationLevel::InvalidOutput, PropertySemantics::Color)
    , hoverColor_("hoverColor", "Hover Color", vec4(1.0f, 0.906f, 0.612f, 1))
    , selectionColor_("selectionColor", "Selection Color", vec4(1.0f, 0.769f, 0.247f, 1))
    , tf_("transferFunction", "Transfer Function",
          TransferFunction({{0.0, vec4(1.0f)}, {1.0, vec4(1.0f)}}))
    , margins_("margins", "Margins", 5, 5, 5, 5)
    , axisMargin_("axisMargin", "Axis Margin", 15.0f, 0.0f, 50.0f)

    , borderWidth_("borderWidth", "Border width", 2, 0, 20)
    , borderColor_("borderColor", "Border color", vec4(0, 0, 0, 1))
    , hovering_("hovering", "Enable Hovering", true)

    , axisStyle_("axisStyle", "Global Axis Style")
    , xAxis_("xAxis", "X Axis")
    , yAxis_("yAxis", "Y Axis", AxisProperty::Orientation::Vertical) {

    util::for_each_in_tuple([&](auto& e) { this->addProperty(e); }, props());

    hoverColor_.setSemantics(PropertySemantics::Color);
    selectionColor_.setSemantics(PropertySemantics::Color);
    borderColor_.setSemantics(PropertySemantics::Color);

    axisStyle_.setCollapsed(true);
    axisStyle_.registerProperties(xAxis_, yAxis_);

    yAxis_.flipped_.set(true);

    pointColor_.setVisible(true);
    tf_.setVisible(!pointColor_.getVisible());
}

PersistenceDiagramPlotGL::Properties::Properties(const PersistenceDiagramPlotGL::Properties& rhs)
    : CompositeProperty(rhs)
    , showPoints_(rhs.showPoints_)
    , radius_(rhs.radius_)
    , lineWidth_(rhs.lineWidth_)
    , lineWidthDiagonal_(rhs.lineWidthDiagonal_)
    , pointColor_(rhs.pointColor_)
    , lineColor_(rhs.lineColor_)
    , hoverColor_(rhs.hoverColor_)
    , selectionColor_(rhs.selectionColor_)
    , tf_(rhs.tf_)
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

PersistenceDiagramPlotGL::Properties* PersistenceDiagramPlotGL::Properties::clone() const {
    return new Properties(*this);
}

PersistenceDiagramPlotGL::PersistenceDiagramPlotGL(Processor* processor)
    : properties_("persistenceDiagram", "PersistenceDiagram")
    , pointShader_("persistencediagram.vert", "scatterplot.geom", "scatterplot.frag")
    , lineShader_("persistencediagramlines.vert", "linerenderer.geom", "linerenderer.frag")
    , xAxis_(nullptr)
    , yAxis_(nullptr)
    , color_(nullptr)
    , axisRenderers_({{properties_.xAxis_, properties_.yAxis_}})
    , picking_(processor, 1, [this](PickingEvent* p) { objectPicked(p); })
    , processor_(processor) {

    if (processor_) {
        pointShader_.onReload(
            [this]() { processor_->invalidate(InvalidationLevel::InvalidOutput); });
        lineShader_.onReload(
            [this]() { processor_->invalidate(InvalidationLevel::InvalidOutput); });
    }
    properties_.hovering_.onChange([this]() {
        if (!properties_.hovering_.get()) {
            hoveredIndices_.clear();
        }
    });
}

void PersistenceDiagramPlotGL::plot(Image& dest, IndexBuffer* indices, bool useAxisRanges) {
    utilgl::activateAndClearTarget(dest);
    plot(dest.getDimensions(), indices, useAxisRanges);
    utilgl::deactivateCurrentTarget();
}

void PersistenceDiagramPlotGL::plot(Image& dest, const Image& src, IndexBuffer* indices,
                                    bool useAxisRanges) {
    utilgl::activateTargetAndCopySource(dest, src);
    plot(dest.getDimensions(), indices, useAxisRanges);
    utilgl::deactivateCurrentTarget();
}

void PersistenceDiagramPlotGL::plot(ImageOutport& dest, IndexBuffer* indices, bool useAxisRanges) {
    utilgl::activateAndClearTarget(dest);
    plot(dest.getDimensions(), indices, useAxisRanges);
    utilgl::deactivateCurrentTarget();
}

void PersistenceDiagramPlotGL::plot(ImageOutport& dest, ImageInport& src, IndexBuffer* indices,
                                    bool useAxisRanges) {
    utilgl::activateTargetAndCopySource(dest, src);
    plot(dest.getDimensions(), indices, useAxisRanges);
    utilgl::deactivateCurrentTarget();
}

void PersistenceDiagramPlotGL::plot(const ivec2& start, const ivec2& size, IndexBuffer* indices,
                                    bool useAxisRanges) {
    utilgl::ViewportState viewport(start.x, start.y, size.x, size.y);
    plot(size, indices, useAxisRanges);
}

void PersistenceDiagramPlotGL::plot(const size2_t& dims, IndexBuffer* indices, bool useAxisRanges) {
    // create a new mesh
    using PosBuffer =
        buffertraits::TypedMeshBufferBase<float, 2, static_cast<int>(BufferType::PositionAttrib)>;
    using PickingBuffer = buffertraits::TypedMeshBufferBase<uint32_t, 1, 4>;

    using PosColorMesh = TypedMesh<PosBuffer, buffertraits::ColorsBuffer, PickingBuffer>;
    PosColorMesh mesh(DrawType::Points, ConnectivityType::None);

    std::vector<PosColorMesh::Vertex> vertices;
    std::vector<uint32_t> indicesDiag;
    std::vector<uint32_t> indicesPoints;
    std::vector<uint32_t> indicesLines;

    // clip everything against plot bbox, i.e. axis ranges or data range
    const vec2 rectMin =
        useAxisRanges ? vec2(properties_.xAxis_.range_.get().x, properties_.yAxis_.range_.get().x)
                      : vec2(minmaxX_.x, minmaxY_.x);
    const vec2 rectMax =
        useAxisRanges ? vec2(properties_.xAxis_.range_.get().y, properties_.yAxis_.range_.get().y)
                      : vec2(minmaxX_.y, minmaxY_.y);

    if (properties_.lineWidthDiagonal_.get() > 0.0f) {
        // diagonal from (x_min,x_min) to (x_max,x_max)
        auto clipped = algorithm::clipLineCohenSutherland(vec2(minmaxX_.x), vec2(minmaxX_.y),
                                                          rectMin, rectMax);
        if (std::get<0>(clipped)) {
            uint32_t indexOffset = static_cast<uint32_t>(vertices.size());

            vertices.push_back({std::get<1>(clipped), properties_.pointColor_.get(), 0});
            vertices.push_back({std::get<2>(clipped), properties_.pointColor_.get(), 0});

            indicesDiag.push_back(indexOffset);
            indicesDiag.push_back(indexOffset + 1);
        }
    }

    std::vector<vec2> xyPairs(xAxis_->getSize());
    auto copyBufferToComponent = [&](std::shared_ptr<const BufferBase> buffer,
                                     int targetComponent) {
        buffer->getRepresentation<BufferRAM>()->dispatch<void, dispatching::filter::Scalars>(
            [&](auto bufferpr) {
                bufferpr->getDataContainer();
                for (auto&& i : util::zip(xyPairs, bufferpr->getDataContainer())) {
                    get<0>(i)[targetComponent] = static_cast<float>(get<1>(i));
                }
            });
    };

    copyBufferToComponent(xAxis_, 0);
    copyBufferToComponent(yAxis_, 1);

    std::vector<uint32_t> selectedIndices;
    if (indices) {
        // copy selected indices
        selectedIndices = indices->getRAMRepresentation()->getDataContainer();
    } else {
        // no indices given, draw all data points
        auto seq = util::make_sequence<uint32_t>(0, static_cast<uint32_t>(xyPairs.size()), 1);
        selectedIndices = std::vector<uint32_t>(seq.begin(), seq.end());
    }

    {
        // vertical lines
        uint32_t indexOffset = static_cast<uint32_t>(vertices.size());
        for (auto& index : selectedIndices) {
            auto& point = xyPairs[index];

            auto clipped = algorithm::clipLineCohenSutherland(vec2(point.x), vec2(point.x, point.y),
                                                              rectMin, rectMax);
            if (std::get<0>(clipped)) {
                vertices.push_back(
                    {std::get<1>(clipped), properties_.lineColor_.get(), getGlobalPickId(index)});
                vertices.push_back(
                    {std::get<2>(clipped), properties_.lineColor_.get(), getGlobalPickId(index)});

                indicesLines.push_back(indexOffset++);
                indicesLines.push_back(indexOffset++);
            }
        }
    }

    // data points
    if (properties_.showPoints_.get()) {
        auto normalizeValue = [range = minmaxC_](double value) {
            return (value - range.x) / (range.y - range.x);
        };
        auto colorBuffer = (color_ ? color_->getRepresentation<BufferRAM>() : nullptr);

        auto getColor = [this,
                         hoverEnabled = (properties_.hovering_.get() && !hoveredIndices_.empty()),
                         buffer = colorBuffer, normalizeValue](uint32_t index) {
            if (hoverEnabled && util::contains(hoveredIndices_, index)) {
                return properties_.hoverColor_.get();
            } else if (util::contains(selectedIndices_, index)) {
                return properties_.selectionColor_.get();
            } else if (color_) {
                return properties_.tf_.get().sample(normalizeValue(buffer->getAsDouble(index)));
            } else {
                return properties_.pointColor_.get();
            }
        };

        uint32_t indexOffset = static_cast<uint32_t>(vertices.size());
        for (auto& index : selectedIndices) {
            const vec2 pLower(xyPairs[index].x);
            const vec2 pUpper(xyPairs[index].x, xyPairs[index].y);

            const vec4 color = getColor(index);
            if (algorithm::insideRect(pLower, rectMin, rectMax)) {
                vertices.push_back({pLower, color, getGlobalPickId(index)});
                indicesPoints.push_back(indexOffset++);
            }
            if (algorithm::insideRect(pUpper, rectMin, rectMax)) {
                vertices.push_back({pUpper, color, getGlobalPickId(index)});
                indicesPoints.push_back(indexOffset++);
            }
        }
    }

    mesh.addVertices(vertices);

    MeshDrawerGL::DrawObject drawer(mesh.getRepresentation<MeshGL>(), mesh.getDefaultMeshInfo());

    utilgl::BlendModeState blending(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    utilgl::DepthFuncState depthFunc(GL_LEQUAL);

    vec2 minmaxX = useAxisRanges ? vec2(properties_.xAxis_.range_.get()) : minmaxX_;
    vec2 minmaxY = useAxisRanges ? vec2(properties_.yAxis_.range_.get()) : minmaxY_;
    // adjust all margins by axis margin
    vec4 margins = properties_.margins_.getAsVec4() + properties_.axisMargin_.get();

    lineShader_.activate();
    lineShader_.setUniform("minmaxX", minmaxX);
    lineShader_.setUniform("minmaxY", minmaxY);
    lineShader_.setUniform("margins", margins);
    lineShader_.setUniform("dims", ivec2(dims));
    utilgl::setShaderUniforms(lineShader_, mesh, "geometry");

    renderLines(dims, indicesDiag, indicesLines);
    lineShader_.deactivate();

    if (properties_.showPoints_.get()) {
        pointShader_.activate();
        pointShader_.setUniform("minmaxX", minmaxX);
        pointShader_.setUniform("minmaxY", minmaxY);
        pointShader_.setUniform("margins", margins);

        renderPoints(dims, indicesPoints);
        pointShader_.deactivate();
    }

    renderAxis(dims);
}

void PersistenceDiagramPlotGL::renderLines(const size2_t& dims,
                                           const std::vector<uint32_t>& diagonalIndices,
                                           const std::vector<uint32_t>& indices) {
    lineShader_.setUniform("roundCaps", true);
    lineShader_.setUniform("screenDim", vec2(dims));
    if (!diagonalIndices.empty()) {
        lineShader_.setUniform("lineWidth", properties_.lineWidthDiagonal_.get());
        lineShader_.setUniform("pickingEnabled", false);

        glDrawElements(GL_LINES, static_cast<uint32_t>(diagonalIndices.size()), GL_UNSIGNED_INT,
                       diagonalIndices.data());
    }

    if (!indices.empty()) {
        lineShader_.setUniform("lineWidth", properties_.lineWidth_.get());
        lineShader_.setUniform("pickingEnabled", true);

        glDrawElements(GL_LINES, static_cast<uint32_t>(indices.size()), GL_UNSIGNED_INT,
                       indices.data());
    }
}

void PersistenceDiagramPlotGL::renderPoints(const size2_t& dims,
                                            const std::vector<uint32_t>& indices) {
    pointShader_.setUniform("pixelSize", vec2(1.0f) / vec2(dims));
    pointShader_.setUniform("dims", ivec2(dims));
    pointShader_.setUniform("maxRadius", properties_.radius_.get());
    pointShader_.setUniform("borderWidth", properties_.borderWidth_.get());
    pointShader_.setUniform("borderColor", properties_.borderColor_.get());
    pointShader_.setUniform("pickingEnabled", true);

    glDrawElements(GL_POINTS, static_cast<uint32_t>(indices.size()), GL_UNSIGNED_INT,
                   indices.data());
}

void PersistenceDiagramPlotGL::setXAxisLabel(const std::string& caption) {
    properties_.xAxis_.setCaption(caption);
}

void PersistenceDiagramPlotGL::setYAxisLabel(const std::string& caption) {
    properties_.yAxis_.setCaption(caption);
}

void PersistenceDiagramPlotGL::setXAxis(std::shared_ptr<const Column> col) {
    setXAxisLabel(col->getHeader());
    setXAxisData(col->getBuffer());
}

void PersistenceDiagramPlotGL::setYAxis(std::shared_ptr<const Column> col) {
    setYAxisLabel(col->getHeader());
    setYAxisData(col->getBuffer());
}

void PersistenceDiagramPlotGL::setXAxisData(std::shared_ptr<const BufferBase> buffer) {
    xAxis_ = buffer;
    if (buffer) {
        auto minmax = util::bufferMinMax(buffer.get(), IgnoreSpecialValues::Yes);
        minmaxX_.x = static_cast<float>(minmax.first.x);
        minmaxX_.y = static_cast<float>(minmax.second.x);

        properties_.xAxis_.setRange(minmaxX_);

        // make sure that vertical axis has same minimum
        minmaxY_.x = minmaxX_.x;
        properties_.yAxis_.setRange(minmaxX_);
    }
}

void PersistenceDiagramPlotGL::setYAxisData(std::shared_ptr<const BufferBase> buffer) {
    yAxis_ = buffer;
    if (buffer) {
        auto minmax = util::bufferMinMax(buffer.get(), IgnoreSpecialValues::Yes);
        minmaxY_.y = static_cast<float>(minmax.second.x);

        properties_.yAxis_.setRange(dvec2(properties_.yAxis_.range_.get().x, minmaxY_.y));
    }
}

void PersistenceDiagramPlotGL::setColorData(std::shared_ptr<const BufferBase> buffer) {
    color_ = buffer;
    if (buffer) {
        auto minmax = util::bufferMinMax(buffer.get(), IgnoreSpecialValues::Yes);
        minmaxC_.x = static_cast<float>(minmax.first.x);
        minmaxC_.y = static_cast<float>(minmax.second.x);
    }
    properties_.tf_.setVisible(buffer != nullptr);
}

void PersistenceDiagramPlotGL::setIndexColumn(
    std::shared_ptr<const TemplateColumn<uint32_t>> indexcol) {
    indexColumn_ = indexcol;

    if (indexColumn_) {
        picking_.resize(indexColumn_->getSize());
    }
}

void PersistenceDiagramPlotGL::setSelectedIndices(const std::unordered_set<size_t>& indices) {
    selectedIndices_ = indices;
}

auto PersistenceDiagramPlotGL::addToolTipCallback(std::function<ToolTipFunc> callback)
    -> ToolTipCallbackHandle {
    return tooltipCallback_.add(callback);
}

auto PersistenceDiagramPlotGL::addSelectionChangedCallback(std::function<SelectionFunc> callback)
    -> SelectionCallbackHandle {
    return selectionChangedCallback_.add(callback);
}

void PersistenceDiagramPlotGL::renderAxis(const size2_t& dims) {

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

void PersistenceDiagramPlotGL::objectPicked(PickingEvent* p) {
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
            hoveredIndices_.insert(id);
            if (processor_) {
                processor_->invalidate(InvalidationLevel::InvalidOutput);
            }
        } else if (p->getHoverState() == PickingHoverState::Exit) {
            hoveredIndices_.erase(id);
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

uint32_t PersistenceDiagramPlotGL::getGlobalPickId(uint32_t localIndex) const {
    return static_cast<uint32_t>(picking_.getPickingId(localIndex));
}

}  // namespace plot

}  // namespace inviwo
