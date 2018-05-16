/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018 Inviwo Foundation
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

#include <modules/plottinggl/plotters/persistencediagramplotgl.h>
#include <modules/opengl/buffer/buffergl.h>
#include <modules/opengl/buffer/bufferobject.h>
#include <modules/opengl/texture/textureutils.h>
#include <modules/opengl/openglutils.h>
#include <modules/opengl/shader/shaderutils.h>
#include <modules/opengl/geometry/meshgl.h>
#include <modules/opengl/volume/volumeutils.h>
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

PropertyClassIdentifier(PersistenceDiagramPlotGL::Properties,
                        "org.inviwo.PersistenceDiagramPlotGL.Properties");

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
    , hoverColor_("hoverColor", "Hover color", vec4(1.0f, 0.77f, 0.25f, 1))
    , tf_("transferFunction", "Transfer Function",
          TransferFunction({{0.0, vec4(1.0f)}, {1.0, vec4(1.0f)}}))
    , margins_("margins", "Margins", 5, 5, 5, 5)
    , axisMargin_("axisMargin", "Axis Margin", 15.0f, 0.0f, 50.0f)

    , borderWidth_("borderWidth", "Border width", 2, 0, 20)
    , borderColor_("borderColor", "Border color", vec4(0, 0, 0, 1))
    , hovering_("hovering", "Enable Hovering", true)

    , xAxis_("xAxis", "X Axis")
    , yAxis_("yAxis", "Y Axis", AxisProperty::Orientation::Vertical) {
    addProperty(showPoints_);
    addProperty(radius_);
    addProperty(lineWidth_);
    addProperty(lineWidthDiagonal_);
    addProperty(pointColor_);
    addProperty(lineColor_);
    hoverColor_.setSemantics(PropertySemantics::Color);
    addProperty(hoverColor_);
    addProperty(tf_);
    addProperty(margins_);
    addProperty(axisMargin_);

    borderColor_.setSemantics(PropertySemantics::Color);
    addProperty(borderWidth_);
    addProperty(borderColor_);

    addProperty(hovering_);

    addProperty(xAxis_);
    addProperty(yAxis_);

    pointColor_.setVisible(true);
    tf_.setVisible(!pointColor_.getVisible());
}

PersistenceDiagramPlotGL::Properties::Properties(const PersistenceDiagramPlotGL::Properties &rhs)
    : CompositeProperty(rhs)
    , showPoints_(rhs.showPoints_)
    , radius_(rhs.radius_)
    , lineWidth_(rhs.lineWidth_)
    , lineWidthDiagonal_(rhs.lineWidthDiagonal_)
    , pointColor_(rhs.pointColor_)
    , lineColor_(rhs.lineColor_)
    , hoverColor_(rhs.hoverColor_)
    , tf_(rhs.tf_)
    , margins_(rhs.margins_)
    , axisMargin_(rhs.axisMargin_)
    , borderWidth_(rhs.borderWidth_)
    , borderColor_(rhs.borderColor_)
    , hovering_(rhs.hovering_)
    , xAxis_(rhs.xAxis_)
    , yAxis_(rhs.yAxis_) {
    addProperty(showPoints_);
    addProperty(radius_);
    addProperty(lineWidth_);
    addProperty(lineWidthDiagonal_);
    addProperty(pointColor_);
    addProperty(lineColor_);
    addProperty(hoverColor_);
    addProperty(tf_);
    addProperty(margins_);
    addProperty(axisMargin_);
    addProperty(borderColor_);
    addProperty(borderWidth_);
    addProperty(hovering_);
    addProperty(xAxis_);
    addProperty(yAxis_);
}

PersistenceDiagramPlotGL::Properties &PersistenceDiagramPlotGL::Properties::operator=(
    const PersistenceDiagramPlotGL::Properties &that) {
    if (this != &that) {
        CompositeProperty::operator=(that);
        showPoints_ = that.showPoints_;
        radius_ = that.radius_;
        lineWidth_ = that.lineWidth_;
        lineWidthDiagonal_ = that.lineWidthDiagonal_;
        pointColor_ = that.pointColor_;
        lineColor_ = that.lineColor_;
        hoverColor_ = that.hoverColor_;
        tf_ = that.tf_;
        margins_ = that.margins_;
        axisMargin_ = that.axisMargin_;
        borderWidth_ = that.borderWidth_;
        borderColor_ = that.borderColor_;
        hovering_ = that.hovering_;
        xAxis_ = that.xAxis_;
        yAxis_ = that.yAxis_;
    }
    return *this;
}

PersistenceDiagramPlotGL::Properties *PersistenceDiagramPlotGL::Properties::clone() const {
    return new Properties(*this);
}

PersistenceDiagramPlotGL::PersistenceDiagramPlotGL()
    : properties_("persistenceDiagram", "PersistenceDiagram")
    , pointShader_("persistencediagram.vert", "scatterplot.geom", "scatterplot.frag")
    , lineShader_("persistencediagramlines.vert", "linerenderer.geom", "linerenderer.frag")
    , xAxis_(nullptr)
    , yAxis_(nullptr)
    , color_(nullptr)
    , axisRenderers_({{properties_.xAxis_, properties_.yAxis_}}) {
    properties_.hovering_.onChange([this]() { picking_.setEnabled(properties_.hovering_.get()); });
}

PersistenceDiagramPlotGL::PersistenceDiagramPlotGL(Processor *processor)
    : PersistenceDiagramPlotGL() {
    processor_ = processor;
    picking_ = PickingMapper(processor, 1, [this](PickingEvent *p) { objectPicked(p); });

    pointShader_.onReload([=]() { processor->invalidate(InvalidationLevel::InvalidOutput); });
    lineShader_.onReload([=]() { processor->invalidate(InvalidationLevel::InvalidOutput); });
}

void PersistenceDiagramPlotGL::plot(Image &dest, IndexBuffer *indices, bool useAxisRanges) {
    utilgl::activateAndClearTarget(dest);
    plot(dest.getDimensions(), indices, useAxisRanges);
    utilgl::deactivateCurrentTarget();
}

void PersistenceDiagramPlotGL::plot(Image &dest, const Image &src, IndexBuffer *indices,
                                    bool useAxisRanges) {
    utilgl::activateTargetAndCopySource(dest, src);
    plot(dest.getDimensions(), indices, useAxisRanges);
    utilgl::deactivateCurrentTarget();
}

void PersistenceDiagramPlotGL::plot(ImageOutport &dest, IndexBuffer *indices, bool useAxisRanges) {
    utilgl::activateAndClearTarget(dest);
    plot(dest.getDimensions(), indices, useAxisRanges);
    utilgl::deactivateCurrentTarget();
}

void PersistenceDiagramPlotGL::plot(ImageOutport &dest, ImageInport &src, IndexBuffer *indices,
                                    bool useAxisRanges) {
    utilgl::activateTargetAndCopySource(dest, src);
    plot(dest.getDimensions(), indices, useAxisRanges);
    utilgl::deactivateCurrentTarget();
}

void PersistenceDiagramPlotGL::plot(const ivec2 &start, const ivec2 &size, IndexBuffer *indices,
                                    bool useAxisRanges) {
    utilgl::ViewportState viewport(start.x, start.y, size.x, size.y);
    plot(size, indices, useAxisRanges);
}

namespace detail {

// clip line at the given rectangle using Cohen-Sutherland algorithm
std::tuple<bool, vec2, vec2> clipLineCohenSutherland(vec2 p1, vec2 p2, const vec2 &rectMin,
                                                     const vec2 &rectMax) {
    // outcodes used by algorithm:
    //    0x0000: inside
    //    0x0001: left
    //    0x0010: right
    //    0x0100: bottom
    //    0x1000: top
    auto getOutcode = [&](const vec2 &pos) {
        int outcode = 0;
        if (pos.x < rectMin.x) {
            outcode |= 0x0001;  // on left outside
        } else if (pos.x > rectMax.x) {
            outcode |= 0x0010;  // on right outside
        }
        if (pos.y < rectMin.y) {
            outcode |= 0x0100;  // below rect
        } else if (pos.y > rectMax.y) {
            outcode |= 0x1000;  // above rect
        }
        return outcode;
    };

    int outcodeP1 = getOutcode(p1);
    int outcodeP2 = getOutcode(p2);

    bool acceptLine = true;
    while (true) {
        if (!outcodeP1 && !outcodeP2) {  // both points inside -> accept
            break;
        } else if (outcodeP1 & outcodeP2) {  // common shared outside -> reject
            acceptLine = false;
            break;
        } else {
            // pick one outside code
            int outcode = outcodeP1 ? outcodeP1 : outcodeP2;

            // compute intersection
            vec2 p;
            if (outcode & 0x0001) {  // left
                p.x = rectMin.x;
                p.y = p1.y + (p2.y - p1.y) * (rectMin.x - p1.x) / (p2.x - p1.x);
            } else if (outcode & 0x0010) {  // right
                p.x = rectMax.x;
                p.y = p1.y + (p2.y - p1.y) * (rectMax.x - p1.x) / (p2.x - p1.x);
            } else if (outcode & 0x0100) {  // bottom
                p.x = p1.x + (p2.x - p1.x) * (rectMin.y - p1.y) / (p2.y - p1.y);
                p.y = rectMin.y;
            } else if (outcode & 0x1000) {  // top
                p.x = p1.x + (p2.x - p1.x) * (rectMax.y - p1.y) / (p2.y - p1.y);
                p.y = rectMax.y;
            }
            // adjust point matching selected outcode
            if (outcode == outcodeP1) {
                p1 = p;
                outcodeP1 = getOutcode(p1);
            } else {
                p2 = p;
                outcodeP2 = getOutcode(p2);
            }
        }
    }
    return std::make_tuple(acceptLine, p1, p2);
}

bool insideRect(const vec2 &point, const vec2 &rectMin, const vec2 &rectMax) {
    vec2 s = glm::step(rectMin, point) - (1.0f - glm::step(point, rectMax));
    return s.x * s.y > 0.0;
}

}  // namespace detail

void PersistenceDiagramPlotGL::plot(const size2_t &dims, IndexBuffer *indices, bool useAxisRanges) {
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
        auto clipped =
            detail::clipLineCohenSutherland(vec2(minmaxX_.x), vec2(minmaxX_.y), rectMin, rectMax);
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
                for (auto &&i : util::zip(xyPairs, bufferpr->getDataContainer())) {
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
        for (auto &index : selectedIndices) {
            auto &point = xyPairs[index];

            auto clipped = detail::clipLineCohenSutherland(vec2(point.x), vec2(point.x, point.y),
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

        auto getColor = [
            this, hoverEnabled = (properties_.hovering_.get() && !hoveredIndices_.empty()),
            buffer = colorBuffer, normalizeValue
        ](uint32_t index) {
            if (hoverEnabled && util::contains(hoveredIndices_, index)) {
                return properties_.hoverColor_.get();
            } else if (color_) {
                return properties_.tf_.get().sample(normalizeValue(buffer->getAsDouble(index)));
            } else {
                return properties_.pointColor_.get();
            }
        };

        uint32_t indexOffset = static_cast<uint32_t>(vertices.size());
        for (auto &index : selectedIndices) {
            const vec2 pLower(xyPairs[index].x);
            const vec2 pUpper(xyPairs[index].x, xyPairs[index].y);

            const vec4 color = getColor(index);
            if (detail::insideRect(pLower, rectMin, rectMax)) {
                vertices.push_back({pLower, color, getGlobalPickId(index)});
                indicesPoints.push_back(indexOffset++);
            }
            if (detail::insideRect(pUpper, rectMin, rectMax)) {
                vertices.push_back({pUpper, color, getGlobalPickId(index)});
                indicesPoints.push_back(indexOffset++);
            }
        }
    }

    mesh.addVertices(vertices);
    LogGLError(__FILE__, __FUNCTION__, __LINE__);

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

void PersistenceDiagramPlotGL::renderLines(const size2_t &dims,
                                           const std::vector<uint32_t> &diagonalIndices,
                                           const std::vector<uint32_t> &indices) {
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

void PersistenceDiagramPlotGL::renderPoints(const size2_t &dims,
                                            const std::vector<uint32_t> &indices) {
    pointShader_.setUniform("pixelSize", vec2(1.0f) / vec2(dims));
    pointShader_.setUniform("dims", ivec2(dims));
    pointShader_.setUniform("maxRadius", properties_.radius_.get());
    pointShader_.setUniform("borderWidth", properties_.borderWidth_.get());
    pointShader_.setUniform("borderColor", properties_.borderColor_.get());

    glDrawElements(GL_POINTS, static_cast<uint32_t>(indices.size()), GL_UNSIGNED_INT,
                   indices.data());
}

void PersistenceDiagramPlotGL::setXAxisLabel(const std::string &label) {
    properties_.xAxis_.setTitle(label);
}

void PersistenceDiagramPlotGL::setYAxisLabel(const std::string &label) {
    properties_.yAxis_.setTitle(label);
}

void PersistenceDiagramPlotGL::setXAxis(std::shared_ptr<const plot::Column> col) {
    setXAxisLabel(col->getHeader());
    setXAxisData(col->getBuffer());
}

void PersistenceDiagramPlotGL::setYAxis(std::shared_ptr<const plot::Column> col) {
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
        // minmaxY_.x = static_cast<float>(minmax.first.x);
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

void PersistenceDiagramPlotGL::renderAxis(const size2_t &dims) {

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

void PersistenceDiagramPlotGL::objectPicked(PickingEvent *p) {
    auto idToDataFrameIndex = [this](uint32_t id) -> std::tuple<bool, uint32_t> {
        if (!indexColumn_) {
            return std::tuple<bool, uint32_t>{false, 0};
        }

        auto &indexCol = indexColumn_->getTypedBuffer()->getRAMRepresentation()->getDataContainer();

        auto it = util::find(indexCol, static_cast<uint32_t>(id));
        if (it != indexCol.end()) {
            return std::tuple<bool, uint32_t>{true, *it};
        } else {
            return std::tuple<bool, uint32_t>{false, 0};
        }
    };

    const uint32_t id = static_cast<uint32_t>(p->getPickedId());
    auto rowIndex = idToDataFrameIndex(id);

    if (p->getState() == PickingState::Started) {
        hoveredIndices_.insert(id);
        processor_->invalidate(InvalidationLevel::InvalidOutput);
    } else if (p->getState() == PickingState::Finished) {
        hoveredIndices_.erase(id);
        processor_->invalidate(InvalidationLevel::InvalidOutput);
    }

    auto logRowData = [&]() {
        if (std::get<0>(rowIndex) && xAxis_ && yAxis_) {
            LogWarn("Index: " << std::get<1>(rowIndex) << "\n"
                              << properties_.xAxis_.getTitle() << ": "
                              << xAxis_->getRepresentation<BufferRAM>()->getAsDouble(id) << "\n"
                              << properties_.yAxis_.getTitle() << ": "
                              << yAxis_->getRepresentation<BufferRAM>()->getAsDouble(id));
        }
    };

    if (p->getEvent()->hash() == MouseEvent::chash()) {
        auto me = p->getEventAs<MouseEvent>();
        if (me->button() == MouseButton::Left) {
            if (me->state() == MouseState::Release) {
                // print information on current element
                logRowData();
            }
            me->markAsUsed();
        }
    } else if (p->getEvent()->hash() == TouchEvent::chash()) {
        auto touchEvent = p->getEventAs<TouchEvent>();

        if (touchEvent->touchPoints().size() == 1) {
            // allow interaction only for a single touch point
            const auto &touchPoint = touchEvent->touchPoints().front();

            if (touchPoint.state() == TouchState::Started) {
                // print information on current element
                logRowData();
            }
            p->markAsUsed();
        }
    }
}

uint32_t PersistenceDiagramPlotGL::getGlobalPickId(uint32_t localIndex) const {
    return static_cast<uint32_t>(picking_.getPickingId(localIndex));
}

}  // namespace plot

}  // namespace inviwo
