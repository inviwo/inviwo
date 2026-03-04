/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2026 Inviwo Foundation
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

#include <modules/oit/processors/linerasterizer.h>

#include <inviwo/core/algorithm/boundingbox.h>
#include <inviwo/core/datastructures/geometry/geometrytype.h>
#include <inviwo/core/datastructures/geometry/mesh.h>
#include <inviwo/core/ports/meshport.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/processors/processorinfo.h>
#include <inviwo/core/processors/processorstate.h>
#include <inviwo/core/processors/processortags.h>
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/properties/invalidationlevel.h>
#include <inviwo/core/properties/listproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/propertysemantics.h>
#include <inviwo/core/util/document.h>
#include <inviwo/core/util/glmmat.h>
#include <inviwo/core/util/glmutils.h>
#include <inviwo/core/util/glmvec.h>
#include <inviwo/core/util/stringconversion.h>
#include <modules/base/properties/transformlistproperty.h>
#include <modules/basegl/datastructures/stipplingdata.h>
#include <modules/basegl/properties/linesettingsproperty.h>
#include <modules/basegl/properties/stipplingproperty.h>
#include <modules/oit/datastructures/transformedrasterization.h>
#include <modules/oit/ports/rasterizationport.h>
#include <modules/oit/rendering/fragmentlistrenderer.h>
#include <modules/opengl/geometry/meshgl.h>
#include <modules/opengl/inviwoopengl.h>
#include <modules/opengl/openglutils.h>
#include <modules/opengl/rendering/meshdrawergl.h>
#include <modules/opengl/shader/shader.h>
#include <modules/opengl/shader/shaderobject.h>
#include <modules/opengl/shader/shadertype.h>
#include <modules/opengl/shader/shaderutils.h>

#include <cstddef>
#include <map>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

#include <fmt/core.h>
#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>

namespace inviwo {
class Rasterization;

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo LineRasterizer::processorInfo_{
    "org.inviwo.LineRasterizer",  // Class identifier
    "Line Rasterizer",            // Display name
    "Mesh Rendering",             // Category
    CodeState::Stable,            // Code state
    Tags::GL,                     // Tags
    "Render input meshes as lines, allows for order-independent transparency."_help,
};
const ProcessorInfo& LineRasterizer::getProcessorInfo() const { return processorInfo_; }

LineRasterizer::LineRasterizer()
    : Rasterizer()
    , inport_("geometry", "Input meshes"_help)
    , lineSettings_("lineSettings", "Line Settings")
    , forceOpaque_("forceOpaque", "Shade Opaque",
                   "use simple depth checks instead of fragment lists"_help, false,
                   InvalidationLevel::InvalidResources)
    , overwriteColor_("overwriteColor", "Overwrite Color", false,
                      InvalidationLevel::InvalidResources)
    , constantColor_{"constantColor",
                     "Constant Color",
                     vec4(1, 0.565f, 0.004f, 1),
                     vec4(0.0f),
                     vec4(1.0f),
                     vec4(0.01f),
                     InvalidationLevel::InvalidOutput,
                     PropertySemantics::Color}
    , useUniformAlpha_("useUniformAlpha", "Uniform Alpha", false,
                       InvalidationLevel::InvalidResources)
    , uniformAlpha_("alphaValue", "Alpha", 0.7f, 0, 1, 0.1f, InvalidationLevel::InvalidOutput)
    , lineShaders_{
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
            [](int mode, Shader& shader) {
                shader[ShaderType::Geometry]->addShaderDefine("ENABLE_ADJACENCY",
                                                              fmt::to_string(mode));
            }}},
          [this](Shader& shader) -> void {
              shader.onReload([this]() { invalidate(InvalidationLevel::InvalidResources); });
              configureShader(shader);
          }} {

    addPort(inport_);

    addProperties(forceOpaque_, lineSettings_, overwriteColor_, constantColor_, useUniformAlpha_,
                  uniformAlpha_);

    constantColor_.setVisible(overwriteColor_.get());
    overwriteColor_.onChange([this]() {
        constantColor_.setVisible(overwriteColor_.get());
        uniformAlpha_.setVisible(false);
    });
    uniformAlpha_.setVisible(useUniformAlpha_.get());
    useUniformAlpha_.onChange([this]() {
        uniformAlpha_.setVisible(useUniformAlpha_.get());
        constantColor_.setVisible(false);
    });
    inport_.onChange([this]() { invalidate(InvalidationLevel::InvalidResources); });

    invalidate(InvalidationLevel::InvalidResources);
}

void LineRasterizer::initializeResources() {
    for (auto&& [state, shader] : lineShaders_.getShaders()) {
        configureShader(shader);
    }
}

void LineRasterizer::configureShader(Shader& shader) {
    Rasterizer::configureShader(shader);
    auto fso = shader.getFragmentShaderObject();

    fso->setShaderDefine("USE_FRAGMENT_LIST",
                         !forceOpaque_.get() && FragmentListRenderer::supportsFragmentLists());

    fso->setShaderDefine("ENABLE_PSEUDO_LIGHTING", lineSettings_.pseudoLighting.get());
    fso->setShaderDefine("ENABLE_ROUND_DEPTH_PROFILE", lineSettings_.roundDepthProfile.get());
    fso->setShaderDefine("UNIFORM_ALPHA", useUniformAlpha_.get());
    fso->setShaderDefine("OVERWRITE_COLOR", overwriteColor_.get());

    utilgl::addShaderDefines(shader, lineSettings_.stippling.mode.getSelectedValue());

    shader.build();
}

void LineRasterizer::setUniforms(Shader& shader) {
    Rasterizer::setUniforms(shader);

    shader.setUniform("lineWidth", lineSettings_.lineWidth);
    shader.setUniform("antialiasing", lineSettings_.antialiasing);
    shader.setUniform("miterLimit", lineSettings_.miterLimit);
    shader.setUniform("roundCaps", lineSettings_.roundCaps);
    // Stippling settings
    shader.setUniform("stippling.length", lineSettings_.stippling.length);
    shader.setUniform("stippling.spacing", lineSettings_.stippling.spacing);
    shader.setUniform("stippling.offset", lineSettings_.stippling.offset);
    shader.setUniform("stippling.worldScale", lineSettings_.stippling.worldScale);
    // Alpha
    if (useUniformAlpha_.get()) shader.setUniform("uniformAlpha", uniformAlpha_.get());
    if (overwriteColor_.get()) shader.setUniform("overwriteColor", constantColor_.get());
}

void LineRasterizer::rasterize(const ivec2& imageSize, const mat4& worldMatrixTransform) {

    utilgl::BlendModeState blending(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    utilgl::DepthMaskState depthMask(true);
    utilgl::DepthFuncState depthFunc(GL_LEQUAL);

    auto setup = [&](Shader& shader, const auto& transform) {
        setUniforms(shader);
        utilgl::setShaderUniforms(shader, transform, "geometry");
        shader.setUniform("screenDim", vec2(imageSize));
    };

    for (auto mesh : inport_) {
        if (mesh->getNumberOfBuffers() == 0) return;

        MeshDrawerGL::DrawObject drawer(*mesh);
        auto transform = CompositeTransform{mesh->getModelMatrix(),
                                            mesh->getWorldMatrix() * worldMatrixTransform};

        if (mesh->getNumberOfIndices() > 0) {
            for (size_t i = 0; i < mesh->getNumberOfIndices(); ++i) {
                if (mesh->getIndexMeshInfo(i).dt != DrawType::Lines) continue;

                auto& shader = lineShaders_.getShader(*mesh, mesh->getIndexMeshInfo(i));
                utilgl::Activate activate{&shader};
                setup(shader, transform);
                drawer.draw(i);
            }
        } else if (mesh->getDefaultMeshInfo().dt == DrawType::Lines) {
            auto& shader = lineShaders_.getShader(*mesh);
            utilgl::Activate activate{&shader};
            setup(shader, transform);
            drawer.draw();
        }
    }
}

std::optional<mat4> LineRasterizer::boundingBox() const { return util::boundingBox(inport_)(); }

UseFragmentList LineRasterizer::usesFragmentLists() const {
    return !forceOpaque_ && FragmentListRenderer::supportsFragmentLists() ? UseFragmentList::Yes
                                                                          : UseFragmentList::No;
}

Document LineRasterizer::getInfo() const {
    Document doc;
    const auto size = inport_.getVectorData().size();
    doc.append("p", fmt::format("Line rasterization functor with {} line {}. {}.", size,
                                (size == 1) ? " mesh" : " meshes",
                                usesFragmentLists() == UseFragmentList::Yes ? "Using A-buffer"
                                                                            : "Rendering opaque"));
    return doc;
}

}  // namespace inviwo
