/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2024 Inviwo Foundation
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

#include <modules/basegl/rendering/linerenderer.h>

#include <inviwo/core/datastructures/geometry/geometrytype.h>          // for BufferType, Connec...
#include <inviwo/core/datastructures/geometry/mesh.h>                  // for Mesh, Mesh::MeshInfo
#include <inviwo/core/util/glmvec.h>                                   // for size2_t, vec2
#include <inviwo/core/util/stringconversion.h>                         // for toString
#include <modules/basegl/datastructures/linesettings.h>                // for LineSettings
#include <modules/basegl/datastructures/linesettingsinterface.h>       // for LineSettingsInterface
#include <modules/basegl/datastructures/meshshadercache.h>             // for MeshShaderCache::R...
#include <modules/basegl/datastructures/stipplingsettingsinterface.h>  // for StipplingSettingsI...
#include <modules/basegl/properties/stipplingproperty.h>               // for addShaderDefines
#include <modules/opengl/geometry/meshgl.h>                            // for MeshGL
#include <modules/opengl/inviwoopengl.h>                               // for GL_ONE, GL_ONE_MIN...
#include <modules/opengl/openglutils.h>                                // for BlendModeState
#include <modules/opengl/rendering/meshdrawergl.h>                     // for MeshDrawerGL::Draw...
#include <modules/opengl/shader/shader.h>                              // for Shader
#include <modules/opengl/shader/shaderobject.h>                        // for ShaderObject
#include <modules/opengl/shader/shadertype.h>                          // for ShaderType, Shader...
#include <modules/opengl/shader/shaderutils.h>                         // for setShaderUniforms
#include <modules/opengl/texture/textureunit.h>
#include <modules/opengl/texture/textureutils.h>
#include <modules/opengl/image/layergl.h>

#include <cstddef>     // for size_t
#include <functional>  // for __base, function
#include <map>         // for __map_iterator, map
#include <string>      // for string
#include <utility>     // for pair
#include <vector>      // for vector

namespace inviwo {

namespace algorithm {

namespace detail {

std::vector<MeshShaderCache::Requirement> defaultRequirements(
    const std::vector<MeshShaderCache::Requirement>& additional = {}) {

    std::vector<MeshShaderCache::Requirement> requirements = {
        {BufferType::PositionAttrib, MeshShaderCache::Mandatory, "vec3"},
        {BufferType::ColorAttrib, MeshShaderCache::Optional, "vec4"},
        {BufferType::IndexAttrib, MeshShaderCache::Optional, "uint"},
        {BufferType::PickingAttrib, MeshShaderCache::Optional, "uint"},
        {BufferType::ScalarMetaAttrib, MeshShaderCache::Optional, "float"},
        {[](const Mesh&, Mesh::MeshInfo mi) -> int {
             return mi.ct == ConnectivityType::Adjacency ||
                            mi.ct == ConnectivityType::StripAdjacency
                        ? 1
                        : 0;
         },
         [](int mode, Shader& shader) {
             shader[ShaderType::Geometry]->addShaderDefine("ENABLE_ADJACENCY", toString(mode));
         }}};

    requirements.insert(requirements.end(), additional.begin(), additional.end());
    return requirements;
}

}  // namespace detail

LineRenderer::LineRenderer(const LineSettingsInterface* settings) : LineRenderer({}, settings) {}

LineRenderer::LineRenderer(const std::vector<MeshShaderCache::Requirement>& requirements,
                           const LineSettingsInterface* settings)
    : settings_(settings)
    , lineShaders_{{{ShaderType::Vertex, std::string{"linerenderer.vert"}},
                    {ShaderType::Geometry, std::string{"linerenderer.geom"}},
                    {ShaderType::Fragment, std::string{"linerenderer.frag"}}},

                   detail::defaultRequirements(requirements),
                   [this](Shader& shader) -> void { configureShader(shader); }}
    , tfLookup_{settings_.metaColor} {}

void LineRenderer::render(const Mesh& mesh, const Camera& camera, size2_t screenDim,
                          const LineSettingsInterface* settings) {
    render(mesh, camera, screenDim, settings, [](Shader&) {});
}

void LineRenderer::render(const Mesh& mesh, const Camera& camera, size2_t screenDim,
                          const LineSettingsInterface* settings,
                          const std::function<void(Shader&)>& func) {
    if (mesh.getNumberOfBuffers() == 0) return;
    // Changing these settings require recompilation
    if (settings_.getPseudoLighting() != settings->getPseudoLighting() ||
        settings_.getRoundDepthProfile() != settings->getRoundDepthProfile() ||
        settings_.getOverrideColor() != settings->getOverrideColor() ||
        settings_.getOverrideAlpha() != settings->getOverrideAlpha() ||
        settings_.getUseMetaColor() != settings->getUseMetaColor() ||
        settings_.getStippling().getMode() != settings->getStippling().getMode()) {
        settings_ = LineSettings(settings);
        configureShaders();
    } else {
        settings_ = LineSettings(settings);
    }

    utilgl::BlendModeState blending(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    TextureUnit texUnit;
    if (settings_.getUseMetaColor()) {
        if (const auto* tfLayer = tfLookup_.getRepresentation<LayerGL>()) {
            tfLayer->bindTexture(texUnit.getEnum());
        }
    }

    MeshDrawerGL::DrawObject drawer(mesh.getRepresentation<MeshGL>(), mesh.getDefaultMeshInfo());
    if (mesh.getNumberOfIndicies() > 0) {
        for (size_t i = 0; i < mesh.getNumberOfIndicies(); ++i) {
            if (mesh.getIndexMeshInfo(i).dt != DrawType::Lines) continue;
            auto& shader = lineShaders_.getShader(mesh, mesh.getIndexMeshInfo(i));
            shader.activate();
            shader.setUniform("metaColor", texUnit.getUnitNumber());
            setUniforms(shader, mesh, camera, screenDim, func);
            drawer.draw(i);
            shader.deactivate();
        }
    } else {
        auto& shader = lineShaders_.getShader(mesh);
        if (mesh.getDefaultMeshInfo().dt != DrawType::Lines) return;

        shader.activate();
        shader.setUniform("metaColor", texUnit.getUnitNumber());
        setUniforms(shader, mesh, camera, screenDim, func);

        drawer.draw();
        shader.deactivate();
    }
}

void LineRenderer::setUniforms(Shader& lineShader, const Mesh& mesh, const Camera& camera,
                               size2_t screenDim, const std::function<void(Shader&)>& func) {
    lineShader.setUniform("screenDim", vec2(screenDim));
    utilgl::setShaderUniforms(lineShader, camera, "camera");

    lineShader.setUniform("lineWidth", settings_.getWidth());
    lineShader.setUniform("antialiasing", settings_.getAntialiasingWidth());
    lineShader.setUniform("miterLimit", settings_.getMiterLimit());
    lineShader.setUniform("roundCaps", settings_.getRoundCaps());
    lineShader.setUniform("defaultColor", settings_.getDefaultColor());
    lineShader.setUniform("config.color", settings_.getOverrideColorValue());
    lineShader.setUniform("config.alpha", settings_.getOverrideAlphaValue());
    // Stippling settings
    lineShader.setUniform("stippling.length", settings_.getStippling().getLength());
    lineShader.setUniform("stippling.spacing", settings_.getStippling().getSpacing());
    lineShader.setUniform("stippling.offset", settings_.getStippling().getOffset());
    lineShader.setUniform("stippling.worldScale", settings_.getStippling().getWorldScale());
    utilgl::setShaderUniforms(lineShader, mesh, "geometry");
    func(lineShader);
}

void LineRenderer::configureShaders() {
    for (auto& item : lineShaders_.getShaders()) {
        configureShader(item.second);
    }
}
void LineRenderer::configureShader(Shader& shader) {
    shader[ShaderType::Fragment]->setShaderDefine("ENABLE_PSEUDO_LIGHTING",
                                                  settings_.getPseudoLighting());
    shader[ShaderType::Fragment]->setShaderDefine("ENABLE_ROUND_DEPTH_PROFILE",
                                                  settings_.getRoundDepthProfile());
    shader[ShaderType::Vertex]->setShaderDefine("OVERRIDE_COLOR", settings_.getOverrideColor());
    shader[ShaderType::Vertex]->setShaderDefine("OVERRIDE_ALPHA", settings_.getOverrideAlpha());
    shader[ShaderType::Vertex]->setShaderDefine("USE_SCALARMETACOLOR", settings_.getUseMetaColor());

    utilgl::addShaderDefines(shader, settings_.getStippling().getMode());
    shader.build();
}

}  // namespace algorithm

}  // namespace inviwo
