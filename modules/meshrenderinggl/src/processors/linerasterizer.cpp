/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2020 Inviwo Foundation
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

#include <modules/meshrenderinggl/processors/linerasterizer.h>
#include <modules/opengl/rendering/meshdrawergl.h>
#include <modules/opengl/shader/shaderutils.h>
#include <inviwo/core/algorithm/boundingbox.h>

#include <fmt/format.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo LineRasterizer::processorInfo_{
    "org.inviwo.LineRasterizer",  // Class identifier
    "Line Rasterizer",            // Display name
    "Mesh Rendering",             // Category
    CodeState::Stable,            // Code state
    Tags::GL,                     // Tags
};
const ProcessorInfo LineRasterizer::getProcessorInfo() const { return processorInfo_; }

LineRasterizer::LineRasterizer()
    : Processor()
    , inport_("geometry")
    , outport_("rasterization")
    , lineSettings_("lineSettings", "Line Settings")
    , writeDepth_("writeDepth", "Write Depth Layer", true)
    , forceOpaque_("forceOpaque", "Shade Opaque", false, InvalidationLevel::InvalidResources)
    , camera_("camera", "Camera", util::boundingBox(inport_))
    , trackball_(&camera_)
    , lineShaders_(new MeshShaderCache(
          {{ShaderType::Vertex, std::string{"linerenderer.vert"}},
           {ShaderType::Geometry, std::string{"linerenderer.geom"}},
           {ShaderType::Fragment, std::string{"oit-linerenderer.frag"}}},
          {{BufferType::PositionAttrib, MeshShaderCache::Mandatory, "vec3"},
           {BufferType::ColorAttrib, MeshShaderCache::Mandatory, "vec4"},
           {BufferType::PickingAttrib, MeshShaderCache::Optional, "uint"},
           {[](const Mesh&, Mesh::MeshInfo mi) -> int {
                return mi.ct == ConnectivityType::Adjacency ||
                               mi.ct == ConnectivityType::StripAdjacency
                           ? 1
                           : 0;
            },
            [this](int mode, Shader& shader) {
                shader[ShaderType::Geometry]->addShaderDefine("ENABLE_ADJACENCY", toString(mode));
                invalidate(InvalidationLevel::InvalidResources);
            }}},
          [this](Shader& shader) -> void { invalidate(InvalidationLevel::InvalidResources); })) {

    addPort(inport_);
    addPort(outport_);

    addProperties(lineSettings_, writeDepth_, forceOpaque_, camera_, trackball_);
    invalidate(InvalidationLevel::InvalidResources);
}

void LineRasterizer::initializeResources() {
    for (auto& shaderPair : lineShaders_->getShaders()) {
        Shader& shader = shaderPair.second;
        configureShader(shader);
    }
    invalidate(InvalidationLevel::Valid);
}

void LineRasterizer::process() {
    if (!inport_.hasData()) return;
    // configureAllShaders();
    for (auto& shaderPair : lineShaders_->getShaders()) {
        Shader& shader = shaderPair.second;
        if (!shader.isReady()) configureShader(shader);

        shader.activate();
        setUniforms(shader);
        shader.deactivate();
    }

    if (!outport_.hasData()) outport_.setData(new LineRasterization(*this));
}

void LineRasterizer::setUniforms(Shader& shader) const {
    utilgl::setUniforms(shader, camera_);

    shader.setUniform("lineWidth", lineSettings_.getWidth());
    shader.setUniform("antialiasing", lineSettings_.getAntialiasingWidth());
    shader.setUniform("miterLimit", lineSettings_.getMiterLimit());
    shader.setUniform("roundCaps", lineSettings_.getRoundCaps());
    // Stippling settings
    shader.setUniform("stippling.length", lineSettings_.getStippling().getLength());
    shader.setUniform("stippling.spacing", lineSettings_.getStippling().getSpacing());
    shader.setUniform("stippling.offset", lineSettings_.getStippling().getOffset());
    shader.setUniform("stippling.worldScale", lineSettings_.getStippling().getWorldScale());
}

void LineRasterizer::configureAllShaders() {
    for (auto shaderPair : lineShaders_->getShaders()) {
        configureShader(shaderPair.second);
    }
}

void LineRasterizer::configureShader(Shader& shader) {
    auto fso = shader.getFragmentShaderObject();

    fso->addShaderExtension("GL_NV_gpu_shader5", true);
    fso->addShaderExtension("GL_EXT_shader_image_load_store", true);
    fso->addShaderExtension("GL_NV_shader_buffer_load", true);
    fso->addShaderExtension("GL_EXT_bindable_uniform", true);

    fso->setShaderDefine("ENABLE_PSEUDO_LIGHTING", lineSettings_.getPseudoLighting());
    fso->setShaderDefine("ENABLE_ROUND_DEPTH_PROFILE", lineSettings_.getRoundDepthProfile());
    fso->setShaderDefine("USE_FRAGMENT_LIST", !forceOpaque_.get());

    utilgl::addShaderDefines(shader, lineSettings_.getStippling().getMode());
    shader.build();
}

// =========== Rasterization =========== //

LineRasterization::LineRasterization(const LineRasterizer& processor)
    : lineShaders_(processor.lineShaders_)
    , meshes_(processor.inport_.getVectorData())
    , forceOpaque_(processor.forceOpaque_)
    , writeDepth_(processor.writeDepth_.get()) {}

void LineRasterization::rasterize(const ivec2& imageSize,
                                  std::function<void(Shader&)> setUniformsRenderer) const {

    utilgl::BlendModeState blending(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    utilgl::DepthMaskState depthMask(writeDepth_);
    utilgl::DepthFuncState depthFunc(GL_LEQUAL);

    for (auto mesh : meshes_) {
        if (mesh->getNumberOfBuffers() == 0) return;

        MeshDrawerGL::DrawObject drawer(mesh->getRepresentation<MeshGL>(),
                                        mesh->getDefaultMeshInfo());
        if (mesh->getNumberOfIndicies() > 0) {
            for (size_t i = 0; i < mesh->getNumberOfIndicies(); ++i) {
                if (mesh->getIndexMeshInfo(i).dt != DrawType::Lines) continue;
                auto& shader = lineShaders_->getShader(*mesh, mesh->getIndexMeshInfo(i));
                if (!shader.isReady()) {
                    LogWarn("Shader not ready.");
                    break;
                }

                shader.activate();
                utilgl::setShaderUniforms(shader, *mesh, "geometry");
                shader.setUniform("screenDim", vec2(imageSize));
                setUniformsRenderer(shader);
                drawer.draw(i);
                shader.deactivate();
            }
        } else {
            auto& shader = lineShaders_->getShader(*mesh);
            if (mesh->getDefaultMeshInfo().dt != DrawType::Lines) return;
            if (!shader.isReady()) {
                LogWarn("Shader not ready.");
                break;
            }

            shader.activate();
            utilgl::setShaderUniforms(shader, *mesh, "geometry");
            shader.setUniform("screenDim", vec2(imageSize));
            setUniformsRenderer(shader);
            drawer.draw();
            shader.deactivate();
        }
    }
}

Document LineRasterization::getInfo() const {
    Document doc;
    doc.append("p", fmt::format("Line rasterization functor with {} line {}", meshes_.size(),
                                (meshes_.size() == 1) ? " mesh." : " meshes."));
    return doc;
}

}  // namespace inviwo
