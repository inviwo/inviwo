/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2019 Inviwo Foundation
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
#include <modules/opengl/rendering/meshdrawergl.h>
#include <modules/opengl/shader/shaderutils.h>
#include <modules/opengl/openglutils.h>

#include <inviwo/core/util/zip.h>
#include <inviwo/core/util/stringconversion.h>
#include <inviwo/core/algorithm/boundingbox.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo SphereRenderer::processorInfo_{
    "org.inviwo.SphereRenderer",  // Class identifier
    "Sphere Renderer",            // Display name
    "Mesh Rendering",             // Category
    CodeState::Stable,            // Code state
    "GL, Brushing, Linking",      // Tags
};
const ProcessorInfo SphereRenderer::getProcessorInfo() const { return processorInfo_; }

SphereRenderer::SphereRenderer()
    : Processor()
    , inport_("geometry")
    , imageInport_("imageInport")
    , brushLinkPort_("brushingAndLinking")
    , outport_("image")
    , renderMode_("renderMode", "Render Mode",
                  {{"entireMesh", "Entire Mesh", RenderMode::EntireMesh},
                   {"pointsOnly", "Points Only", RenderMode::PointsOnly}})
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
    , defaultColor_("defaultColor", "Default Color", vec4(0.7f, 0.7f, 0.7f, 1.0f), vec4(0.0f),
                    vec4(1.0f), vec4(0.01f), InvalidationLevel::InvalidOutput,
                    PropertySemantics::Color)
    , useMetaColor_("useMetaColor", "Use meta color mapping", false,
                    InvalidationLevel::InvalidResources)
    , metaColor_("metaColor", "Meta Color Mapping")
    , selection_(inport_, brushLinkPort_)

    , camera_("camera", "Camera", util::boundingBox(inport_))
    , trackball_(&camera_)
    , lighting_("lighting", "Lighting", &camera_)
    , shaders_{{{ShaderType::Vertex, std::string{"sphereglyph.vert"}},
                {ShaderType::Geometry, std::string{"sphereglyph.geom"}},
                {ShaderType::Fragment, std::string{"sphereglyph.frag"}}},

               {{BufferType::PositionAttrib, MeshShaderCache::Mandatory, "vec3"},
                {BufferType::ColorAttrib, MeshShaderCache::Optional, "vec4"},
                {BufferType::RadiiAttrib, MeshShaderCache::Optional, "float"},
                {BufferType::PickingAttrib, MeshShaderCache::Optional, "uint"},
                {BufferType::ScalarMetaAttrib, MeshShaderCache::Optional, "float"}},

               [&](Shader& shader) -> void {
                   shader.onReload([this]() { invalidate(InvalidationLevel::InvalidResources); });
                   configureShader(shader);
               }} {

    addPort(inport_);
    addPort(imageInport_).setOptional(true);
    addPort(brushLinkPort_);
    addPort(outport_);

    clipping_.addProperties(clipMode_, clipShadingFactor_, shadeClippedArea_);

    sphereProperties_.addProperties(forceRadius_, defaultRadius_, forceColor_, defaultColor_,
                                    useMetaColor_, metaColor_);

    addProperties(renderMode_, sphereProperties_, selection_.properties, clipping_, camera_,
                  lighting_, trackball_);

    clipMode_.onChange([&]() {
        clipShadingFactor_.setReadOnly(clipMode_.get() == GlyphClippingMode::Discard);
        shadeClippedArea_.setReadOnly(clipMode_.get() == GlyphClippingMode::Discard);
    });
}

void SphereRenderer::initializeResources() {
    for (auto& item : shaders_.getShaders()) {
        configureShader(item.second);
    }
}

void SphereRenderer::configureShader(Shader& shader) {
    utilgl::addDefines(shader, lighting_);
    shader[ShaderType::Vertex]->setShaderDefine("FORCE_RADIUS", forceRadius_);
    shader[ShaderType::Vertex]->setShaderDefine("FORCE_COLOR", forceColor_);
    shader[ShaderType::Vertex]->setShaderDefine("USE_SCALARMETACOLOR", useMetaColor_);
    shader[ShaderType::Fragment]->setShaderDefine("SHADE_CLIPPED_AREA", shadeClippedArea_);
    shader[ShaderType::Fragment]->setShaderDefine("DISCARD_CLIPPED_GLYPHS",
                                                  clipMode_.get() == GlyphClippingMode::Discard);
    shader[ShaderType::Geometry]->setShaderDefine("DISCARD_CLIPPED_GLYPHS",
                                                  clipMode_.get() == GlyphClippingMode::Discard);
    shader.build();
}

void SphereRenderer::process() {
    utilgl::activateTargetAndClearOrCopySource(outport_, imageInport_);
    utilgl::BlendModeState blendModeStateGL(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    for (const auto& [port, mesh] : inport_.getSourceVectorData()) {
        if (mesh->getNumberOfBuffers() == 0) continue;

        auto& shader = shaders_.getShader(*mesh);

        shader.activate();

        TextureUnitContainer units;
        utilgl::bindAndSetUniforms(shader, units, metaColor_);

        utilgl::setUniforms(shader, camera_, lighting_, defaultColor_, defaultRadius_,
                            clipShadingFactor_);
        shader.setUniform("viewport", vec4(0.0f, 0.0f, 2.0f / outport_.getDimensions().x,
                                           2.0f / outport_.getDimensions().y));
        MeshDrawerGL::DrawObject drawer(mesh->getRepresentation<MeshGL>(),
                                        mesh->getDefaultMeshInfo());
        utilgl::setShaderUniforms(shader, *mesh, "geometry");
        switch (renderMode_) {
            case RenderMode::PointsOnly: {
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
            case RenderMode::EntireMesh:
                [[fallthrough]];
            default:  // render all parts of the input meshes as points
                drawer.draw(MeshDrawerGL::DrawMode::Points);
                break;
        }

        // render selection only for first mesh input
        if (auto indices = selection_.getIndices(port, *mesh)) {
            shader.setUniform("selectionMix", 1.0f);
            shader.setUniform("selectionScaleFactor", selection_.radiusFactor);
            shader.setUniform("selectionColor", selection_.color);
            glDrawElements(GL_POINTS, static_cast<GLsizei>(indices->size()), GL_UNSIGNED_INT,
                           indices->data());
            // reset selection flag
            shader.setUniform("selectionMix", 0.0f);
        }

        shader.deactivate();
    }

    utilgl::deactivateCurrentTarget();
}

}  // namespace inviwo
