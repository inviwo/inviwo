/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2021-2024 Inviwo Foundation
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

#include <modules/oit/processors/sphererasterizer.h>

#include <inviwo/core/algorithm/boundingbox.h>                          // for boundingBox
#include <inviwo/core/datastructures/geometry/geometrytype.h>           // for BufferType
#include <inviwo/core/datastructures/geometry/mesh.h>                   // for Mesh::MeshInfo
#include <inviwo/core/datastructures/image/layer.h>                     // for Layer
#include <inviwo/core/datastructures/representationconverter.h>         // for Representat...
#include <inviwo/core/datastructures/representationconverterfactory.h>  // for Representat...
#include <inviwo/core/datastructures/transferfunction.h>                // for TransferFun...
#include <inviwo/core/interaction/cameratrackball.h>                    // for CameraTrack...
#include <inviwo/core/ports/meshport.h>                                 // for MeshFlatMul...
#include <inviwo/core/properties/simplelightingproperty.h>              // for SimpleLight...
#include <inviwo/core/util/document.h>                                  // for Document
#include <inviwo/core/util/glmmat.h>                                    // for mat4
#include <inviwo/core/util/glmutils.h>                                  // for Matrix
#include <inviwo/core/util/glmvec.h>                                    // for ivec2, vec4
#include <modules/base/properties/transformlistproperty.h>              // for TransformLi...
#include <modules/basegl/datastructures/meshshadercache.h>              // for MeshShaderC...
#include <modules/oit/datastructures/transformedrasterization.h>        // for Transformed...
#include <modules/oit/ports/rasterizationport.h>                        // for Rasterizati...
#include <modules/oit/rendering/fragmentlistrenderer.h>                 // for FragmentLis...
#include <modules/opengl/geometry/meshgl.h>                             // for MeshGL
#include <modules/opengl/image/layergl.h>                               // for LayerGL
#include <modules/opengl/inviwoopengl.h>                                // for GL_DEPTH_TEST
#include <modules/opengl/openglcapabilities.h>                          // for OpenGLCapab...
#include <modules/opengl/openglutils.h>                                 // for BlendModeState
#include <modules/opengl/rendering/meshdrawergl.h>                      // for MeshDrawerGL
#include <modules/opengl/shader/shader.h>                               // for Shader
#include <modules/opengl/shader/shaderobject.h>                         // for ShaderObject
#include <modules/opengl/shader/shadertype.h>                           // for ShaderType
#include <modules/opengl/shader/shaderutils.h>                          // for setShaderUn...
#include <modules/opengl/texture/textureunit.h>                         // for TextureUnit

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
    : Rasterizer()
    , inport_("inport", R"(
        The input mesh uses the following buffers:
        * PositionAttrib   vec3
        * ColorAttrib      vec4   (optional will fall-back to use __Custom Color__)
        * IndexAttrib      uint   (optional used for labeling and BnL)
        * RadiiAttrib      float  (optional will fall-back to use __Custom Radius__)
        * PickingAttrib    uint32 (optional will fall-back to not draw any picking)
        * ScalarMetaAttrib float  (optional used for custom coloring)
    )"_unindentHelp)
    , renderMode_("renderMode", "Render Mode",
                  "render only input meshes marked as points or everything"_help,
                  {{"entireMesh", "Entire Mesh", RenderMode::EntireMesh},
                   {"pointsOnly", "Points Only", RenderMode::PointsOnly}})
    , forceOpaque_(
          "forceOpaque", "Shade Opaque",
          "Draw the mesh opaquely instead of transparent. Disables all transparency settings"_help,
          false, InvalidationLevel::InvalidResources)
    , bnl_{}
    , clip_{}
    , config_{}
    , labels_{}
    , periodic_{}
    , texture_{"sphereTexture", "Texture to apply to spheres"_help}

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
               }} {

    addPort(inport_);
    addPort(texture_.inport, "Textures").setOptional(true);
    addPort(bnl_.inport);

    addProperties(renderMode_, forceOpaque_, config_.config, labels_.labels, bnl_.highlight,
                  bnl_.select, bnl_.filter, periodic_.periodicity, clip_.clipping);
}

void SphereRasterizer::initializeResources() {
    labels_.initializeResources();

    for (auto& [state, shader] : shaders_.getShaders()) {
        configureShader(shader);
    }
}

void SphereRasterizer::configureShader(Shader& shader) {
    Rasterizer::configureShader(shader);
    utilgl::addDefines(shader, labels_, periodic_, config_, clip_);
    shader.build();
}

void SphereRasterizer::setUniforms(Shader& shader) {
    Rasterizer::setUniforms(shader);
    utilgl::setUniforms(shader, config_, clip_, bnl_, periodic_, labels_, texture_);
}

UseFragmentList SphereRasterizer::usesFragmentLists() const {
    return !forceOpaque_.get() && FragmentListRenderer::supportsFragmentLists()
               ? UseFragmentList::Yes
               : UseFragmentList::No;
}

void SphereRasterizer::rasterize(const ivec2& imageSize, const mat4& worldMatrixTransform) {

    bnl_.update();

    utilgl::BlendModeState blendModeStateGL(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    utilgl::GlBoolState depthTest(GL_DEPTH_TEST, usesFragmentLists() == UseFragmentList::No);

    TextureUnitContainer cont;
    utilgl::bind(cont, bnl_, labels_, config_, texture_);

    for (auto mesh : inport_) {
        auto& shader = shaders_.getShader(*mesh);
        utilgl::Activate activate{&shader};

        setUniforms(shader);
        shader.setUniform("viewport", vec4(0.0f, 0.0f, 2.0f / imageSize.x, 2.0f / imageSize.y));

        const auto transform = CompositeTransform(mesh->getModelMatrix(),
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
    }
}

Document SphereRasterizer::getInfo() const {
    Document doc;
    const auto size = inport_.getVectorData().size();
    doc.append("p", fmt::format("Sphere mesh rasterization functor with {} mesh{}. {}.", size,
                                (size == 1) ? "" : "es",
                                usesFragmentLists() == UseFragmentList::Yes ? "Using A-buffer"
                                                                            : "Rendering opaque"));
    return doc;
}

std::optional<mat4> SphereRasterizer::boundingBox() const { return util::boundingBox(inport_)(); }

}  // namespace inviwo
