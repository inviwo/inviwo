/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2017 Inviwo Foundation
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
#include <modules/opengl/texture/textureutils.h>
#include <modules/opengl/openglutils.h>
#include <glm/gtx/transform.hpp>

namespace inviwo {

namespace plot {

PropertyClassIdentifier(ScatterPlotGL::Properties, "org.inviwo.ScatterPlotGL.Properties");

ScatterPlotGL::Properties::Properties(std::string identifier, std::string displayName,
                                      InvalidationLevel invalidationLevel,
                                      PropertySemantics semantics)
    : CompositeProperty(identifier, displayName, invalidationLevel, semantics)
    , useCircle_("useCircle", "Use Circles (else squares)", true)
    , radiusRange_("radius", "Radius", 5, 0, 10, 0.01f)
    , minRadius_("minRadius", "Min Radius", 0.1f, 0, 10, 0.01f)
    , color_("color", "Color", vec4(1, 0, 0, 1), vec4(0), vec4(1), vec4(0.1f),
             InvalidationLevel::InvalidOutput, PropertySemantics::Color)
    , tf_("transferFunction", "Transfer Function")
    , margins_("margins", "Margins", 5, 5, 5, 5)
    , axisMargin_("axisMargin", "Axis Margin", 15.0f, 0.0f, 50.0f)

    , borderWidth_("borderWidth", "Border width", 2, 0, 20)
    , borderColor_("borderColor", "Border color", vec4(0, 0, 0, 1))

    , xAxis_("xAxis", "X Axis")
    , yAxis_("yAxis", "Y Axis", AxisProperty::Orientation::Vertical) {
    addProperty(useCircle_);
    addProperty(radiusRange_);
    addProperty(minRadius_);
    addProperty(tf_);
    addProperty(color_);
    addProperty(margins_);
    addProperty(axisMargin_);

    addProperty(borderWidth_);
    addProperty(borderColor_);
    borderColor_.setSemantics(PropertySemantics::Color);

    addProperty(xAxis_);
    addProperty(yAxis_);

    color_.setVisible(true);
    tf_.setVisible(!color_.getVisible());
    minRadius_.setVisible(false);

    tf_.get().clearPoints();
    tf_.get().addPoint(vec2(0, 1), vec4(1));
    tf_.get().addPoint(vec2(1, 1), vec4(1));
    tf_.setCurrentStateAsDefault();
}

ScatterPlotGL::Properties::Properties(const ScatterPlotGL::Properties &rhs)
    : CompositeProperty(rhs)
    , useCircle_(rhs.useCircle_)
    , radiusRange_(rhs.radiusRange_)
    , minRadius_(rhs.minRadius_)
    , color_(rhs.color_)
    , tf_(rhs.tf_)
    , margins_(rhs.margins_)
    , axisMargin_(rhs.axisMargin_)
    , borderWidth_(rhs.borderWidth_)
    , borderColor_(rhs.borderColor_)
    , xAxis_(rhs.xAxis_)
    , yAxis_(rhs.yAxis_) {
    addProperty(useCircle_);
    addProperty(radiusRange_);
    addProperty(minRadius_);
    addProperty(tf_);
    addProperty(color_);
    addProperty(margins_);
    addProperty(axisMargin_);
    addProperty(borderWidth_);
    addProperty(borderColor_);
    addProperty(xAxis_);
    addProperty(yAxis_);
}

ScatterPlotGL::Properties *ScatterPlotGL::Properties::clone() const {
    return new Properties(*this);
}

ScatterPlotGL::Properties &ScatterPlotGL::Properties::operator=(
    const ScatterPlotGL::Properties &that) {
    if (this != &that) {
        CompositeProperty::operator=(that);
        radiusRange_ = that.radiusRange_;
        useCircle_ = that.useCircle_;
        minRadius_ = that.minRadius_;
        color_ = that.color_;
        tf_ = that.tf_;
        margins_ = that.margins_;
        axisMargin_ = that.axisMargin_;
        borderWidth_ = that.borderWidth_;
        borderColor_ = that.borderColor_;
        xAxis_ = that.xAxis_;
        yAxis_ = that.yAxis_;
    }
    return *this;
}

ScatterPlotGL::ScatterPlotGL()
    : properties_("scatterplot", "Scatterplot")
    , shader_("scatterplot.vert", "scatterplot.geom", "scatterplot.frag")
    , xAxis_(nullptr)
    , yAxis_(nullptr)
    , color_(nullptr)
    , radius_(nullptr)
    , axisRenderers_({{properties_.xAxis_, properties_.yAxis_}}) {}

ScatterPlotGL::ScatterPlotGL(Processor *processor) : ScatterPlotGL() {
    shader_.onReload([=]() { processor->invalidate(InvalidationLevel::InvalidOutput); });
}

void ScatterPlotGL::plot(Image &dest, IndexBuffer *indices) {
    utilgl::activateAndClearTarget(dest);
    plot(dest.getDimensions(), indices);
    utilgl::deactivateCurrentTarget();
}

void ScatterPlotGL::plot(Image &dest, const Image &src, IndexBuffer *indices) {
    utilgl::activateTargetAndCopySource(dest, src);
    plot(dest.getDimensions(), indices);
    utilgl::deactivateCurrentTarget();
}

void ScatterPlotGL::plot(ImageOutport &dest, IndexBuffer *indices) {
    utilgl::activateAndClearTarget(dest);
    plot(dest.getDimensions(), indices);
    utilgl::deactivateCurrentTarget();
}

void ScatterPlotGL::plot(ImageOutport &dest, ImageInport &src, IndexBuffer *indices) {
    utilgl::activateTargetAndCopySource(dest, src);
    plot(dest.getDimensions(), indices);
    utilgl::deactivateCurrentTarget();
}

void ScatterPlotGL::plot(const ivec2 &start, const ivec2 &size, IndexBuffer *indices) {
    utilgl::ViewportState viewport(start.x, start.y, size.x, size.y);
    plot(size, indices);
}

void ScatterPlotGL::plot(const size2_t &dims, IndexBuffer *indices) {

    // adjust all margins by axis margin
    vec4 margins = properties_.margins_.getAsVec4() + properties_.axisMargin_.get();

    shader_.activate();

    vec2 pixelSize = vec2(1) / vec2(dims);

    TextureUnitContainer cont;
    shader_.setUniform("minmaxX", minmaxX_);
    shader_.setUniform("minmaxY", minmaxY_);
    shader_.setUniform("borderWidth", properties_.borderWidth_.get());
    shader_.setUniform("borderColor", properties_.borderColor_.get());

    shader_.setUniform("pixelSize", pixelSize);
    shader_.setUniform("dims", ivec2(dims));
    shader_.setUniform("circle", properties_.useCircle_.get() ? 1 : 0);

    auto xbuf = xAxis_->getRepresentation<BufferGL>();
    auto ybuf = yAxis_->getRepresentation<BufferGL>();
    auto xbufObj = xbuf->getBufferObject();
    auto ybufObj = ybuf->getBufferObject();

    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glEnableVertexAttribArray((GLuint)0);
    xbufObj->bind();
    glVertexAttribPointer(0, xbufObj->getGLFormat().channels, xbufObj->getGLFormat().type, GL_FALSE,
                          0, (void *)nullptr);

    glEnableVertexAttribArray((GLuint)1);
    ybufObj->bind();
    glVertexAttribPointer(1, ybufObj->getGLFormat().channels, ybufObj->getGLFormat().type, GL_FALSE,
                          0, (void *)nullptr);

    if (color_) {

        utilgl::bindAndSetUniforms(shader_, cont, properties_.tf_);
        shader_.setUniform("minmaxC", minmaxC_);

        auto cbuf = color_->getRepresentation<BufferGL>();
        auto cbufObj = cbuf->getBufferObject();
        glEnableVertexAttribArray((GLuint)2);
        cbufObj->bind();
        glVertexAttribPointer(2, cbufObj->getGLFormat().channels, cbufObj->getGLFormat().type,
                              GL_FALSE, 0, (void *)nullptr);
        shader_.setUniform("has_color", 1);
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
    shader_.setUniform("margins", margins + vec4(maxRadius, maxRadius, maxRadius, maxRadius));

    utilgl::GlBoolState depth(GL_DEPTH_TEST, radius_ != nullptr);
    if (radius_) {
        shader_.setUniform("minmaxR", minmaxR_);

        auto rbuf = radius_->getRepresentation<BufferGL>();
        auto rbufObj = rbuf->getBufferObject();
        glEnableVertexAttribArray((GLuint)3);
        rbufObj->bind();
        glVertexAttribPointer(3, rbufObj->getGLFormat().channels, rbufObj->getGLFormat().type,
                              GL_FALSE, 0, (void *)nullptr);
        shader_.setUniform("has_radius", 1);
    } else {
        shader_.setUniform("has_radius", 0);
    }

    if (indices) {
        auto glBuf = indices->getRepresentation<BufferGL>();
        glBuf->bind();
        glDrawElements(GL_POINTS, static_cast<GLsizei>(indices->getSize()), glBuf->getFormatType(),
                       nullptr);
    } else {
        glDrawArrays(GL_POINTS, 0, static_cast<GLsizei>(xbuf->getSize()));
    }

    shader_.deactivate();

    if (radius_) {
        glDisableVertexAttribArray(3);
    }
    if (color_) {
        glDisableVertexAttribArray(2);
    }
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(0);
    glDeleteVertexArrays(1, &vao);

    renderAxis(dims);
}

void ScatterPlotGL::setXAxisLabel(const std::string &label) { properties_.xAxis_.setTitle(label); }

void ScatterPlotGL::setYAxisLabel(const std::string &label) { properties_.yAxis_.setTitle(label); }

void ScatterPlotGL::setXAxis(std::shared_ptr<const plot::Column> col) {
    setXAxisLabel(col->getHeader());
    setXAxisData(col->getBuffer());
}

void ScatterPlotGL::setYAxis(std::shared_ptr<const plot::Column> col) {
    setYAxisLabel(col->getHeader());
    setYAxisData(col->getBuffer());
}

void ScatterPlotGL::setXAxisData(std::shared_ptr<const BufferBase> buffer) {
    xAxis_ = buffer;
    if (buffer) {
        auto minmax = util::bufferMinMax(buffer.get(), IgnoreSpecialValues::Yes);
        minmaxX_.x = minmax.first.x;
        minmaxX_.y = minmax.second.x;

        properties_.xAxis_.setRange(minmaxX_);
    }
}

void ScatterPlotGL::setYAxisData(std::shared_ptr<const BufferBase> buffer) {
    yAxis_ = buffer;
    if (buffer) {
        auto minmax = util::bufferMinMax(buffer.get(), IgnoreSpecialValues::Yes);
        minmaxY_.x = minmax.first.x;
        minmaxY_.y = minmax.second.x;

        properties_.yAxis_.setRange(minmaxY_);
    }
}

void ScatterPlotGL::setColorData(std::shared_ptr<const BufferBase> buffer) {
    color_ = buffer;
    if (buffer) {
        auto minmax = util::bufferMinMax(buffer.get(), IgnoreSpecialValues::Yes);
        minmaxC_.x = minmax.first.x;
        minmaxC_.y = minmax.second.x;
    }
    properties_.tf_.setVisible(buffer != nullptr);
    properties_.color_.setVisible(buffer == nullptr);
}

void ScatterPlotGL::setRadiusData(std::shared_ptr<const BufferBase> buffer) {
    radius_ = buffer;
    if (buffer) {
        auto minmax = util::bufferMinMax(buffer.get(), IgnoreSpecialValues::Yes);
        minmaxR_.x = minmax.first.x;
        minmaxR_.y = minmax.second.x;
    }
    properties_.minRadius_.setVisible(buffer != nullptr);
}

void ScatterPlotGL::renderAxis(const size2_t &dims) {

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

}  // namespace plot

}  // namespace inviwo
