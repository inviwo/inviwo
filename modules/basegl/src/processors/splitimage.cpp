/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2019 Inviwo Foundation
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

#include <modules/basegl/processors/splitimage.h>

#include <modules/opengl/openglutils.h>
#include <modules/opengl/sharedopenglresources.h>
#include <modules/opengl/texture/textureutils.h>
#include <modules/opengl/texture/textureunit.h>
#include <modules/opengl/shader/shader.h>
#include <modules/opengl/image/imagegl.h>
#include <modules/opengl/rendering/meshdrawergl.h>
#include <modules/opengl/shader/shaderutils.h>

#include <inviwo/core/interaction/events/pickingevent.h>
#include <inviwo/core/interaction/events/mouseevent.h>
#include <inviwo/core/interaction/events/touchevent.h>
#include <inviwo/core/datastructures/geometry/mesh.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo SplitImage::processorInfo_{
    "org.inviwo.SplitImage",  // Class identifier
    "Split Image",            // Display name
    "Image Operation",        // Category
    CodeState::Stable,        // Code state
    Tags::GL,                 // Tags
};
const ProcessorInfo SplitImage::getProcessorInfo() const { return processorInfo_; }

SplitImage::SplitImage()
    : Processor()
    , inport0_("inputA")
    , inport1_("inputB")
    , outport_("outport")
    , splitDirection_("splitDirection", "Split Direction",
                      {{"vertical", "Vertical", SplitDirection::Vertical},
                       {"horizontal", "Horizontal", SplitDirection::Horizontal}},
                      0)
    , splitPosition_("splitPosition", "Split Position", 0.5f, 0.0f, 1.0f)
    , handlebarWidget_("handlebarWidget", "Handle Bar", true)
    , style_("style", "Style",
             {{"handle", "Handle", SplitterStyle::Handle},
              {"divider", "Divider", SplitterStyle::Divider},
              {"invisible", "Invisible", SplitterStyle::Invisible}})
    , color_("color", "Color", vec4(0.71f, 0.81f, 0.85f, 1.0f), vec4(0.0f), vec4(1.0f))
    , bgColor_("bgColor", "Background Color", vec4(0.27f, 0.3f, 0.31f, 1.0f), vec4(0.0f),
               vec4(1.0f))
    , triColor_("triColor", "Hover Color", vec4(1.0f, 0.666f, 0.0f, 1.0f), vec4(0.0f), vec4(1.0f))
    , width_("width", "Width (pixel)", 9.0f, 0.0f, 50.0f, 0.1f)
    , triSize_("triSize", "Triangle Size", 10.0f, 0.0f, 50.0f)
    , shader_("splitter.vert", "linerenderer.geom", "linerenderer.frag", false)
    , triShader_("standard.vert", "standard.frag")
    , pickingMapper_(this, 1, [&](PickingEvent *e) { handlePickingEvent(e); }) {
    inport0_.setOptional(true);
    inport1_.setOptional(true);
    addPort(inport0_);
    addPort(inport1_);
    addPort(outport_);

    addProperty(splitDirection_);
    addProperty(splitPosition_);

    handlebarWidget_.addProperty(style_);
    handlebarWidget_.addProperty(color_);
    handlebarWidget_.addProperty(bgColor_);
    handlebarWidget_.addProperty(triColor_);
    handlebarWidget_.addProperty(width_);
    handlebarWidget_.addProperty(triSize_);
    addProperty(handlebarWidget_);

    color_.setSemantics(PropertySemantics::Color);
    bgColor_.setSemantics(PropertySemantics::Color);
    triColor_.setSemantics(PropertySemantics::Color);
    width_.setSemantics(PropertySemantics("SpinBox"));
    triSize_.setSemantics(PropertySemantics("SpinBox"));
    shader_.onReload([this]() { invalidate(InvalidationLevel::InvalidResources); });
    triShader_.onReload([this]() { invalidate(InvalidationLevel::InvalidOutput); });
}

void SplitImage::initializeResources() {
    shader_[ShaderType::Geometry]->addShaderDefine("ENABLE_ADJACENCY", "0");

    shader_.build();
    shader_.activate();
    shader_.setUniform("roundCaps", true);
}

void SplitImage::process() {
    utilgl::activateAndClearTarget(outport_);

    if (!lineMesh_) {
        updateMesh();
    }

    if (!triangleMesh_ || triColor_.isModified() || triSize_.isModified()) {
        updateTriMesh();
    }

    if (!inport0_.isReady() && !inport1_.isReady()) {
        // no inputs, render noise

        auto shader = SharedOpenGLResources::getPtr()->getNoiseShader();
        shader->activate();
        utilgl::singleDrawImagePlaneRect();
        shader->deactivate();
    } else {
        utilgl::GlBoolState scissor(GL_SCISSOR_TEST, GL_TRUE);

        const ivec2 dims(outport_.getDimensions());

        ivec4 viewport0;
        ivec4 viewport1;
        if (splitDirection_ == SplitDirection::Vertical) {
            int width = static_cast<int>(splitPosition_ * dims.x);
            viewport0 = ivec4(0, 0, width, dims.y);
            viewport1 = ivec4(width, 0, dims.x - width, dims.y);
        } else {
            int height = static_cast<int>(splitPosition_ * dims.y);
            viewport0 = ivec4(0, dims.y - height, dims.x, height);
            viewport1 = ivec4(0, 0, dims.x, dims.y - height);
        }

        auto renderPort = [&](const ImageInport &inport, const auto &viewport) {
            utilgl::ScissorState scissor(viewport);

            TextureUnit colorUnit, depthUnit, pickingUnit;
            TextureUnitContainer additionalColorUnits;
            Shader *shader = nullptr;
            if (inport.isReady()) {
                shader = SharedOpenGLResources::getPtr()->getImageCopyShader(
                    inport.getData()->getNumberOfColorLayers());
                shader->activate();
                shader->setUniform("dataToClip", mat4(1.0f));

                auto imageGL = inport.getData()->getRepresentation<ImageGL>();
                imageGL->getColorLayerGL()->bindTexture(colorUnit.getEnum());
                shader->setUniform("color_", colorUnit.getUnitNumber());
                if (imageGL->getDepthLayerGL()) {
                    imageGL->getDepthLayerGL()->bindTexture(depthUnit.getEnum());
                    shader->setUniform("depth_", depthUnit.getUnitNumber());
                }
                if (imageGL->getPickingLayerGL()) {
                    imageGL->getPickingLayerGL()->bindTexture(pickingUnit.getEnum());
                    shader->setUniform("picking_", pickingUnit.getUnitNumber());
                }
                for (size_t i = 1; i < imageGL->getNumberOfColorLayers(); ++i) {
                    TextureUnit unit;
                    imageGL->getColorLayerGL(i)->bindTexture(unit.getEnum());
                    shader->setUniform("color" + toString<size_t>(i), unit.getUnitNumber());
                    additionalColorUnits.push_back(std::move(unit));
                }
            } else {
                shader = SharedOpenGLResources::getPtr()->getNoiseShader();
            }

            shader->activate();
            utilgl::singleDrawImagePlaneRect();
            shader->deactivate();
        };

        renderPort(inport0_, viewport0);
        renderPort(inport1_, viewport1);
    }

    if (handlebarWidget_.isChecked()) {
        drawHandlebar();
    }

    utilgl::deactivateCurrentTarget();
}

void SplitImage::drawHandlebar() {
    utilgl::DepthFuncState depthFunc(GL_ALWAYS);

    const vec2 canvasDim(outport_.getDimensions());

    float lineWidth = 0.0f;
    float handleWidth = 0.0f;
    bool drawHandle = (style_ != SplitterStyle::Invisible);
    bool drawBorder = (style_ == SplitterStyle::Handle);

    switch (style_) {
        case SplitterStyle::Handle:
            lineWidth = width_ / 3.0f;
            handleWidth = width_;
            break;
        case SplitterStyle::Divider:
            lineWidth = width_;
            handleWidth = width_ * 0.6f;
            break;
        case SplitterStyle::Invisible:
        default:
            break;
    }

    mat4 m(1.0f);
    m[0][0] = 2.0f;
    m[1][1] = 2.0f;
    m[3] = vec4(splitPosition_ * 2.0f - 1.0f, -1.0f, 0.0f, 1.0f);

    // scale matrix so that handle spans 50 pixel
    const int index = (splitDirection_ == SplitDirection::Vertical) ? 1 : 0;
    const float len = 50.0f / canvasDim[index];

    mat4 mHandle(m);
    mHandle[1][1] *= len;
    mHandle[3][1] = -len;

    mat4 mTri(m);
    mTri[3][1] = 0.0f;

    if (splitDirection_ == SplitDirection::Horizontal) {
        // rotate by 90 degree for horizontal split
        mat4 rotMat = glm::rotate(-glm::half_pi<float>(), vec3(0.0f, 0.0f, 1.0f));
        m = rotMat * m;
        mHandle = rotMat * mHandle;
        mTri = rotMat * mTri;
    }

    utilgl::BlendModeState blending(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    MeshDrawerGL::DrawObject drawer(lineMesh_->getRepresentation<MeshGL>(),
                                    lineMesh_->getDefaultMeshInfo());

    shader_.activate();
    shader_.setUniform("screenDim", canvasDim);
    shader_.setUniform("pickId", static_cast<uint32_t>(pickingMapper_.getPickingId(0)));

    // draw picking buffer
    shader_.setUniform("pickingEnabled", true);
    shader_.setUniform("lineWidth", width_);
    shader_.setUniform("geometry.dataToWorld", m);
    drawer.draw();

    if (drawHandle) {
        // draw background line
        shader_.setUniform("pickingEnabled", false);
        shader_.setUniform("color", bgColor_);
        shader_.setUniform("lineWidth", lineWidth);
        shader_.setUniform("geometry.dataToWorld", m);
        drawer.draw();

        shader_.setUniform("geometry.dataToWorld", mHandle);
        // draw handle border
        if (drawBorder) {
            shader_.setUniform("lineWidth", handleWidth + 4.0f);
            drawer.draw();
        }
        // draw handle
        shader_.setUniform("lineWidth", handleWidth);
        shader_.setUniform("color", color_);
        drawer.draw();
    }
    shader_.deactivate();

    if (hover_) {
        MeshDrawerGL::DrawObject triDrawer(triangleMesh_->getRepresentation<MeshGL>(),
                                           triangleMesh_->getDefaultMeshInfo());
        // rescale triangle positions from pixel coords to [-1][1]
        mTri[0][0] /= canvasDim.x;
        mTri[1][0] /= canvasDim.x;
        mTri[1][1] /= canvasDim.y;
        mTri[0][1] /= canvasDim.y;

        triShader_.activate();
        triShader_.setUniform("dataToClip", mTri);
        triDrawer.draw();
        triShader_.deactivate();
    }
}

void SplitImage::updateMesh() {
    auto mesh = std::make_shared<Mesh>(DrawType::Lines, ConnectivityType::Strip);
    mesh->addBuffer(Mesh::BufferInfo(BufferType::PositionAttrib),
                    util::makeBuffer(std::vector<vec2>{vec2(0.0f), vec2(0.0f, 1.0f)}));
    lineMesh_ = mesh;
}

void SplitImage::updateTriMesh() {
    const float offset = 16.0f;
    const float a = triSize_;                             // side length of equilateral triangle
    const float h = a * glm::root_three<float>() * 0.5f;  // height of the equilateral triangle

    auto mesh = std::make_shared<Mesh>(DrawType::Triangles, ConnectivityType::None);
    mesh->addBuffer(
        Mesh::BufferInfo(BufferType::PositionAttrib),
        util::makeBuffer(std::vector<vec2>{vec2(offset, a * 0.5f), vec2(offset, -a * 0.5f),
                                           vec2(offset + h, 0.0f), vec2(-offset, -a * 0.5f),
                                           vec2(-offset, a * 0.5f), vec2(-offset - h, 0.0f)}));
    mesh->addBuffer(Mesh::BufferInfo(BufferType::ColorAttrib),
                    util::makeBuffer(std::vector<vec4>{6, triColor_}));

    mesh->addIndices(Mesh::MeshInfo(DrawType::Triangles, ConnectivityType::None),
                     util::makeIndexBuffer({0, 1, 2}));
    mesh->addIndices(Mesh::MeshInfo(DrawType::Triangles, ConnectivityType::None),
                     util::makeIndexBuffer({3, 4, 5}));

    triangleMesh_ = mesh;
}

void SplitImage::handlePickingEvent(PickingEvent *e) {
    bool triggerUpdate = false;

    auto moveAction = [this](dvec2 delta) {
        // delta in normalized screen coords [0,1]
        // invert vertical delta
        delta.y = -delta.y;
        const double newValue =
            splitPosition_ + delta[(splitDirection_ == SplitDirection::Vertical) ? 0 : 1];
        splitPosition_.set(static_cast<float>(newValue));
    };

    if (e->getEvent()->hash() == MouseEvent::chash()) {
        auto mouseEvent = e->getEventAs<MouseEvent>();

        const bool leftMouseBtn = (mouseEvent->button() == MouseButton::Left);
        const bool mousePress = (mouseEvent->state() == MouseState::Press);
        const bool mouseMove = (mouseEvent->state() == MouseState::Move);

        if (e->getState() == PickingState::Started) {
            // start hover
            hover_ = true;
            triggerUpdate = true;
        } else if (e->getState() == PickingState::Finished) {
            // end hover
            hover_ = false;
            triggerUpdate = true;
        } else if (e->getState() == PickingState::Updated) {
            if (mouseMove && (mouseEvent->buttonState() & MouseButton::Left) == MouseButton::Left) {
                moveAction(e->getDeltaPosition());
                triggerUpdate = true;
                e->markAsUsed();
            } else if (leftMouseBtn) {
                if (mousePress) {
                    triggerUpdate = true;
                }
                e->markAsUsed();
            }
        }
    } else if (e->getEvent()->hash() == TouchEvent::chash()) {
        auto touchEvent = e->getEventAs<TouchEvent>();

        if (touchEvent->touchPoints().size() == 1) {
            // allow interaction only for a single touch point
            const auto &touchPoint = touchEvent->touchPoints().front();

            if (touchPoint.state() == TouchState::Started) {
                // initial activation since touch event began
                // start hover
                hover_ = true;
                triggerUpdate = true;
            } else if (touchPoint.state() == TouchState::Finished) {
                // end hover
                hover_ = false;
                triggerUpdate = true;
            } else if (touchPoint.state() == TouchState::Updated) {
                const auto delta = touchPoint.pos() - touchPoint.prevPos();
                moveAction(delta);
                triggerUpdate = true;
                e->markAsUsed();
            }
        }
    }
    if (triggerUpdate) {
        invalidate(InvalidationLevel::InvalidOutput);
    }
}

}  // namespace inviwo
