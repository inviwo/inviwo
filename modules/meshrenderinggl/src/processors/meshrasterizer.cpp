/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019-2023 Inviwo Foundation
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

#include <modules/meshrenderinggl/processors/meshrasterizer.h>

#include <inviwo/core/algorithm/boundingbox.h>
#include <inviwo/core/datastructures/buffer/buffer.h>                         // for IndexBuffer
#include <inviwo/core/datastructures/data.h>                                  // for noData
#include <inviwo/core/datastructures/geometry/geometrytype.h>                 // for Connectivit...
#include <inviwo/core/datastructures/geometry/mesh.h>                         // for Mesh, Mesh:...
#include <inviwo/core/datastructures/image/layer.h>                           // for Layer
#include <inviwo/core/datastructures/representationconverter.h>               // for Representat...
#include <inviwo/core/datastructures/representationconverterfactory.h>        // for Representat...
#include <inviwo/core/datastructures/transferfunction.h>                      // for TransferFun...
#include <inviwo/core/ports/datainport.h>                                     // for DataInport
#include <inviwo/core/ports/inportiterable.h>                                 // for InportItera...
#include <inviwo/core/ports/meshport.h>                                       // for MeshFlatMul...
#include <inviwo/core/processors/processor.h>                                 // for Processor
#include <inviwo/core/processors/processorinfo.h>                             // for ProcessorInfo
#include <inviwo/core/processors/processorstate.h>                            // for CodeState
#include <inviwo/core/processors/processortags.h>                             // for Tags, Tags::GL
#include <inviwo/core/properties/boolcompositeproperty.h>                     // for BoolComposi...
#include <inviwo/core/properties/boolproperty.h>                              // for BoolProperty
#include <inviwo/core/properties/buttonproperty.h>                            // for ButtonProperty
#include <inviwo/core/properties/compositeproperty.h>                         // for CompositePr...
#include <inviwo/core/properties/invalidationlevel.h>                         // for Invalidatio...
#include <inviwo/core/properties/listproperty.h>                              // for ListProperty
#include <inviwo/core/properties/optionproperty.h>                            // for OptionProperty
#include <inviwo/core/properties/ordinalproperty.h>                           // for IntProperty
#include <inviwo/core/properties/property.h>                                  // for Property
#include <inviwo/core/properties/propertysemantics.h>                         // for PropertySem...
#include <inviwo/core/properties/simplelightingproperty.h>                    // for SimpleLight...
#include <inviwo/core/properties/transferfunctionproperty.h>                  // for TransferFun...
#include <inviwo/core/util/document.h>                                        // for Document
#include <inviwo/core/util/glmmat.h>                                          // for mat4
#include <inviwo/core/util/glmutils.h>                                        // for Matrix
#include <inviwo/core/util/glmvec.h>                                          // for vec4, ivec2
#include <inviwo/core/util/iterrange.h>                                       // for iter_range
#include <inviwo/core/util/logcentral.h>                                      // for LogCentral
#include <inviwo/core/util/staticstring.h>                                    // for operator+
#include <modules/base/properties/transformlistproperty.h>                    // for TransformLi...
#include <modules/meshrenderinggl/algorithm/calcnormals.h>                    // for CalculateMe...
#include <modules/meshrenderinggl/datastructures/halfedges.h>                 // for HalfEdges
#include <modules/meshrenderinggl/datastructures/transformedrasterization.h>  // for Transformed...
#include <modules/meshrenderinggl/ports/rasterizationport.h>                  // for Rasterizati...
#include <modules/opengl/geometry/meshgl.h>                                   // for MeshGL
#include <modules/opengl/image/layergl.h>                                     // for LayerGL
#include <modules/opengl/inviwoopengl.h>                                      // for GL_BACK
#include <modules/opengl/openglutils.h>                                       // for BlendModeState
#include <modules/opengl/rendering/meshdrawergl.h>                            // for MeshDrawerG...
#include <modules/opengl/shader/shader.h>                                     // for Shader, Sha...
#include <modules/opengl/shader/shaderobject.h>                               // for ShaderObject
#include <modules/opengl/shader/shaderutils.h>                                // for addShaderDe...
#include <modules/opengl/texture/textureunit.h>                               // for TextureUnit

#include <cstddef>        // for size_t
#include <tuple>          // for tuple_eleme...
#include <type_traits>    // for remove_exte...
#include <unordered_set>  // for unordered_set
#include <utility>        // for pair
#include <variant>        // for visit, variant

#include <fmt/core.h>      // for format
#include <glm/mat4x4.hpp>  // for operator*
#include <glm/vec2.hpp>    // for operator/
#include <glm/vec4.hpp>    // for operator*

namespace inviwo {
class Rasterization;

namespace {
void configComposite(BoolCompositeProperty& comp) {
    for (auto* p : comp) {
        if (p != comp.getBoolProperty()) {
            p->readonlyDependsOn(comp, std::not_fn(&BoolCompositeProperty::isChecked));
        }
    }
    auto callback = [&comp]() { comp.setCollapsed(!comp.isChecked()); };
    comp.getBoolProperty()->onChange(callback);
    callback();
}
}  // namespace

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo MeshRasterizer::processorInfo_{
    "org.inviwo.MeshRasterizer",  // Class identifier
    "Mesh Rasterizer",            // Display name
    "Mesh Rendering",             // Category
    CodeState::Experimental,      // Code state
    Tags::GL,                     // Tags
    R"(Mesh Renderer specialized for rendering highly layered and transparent surfaces.
       Example usages: stream surfaces, isosurfaces, separatrices.
       
       Encompasses the work from:
       * IRIS: Illustrative Rendering of Integral Surfaces, IEEE TVCG (2010), Hummel et al.
       * Smoke Surfaces: An Interactive Flow Visualization Technique
         Inspired by Real-World Flow Experiments, IEEE TVCG (2008), von Funck et al.
       
       Fragment lists are used to render the transparent pixels with correct alpha blending.
       Many different alpha modes, shading modes, coloring modes are available.)"_unindentHelp};

const ProcessorInfo MeshRasterizer::getProcessorInfo() const { return processorInfo_; }

MeshRasterizer::MeshRasterizer()
    : RasterizationProcessor()
    , inport_("geometry", "Input meshes"_help)
    , lightingProperty_("lighting", "Lighting")
    , forceOpaque_("forceOpaque", "Shade Opaque",
                   "Draw the mesh opaquly instead of transparent. Disables all transparency"_help,
                   false, InvalidationLevel::InvalidResources)
    , drawSilhouette_("drawSilhouette", "Draw Silhouette", false,
                      InvalidationLevel::InvalidResources)
    , silhouetteColor_("silhouetteColor", "Silhouette Color", {0.f, 0.f, 0.f, 1.f})
    , normalSource_(
          "normalSource", "Normals Source",
          {
              {"inputVertex", "Input: Vertex Normal", NormalSource::InputVertex},
              {"generateVertex", "Generate: Vertex Normal", NormalSource::GenerateVertex},
              {"generateTriangle", "Generate: Triangle Normal", NormalSource::GenerateTriangle},
          },
          0, InvalidationLevel::InvalidResources)
    , normalComputationMode_(
          "normalComputationMode", "Normals Computation",
          {{"noWeighting", "No Weighting", meshutil::CalculateMeshNormalsMode::NoWeighting},
           {"area", "Area-weighting", meshutil::CalculateMeshNormalsMode::WeightArea},
           {"angle", "Angle-weighting", meshutil::CalculateMeshNormalsMode::WeightAngle},
           {"nmax", "Based on N.Max", meshutil::CalculateMeshNormalsMode::WeightNMax}},
          3, InvalidationLevel::InvalidResources)
    , faceSettings_{true, false}
    , shader_(std::make_shared<Shader>("fancymeshrenderer.vert", "fancymeshrenderer.geom",
                                       "fancymeshrenderer.frag", Shader::Build::No)) {
    // input and output ports
    addPort(inport_).onChange([this]() { updateMeshes(); });

    drawSilhouette_.onChange([this]() { updateMeshes(); });

    addProperties(lightingProperty_, forceOpaque_, drawSilhouette_, silhouetteColor_, normalSource_,
                  normalComputationMode_, alphaSettings_, edgeSettings_, faceSettings_[0].show_,
                  faceSettings_[1].show_);

    silhouetteColor_.visibilityDependsOn(drawSilhouette_, [](const auto& p) { return p.get(); });
    normalComputationMode_.visibilityDependsOn(
        normalSource_, [](const auto& p) { return p.get() == NormalSource::GenerateVertex; });

    alphaSettings_.visibilityDependsOn(forceOpaque_, [](const auto& p) { return !p.get(); });

    auto edgeVis = [this](auto) {
        return drawSilhouette_.get() || faceSettings_[0].showEdges_.get() ||
               faceSettings_[1].showEdges_.get();
    };
    edgeSettings_.visibilityDependsOn(drawSilhouette_, edgeVis);
    edgeSettings_.visibilityDependsOn(faceSettings_[0].showEdges_, edgeVis);
    edgeSettings_.visibilityDependsOn(faceSettings_[1].showEdges_, edgeVis);

    lightingProperty_.setCollapsed(true);

    silhouetteColor_.setSemantics(PropertySemantics::Color);

    faceSettings_[1].frontPart_ = &faceSettings_[0];
}

MeshRasterizer::AlphaSettings::AlphaSettings()
    : CompositeProperty(
          "alphaContainer", "Alpha",
          "Assemble construction of the alpha value out of many factors (which are summed up)"_help)
    , enableUniform_("alphaUniform", "Uniform", "uniform alpha value"_help, true,
                     InvalidationLevel::InvalidResources)
    , uniformScaling_("alphaUniformScaling", "Scaling", 0.5f, 0.f, 1.f, 0.01f)
    , minimumAlpha_("minimumAlpha", "Minimum Alpha", 0.1f, 0.f, 1.f, 0.01f)
    , enableAngleBased_(
          "alphaAngleBased", "Angle-based",
          "based on the angle between the pixel normal and the direction to the camera "_help,
          false, InvalidationLevel::InvalidResources)
    , angleBasedExponent_("alphaAngleBasedExponent", "Exponent", 1.f, 0.f, 5.f, 0.01f)
    , enableNormalVariation_(
          "alphaNormalVariation", "Normal variation",
          "based on the variation (norm of the derivative) of the pixel normal"_help, false,
          InvalidationLevel::InvalidResources)
    , normalVariationExponent_("alphaNormalVariationExponent", "Exponent", 1.f, 0.f, 5.f, 0.01f)
    , enableDensity_(
          "alphaDensity", "Density-based",
          "based on the size of the triangle / density of the smoke volume inside the triangle"_help,
          false, InvalidationLevel::InvalidResources)
    , baseDensity_("alphaBaseDensity", "Base density", 1.f, 0.f, 2.f, 0.01f)
    , densityExponent_("alphaDensityExponent", "Exponent", 1.f, 0.f, 5.f, 0.01f)
    , enableShape_(
          "alphaShape", "Shape-based",
          "based on the shape of the triangle. The more stretched, the more transparent"_help,
          false, InvalidationLevel::InvalidResources)
    , shapeExponent_("alphaShapeExponent", "Exponent", 1.f, 0.f, 5.f, 0.01f) {
    addProperties(minimumAlpha_, enableUniform_, uniformScaling_, enableAngleBased_,
                  angleBasedExponent_, enableNormalVariation_, normalVariationExponent_,
                  enableDensity_, baseDensity_, densityExponent_, enableShape_, shapeExponent_);

    const auto get = [](const auto& p) { return p.get(); };

    uniformScaling_.visibilityDependsOn(enableUniform_, get);
    angleBasedExponent_.visibilityDependsOn(enableAngleBased_, get);
    normalVariationExponent_.visibilityDependsOn(enableNormalVariation_, get);
    baseDensity_.visibilityDependsOn(enableDensity_, get);
    densityExponent_.visibilityDependsOn(enableDensity_, get);
    shapeExponent_.visibilityDependsOn(enableShape_, get);
}

void MeshRasterizer::AlphaSettings::setUniforms(Shader& shader, std::string_view prefix) const {
    std::array<std::pair<std::string_view, std::variant<float>>, 7> uniforms{
        {{"minAlpha", minimumAlpha_},
         {"uniformScale", uniformScaling_},
         {"angleExp", angleBasedExponent_},
         {"normalExp", normalVariationExponent_},
         {"baseDensity", baseDensity_},
         {"densityExp", densityExponent_},
         {"shapeExp", shapeExponent_}}};

    for (const auto& [key, val] : uniforms) {
        std::visit([&, akey = key](
                       auto& aval) { shader.setUniform(fmt::format("{}{}", prefix, akey), aval); },
                   val);
    }
}

MeshRasterizer::EdgeSettings::EdgeSettings()
    : CompositeProperty("edges", "Edges", "Settings for the display of triangle edges"_help)
    , edgeThickness_("thickness", "Thickness",
                     util::ordinalScale(2.f, 10.f).set("The thickness of the edges in pixels"_help))
    , depthDependent_("depth", "Depth dependent",
                      "If checked, the thickness also depends on the depth. If unchecked, every"
                      "edge has the same size in screen space regardless of the distance"_help,
                      false, InvalidationLevel::InvalidResources)
    , smoothEdges_("smooth", "Smooth edges", "If checked, a simple anti-alising is used"_help, true,
                   InvalidationLevel::InvalidResources) {
    addProperties(edgeThickness_, depthDependent_, smoothEdges_);
}

MeshRasterizer::HatchingSettings::HatchingSettings()
    : hatching_("hatching", "Hatching Settings", false)
    , mode_("hatchingMode", "Hatching",
            {{"u", "U", HatchingMode::U},
             {"v", "V", HatchingMode::V},
             {"uv", "UV", HatchingMode::UV}},
            0, InvalidationLevel::InvalidResources)
    , steepness_("steepness", "Steepness", 5, 1, 10)
    , baseFrequencyU_("frequencyU", "U-Frequency", 3, 1, 10)
    , baseFrequencyV_("frequencyV", "V-Frequency", 3, 1, 10)
    , modulation_("modulation", "Modulation", false)
    , modulationMode_("modulationMode", "Modulation",
                      {{"u", "U", HatchingMode::U},
                       {"v", "V", HatchingMode::V},
                       {"uv", "UV", HatchingMode::UV}})
    , modulationAnisotropy_("modulationAnisotropy", "Anisotropy", 0.5f, -1.f, 1.f, 0.01f)
    , modulationOffset_("modulationOffset", "Offset", 0.f, 0.f, 1.f, 0.01f)
    , color_("color", "Color", {0.f, 0.f, 0.f})
    , strength_("strength", "Strength", 0.5f, 0.f, 1.f, 0.01f)
    , blendingMode_("blending", "Blending",
                    {{"mult", "Multiplicative", HatchingBlendingMode::Multiplicative},
                     {"add", "Additive", HatchingBlendingMode::Additive}}) {

    hatching_.getBoolProperty()->setInvalidationLevel(InvalidationLevel::InvalidResources);
    configComposite(hatching_);

    // init properties
    color_.setSemantics(PropertySemantics::Color);

    // add to container
    modulation_.addProperties(modulationMode_, modulationAnisotropy_, modulationOffset_);
    configComposite(modulation_);

    hatching_.addProperties(mode_, steepness_, baseFrequencyU_, baseFrequencyV_, modulation_,
                            color_, strength_, blendingMode_);

    baseFrequencyU_.visibilityDependsOn(
        mode_, [](const auto& prop) { return prop.get() != HatchingMode::V; });
    baseFrequencyV_.visibilityDependsOn(
        mode_, [](const auto& prop) { return prop.get() != HatchingMode::U; });

    modulation_.visibilityDependsOn(
        mode_, [](const auto& prop) { return prop.get() == HatchingMode::UV; });
}

MeshRasterizer::FaceSettings::FaceSettings(bool frontFace)
    : frontFace_(frontFace)
    , show_(frontFace ? "frontcontainer" : "backcontainer", frontFace ? "Front Face" : "Back Face",
            "Shows or hides that face (culling)"_help, true)
    , sameAsFrontFace_("same", "Same as Front Face",
                       "use the settings from the front face, disables all"
                       "other settings for the back face"_help)
    , copyFrontToBack_("copy", "Copy Front to Back",
                       "Copies all settings from the front face to the back face"_help)
    , transferFunction_("tf", "Transfer Function")
    , externalColor_("extraColor", "Color", {1.f, 0.3f, 0.01f})
    , colorSource_("colorSource", "Color Source",
                   "The source of the color: vertex color, transfer function,"
                   "or external constant color"_help,
                   {{"vertexColor", "VertexColor", ColorSource::VertexColor},
                    {"tf", "Transfer Function", ColorSource::TransferFunction},
                    {"external", "Constant Color", ColorSource::ExternalColor}},
                   2, InvalidationLevel::InvalidResources)
    , separateUniformAlpha_("separateUniformAlpha", "Separate Uniform Alpha",
                            "Overwrite alpha settings from above with a constant alpha value"_help)
    , uniformAlpha_("uniformAlpha", "Uniform Alpha", 0.5f, 0.f, 1.f, 0.01f)
    , shadingMode_("shadingMode", "Shading Mode",
                   "The shading that is applied to the pixel color"_help,
                   {{"off", "Off", ShadingMode::Off}, {"phong", "Phong", ShadingMode::Phong}})
    , showEdges_("showEdges", "Show Edges", "Show triangle edges"_help, false,
                 InvalidationLevel::InvalidResources)
    , edgeColor_("edgeColor", "Edge color",
                 util::ordinalColor(vec3{0.0f}).set("The color of the edges"_help))
    , edgeOpacity_("edgeOpacity", "Edge Opacity",
                   R"(Blending of the edge color:
        0-1: blending factor of the edge color into the triangle color, alpha unmodified;
        1-2: full edge color and alpha is increased to fully opaque)"_unindentHelp,
                   0.5f, {0.f, ConstraintBehavior::Immutable}, {2.f, ConstraintBehavior::Immutable},
                   0.01f)
    , hatching_() {

    externalColor_.setSemantics(PropertySemantics::Color);
    edgeColor_.setSemantics(PropertySemantics::Color);

    if (!frontFace) {
        show_.addProperties(sameAsFrontFace_, copyFrontToBack_);
        copyFrontToBack_.onChange([this]() { copyFrontToBack(); });
    }
    show_.addProperties(colorSource_, transferFunction_, externalColor_, separateUniformAlpha_,
                        uniformAlpha_, shadingMode_, showEdges_, edgeColor_, edgeOpacity_,
                        hatching_.hatching_);

    configComposite(show_);

    const auto get = [](const auto& p) { return p.get(); };
    edgeColor_.visibilityDependsOn(showEdges_, get);
    edgeOpacity_.visibilityDependsOn(showEdges_, get);

    uniformAlpha_.visibilityDependsOn(separateUniformAlpha_,
                                      [](const auto& prop) { return prop.get(); });

    transferFunction_.visibilityDependsOn(
        colorSource_, [](const auto& prop) { return prop.get() == ColorSource::TransferFunction; });

    externalColor_.visibilityDependsOn(
        colorSource_, [](const auto& prop) { return prop.get() == ColorSource::ExternalColor; });
}

void MeshRasterizer::FaceSettings::copyFrontToBack() {
    for (auto src : frontPart_->show_) {
        if (auto dst = show_.getPropertyByIdentifier(src->getIdentifier())) {
            dst->set(src);
        }
    }
}

void MeshRasterizer::initializeResources() {
    // shading defines
    utilgl::addShaderDefines(*shader_, lightingProperty_);

    const std::array<std::pair<std::string, bool>, 15> defines = {
        {{"USE_FRAGMENT_LIST", !forceOpaque_.get()},
         {"ALPHA_UNIFORM", alphaSettings_.enableUniform_},
         {"ALPHA_ANGLE_BASED", alphaSettings_.enableAngleBased_},
         {"ALPHA_NORMAL_VARIATION", alphaSettings_.enableNormalVariation_},
         {"ALPHA_DENSITY", alphaSettings_.enableDensity_},
         {"ALPHA_SHAPE", alphaSettings_.enableShape_},
         {"DRAW_EDGES", faceSettings_[0].showEdges_ || faceSettings_[1].showEdges_},
         {"DRAW_EDGES_DEPTH_DEPENDENT", edgeSettings_.depthDependent_},
         {"DRAW_EDGES_SMOOTHING", edgeSettings_.smoothEdges_},
         {"MESH_HAS_ADJACENCY", meshHasAdjacency_},
         {"DRAW_SILHOUETTE", drawSilhouette_},
         {"SEND_TEX_COORD", faceSettings_[0].hatching_.hatching_.isChecked() ||
                                faceSettings_[1].hatching_.hatching_.isChecked()},
         {"SEND_SCALAR", faceSettings_[0].colorSource_ == ColorSource::TransferFunction ||
                             faceSettings_[1].colorSource_ == ColorSource::TransferFunction},
         {"SEND_COLOR", faceSettings_[0].colorSource_ == ColorSource::VertexColor ||
                            faceSettings_[1].colorSource_ == ColorSource::VertexColor}}};

    for (auto&& [key, val] : defines) {
        for (auto&& so : shader_->getShaderObjects()) {
            so.setShaderDefine(key, val);
        }
    }
}

void MeshRasterizer::FaceSettings::setUniforms(Shader& shader, std::string_view prefix) const {

    std::array<std::pair<std::string_view, std::variant<bool, int, float, vec4>>, 15> uniforms{
        {{"externalColor", vec4{*externalColor_, 1.0f}},
         {"colorSource", static_cast<int>(*colorSource_)},
         {"separateUniformAlpha", separateUniformAlpha_},
         {"uniformAlpha", uniformAlpha_},
         {"shadingMode", static_cast<int>(*shadingMode_)},
         {"showEdges", showEdges_},
         {"edgeColor", vec4{*edgeColor_, *edgeOpacity_}},
         {"hatchingMode", (hatching_.mode_.get() == HatchingMode::UV)
                              ? 3 + static_cast<int>(hatching_.modulationMode_.get())
                              : static_cast<int>(hatching_.mode_.get())},
         {"hatchingSteepness", hatching_.steepness_.get()},
         {"hatchingFreqU", hatching_.baseFrequencyU_.getMaxValue() - hatching_.baseFrequencyU_},
         {"hatchingFreqV", hatching_.baseFrequencyV_.getMaxValue() - hatching_.baseFrequencyV_},
         {"hatchingModulationAnisotropy", hatching_.modulationAnisotropy_},
         {"hatchingModulationOffset", hatching_.modulationOffset_},
         {"hatchingColor", vec4(hatching_.color_.get(), hatching_.strength_.get())},
         {"hatchingBlending", static_cast<int>(hatching_.blendingMode_.get())}}};

    for (const auto& [key, val] : uniforms) {
        std::visit([&, akey = key](
                       auto aval) { shader.setUniform(fmt::format("{}{}", prefix, akey), aval); },
                   val);
    }
}
void MeshRasterizer::rasterize(const ivec2& imageSize, const mat4& worldMatrixTransform,
                               std::function<void(Shader&)> setUniformsRenderer,
                               std::function<void(Shader&)> initializeShader) {

    if (!faceSettings_[0].show_ && !faceSettings_[1].show_) {
        outport_.setData(nullptr);
        LogWarn("Both sides are disabled, not rendering anything.");
        return;  // everything is culled
    }

    initializeShader(*shader_);

    shader_->activate();

    // general settings for camera, lighting, picking, mesh data
    utilgl::setUniforms(*shader_, lightingProperty_);

    // update face render settings
    for (size_t j = 0; j < faceSettings_.size(); ++j) {
        const std::string prefix = fmt::format("renderSettings[{}].", j);
        auto& face = faceSettings_[faceSettings_[1].sameAsFrontFace_.get() ? 0 : j];
        face.setUniforms(*shader_, prefix);
    }

    // update alpha settings
    alphaSettings_.setUniforms(*shader_, "alphaSettings.");

    // update other global fragment shader settings
    shader_->setUniform("silhouetteColor", silhouetteColor_);

    // update geometry shader settings
    shader_->setUniform("geomSettings.edgeWidth", edgeSettings_.edgeThickness_);
    shader_->setUniform("geomSettings.triangleNormal",
                        normalSource_ == NormalSource::GenerateTriangle);

    shader_->deactivate();

    std::array<bool, 2> showFace = {faceSettings_[0].show_, faceSettings_[1].show_};

    std::array<const Layer*, 2> tfTextures = {
        faceSettings_[0].transferFunction_->getData(),
        faceSettings_[faceSettings_[1].sameAsFrontFace_.get() ? 0 : 1]
            .transferFunction_->getData()};

    shader_->activate();

    // set transfer function textures
    std::array<TextureUnit, 2> transFuncUnit;
    for (size_t j = 0; j < 2; ++j) {
        const LayerGL* transferFunctionGL = tfTextures[j]->getRepresentation<LayerGL>();
        transferFunctionGL->bindTexture(transFuncUnit[j].getEnum());
        shader_->setUniform(fmt::format("transferFunction{}", j), transFuncUnit[j].getUnitNumber());
    }

    shader_->setUniform("halfScreenSize", imageSize / ivec2(2));

    // call the callback provided by the renderer calling this function
    setUniformsRenderer(*shader_);
    {
        // various OpenGL states: depth, blending, culling
        utilgl::GlBoolState depthTest(GL_DEPTH_TEST, forceOpaque_);
        utilgl::DepthMaskState depthMask(forceOpaque_ ? GL_TRUE : GL_FALSE);

        utilgl::CullFaceState culling(!showFace[0] && showFace[1]   ? GL_FRONT
                                      : showFace[0] && !showFace[1] ? GL_BACK
                                                                    : GL_NONE);
        utilgl::BlendModeState blendModeState(forceOpaque_ ? GL_ONE : GL_SRC_ALPHA,
                                              forceOpaque_ ? GL_ZERO : GL_ONE_MINUS_SRC_ALPHA);

        // Finally, draw it
        for (auto mesh : enhancedMeshes_) {
            MeshDrawerGL::DrawObject drawer{mesh->getRepresentation<MeshGL>(),
                                            mesh->getDefaultMeshInfo()};
            auto transform = CompositeTransform(mesh->getModelMatrix(),
                                                mesh->getWorldMatrix() * worldMatrixTransform);
            utilgl::setShaderUniforms(*shader_, transform, "geometry");
            shader_->setUniform("pickingEnabled", meshutil::hasPickIDBuffer(mesh.get()));

            drawer.draw();
        }
    }

    shader_->deactivate();
}

void MeshRasterizer::updateMeshes() {
    enhancedMeshes_.clear();
    for (auto mesh : inport_) {
        std::shared_ptr<Mesh> copy = nullptr;

        if (drawSilhouette_) {
            copy = std::make_shared<Mesh>(*mesh, noData);
            for (auto&& [info, buffer] : mesh->getBuffers()) {
                copy->addBuffer(info, std::shared_ptr<BufferBase>(buffer->clone()));
            }

            // create adjacency information
            const auto halfEdges = HalfEdges{*mesh};

            // add new index buffer with adjacency information
            copy->addIndices(
                {DrawType::Triangles, ConnectivityType::Adjacency},
                std::make_shared<IndexBuffer>(halfEdges.createIndexBufferWithAdjacency()));

            if (!meshHasAdjacency_) {
                meshHasAdjacency_ = true;
                initializeResources();
            }
        } else {
            if (meshHasAdjacency_) {
                meshHasAdjacency_ = false;
                initializeResources();
            }
        }

        if (normalSource_.get() == NormalSource::GenerateVertex) {
            if (!copy) copy = std::shared_ptr<Mesh>(mesh->clone());
            meshutil::calculateMeshNormals(*copy, normalComputationMode_);
        }
        enhancedMeshes_.push_back(copy ? copy : mesh);
    }
}

std::optional<mat4> MeshRasterizer::boundingBox() const { return util::boundingBox(inport_)(); }

Document MeshRasterizer::getInfo() const {
    Document doc;
    doc.append("p", fmt::format("Mesh rasterization functor with {} {}. {}", enhancedMeshes_.size(),
                                (enhancedMeshes_.size() == 1) ? " mesh" : " meshes",
                                usesFragmentLists() ? "Using A-buffer" : "Rendering opaque"));
    return doc;
}

}  // namespace inviwo
