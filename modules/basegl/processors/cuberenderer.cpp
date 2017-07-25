/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017 Inviwo Foundation
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

#include <modules/basegl/processors/cuberenderer.h>
#include <modules/opengl/rendering/meshdrawergl.h>
#include <modules/opengl/shader/shaderutils.h>
#include <modules/opengl/openglutils.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo CubeRenderer::processorInfo_{
    "org.inviwo.CubeRenderer",  // Class identifier
    "Cube Renderer",            // Display name
    "Mesh Rendering",           // Category
    CodeState::Stable,          // Code state
    Tags::GL,                   // Tags
};
const ProcessorInfo CubeRenderer::getProcessorInfo() const { return processorInfo_; }

CubeRenderer::CubeRenderer()
    : Processor()
    , inport_("geometry")
    , imageInport_("imageInport")
    , outport_("image")
    , cubeProperties_("cubeProperties", "Cube Properties")
    , overrideCubeSize_("overrideCubeSize", "Override Cube Size", false,
                        InvalidationLevel::InvalidResources)
    , customSize_("customSize", "Custom Size", 0.05f, 0.00001f, 2.0f, 0.01f)
    , overrideCubeColor_("overrideCubeColor", "Override Cube Color", false,
                         InvalidationLevel::InvalidResources)
    , customColor_("customColor", "Custom Color", vec4(0.7f, 0.7f, 0.7f, 1.0f), vec4(0.0f),
                   vec4(1.0f))
    , camera_("camera", "Camera")
    , lighting_("lighting", "Lighting", &camera_)
    , trackball_(&camera_)
    , shader_("cubeglyph.vert", "cubeglyph.geom", "cubeglyph.frag", false) {

    outport_.addResizeEventListener(&camera_);

    addPort(inport_);
    addPort(imageInport_);
    addPort(outport_);
    imageInport_.setOptional(true);

    customColor_.setSemantics(PropertySemantics::Color);

    cubeProperties_.addProperty(overrideCubeSize_);
    cubeProperties_.addProperty(customSize_);
    cubeProperties_.addProperty(overrideCubeColor_);
    cubeProperties_.addProperty(customColor_);

    addProperty(cubeProperties_);

    addProperty(camera_);
    addProperty(lighting_);
    addProperty(trackball_);

    shader_.onReload([this]() { invalidate(InvalidationLevel::InvalidResources); });
    shader_.getVertexShaderObject()->addShaderExtension("GL_EXT_geometry_shader4", true);
}

void CubeRenderer::process() {
    if (imageInport_.isConnected()) {
        utilgl::activateTargetAndCopySource(outport_, imageInport_, ImageType::ColorDepthPicking);
    } else {
        utilgl::activateAndClearTarget(outport_, ImageType::ColorDepthPicking);
    }

    shader_.activate();
    utilgl::BlendModeState blendModeStateGL(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    utilgl::setUniforms(shader_, camera_, lighting_, customColor_, customSize_);

    shader_.setUniform("viewport", vec4(0.0f, 0.0f, 2.0f / outport_.getDimensions().x,
                                        2.0f / outport_.getDimensions().y));
    drawMeshes();

    shader_.deactivate();
    utilgl::deactivateCurrentTarget();
}

void CubeRenderer::initializeResources() {
    utilgl::addShaderDefines(shader_, lighting_);

    if (overrideCubeSize_.get()) {
        shader_.getVertexShaderObject()->addShaderDefine("UNIFORM_SIZE");
    } else {
        shader_.getVertexShaderObject()->removeShaderDefine("UNIFORM_SIZE");
    }

    if (overrideCubeColor_.get()) {
        shader_.getVertexShaderObject()->addShaderDefine("UNIFORM_COLOR");
    } else {
        shader_.getVertexShaderObject()->removeShaderDefine("UNIFORM_COLOR");
    }

    shader_.build();
}

void CubeRenderer::drawMeshes() {
    
            for (const auto& elem : inport_) {
                MeshDrawerGL::DrawObject drawer(elem->getRepresentation<MeshGL>(),
                                                elem->getDefaultMeshInfo());
                utilgl::setShaderUniforms(shader_, *elem, "geometry");
                shader_.setUniform("pickingEnabled", meshutil::hasPickIDBuffer(elem.get()));
                drawer.draw(MeshDrawerGL::DrawMode::Points);
            }
    
    
}

}  // namespace inviwo
