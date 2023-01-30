/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2022 Inviwo Foundation
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

#include <inviwo/core/algorithm/boundingbox.h>                         // for boundingBox
#include <inviwo/core/datastructures/geometry/geometrytype.h>          // for BufferType, DrawType
#include <inviwo/core/datastructures/geometry/mesh.h>                  // for Mesh::MeshInfo, Mesh
#include <inviwo/core/ports/imageport.h>                               // for ImageOutport, Base...
#include <inviwo/core/ports/meshport.h>                                // for MeshFlatMultiInport
#include <inviwo/core/processors/processor.h>                          // for Processor
#include <inviwo/core/processors/processorinfo.h>                      // for ProcessorInfo
#include <inviwo/core/processors/processorstate.h>                     // for CodeState, CodeSta...
#include <inviwo/core/processors/processortags.h>                      // for Tags
#include <inviwo/core/properties/boolproperty.h>                       // for BoolProperty
#include <inviwo/core/properties/cameraproperty.h>                     // for CameraProperty
#include <inviwo/core/properties/compositeproperty.h>                  // for CompositeProperty
#include <inviwo/core/properties/invalidationlevel.h>                  // for InvalidationLevel
#include <inviwo/core/properties/optionproperty.h>                     // for OptionProperty
#include <inviwo/core/properties/ordinalproperty.h>                    // for FloatProperty, ord...
#include <inviwo/core/util/glmvec.h>                                   // for size2_t, vec4
#include <inviwo/core/util/staticstring.h>                             // for operator+
#include <modules/basegl/datastructures/meshshadercache.h>             // for MeshShaderCache::R...
#include <modules/brushingandlinking/ports/brushingandlinkingports.h>  // for BrushingAndLinking...
#include <modules/opengl/geometry/meshgl.h>                            // for MeshGL
#include <modules/opengl/inviwoopengl.h>                               // for glDrawElements
#include <modules/opengl/openglutils.h>                                // for BlendModeState
#include <modules/opengl/rendering/meshdrawergl.h>                     // for MeshDrawerGL, Mesh...
#include <modules/opengl/shader/shader.h>                              // for Shader
#include <modules/opengl/shader/shaderobject.h>                        // for ShaderObject
#include <modules/opengl/shader/shadertype.h>                          // for ShaderType, Shader...
#include <modules/opengl/shader/shaderutils.h>                         // for addDefines, setSha...
#include <modules/opengl/texture/textureunit.h>                        // for TextureUnitContainer
#include <modules/opengl/texture/textureutils.h>                       // for activateTargetAndC...
#include <modules/opengl/buffer/buffergl.h>

#include <map>          // for __map_iterator, map
#include <tuple>        // for tuple_element<>::type
#include <type_traits>  // for remove_extent_t

#include <glm/vec2.hpp>  // for vec<>::(anonymous)

#include <string_view>

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
        * PositionAttrib   vec3
        * ColorAttrib      vec4   (optional will fall-back to use __Custom Color__)
        * IndexAttrib      uint   (optional used for labeling and BnL)
        * RadiiAttrib      float  (optional will fall-back to use __Custom Radius__)
        * PickingAttrib    uint32 (optional will fall-back to not draw any picking)
        * ScalarMetaAttrib float  (optional used for custom coloring)
    )"_unindentHelp}
    , imageInport_{"imageInport", "Optional background image"_help}
    , sphereTexture_{"sphereTexture", "Texture to apply to spheres"_help,
                     OutportDeterminesSize::Yes}
    , brushLinkPort_{"brushingAndLinking"}
    , outport_{"image",
               "output image containing the rendered spheres and the optional input image"_help}
    , renderMode_{"renderMode",
                  "Render Mode",
                  "render only input meshes marked as points or everything"_help,
                  {{"entireMesh", "Entire Mesh", RenderMode::EntireMesh},
                   {"pointsOnly", "Points Only", RenderMode::PointsOnly}}}
    , clipping_{"clipping", "Clipping"}
    , clipMode_{"clipMode",
                "Clip Mode",
                "defines the handling of spheres clipped at the camera"_help,
                {{"discard", "Discard Glyph", GlyphClippingMode::Discard},
                 {"cut", "Cut Glypyh", GlyphClippingMode::Cut}},
                0,
                InvalidationLevel::InvalidResources}
    , clipShadingFactor_{"clipShadingFactor", "Clip Surface Adjustment",
                         util::ordinalScale(0.9f, 2.0f)
                             .set("brighten/darken glyph color on clip surface"_help)}
    , shadeClippedArea_{"shadeClippedArea", "Shade Clipped Area",
                        "enable illumination computations for the clipped surface"_help, false,
                        InvalidationLevel::InvalidResources}
    , sphereProperties_{"sphereProperties", "Sphere Properties"}
    , forceRadius_{"forceRadius", "Force Radius",
                   "enable a fixed user-defined radius for all spheres"_help, false,
                   InvalidationLevel::InvalidResources}
    , defaultRadius_{"defaultRadius", "Default Radius",
                     util::ordinalLength(0.05f, 2.0f)
                         .setMin(0.00001f)
                         .set("radius of the rendered spheres (in world coordinates)"_help)}
    , forceColor_{"forceColor", "Force Color",
                  "if enabled, all spheres will share the same custom color"_help, false,
                  InvalidationLevel::InvalidResources}
    , defaultColor_{"defaultColor", "Default Color",
                    util::ordinalColor(vec4(0.7f, 0.7f, 0.7f, 1.0f))
                        .set("custom color when overwriting the input colors"_help)}
    , useMetaColor_{"useMetaColor", "Use meta color mapping", false,
                    InvalidationLevel::InvalidResources}
    , metaColor_{"metaColor", "Meta Color Mapping"}
    , showLabels_{"showLabels", "Show Labels", false}
    , labelFont_{"labelFont", "Label Font", font::getFont(font::FontType::Default),
                 InvalidationLevel::InvalidResources}
    , labelFontSize_{"labelFontSize", "Label Font Size",
                     util::ordinalCount<int>(14, 144)
                         .set(PropertySemantics{"Fontsize"})
                         .set(InvalidationLevel::InvalidResources)}
    , labelColor_{"labelColor", "Label Color", util::ordinalColor(vec4(0.1f, 0.1f, 0.1f, 1.0f))}
    , labelSize_{"labelSize", "Label Size", util::ordinalLength(0.3f, 1.0f)}
    , labelAspect_{1.0f}
    , showHighlighted_{"showHighlighted", "Show Highlighted",
                       "Parameters for color overlay of highlighted data"_help, true,
                       vec3{0.35f, 0.75f, 0.93f}}
    , showSelected_{"showSelected", "Show Selected",
                    "Parameters for color overlay of a selection"_help, true,
                    vec3{1.0f, 0.84f, 0.0f}}
    , showFiltered_{"showFiltered", "Show Filtered",
                    "Parameters for color overlay of filtered data"_help, false,
                    vec3{0.5f, 0.5f, 0.5f}}

    , camera_{"camera", "Camera",
              [this, bbFunctor = util::boundingBox(inport_)]() -> std::optional<mat4> {
                  if (auto bb = bbFunctor()) {
                      (*bb)[0] *= repeat_.get(0);
                      (*bb)[1] *= repeat_.get(1);
                      (*bb)[2] *= repeat_.get(2);
                      return bb;
                  } else {
                      return std::nullopt;
                  }
              }}
    , trackball_{&camera_}
    , lighting_{"lighting", "Lighting", &camera_}
    , periodicity_{"periodicity", "Periodicity", false}
    , basis_{"basis", "Basis", util::ordinalSymmetricVector(mat4{1.0f}, util::filled<mat4>(100.0f))}
    , shift_{"shift",
             "Shift",
             vec3{0.0f},
             {vec3{-1.0f}, ConstraintBehavior::Ignore},
             {vec3{1.0f}, ConstraintBehavior::Ignore}}
    , repeat_{"repeat",
              "Repeat",
              ivec3{1},
              {ivec3{1}, ConstraintBehavior::Immutable},
              {ivec3{10}, ConstraintBehavior::Ignore}}
    , duplicateCutoff_{"duplicateCutoff", "Duplicate Cutoff",
                       util::ordinalLength(0.0f, 1.0f).set(InvalidationLevel::InvalidResources)}

    , shaders_{{{ShaderType::Vertex, std::string{"sphereglyph.vert"}},
                {ShaderType::Geometry, std::string{"sphereglyph.geom"}},
                {ShaderType::Fragment, std::string{"sphereglyph.frag"}}},

               {{BufferType::PositionAttrib, MeshShaderCache::Mandatory, "vec3"},
                {BufferType::ColorAttrib, MeshShaderCache::Optional, "vec4"},
                {BufferType::IndexAttrib, MeshShaderCache::Optional, "uint"},
                {BufferType::RadiiAttrib, MeshShaderCache::Optional, "float"},
                {BufferType::PickingAttrib, MeshShaderCache::Optional, "uint"},
                {BufferType::ScalarMetaAttrib, MeshShaderCache::Optional, "float"},
                {[this](const Mesh&, Mesh::MeshInfo mi) -> int {
                     return brushLinkPort_.isConnected() ? 1 : 0;
                 },
                 [](int mode, Shader& shader) {
                     shader[ShaderType::Vertex]->setShaderDefine("ENABLE_BNL", mode == 1);
                 }},
                {[this](const Mesh&, Mesh::MeshInfo mi) -> int {
                     return sphereTexture_.hasData() ? 1 : 0;
                 },
                 [](int mode, Shader& shader) {
                     shader[ShaderType::Fragment]->setShaderDefine("ENABLE_TEXTURING", mode == 1);
                 }}},

               [&](Shader& shader) -> void {
                   shader.onReload([this]() { invalidate(InvalidationLevel::InvalidResources); });
                   configureShader(shader);
               }}
    , bnlBuffer{32}
    , atlas_{}
    , textRenderer_{} {

    addPort(inport_);
    addPort(imageInport_).setOptional(true);
    addPort(sphereTexture_, "Textures").setOptional(true);
    addPort(brushLinkPort_);
    addPort(outport_);

    showLabels_.addProperties(labelFont_, labelColor_, labelSize_, labelFontSize_);
    showLabels_.getBoolProperty()->setInvalidationLevel(InvalidationLevel::InvalidResources);
    clipping_.addProperties(clipMode_, clipShadingFactor_, shadeClippedArea_);

    sphereProperties_.addProperties(forceRadius_, defaultRadius_, forceColor_, defaultColor_,
                                    useMetaColor_, metaColor_);

    periodicity_.addProperties(shift_, repeat_, duplicateCutoff_, basis_);
    periodicity_.getBoolProperty()->setInvalidationLevel(InvalidationLevel::InvalidResources);

    addProperties(renderMode_, sphereProperties_, showLabels_, showHighlighted_, showSelected_,
                  showFiltered_, periodicity_, clipping_, camera_, lighting_, trackball_);

    clipShadingFactor_.readonlyDependsOn(
        clipMode_, [](const auto& p) { return p == GlyphClippingMode::Discard; });
    shadeClippedArea_.readonlyDependsOn(
        clipMode_, [](const auto& p) { return p == GlyphClippingMode::Discard; });
}

void SphereRenderer::initializeResources() {
    if (showLabels_) {
        textRenderer_.setFont(labelFont_.getSelectedValue());
        textRenderer_.setFontSize(labelFontSize_.get());

        size2_t charSize{0};
        for (int i = 0; i < 10; ++i) {
            charSize = glm::max(charSize,
                                textRenderer_.computeBoundingBox(fmt::to_string(i)).glyphsExtent);
        }

        const auto labelSize = charSize * size2_t{3, 1} + size2_t{2, 2};
        labelAspect_ = float(labelSize.y) / float(labelSize.x);
        const auto atlasSize = size2_t{30, 30};
        if (!atlas_ || labelSize * atlasSize != atlas_->getDimensions()) {
            atlas_ = std::make_shared<Texture2D>(labelSize * atlasSize,
                                                 GLFormats::get(DataVec4UInt8::id()), GL_LINEAR);
            atlas_->initialize(nullptr);
        }

        utilgl::BlendModeState blendModeStateGL(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        {
            auto state = textRenderer_.setupRenderState(atlas_, vec4{0.0});
            for (size_t i = 0; i < atlasSize.y; ++i) {
                for (size_t j = 0; j < atlasSize.x; ++j) {
                    auto str = fmt::to_string(i * atlasSize.x + j);
                    const auto ttb = textRenderer_.computeBoundingBox(str);
                    const size2_t offset = (labelSize - ttb.glyphsExtent) / 2;
                    const auto pos = static_cast<ivec2>(labelSize * size2_t{i, j} + offset);
                    textRenderer_.render(ttb, pos, str, vec4{1, 1, 1, 1});
                }
            }
        }
    }

    for (auto& [state, shader] : shaders_.getShaders()) {
        configureShader(shader);
    }
}

void SphereRenderer::configureShader(Shader& shader) {
    utilgl::addDefines(shader, lighting_);
    shader[ShaderType::Vertex]->setShaderDefine("FORCE_RADIUS", forceRadius_);
    shader[ShaderType::Vertex]->setShaderDefine("FORCE_COLOR", forceColor_);
    shader[ShaderType::Vertex]->setShaderDefine("USE_SCALARMETACOLOR", useMetaColor_);
    shader[ShaderType::Vertex]->setShaderDefine("ENABLE_PERIODICITY", periodicity_.isChecked());
    shader[ShaderType::Geometry]->setShaderDefine("ENABLE_PERIODICITY", periodicity_.isChecked());
    shader[ShaderType::Geometry]->setShaderDefine("DISCARD_CLIPPED_GLYPHS",
                                                  clipMode_.get() == GlyphClippingMode::Discard);
    shader[ShaderType::Geometry]->setShaderDefine(
        "ENABLE_DUPLICATE", periodicity_.isChecked() && duplicateCutoff_.get() > 0.0f);

    shader[ShaderType::Fragment]->setShaderDefine("SHADE_CLIPPED_AREA", shadeClippedArea_);
    shader[ShaderType::Fragment]->setShaderDefine("DISCARD_CLIPPED_GLYPHS",
                                                  clipMode_.get() == GlyphClippingMode::Discard);
    shader[ShaderType::Fragment]->setShaderDefine("ENABLE_LABELS", showLabels_.isChecked());

    shader.build();
}

namespace {
std::uint32_t bit_ceil(std::uint32_t v) {
    v--;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    v++;
    return v;
}
}  // namespace

void SphereRenderer::process() {
    utilgl::BlendModeState blendModeStateGL(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    utilgl::activateTargetAndClearOrCopySource(outport_, imageInport_);

    TextureUnit labelsUnit;
    TextureUnit bnlUnit;
    TextureUnit metaColorUnit;
    TextureUnit sphereTextureUnit;

    if (showLabels_) {
        utilgl::bindTexture(*atlas_, labelsUnit);
    }

    if (brushLinkPort_.isConnected()) {
        if (brushLinkPort_.isChanged() || showFiltered_.isModified() ||
            showSelected_.isModified() || showHighlighted_.isModified()) {
            const auto size = brushLinkPort_.getManager().getMax() + 1;
            if (size > bnlBuffer.getSize()) {
                bnlBuffer.setSize(bit_ceil(size));
            }

            auto bnlData = bnlBuffer.map(GL_WRITE_ONLY);
            std::fill(bnlData.begin(), bnlData.end(), 0);
            if (showSelected_) {
                for (auto i : brushLinkPort_.getSelectedIndices()) {
                    bnlData[i] = 1;
                }
            }
            if (showHighlighted_) {
                for (auto i : brushLinkPort_.getHighlightedIndices()) {
                    bnlData[i] = 2;
                }
            }
            if (showFiltered_) {
                for (auto i : brushLinkPort_.getFilteredIndices()) {
                    bnlData[i] = 3;
                }
            }
            bnlBuffer.unmap();
        }
        utilgl::bindTexture(bnlBuffer, bnlUnit);
    }

    utilgl::bindTexture(metaColor_, metaColorUnit);

    if (sphereTexture_.hasData()) {
        utilgl::bindColorTexture(*sphereTexture_.getData(), sphereTextureUnit);
    }

    for (const auto& [port, mesh] : inport_.getSourceVectorData()) {
        if (mesh->getNumberOfBuffers() == 0) continue;

        auto& shader = shaders_.getShader(*mesh);
        shader.activate();
        shader.setUniform(metaColor_.getIdentifier(), metaColorUnit);
        shader.setUniform("labelAspect", labelAspect_);
        shader.setUniform("bnl", bnlUnit);
        shader.setUniform("labels", labelsUnit);
        shader.setUniform("sphereTexture", sphereTextureUnit);
        shader.setUniform("basisOffset", mesh->getModelMatrix());
        utilgl::setUniforms(shader, camera_, lighting_, defaultColor_, defaultRadius_,
                            clipShadingFactor_, showFiltered_, showSelected_, showHighlighted_,
                            labelColor_, labelSize_, basis_, repeat_, shift_, duplicateCutoff_);
        shader.setUniform("viewport", vec4(0.0f, 0.0f, 2.0f / outport_.getDimensions().x,
                                           2.0f / outport_.getDimensions().y));
        MeshDrawerGL::DrawObject drawer(*mesh);
        utilgl::setShaderUniforms(shader, *mesh, "geometry");
        const auto instances = static_cast<size_t>(glm::compMul(repeat_.get()));

        switch (renderMode_) {
            case RenderMode::PointsOnly: {
                // render only index buffers marked as points (or the entire mesh if none
                // exists)
                if (mesh->getNumberOfIndicies() > 0) {
                    for (size_t i = 0; i < mesh->getNumberOfIndicies(); ++i) {
                        auto meshinfo = mesh->getIndexMeshInfo(i);
                        if ((meshinfo.dt == DrawType::Points) ||
                            (meshinfo.dt == DrawType::NotSpecified)) {
                            drawer.drawInstanced(MeshDrawerGL::DrawMode::Points, i, instances);
                        }
                    }
                } else {
                    // no index buffers, check mesh default draw type
                    auto drawtype = mesh->getDefaultMeshInfo().dt;
                    if ((drawtype == DrawType::Points) || (drawtype == DrawType::NotSpecified)) {
                        drawer.drawInstanced(MeshDrawerGL::DrawMode::Points, instances);
                    }
                }
                break;
            }
            case RenderMode::EntireMesh:
                [[fallthrough]];
            default:  // render all parts of the input meshes as points
                drawer.drawInstanced(MeshDrawerGL::DrawMode::Points, instances);
                break;
        }
        shader.deactivate();
    }

    utilgl::deactivateCurrentTarget();
}

}  // namespace inviwo
