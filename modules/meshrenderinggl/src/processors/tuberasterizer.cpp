/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2020 Inviwo Foundation
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

#include <modules/meshrenderinggl/processors/tuberasterizer.h>
#include <modules/meshrenderinggl/rendering/fragmentlistrenderer.h>
#include <modules/meshrenderinggl/datastructures/transformedrasterization.h>

#include <modules/opengl/image/layergl.h>
#include <modules/opengl/texture/textureutils.h>
#include <modules/opengl/shader/shaderutils.h>
#include <modules/opengl/openglutils.h>
#include <modules/opengl/rendering/meshdrawergl.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/algorithm/boundingbox.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo TubeRasterizer::processorInfo_{
    "org.inviwo.TubeRasterizer",  // Class identifier
    "Tube Rasterizer",            // Display name
    "Mesh Rendering",             // Category
    CodeState::Stable,            // Code state
    Tags::GL,                     // Tags
};
const ProcessorInfo TubeRasterizer::getProcessorInfo() const { return processorInfo_; }

TubeRasterizer::TubeRasterizer()
    : Processor()
    , inport_("meshes")
    , outport_("rasterization")
    , tubeProperties_("tubeProperties", "Tube Properties")
    , forceRadius_("forceRadius", "Force Radius", false, InvalidationLevel::InvalidResources)
    , defaultRadius_("defaultRadius", "Tube Radius", 0.01f, 0.0001f, 2.f, 0.0001f)
    , forceColor_("forceColor", "Force Color", false, InvalidationLevel::InvalidResources)
    , defaultColor_("defaultColor", "Default Color", vec4(0.7f, 0.7f, 0.7f, 1.0f), vec4(0.0f),
                    vec4(1.0f), vec4(0.01f), InvalidationLevel::InvalidOutput,
                    PropertySemantics::Color)
    , useMetaColor_("useMetaColor", "Use meta color mapping", false,
                    InvalidationLevel::InvalidResources)
    , metaColor_("metaColor", "Meta Color Mapping")
    , forceOpaque_("forceOpaque", "Shade Opaque")
    , transformSetting_("transformSettings", "Additional Transform")
    , lighting_("lighting", "Lighting")
    , shaderItems_{{{ShaderType::Vertex, "tuberendering.vert"},
                    {ShaderType::Geometry, "tuberendering.geom"},
                    {ShaderType::Fragment, "tuberendering.frag"}}}
    , shaderRequirements_{{{BufferType::PositionAttrib, MeshShaderCache::Mandatory, "vec3"},
                           {BufferType::ColorAttrib, MeshShaderCache::Optional, "vec4"},
                           {BufferType::RadiiAttrib, MeshShaderCache::Optional, "float"},
                           {BufferType::PickingAttrib, MeshShaderCache::Optional, "uint"},
                           {BufferType::ScalarMetaAttrib, MeshShaderCache::Optional, "float"}}}
    , adjacencyShaders_{new MeshShaderCache(shaderItems_, shaderRequirements_,
                                            [&](Shader& shader) -> void {
                                                shader.onReload([this]() {
                                                    invalidate(InvalidationLevel::InvalidResources);
                                                });
                                                for (auto& obj : shader.getShaderObjects()) {
                                                    obj.addShaderDefine("HAS_ADJACENCY");
                                                }
                                                invalidate(InvalidationLevel::InvalidResources);
                                            })}

    , shaders_{new MeshShaderCache(shaderItems_, shaderRequirements_, [&](Shader& shader) -> void {
        shader.onReload([this]() { invalidate(InvalidationLevel::InvalidResources); });
    })} {
    addPort(inport_);
    addPort(outport_);

    tubeProperties_.addProperties(forceRadius_, defaultRadius_, forceColor_, defaultColor_,
                                  useMetaColor_, metaColor_);
    addProperties(tubeProperties_, transformSetting_, forceOpaque_, lighting_);
    transformSetting_.setCollapsed(true);
}

void TubeRasterizer::initializeResources() {
    LogWarn("Initializing");
    for (auto& item : adjacencyShaders_->getShaders()) {
        configureShader(item.second);
    }
    for (auto& item : shaders_->getShaders()) {
        configureShader(item.second);
    }
}

void TubeRasterizer::configureShader(Shader& shader) {
    utilgl::addDefines(shader, lighting_);
    LogWarn("Configuring!");

    // auto fso = shader.getFragmentShaderObject();
    // fso->addShaderExtension("GL_NV_gpu_shader5", true);
    // fso->addShaderExtension("GL_EXT_shader_image_load_store", true);
    // fso->addShaderExtension("GL_NV_shader_buffer_load", true);
    // fso->addShaderExtension("GL_EXT_bindable_uniform", true);
    // fso->setShaderDefine("USE_FRAGMENT_LIST",
    //                      !forceOpaque_.get() && FragmentListRenderer::supportsFragmentLists());

    auto vso = shader.getVertexShaderObject();
    vso->setShaderDefine("FORCE_RADIUS", forceRadius_);
    vso->setShaderDefine("FORCE_COLOR", forceColor_);
    vso->setShaderDefine("USE_SCALARMETACOLOR", useMetaColor_);

    shader.build();
    if (!shader.isReady()) {
        LogWarn("Config/build didn't work");
        return;
    }
}

void TubeRasterizer::process() {
    auto setAllUniforms = [this](Shader& shader) {
        if (!shader.isReady()) configureShader(shader);
        if (!shader.isReady()) {
            LogWarn("Cannot set uniform");
            return;
        }

        shader.activate();
        setUniforms(shader);
        shader.deactivate();
    };

    for (auto& shaderPair : shaders_->getShaders()) {
        setAllUniforms(shaderPair.second);
    }
    for (auto& shaderPair : adjacencyShaders_->getShaders()) {
        setAllUniforms(shaderPair.second);
    }

    std::shared_ptr<const Rasterization> rasterization = std::make_shared<TubeRasterization>(*this);

    // If transform is applied, wrap rasterization.
    if (transformSetting_.transforms_.size() > 0) {
        outport_.setData(std::make_shared<TransformedRasterization>(rasterization,
                                                                    transformSetting_.getMatrix()));
    } else {
        outport_.setData(rasterization);
    }
}

void TubeRasterizer::setUniforms(Shader& shader) const {
    utilgl::setUniforms(shader, lighting_, defaultColor_, defaultRadius_);
}

// =========== Rasterization =========== //

TubeRasterization::TubeRasterization(const TubeRasterizer& processor)
    : adjacencyShaders_(processor.adjacencyShaders_)
    , shaders_(processor.adjacencyShaders_)
    , meshes_(processor.inport_.getVectorData())
    , tfTexture_(processor.metaColor_->getData())
    , forceOpaque_(processor.forceOpaque_.get()) {}

void TubeRasterization::rasterize(const ivec2&, const mat4& worldMatrixTransform,
                                  std::function<void(Shader&)> setUniformsRenderer) const {

    const auto hasLineAdjacency = [](Mesh::MeshInfo mi) {
        return mi.dt == DrawType::Lines &&
               (mi.ct == ConnectivityType::StripAdjacency || mi.ct == ConnectivityType::Adjacency);
    };
    const auto hasLine = [](Mesh::MeshInfo mi) {
        return mi.dt == DrawType::Lines &&
               (mi.ct == ConnectivityType::None || mi.ct == ConnectivityType::Strip);
    };

    const auto hasAnyLine = [](const Mesh& mesh, auto test) {
        if (mesh.getNumberOfIndicies() > 0) {
            for (size_t i = 0; i < mesh.getNumberOfIndicies(); ++i) {
                if (test(mesh.getIndexMeshInfo(i))) return true;
            }
        } else {
            if (test(mesh.getDefaultMeshInfo())) return true;
        }
        return false;
    };

    // The geometry shader generates a six-sided bounding box for each line segment. The
    // fragment shader does not consider if the current fragment is on a front- or backface. The
    // ray-cylinder intersection test will thus give the same result for both, hence resulting
    // in z-fighting. To avoid this we turn on face culling.
    utilgl::CullFaceState cullstate(GL_BACK);

    const auto draw = [this, hasAnyLine, &setUniformsRenderer, &worldMatrixTransform](
                          const Mesh& mesh, Shader& shader, auto test) {
        if (!hasAnyLine(mesh, test)) return;

        if (!shader.isReady()) {
            LogWarn("Cannot draw");
            return;
        }
        shader.activate();
        // TextureUnitContainer units;
        // utilgl::bindAndSetUniforms(shader, units, metaColor_);

        // TextureUnit transFuncUnit;
        // const LayerGL* transferFunctionGL = tfTexture_->getRepresentation<LayerGL>();
        // transferFunctionGL->bindTexture(transFuncUnit.getEnum());
        // shader.setUniform("metaColor", transFuncUnit.getUnitNumber());

        utilgl::GlBoolState depthTest(GL_DEPTH_TEST, true);
        MeshDrawerGL::DrawObject drawer(mesh.getRepresentation<MeshGL>(),
                                        mesh.getDefaultMeshInfo());

        auto transform =
            CompositeTransform(mesh.getModelMatrix(), mesh.getWorldMatrix() * worldMatrixTransform);
        utilgl::setShaderUniforms(shader, transform, "geometry");
        setUniformsRenderer(shader);

        if (mesh.getNumberOfIndicies() > 0) {
            for (size_t i = 0; i < mesh.getNumberOfIndicies(); ++i) {
                const auto mi = mesh.getIndexMeshInfo(i);
                if (test(mi)) {
                    drawer.draw(i);
                }
            }
        } else {
            // no index buffers, check mesh default draw type
            const auto mi = mesh.getDefaultMeshInfo();
            if (test(mi)) {
                drawer.draw();
            }
        }
        shader.deactivate();
    };

    for (const auto& mesh : meshes_) {
        draw(*mesh, adjacencyShaders_->getShader(*mesh), hasLineAdjacency);
        draw(*mesh, shaders_->getShader(*mesh), hasLine);
    }
}  // namespace inviwo

bool TubeRasterization::usesFragmentLists() const {
    return !forceOpaque_ && FragmentListRenderer::supportsFragmentLists();
}

Document TubeRasterization::getInfo() const {
    Document doc;
    doc.append("p", fmt::format("Tube rasterization functor with {} line {}. {}.", meshes_.size(),
                                (meshes_.size() == 1) ? " mesh" : " meshes",
                                usesFragmentLists() ? "Using A-buffer" : "Rendering opaque"));
    return doc;
}

Rasterization* TubeRasterization::clone() const { return new TubeRasterization(*this); }

}  // namespace inviwo
