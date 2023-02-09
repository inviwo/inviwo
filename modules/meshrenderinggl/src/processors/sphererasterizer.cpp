/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2021-2022 Inviwo Foundation
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

#include <inviwo/core/algorithm/boundingbox.h>                                // for boundingBox
#include <inviwo/core/datastructures/geometry/geometrytype.h>                 // for BufferType
#include <inviwo/core/datastructures/geometry/mesh.h>                         // for Mesh::MeshInfo
#include <inviwo/core/datastructures/image/layer.h>                           // for Layer
#include <inviwo/core/datastructures/representationconverter.h>               // for Representat...
#include <inviwo/core/datastructures/representationconverterfactory.h>        // for Representat...
#include <inviwo/core/datastructures/transferfunction.h>                      // for TransferFun...
#include <inviwo/core/interaction/cameratrackball.h>                          // for CameraTrack...
#include <inviwo/core/ports/meshport.h>                                       // for MeshFlatMul...
#include <inviwo/core/properties/simplelightingproperty.h>                    // for SimpleLight...
#include <inviwo/core/util/document.h>                                        // for Document
#include <inviwo/core/util/glmmat.h>                                          // for mat4
#include <inviwo/core/util/glmutils.h>                                        // for Matrix
#include <inviwo/core/util/glmvec.h>                                          // for ivec2, vec4
#include <modules/base/properties/transformlistproperty.h>                    // for TransformLi...
#include <modules/basegl/datastructures/meshshadercache.h>                    // for MeshShaderC...
#include <modules/meshrenderinggl/datastructures/transformedrasterization.h>  // for Transformed...
#include <modules/meshrenderinggl/ports/rasterizationport.h>                  // for Rasterizati...
#include <modules/meshrenderinggl/rendering/fragmentlistrenderer.h>           // for FragmentLis...
#include <modules/opengl/geometry/meshgl.h>                                   // for MeshGL
#include <modules/opengl/image/layergl.h>                                     // for LayerGL
#include <modules/opengl/inviwoopengl.h>                                      // for GL_DEPTH_TEST
#include <modules/opengl/openglcapabilities.h>                                // for OpenGLCapab...
#include <modules/opengl/openglutils.h>                                       // for BlendModeState
#include <modules/opengl/rendering/meshdrawergl.h>                            // for MeshDrawerGL
#include <modules/opengl/shader/shader.h>                                     // for Shader
#include <modules/opengl/shader/shaderobject.h>                               // for ShaderObject
#include <modules/opengl/shader/shadertype.h>                                 // for ShaderType
#include <modules/opengl/shader/shaderutils.h>                                // for setShaderUn...
#include <modules/opengl/texture/textureunit.h>                               // for TextureUnit

#include <cstddef>        // for size_t
#include <map>            // for __map_iterator
#include <type_traits>    // for remove_exte...
#include <unordered_set>  // for unordered_set
#include <utility>        // for pair

#include <fmt/core.h>      // for format
#include <glm/mat4x4.hpp>  // for operator*
#include <glm/vec2.hpp>    // for vec<>::(ano...
#include <glm/vec4.hpp>    // for operator*

namespace inviwo {
class Rasterization;

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo SphereRasterizer::processorInfo_{
    "org.inviwo.SphereRasterizer",  // Class identifier
    "Sphere Rasterizer",            // Display name
    "Mesh Rendering",               // Category
    CodeState::Experimental,        // Code state
    "GL, Rendering",                // Tags
    R"(Create a rasterization object to render one or more point meshes using
       spherical glyphs in OpenGL. The glyphs are resolution independent and
       consist only of a single point.)"_unindentHelp};

const ProcessorInfo SphereRasterizer::getProcessorInfo() const { return processorInfo_; }

SphereRasterizer::SphereRasterizer()
    : Processor()
    , inport_("inport", R"(
        The input mesh uses the following buffers:
        * PositionAttrib   vec3
        * ColorAttrib      vec4   (optional will fall-back to use __Custom Color__)
        * IndexAttrib      uint   (optional used for labeling and BnL)
        * RadiiAttrib      float  (optional will fall-back to use __Custom Radius__)
        * PickingAttrib    uint32 (optional will fall-back to not draw any picking)
        * ScalarMetaAttrib float  (optional used for custom coloring)
    )"_unindentHelp)
    , outport_("rasterization", "rasterization functor rendering spheres"_help)
    , renderMode_("renderMode", "Render Mode",
                  "render only input meshes marked as points or everything"_help,
                  {{"entireMesh", "Entire Mesh", RenderMode::EntireMesh},
                   {"pointsOnly", "Points Only", RenderMode::PointsOnly}})
    , forceOpaque_("forceOpaque", "Shade Opaque", false, InvalidationLevel::InvalidResources)
    , bnl_{}
    , clip_{}
    , config_{}
    , labels_{}
    , periodic_{}
    , texture_{"sphereTexture", "Texture to apply to spheres"_help}

    , camera_("camera", "Camera", util::boundingBox(inport_))
    , lighting_("lighting", "Lighting", &camera_)
    , transformSetting_("transformSettings", "Additional Transform")

    , shaders_{{{ShaderType::Vertex, std::string{"sphereglyph.vert"}},
                {ShaderType::Geometry, std::string{"sphereglyph.geom"}},
                {ShaderType::Fragment, std::string{"oit-sphereglyph.frag"}}},

               {{BufferType::PositionAttrib, MeshShaderCache::Mandatory, "vec3"},
                {BufferType::ColorAttrib, MeshShaderCache::Optional, "vec4"},
                {BufferType::IndexAttrib, MeshShaderCache::Optional, "uint"},
                {BufferType::RadiiAttrib, MeshShaderCache::Optional, "float"},
                {BufferType::PickingAttrib, MeshShaderCache::Optional, "uint"},
                {BufferType::ScalarMetaAttrib, MeshShaderCache::Optional, "float"},
                bnl_.getRequirement(),
                texture_.getRequirement()},

               [&](Shader& shader) -> void {
                   shader.onReload([this]() { invalidate(InvalidationLevel::InvalidResources); });
                   configureShader(shader);
               }}

    , oitExtensionsAvailable_(
          OpenGLCapabilities::isExtensionSupported("GL_NV_gpu_shader5") &&
          OpenGLCapabilities::isExtensionSupported("GL_EXT_shader_image_load_store") &&
          OpenGLCapabilities::isExtensionSupported("GL_NV_shader_buffer_load") &&
          OpenGLCapabilities::isExtensionSupported("GL_EXT_bindable_uniform")) {

    addPort(inport_);
    addPort(texture_.inport, "Textures").setOptional(true);
    addPort(bnl_.inport);
    addPort(outport_);

    addProperties(renderMode_, forceOpaque_, config_.config, labels_.labels, bnl_.highlight,
                  bnl_.select, bnl_.filter, periodic_.periodicity, transformSetting_,
                  clip_.clipping, camera_, lighting_);

    transformSetting_.setCollapsed(true);
    camera_.setCollapsed(true);
    lighting_.setCollapsed(true);
}

void SphereRasterizer::initializeResources() {
    labels_.initializeResources();

    for (auto& [state, shader] : shaders_.getShaders()) {
        configureShader(shader);
    }
}

void SphereRasterizer::configureShader(Shader& shader) {
    utilgl::addDefines(shader, lighting_, labels_, periodic_, config_, clip_);

    auto* fso = shader.getFragmentShaderObject();
    fso->addShaderExtension("GL_NV_gpu_shader5", oitExtensionsAvailable_);
    fso->addShaderExtension("GL_EXT_shader_image_load_store", oitExtensionsAvailable_);
    fso->addShaderExtension("GL_NV_shader_buffer_load", oitExtensionsAvailable_);
    fso->addShaderExtension("GL_EXT_bindable_uniform", oitExtensionsAvailable_);
    fso->setShaderDefine("USE_FRAGMENT_LIST", usesFragmentLists());

    shader.build();
}

void SphereRasterizer::configureOITShader(Shader& shader) {}

bool SphereRasterizer::usesFragmentLists() const {
    return !forceOpaque_.get() && FragmentListRenderer::supportsFragmentLists() &&
           oitExtensionsAvailable_;
}

void SphereRasterizer::process() {
    auto rasterization = std::make_shared<SphereRasterization>(
        std::dynamic_pointer_cast<SphereRasterizer>(shared_from_this()));

    bnl_.update();

    if (transformSetting_.transforms_.empty()) {
        outport_.setData(rasterization);
    } else {
        outport_.setData(std::make_shared<TransformedRasterization>(rasterization,
                                                                    transformSetting_.getMatrix()));
    }
}

void SphereRasterizer::rasterize(const ivec2& imageSize, const mat4& worldMatrixTransform,
                                 std::function<void(Shader&)> setUniforms) {

    utilgl::BlendModeState blendModeStateGL(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    utilgl::GlBoolState depthTest(GL_DEPTH_TEST, !usesFragmentLists());

    TextureUnitContainer cont;
    utilgl::bind(cont, bnl_, labels_, config_, texture_);

    for (auto mesh : inport_) {
        auto& shader = shaders_.getShader(*mesh);
        shader.activate();

        utilgl::setUniforms(shader, lighting_, config_, clip_, bnl_, periodic_, labels_, texture_);
        shader.setUniform("viewport", vec4(0.0f, 0.0f, 2.0f / imageSize.x, 2.0f / imageSize.y));
        setUniforms(shader);

        auto transform = CompositeTransform(mesh->getModelMatrix(),
                                            mesh->getWorldMatrix() * worldMatrixTransform);
        utilgl::setShaderUniforms(shader, transform, "geometry");

        MeshDrawerGL::DrawObject drawer(*mesh);
        switch (renderMode_) {
            case RenderMode::PointsOnly: {
                drawer.drawOnlyInstanced(MeshDrawerGL::DrawMode::Points, periodic_.instances());
                break;
            }
            case RenderMode::EntireMesh: {
                drawer.drawInstanced(MeshDrawerGL::DrawMode::Points, periodic_.instances());
                break;
            }
        }

        shader.deactivate();
    }
}

SphereRasterization::SphereRasterization(std::shared_ptr<SphereRasterizer> processor)
    : Rasterization{}, processor_{processor} {}

void SphereRasterization::rasterize(const ivec2& imageSize, const mat4& worldMatrixTransform,
                                    std::function<void(Shader&)> setUniforms) const {
    processor_->rasterize(imageSize, worldMatrixTransform, setUniforms);
}

bool SphereRasterization::usesFragmentLists() const { return processor_->usesFragmentLists(); }

Document SphereRasterization::getInfo() const {
    Document doc;
    const auto size = processor_->inport_.getVectorData().size();
    doc.append("p", fmt::format("Sphere mesh rasterization functor with {} mesh{}. {}.", size,
                                (size == 1) ? "" : "es",
                                usesFragmentLists() ? "Using A-buffer" : "Rendering opaque"));
    return doc;
}

}  // namespace inviwo
