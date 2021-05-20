/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2021 Inviwo Foundation
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

#include <modules/meshrenderinggl/processors/sphererasterizer.h>

#include <inviwo/core/datastructures/image/layer.h>
#include <inviwo/core/util/zip.h>
#include <inviwo/core/util/stringconversion.h>
#include <inviwo/core/algorithm/boundingbox.h>
#include <inviwo/core/util/document.h>
#include <modules/opengl/rendering/meshdrawergl.h>
#include <modules/opengl/shader/shaderutils.h>
#include <modules/opengl/openglutils.h>
#include <modules/opengl/openglcapabilities.h>
#include <modules/opengl/image/layergl.h>

#include <modules/meshrenderinggl/rendering/fragmentlistrenderer.h>

#include <fmt/format.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo SphereRasterizer::processorInfo_{
    "org.inviwo.SphereRasterizer",  // Class identifier
    "Sphere Rasterizer",            // Display name
    "Mesh Rendering",               // Category
    CodeState::Experimental,        // Code state
    "GL, Rendering",                // Tags
};
const ProcessorInfo SphereRasterizer::getProcessorInfo() const { return processorInfo_; }

SphereRasterizer::SphereRasterizer()
    : Processor()
    , inport_("inport")
    , outport_("rasterization")
    , renderMode_("renderMode", "Render Mode",
                  {{"entireMesh", "Entire Mesh", RenderMode::EntireMesh},
                   {"pointsOnly", "Points Only", RenderMode::PointsOnly}})
    , forceOpaque_("forceOpaque", "Shade Opaque", false, InvalidationLevel::InvalidResources)
    , useUniformAlpha_("useUniformAlpha", "Uniform Alpha", false,
                       InvalidationLevel::InvalidResources)
    , uniformAlpha_("alphaValue", "Alpha", 0.7f, 0, 1, 0.1f, InvalidationLevel::InvalidOutput)
    , clipping_("clipping", "Clipping")
    , clipMode_("clipMode", "Clip Mode",
                {{"discard", "Discard Glyph", GlyphClippingMode::Discard},
                 {"cut", "Cut Glypyh", GlyphClippingMode::Cut}},
                0, InvalidationLevel::InvalidResources)
    , clipShadingFactor_("clipShadingFactor", "Clip Surface Adjustment", 0.9f, 0.0f, 2.0f)
    , shadeClippedArea_("shadeClippedArea", "Shade Clipped Area", false,
                        InvalidationLevel::InvalidResources)
    , sphereProperties_("sphereProperties", "Sphere Properties")
    , forceRadius_("forceRadius", "Force Radius", false, InvalidationLevel::InvalidResources)
    , defaultRadius_("defaultRadius", "Default Radius", 0.05f, 0.00001f, 2.0f, 0.01f)
    , forceColor_("forceColor", "Force Color", false, InvalidationLevel::InvalidResources)
    , defaultColor_("defaultColor", "Default Color",
                    util::ordinalColor(vec4(0.7f, 0.7f, 0.7f, 1.0f)))
    , useMetaColor_("useMetaColor", "Use meta color mapping", false,
                    InvalidationLevel::InvalidResources)
    , metaColor_("metaColor", "Meta Color Mapping")

    , camera_("camera", "Camera", util::boundingBox(inport_))
    , trackball_(&camera_)
    , lighting_("lighting", "Lighting", &camera_)
    , transformSetting_("transformSettings", "Additional Transform")
    , shaders_{new MeshShaderCache(
          {{ShaderType::Vertex, std::string{"sphereglyph.vert"}},
           {ShaderType::Geometry, std::string{"sphereglyph.geom"}},
           {ShaderType::Fragment, std::string{"oit-sphereglyph.frag"}}},

          {{BufferType::PositionAttrib, MeshShaderCache::Mandatory, "vec3"},
           {BufferType::ColorAttrib, MeshShaderCache::Optional, "vec4"},
           {BufferType::RadiiAttrib, MeshShaderCache::Optional, "float"},
           {BufferType::ScalarMetaAttrib, MeshShaderCache::Optional, "float"}},

          [&](Shader& shader) -> void {
              shader.onReload([this]() { invalidate(InvalidationLevel::InvalidResources); });
              configureShader(shader);
          })}
    , oitExtensionsAvailable_(
          OpenGLCapabilities::isExtensionSupported("GL_NV_gpu_shader5") &&
          OpenGLCapabilities::isExtensionSupported("GL_EXT_shader_image_load_store") &&
          OpenGLCapabilities::isExtensionSupported("GL_NV_shader_buffer_load") &&
          OpenGLCapabilities::isExtensionSupported("GL_EXT_bindable_uniform")) {

    addPort(inport_);
    addPort(outport_);

    clipping_.addProperties(clipMode_, clipShadingFactor_, shadeClippedArea_);

    sphereProperties_.addProperties(forceRadius_, defaultRadius_, forceColor_, defaultColor_,
                                    useMetaColor_, metaColor_);

    addProperties(renderMode_, forceOpaque_, useUniformAlpha_, uniformAlpha_, sphereProperties_,
                  clipping_, transformSetting_, camera_, lighting_, trackball_);

    clipShadingFactor_.readonlyDependsOn(
        clipMode_, [](const auto& p) { return p == GlyphClippingMode::Discard; });
    shadeClippedArea_.readonlyDependsOn(
        clipMode_, [](const auto& p) { return p == GlyphClippingMode::Discard; });

    transformSetting_.setCollapsed(true);
    camera_.setCollapsed(true);
    lighting_.setCollapsed(true);
    trackball_.setCollapsed(true);
}

void SphereRasterizer::process() {
    std::shared_ptr<const Rasterization> rasterization =
        std::make_shared<SphereRasterization>(*this);

    auto data = (transformSetting_.transforms_.empty())
                    ? rasterization
                    : std::make_shared<TransformedRasterization>(rasterization,
                                                                 transformSetting_.getMatrix());
    outport_.setData(data);
}

void SphereRasterizer::initializeResources() {
    for (auto& item : shaders_->getShaders()) {
        configureShader(item.second);
    }
}

void SphereRasterizer::configureShader(Shader& shader) {
    utilgl::addDefines(shader, lighting_);
    shader[ShaderType::Vertex]->setShaderDefine("FORCE_RADIUS", forceRadius_);
    shader[ShaderType::Vertex]->setShaderDefine("FORCE_COLOR", forceColor_);
    shader[ShaderType::Vertex]->setShaderDefine("USE_SCALARMETACOLOR", useMetaColor_);
    shader[ShaderType::Fragment]->setShaderDefine("SHADE_CLIPPED_AREA", shadeClippedArea_);
    shader[ShaderType::Fragment]->setShaderDefine("DISCARD_CLIPPED_GLYPHS",
                                                  clipMode_.get() == GlyphClippingMode::Discard);
    shader[ShaderType::Geometry]->setShaderDefine("DISCARD_CLIPPED_GLYPHS",
                                                  clipMode_.get() == GlyphClippingMode::Discard);

    configureOITShader(shader);
    shader.build();
}

void SphereRasterizer::configureOITShader(Shader& shader) {
    auto fso = shader.getFragmentShaderObject();

    if (oitExtensionsAvailable_) {
        fso->addShaderExtension("GL_NV_gpu_shader5", true);
        fso->addShaderExtension("GL_EXT_shader_image_load_store", true);
        fso->addShaderExtension("GL_NV_shader_buffer_load", true);
        fso->addShaderExtension("GL_EXT_bindable_uniform", true);
    }

    fso->setShaderDefine("USE_FRAGMENT_LIST", !forceOpaque_.get() &&
                                                  FragmentListRenderer::supportsFragmentLists() &&
                                                  oitExtensionsAvailable_);

    fso->setShaderDefine("UNIFORM_ALPHA", useUniformAlpha_.get());
}

SphereRasterization::SphereRasterization(const SphereRasterizer& processor)
    : renderMode_(processor.renderMode_)
    , uniformAlpha_(processor.uniformAlpha_)
    , forceOpaque_(processor.forceOpaque_)
    , clipShadingFactor_(processor.clipShadingFactor_)
    , defaultRadius_(processor.defaultRadius_)
    , defaultColor_(processor.defaultColor_)
    , metaColorTF_(processor.metaColor_->getData())
    , lighting_(processor.lighting_.getState())
    , shaders_(processor.shaders_)
    , meshes_(processor.inport_.getVectorData())
    , oitExtensionsAvailable_(processor.oitExtensionsAvailable_) {}

void SphereRasterization::rasterize(const ivec2& imageSize, const mat4& worldMatrixTransform,
                                    std::function<void(Shader&)> setUniforms) const {

    utilgl::BlendModeState blendModeStateGL(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    utilgl::GlBoolState depthTest(GL_DEPTH_TEST, !usesFragmentLists());

    for (auto mesh : meshes_) {
        if (mesh->getNumberOfBuffers() == 0) continue;

        auto& shader = shaders_->getShader(*mesh);

        shader.activate();

        TextureUnit texUnit;
        const LayerGL* tf = metaColorTF_->getRepresentation<LayerGL>();
        tf->bindTexture(texUnit.getEnum());
        shader.setUniform("metaColor", texUnit.getUnitNumber());
        shader.setUniform("defaultColor", defaultColor_);
        shader.setUniform("defaultRadius", defaultRadius_);
        shader.setUniform("clipShadingFactor", clipShadingFactor_);
        shader.setUniform("uniformAlpha", uniformAlpha_);
        utilgl::setShaderUniforms(shader, lighting_, "lighting");
        shader.setUniform("viewport", vec4(0.0f, 0.0f, 2.0f / imageSize.x, 2.0f / imageSize.y));
        MeshDrawerGL::DrawObject drawer(mesh->getRepresentation<MeshGL>(),
                                        mesh->getDefaultMeshInfo());
        setUniforms(shader);
        auto transform = CompositeTransform(mesh->getModelMatrix(),
                                            mesh->getWorldMatrix() * worldMatrixTransform);
        utilgl::setShaderUniforms(shader, transform, "geometry");
        switch (renderMode_) {
            case SphereRasterizer::RenderMode::PointsOnly: {
                // render only index buffers marked as points (or the entire mesh if none
                // exists)
                if (mesh->getNumberOfIndicies() > 0) {
                    for (size_t i = 0; i < mesh->getNumberOfIndicies(); ++i) {
                        auto meshinfo = mesh->getIndexMeshInfo(i);
                        if ((meshinfo.dt == DrawType::Points) ||
                            (meshinfo.dt == DrawType::NotSpecified)) {
                            drawer.draw(MeshDrawerGL::DrawMode::Points, i);
                        }
                    }
                } else {
                    // no index buffers, check mesh default draw type
                    auto drawtype = mesh->getDefaultMeshInfo().dt;
                    if ((drawtype == DrawType::Points) || (drawtype == DrawType::NotSpecified)) {
                        drawer.draw(MeshDrawerGL::DrawMode::Points);
                    }
                }
                break;
            }
            case SphereRasterizer::RenderMode::EntireMesh:
                [[fallthrough]];
            default:  // render all parts of the input meshes as points
                drawer.draw(MeshDrawerGL::DrawMode::Points);
                break;
        }

        shader.deactivate();
    }
}

bool SphereRasterization::usesFragmentLists() const {
    return !forceOpaque_ && FragmentListRenderer::supportsFragmentLists() && oitExtensionsAvailable_;
}

Document SphereRasterization::getInfo() const {
    Document doc;
    doc.append("p", fmt::format("Sphere mesh rasterization functor with {} mesh{}. {}.",
                                meshes_.size(), (meshes_.size() == 1) ? "" : "es",
                                usesFragmentLists() ? "Using A-buffer" : "Rendering opaque"));
    return doc;
}

Rasterization* SphereRasterization::clone() const { return new SphereRasterization(*this); }

}  // namespace inviwo
