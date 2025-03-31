/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2025 Inviwo Foundation
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

#include <modules/basegl/processors/pointrenderer.h>

#include <inviwo/core/algorithm/boundingbox.h>         // for boundingBox
#include <inviwo/core/datastructures/geometry/mesh.h>  // for Mesh
#include <inviwo/core/ports/imageport.h>               // for BaseImageInport, ImageInport, Imag...
#include <inviwo/core/ports/meshport.h>                // for MeshFlatMultiInport
#include <inviwo/core/processors/processor.h>          // for Processor
#include <inviwo/core/processors/processorinfo.h>      // for ProcessorInfo
#include <inviwo/core/processors/processorstate.h>     // for CodeState, CodeState::Stable
#include <inviwo/core/processors/processortags.h>      // for Tags, Tags::GL
#include <inviwo/core/properties/cameraproperty.h>     // for CameraProperty
#include <inviwo/core/properties/invalidationlevel.h>  // for InvalidationLevel, InvalidationLev...
#include <inviwo/core/properties/ordinalproperty.h>    // for FloatProperty
#include <inviwo/core/properties/propertysemantics.h>  // for PropertySemantics, PropertySemanti...
#include <inviwo/core/util/glmvec.h>                   // for vec4
#include <modules/opengl/geometry/meshgl.h>            // for MeshGL
#include <modules/opengl/inviwoopengl.h>               // for GL_ONE_MINUS_SRC_ALPHA, GL_POINT
#include <modules/opengl/openglutils.h>                // for BlendModeState, GlBoolState, Polyg...
#include <modules/opengl/rendering/meshdrawergl.h>     // for MeshDrawerGL, MeshDrawerGL::DrawOb...
#include <modules/opengl/shader/shader.h>              // for Shader
#include <modules/opengl/shader/shaderutils.h>         // for setShaderUniforms, setUniforms
#include <modules/opengl/texture/textureutils.h>       // for activateTargetAndClearOrCopySource
#include <modules/opengl/texture/textureunit.h>

#include <functional>   // for __base
#include <memory>       // for shared_ptr, shared_ptr<>::element_...
#include <string>       // for string
#include <string_view>  // for string_view
#include <type_traits>  // for remove_extent_t
#include <vector>       // for vector

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo PointRenderer::processorInfo_{
    "org.inviwo.PointRenderer",  // Class identifier
    "Point Renderer",            // Display name
    "Mesh Rendering",            // Category
    CodeState::Stable,           // Code state
    Tags::GL | Tag{"Plotting"},  // Tags
    R"(This processor renders a set of meshes as 2D points using OpenGL.)"_unindentHelp,
};
const ProcessorInfo& PointRenderer::getProcessorInfo() const { return processorInfo_; }

PointRenderer::PointRenderer()
    : Processor{}
    , inport_{"geometry"}
    , imageInport_{"imageInport"}
    , outport_{"image"}
    , bnl_{}

    , renderMode_{"renderMode",
                  "Render Mode",
                  "render only input meshes marked as points or everything"_help,
                  {{"entireMesh", "Entire Mesh", RenderMode::EntireMesh},
                   {"pointsOnly", "Points Only", RenderMode::PointsOnly}}}
    , depthTest_{"depthTest", "Enable Depth Test", "Toggles the depth test during rendering"_help,
                 true}

    , config_{"pointProperties", "Point Properties"}
    , borderWidth_{"borderWidth", "Border Width (pixel)", util::ordinalLength(2.0f, 10.0f)}
    , borderColor_{"borderColor", "Border Color", util::ordinalColor(vec4{0.0f, 0.0f, 0.0f, 1.0f})}
    , antialising_{"antialising", "Antialising (pixel)",
                   util::ordinalLength(1.5f, 5.0f)
                       .set("Width of the antialised point edge (in pixel), this determines the "
                            "softness along the outer edge of the point"_help)}

    , labels_{}
    , periodic_{}
    , texture_{"pointTexture", "Texture to apply to points"_help}
    , camera_{"camera", "Camera", util::boundingBox(inport_)}
    , trackball_{&camera_}

    , shaders_{{{ShaderType::Vertex, std::string{"pointrenderer.vert"}},
                {ShaderType::Geometry, std::string{"pointrenderer.geom"}},
                {ShaderType::Fragment, std::string{"pointrenderer.frag"}}},

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
    addPort(labels_.strings);
    addPort(outport_);

    config_.config.addProperties(borderWidth_, borderColor_, antialising_);
    config_.radius.setMaxValue(32);
    config_.radius.set(8);
    config_.radius.setCurrentStateAsDefault();

    addProperties(renderMode_, depthTest_, config_.config, labels_.labels, texture_.texture,
                  bnl_.highlight, bnl_.select, bnl_.filter, periodic_.periodicity, camera_,
                  trackball_);
}

void PointRenderer::initializeResources() {
    for (auto& [state, shader] : shaders_.getShaders()) {
        configureShader(shader);
    }
}

void PointRenderer::configureShader(Shader& shader) {
    utilgl::addDefines(shader, labels_, periodic_, config_, texture_);
    shader.build();
}

void PointRenderer::process() {
    const utilgl::BlendModeState blendModeStateGL(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    const utilgl::activateTargetAndClearOrCopySource(outport_, imageInport_);

    bnl_.update();
    labels_.update();

    TextureUnitContainer cont;
    utilgl::bind(cont, bnl_, labels_, config_, texture_);

    const utilgl::GlBoolState pointSprite(GL_PROGRAM_POINT_SIZE, true);
    const utilgl::GlBoolState depthTest(GL_DEPTH_TEST, depthTest_);

    for (const auto& mesh : inport_) {
        auto& shader = shaders_.getShader(*mesh);
        shader.activate();

        utilgl::setUniforms(shader, camera_, config_, bnl_, periodic_, labels_, texture_,
                            borderWidth_, borderColor_, antialising_);
        shader.setUniform("viewport", vec4(0.0f, 0.0f, 2.0f / outport_.getDimensions().x,
                                           2.0f / outport_.getDimensions().y));
        utilgl::setShaderUniforms(shader, *mesh, "geometry");

        MeshDrawerGL::DrawObject drawer(*mesh);
        switch (renderMode_.get()) {
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
