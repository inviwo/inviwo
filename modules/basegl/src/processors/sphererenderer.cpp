/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2024 Inviwo Foundation
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

#include <modules/basegl/processors/sphererenderer.h>

#include <inviwo/core/datastructures/geometry/geometrytype.h>  // for BufferType, DrawType
#include <inviwo/core/datastructures/geometry/mesh.h>          // for Mesh::MeshInfo, Mesh
#include <inviwo/core/util/glmvec.h>                           // for size2_t, vec4
#include <inviwo/core/util/staticstring.h>                     // for operator+
#include <modules/opengl/geometry/meshgl.h>                    // for MeshGL
#include <modules/opengl/openglutils.h>                        // for BlendModeState
#include <modules/opengl/rendering/meshdrawergl.h>             // for MeshDrawerGL, Mesh...
#include <modules/opengl/shader/shader.h>                      // for Shader
#include <modules/opengl/shader/shaderobject.h>                // for ShaderObject
#include <modules/opengl/shader/shadertype.h>                  // for ShaderType, Shader...
#include <modules/opengl/shader/shaderutils.h>                 // for addDefines, setSha...
#include <modules/opengl/texture/textureunit.h>                // for TextureUnitContainer
#include <modules/opengl/texture/textureutils.h>               // for activateTargetAndC...

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo SphereRenderer::processorInfo_{
    "org.inviwo.SphereRenderer",  // Class identifier
    "Sphere Renderer",            // Display name
    "Mesh Rendering",             // Category
    CodeState::Stable,            // Code state
    "GL, Brushing, Linking",      // Tags
    R"(This processor renders a set of point meshes using spherical glyphs in OpenGL.
    The glyphs are resolution independent and consist only of a single point.
    )"_unindentHelp};
const ProcessorInfo SphereRenderer::getProcessorInfo() const { return processorInfo_; }

SphereRenderer::SphereRenderer()
    : Processor()
    , inport_{"geometry", R"(
        The input mesh uses the following buffers:
        * *PositionAttrib*   `vec3`
        * *ColorAttrib*      `vec4`   (optional will fall-back to use __Custom Color__)
        * *IndexAttrib*      `uint`   (optional used for labeling and BnL)
        * *RadiiAttrib*      `float`  (optional will fall-back to use __Custom Radius__)
        * *PickingAttrib*    `uint32` (optional will fall-back to not draw any picking)
        * *ScalarMetaAttrib* `float`  (optional used for custom coloring)
    )"_unindentHelp}
    , imageInport_{"imageInport", "Optional background image"_help}
    , outport_{"image",
               "output image containing the rendered spheres and the optional input image"_help}
    , renderMode_{"renderMode",
                  "Render Mode",
                  "render only input meshes marked as points or everything"_help,
                  {{"entireMesh", "Entire Mesh", RenderMode::EntireMesh},
                   {"pointsOnly", "Points Only", RenderMode::PointsOnly}}}
    , bnl_{}
    , clip_{}
    , config_{}
    , labels_{}
    , periodic_{}
    , texture_{"sphereTexture", "Texture to apply to spheres"_help}
    , camera_{"camera", "Camera", periodic_.boundingBox(inport_)}
    , trackball_{&camera_}
    , lighting_{"lighting", "Lighting", &camera_}

    , shaders_{{{ShaderType::Vertex, std::string{"sphereglyph.vert"}},
                {ShaderType::Geometry, std::string{"sphereglyph.geom"}},
                {ShaderType::Fragment, std::string{"sphereglyph.frag"}}},

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
    addPort(imageInport_).setOptional(true);
    addPort(texture_.inport, "Textures").setOptional(true);
    addPort(bnl_.inport);
    addPort(outport_);

    addProperties(renderMode_, config_.config, labels_.labels, bnl_.highlight, bnl_.select,
                  bnl_.filter, periodic_.periodicity, clip_.clipping, camera_, lighting_,
                  trackball_);
}

void SphereRenderer::initializeResources() {
    labels_.initializeResources();

    for (auto& [state, shader] : shaders_.getShaders()) {
        configureShader(shader);
    }
}

void SphereRenderer::configureShader(Shader& shader) {
    utilgl::addDefines(shader, lighting_, labels_, periodic_, config_, clip_);
    shader.build();
}

void SphereRenderer::process() {
    utilgl::BlendModeState blendModeStateGL(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    utilgl::activateTargetAndClearOrCopySource(outport_, imageInport_);

    bnl_.update();

    TextureUnitContainer cont;
    utilgl::bind(cont, bnl_, labels_, config_, texture_);

    for (const auto& mesh : inport_) {
        auto& shader = shaders_.getShader(*mesh);
        shader.activate();

        utilgl::setUniforms(shader, camera_, lighting_, config_, clip_, bnl_, periodic_, labels_,
                            texture_);
        shader.setUniform("viewport", vec4(0.0f, 0.0f, 2.0f / outport_.getDimensions().x,
                                           2.0f / outport_.getDimensions().y));

        utilgl::setShaderUniforms(shader, *mesh, "geometry");

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

    utilgl::deactivateCurrentTarget();
}

}  // namespace inviwo
