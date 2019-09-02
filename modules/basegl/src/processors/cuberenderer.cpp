/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2019 Inviwo Foundation
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
#include <modules/opengl/texture/textureutils.h>
#include <modules/opengl/openglutils.h>
#include <modules/basegl/datastructures/meshshadercache.h>
#include <inviwo/core/algorithm/boundingbox.h>

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
    , forceSize_("forceSize", "Force Size", false, InvalidationLevel::InvalidResources)
    , defaultSize_("defaultSize", "Default Size", 0.05f, 0.00001f, 2.0f, 0.01f)
    , forceColor_("forceColor", "Force Color", false, InvalidationLevel::InvalidResources)
    , defaultColor_("defaultColor", "Default Color", vec4(0.7f, 0.7f, 0.7f, 1.0f), vec4(0.0f),
                    vec4(1.0f))
    , useMetaColor_("useMetaColor", "Use meta color mapping", false,
                    InvalidationLevel::InvalidResources)
    , metaColor_("metaColor", "Meta Color Mapping")

    , camera_("camera", "Camera", util::boundingBox(inport_))
    , trackball_(&camera_)
    , lighting_("lighting", "Lighting", &camera_)
    , shaders_{{{ShaderType::Vertex, std::string{"cubeglyph.vert"}},
                {ShaderType::Geometry, std::string{"cubeglyph.geom"}},
                {ShaderType::Fragment, std::string{"cubeglyph.frag"}}},

               {{BufferType::PositionAttrib, MeshShaderCache::Mandatory, "vec3"},
                {BufferType::ColorAttrib, MeshShaderCache::Optional, "vec4"},
                {BufferType::RadiiAttrib, MeshShaderCache::Optional, "float"},
                {BufferType::PickingAttrib, MeshShaderCache::Optional, "uint"},
                {BufferType::ScalarMetaAttrib, MeshShaderCache::Optional, "float"}},

               [&](Shader& shader) -> void {
                   shader.onReload([this]() { invalidate(InvalidationLevel::InvalidResources); });
                   configureShader(shader);
               }} {

    addPort(inport_);
    addPort(imageInport_).setOptional(true);
    addPort(outport_);

    cubeProperties_.addProperty(forceSize_);
    cubeProperties_.addProperty(defaultSize_);
    cubeProperties_.addProperty(forceColor_);
    cubeProperties_.addProperty(defaultColor_);
    cubeProperties_.addProperty(useMetaColor_);
    cubeProperties_.addProperty(metaColor_);
    defaultColor_.setSemantics(PropertySemantics::Color);

    addProperty(cubeProperties_);

    addProperty(camera_);
    addProperty(lighting_);
    addProperty(trackball_);
}

void CubeRenderer::initializeResources() {
    for (auto& item : shaders_.getShaders()) {
        configureShader(item.second);
    }
}

void CubeRenderer::configureShader(Shader& shader) {
    utilgl::addDefines(shader, lighting_);
    shader[ShaderType::Vertex]->setShaderDefine("FORCE_SIZE", forceSize_);
    shader[ShaderType::Vertex]->setShaderDefine("FORCE_COLOR", forceColor_);
    shader[ShaderType::Vertex]->setShaderDefine("USE_SCALARMETACOLOR", useMetaColor_);
    shader.build();
}

void CubeRenderer::process() {
    utilgl::activateTargetAndClearOrCopySource(outport_, imageInport_);

    for (const auto& mesh : inport_) {
        auto& shader = shaders_.getShader(*mesh);

        shader.activate();
        utilgl::BlendModeState blendModeStateGL(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        TextureUnitContainer units;
        utilgl::bindAndSetUniforms(shader, units, metaColor_);
        utilgl::setUniforms(shader, camera_, lighting_, defaultColor_, defaultSize_);
        shader.setUniform("viewport", vec4(0.0f, 0.0f, 2.0f / outport_.getDimensions().x,
                                           2.0f / outport_.getDimensions().y));

        MeshDrawerGL::DrawObject drawer(mesh->getRepresentation<MeshGL>(),
                                        mesh->getDefaultMeshInfo());
        utilgl::setShaderUniforms(shader, *mesh, "geometry");

        drawer.draw(MeshDrawerGL::DrawMode::Points);

        shader.deactivate();
    }
    utilgl::deactivateCurrentTarget();
}

}  // namespace inviwo
