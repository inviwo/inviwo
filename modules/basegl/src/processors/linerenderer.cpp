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

#include <modules/basegl/processors/linerenderer.h>
#include <modules/opengl/rendering/meshdrawergl.h>
#include <modules/opengl/shader/shaderutils.h>
#include <inviwo/core/algorithm/boundingbox.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo LineRenderer::processorInfo_{
    "org.inviwo.LineRenderer",  // Class identifier
    "Line Renderer",            // Display name
    "Mesh Rendering",           // Category
    CodeState::Stable,          // Code state
    Tags::GL,                   // Tags
};
const ProcessorInfo LineRenderer::getProcessorInfo() const { return processorInfo_; }

LineRenderer::LineRenderer()
    : Processor()
    , inport_("geometry")
    , imageInport_("imageInport")
    , outport_("image")
    , lineWidth_("lineWidth", "Line Width (pixel)", 1.0f, 0.0f, 50.0f, 0.1f)
    , antialiasing_("antialiasing", "Antialiasing (pixel)", 0.5f, 0.0f, 10.0f, 0.1f)
    , miterLimit_("miterLimit", "Miter Limit", 0.8f, 0.0f, 1.0f, 0.1f)
    , roundCaps_("roundCaps", "Round Caps", true)
    , pseudoLighting_("pseudoLighting", "Pseudo Lighting", true,
                      InvalidationLevel::InvalidResources)
    , roundDepthProfile_("roundDepthProfile", "Round Depth Profile", true,
                         InvalidationLevel::InvalidResources)
    , writeDepth_("writeDepth", "Write Depth Layer", true)
    , stippling_("stippling", "Stippling")
    , camera_("camera", "Camera", util::boundingBox(inport_))
    , trackball_(&camera_)
    , lineShaders_{
          {{ShaderType::Vertex, std::string{"linerenderer.vert"}},
           {ShaderType::Geometry, std::string{"linerenderer.geom"}},
           {ShaderType::Fragment, std::string{"linerenderer.frag"}}},

          {{BufferType::PositionAttrib, MeshShaderCache::Mandatory, "vec3"},
           {BufferType::ColorAttrib, MeshShaderCache::Mandatory, "vec4"},
           {BufferType::PickingAttrib, MeshShaderCache::Optional, "uint"},
           {[](const Mesh&, Mesh::MeshInfo mi) -> int {
                return mi.ct == ConnectivityType::Adjacency ||
                               mi.ct == ConnectivityType::StripAdjacency
                           ? 1
                           : 0;
            },
            [](int mode, Shader& shader) {
                shader[ShaderType::Geometry]->addShaderDefine("ENABLE_ADJACENCY", toString(mode));
            }}},
          [&](Shader& shader) -> void {
              shader.onReload([this]() { invalidate(InvalidationLevel::InvalidResources); });
              configureShader(shader);
          }} {

    addPort(inport_);
    addPort(imageInport_);
    addPort(outport_);
    imageInport_.setOptional(true);

    addProperty(lineWidth_);
    addProperty(antialiasing_);
    addProperty(miterLimit_);
    addProperty(roundCaps_);
    addProperty(pseudoLighting_);
    addProperty(roundDepthProfile_);
    addProperty(writeDepth_);

    addProperty(stippling_);

    addProperty(camera_);
    addProperty(trackball_);
}

void LineRenderer::initializeResources() {
    for (auto& item : lineShaders_.getShaders()) {
        configureShader(item.second);
    }
}

void LineRenderer::configureShader(Shader& shader) {
    shader[ShaderType::Fragment]->setShaderDefine("ENABLE_PSEUDO_LIGHTING", pseudoLighting_);
    shader[ShaderType::Fragment]->setShaderDefine("ENABLE_ROUND_DEPTH_PROFILE", roundDepthProfile_);

    utilgl::addShaderDefines(shader, stippling_);
    shader.build();
}

void LineRenderer::process() {
    utilgl::activateTargetAndClearOrCopySource(outport_, imageInport_);

    utilgl::BlendModeState blending(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    utilgl::DepthMaskState depthMask(writeDepth_.get());

    utilgl::DepthFuncState depthFunc(GL_LEQUAL);

    drawMeshes();

    utilgl::deactivateCurrentTarget();
}

void LineRenderer::drawMeshes() {
    for (const auto& mesh : inport_) {
        if (mesh->getNumberOfBuffers() == 0) continue;
        MeshDrawerGL::DrawObject drawer(mesh->getRepresentation<MeshGL>(),
                                        mesh->getDefaultMeshInfo());
        if (mesh->getNumberOfIndicies() > 0) {
            for (std::size_t i = 0; i < mesh->getNumberOfIndicies(); ++i) {
                if (mesh->getIndexMeshInfo(i).dt != DrawType::Lines) continue;
                auto& shader = lineShaders_.getShader(*mesh, mesh->getIndexMeshInfo(i));
                shader.activate();
                shader.setUniform("screenDim", vec2(outport_.getDimensions()));
                utilgl::setUniforms(shader, camera_, lineWidth_, antialiasing_, miterLimit_,
                                    roundCaps_, stippling_);

                utilgl::setShaderUniforms(shader, *mesh, "geometry");
                drawer.draw(i);
                shader.deactivate();
            }
        } else {
            auto& shader = lineShaders_.getShader(*mesh);
            if (mesh->getDefaultMeshInfo().dt != DrawType::Lines) continue;
            shader.activate();
            shader.setUniform("screenDim", vec2(outport_.getDimensions()));
            utilgl::setUniforms(shader, camera_, lineWidth_, antialiasing_, miterLimit_, roundCaps_,
                                stippling_);
            utilgl::setShaderUniforms(shader, *mesh, "geometry");

            drawer.draw();
            shader.deactivate();
        }
    }
}

}  // namespace inviwo
