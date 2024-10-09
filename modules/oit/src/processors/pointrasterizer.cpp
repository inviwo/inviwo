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

#include <modules/oit/processors/pointrasterizer.h>

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
const ProcessorInfo PointRasterizer::processorInfo_{
    "org.inviwo.PointRasterizer",  // Class identifier
    "Point Rasterizer",            // Display name
    "Mesh Rendering",              // Category
    CodeState::Experimental,       // Code state
    "GL, Rendering",               // Tags
    R"(Create a rasterization object to render one or more point meshes
       in OpenGL.)"_unindentHelp};

const ProcessorInfo PointRasterizer::getProcessorInfo() const { return processorInfo_; }

PointRasterizer::PointRasterizer()
    : Rasterizer()
    , inport_("inport", R"(
        The input mesh uses the following buffers:
        * PositionAttrib   vec3
        * ColorAttrib      vec4   (optional will fall-back to use __Custom Color__)
        * PickingAttrib    uint32 (optional will fall-back to not draw any picking)
    )"_unindentHelp)
    , forceOpaque_(
          "forceOpaque", "Shade Opaque",
          "Draw the mesh opaquely instead of transparent. Disables all transparency settings"_help,
          false, InvalidationLevel::InvalidResources)
    , pointSize_("pointSize", "Point Size (pixel)", 1.0f, 0.00001f, 50.0f, 0.1f)
    , borderWidth_("borderWidth", "Border Width (pixel)", 2.0f, 0.0f, 50.0f, 0.1f)
    , borderColor_("borderColor", "Border Color", vec4(0.0f, 0.0f, 0.0f, 1.0f), vec4(0.0f),
                   vec4(1.0f), vec4(0.01f), InvalidationLevel::InvalidOutput,
                   PropertySemantics::Color)
    , antialising_("antialising", "Antialising (pixel)", 1.5f, 0.0f, 10.0f, 0.1f)
    , shader_{"pointrenderer.vert", "oit-pointrenderer.frag", Shader::Build::No} {

    addPort(inport_);

    addProperties(forceOpaque_, pointSize_, borderWidth_, borderColor_, antialising_);

    inport_.onChange([this]() { invalidate(InvalidationLevel::InvalidResources); });
}

void PointRasterizer::initializeResources() { configureShader(shader_); }

void PointRasterizer::configureShader(Shader& shader) {
    Rasterizer::configureShader(shader);

    auto fso = shader.getFragmentShaderObject();
    fso->setShaderDefine("USE_FRAGMENT_LIST",
                         !forceOpaque_.get() && FragmentListRenderer::supportsFragmentLists());
    shader.build();
}

void PointRasterizer::setUniforms(Shader& shader) {
    Rasterizer::setUniforms(shader);
    utilgl::setUniforms(shader, pointSize_, borderWidth_, borderColor_, antialising_);
}

UseFragmentList PointRasterizer::usesFragmentLists() const {
    return !forceOpaque_.get() && FragmentListRenderer::supportsFragmentLists()
               ? UseFragmentList::Yes
               : UseFragmentList::No;
}

void PointRasterizer::rasterize(const ivec2& imageSize, const mat4& worldMatrixTransform) {
    utilgl::BlendModeState blendModeStateGL(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    utilgl::GlBoolState depthTest(GL_DEPTH_TEST, usesFragmentLists() == UseFragmentList::No);
    utilgl::GlBoolState pointSprite(GL_PROGRAM_POINT_SIZE, true);
    utilgl::PolygonModeState polygon(GL_POINT, 1.0f, pointSize_.get());

    for (auto& elem : inport_.getVectorData()) {
        shader_.activate();
        setUniforms(shader_);

        const auto transform = CompositeTransform(elem->getModelMatrix(),
                                                  elem->getWorldMatrix() * worldMatrixTransform);
        utilgl::setShaderUniforms(shader_, transform, "geometry");

        MeshDrawerGL::DrawObject drawer(elem->getRepresentation<MeshGL>(),
                                        elem->getDefaultMeshInfo());
        drawer.draw(MeshDrawerGL::DrawMode::Points);
    }
}

Document PointRasterizer::getInfo() const {
    Document doc;
    const auto size = inport_.getVectorData().size();
    doc.append("p", fmt::format("Point mesh rasterization functor with {} mesh{}. {}.", size,
                                (size == 1) ? "" : "es",
                                usesFragmentLists() == UseFragmentList::Yes ? "Using A-buffer"
                                                                            : "Rendering opaque"));
    return doc;
}

std::optional<mat4> PointRasterizer::boundingBox() const { return util::boundingBox(inport_)(); }

}  // namespace inviwo
