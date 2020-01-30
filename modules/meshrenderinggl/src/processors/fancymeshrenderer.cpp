/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019 Inviwo Foundation
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

#include <modules/meshrenderinggl/processors/fancymeshrenderer.h>

#include <modules/opengl/geometry/meshgl.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/rendering/meshdrawerfactory.h>
#include <modules/opengl/openglutils.h>
#include <modules/opengl/texture/textureutils.h>
#include <modules/opengl/shader/shaderutils.h>
#include <modules/base/algorithm/dataminmax.h>
#include <modules/opengl/image/layergl.h>
#include <modules/opengl/openglcapabilities.h>
#include <modules/opengl/rendering/meshdrawergl.h>

#include <sstream>
#include <chrono>

#include <fmt/format.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo FancyMeshRenderer::processorInfo_{
    "org.inviwo.FancyMeshRenderer",  // Class identifier
    "Fancy Mesh Renderer",           // Display name
    "Mesh Rendering",                // Category
    CodeState::Experimental,         // Code state
    Tags::GL,                        // Tags
};
const ProcessorInfo FancyMeshRenderer::getProcessorInfo() const { return processorInfo_; }

FancyMeshRenderer::FancyMeshRenderer()
    : Processor()
    , inport_("geometry")
    , imageInport_("imageInport")
    , outport_("image")
    , camera_("camera", "Camera", vec3(0.0f, 0.0f, 2.0f), vec3(0.0f, 0.0f, 0.0f),
              vec3(0.0f, 1.0f, 0.0f), &inport_)
    , trackball_(&camera_)
    , lightingProperty_("lighting", "Lighting", &camera_)
    , forceOpaque_("forceOpaque", "Shade Opaque", false)
    , drawSilhouette_("drawSilhouette", "Draw Silhouette")
    , silhouetteColor_("silhouetteColor", "Silhouette Color", {0.f, 0.f, 0.f, 1.f})
    , normalSource_(
          "normalSource", "Normals Source",
          {
              {"inputVertex", "Input: Vertex Normal", NormalSource::InputVertex},
              {"generateVertex", "Generate: Vertex Normal", NormalSource::GenerateVertex},
              {"generateTriangle", "Generate: Triangle Normal", NormalSource::GenerateTriangle},
          },
          0)
    , normalComputationMode_(
          "normalComputationMode", "Normals Computation",
          {{"noWeighting", "No Weighting", meshutil::CalculateMeshNormalsMode::NoWeighting},
           {"area", "Area-weighting", meshutil::CalculateMeshNormalsMode::WeightArea},
           {"angle", "Angle-weighting", meshutil::CalculateMeshNormalsMode::WeightAngle},
           {"nmax", "Based on N.Max", meshutil::CalculateMeshNormalsMode::WeightNMax}},
          3)
    , faceSettings_{true, false}
    , propUseIllustrationBuffer_("illustrationBuffer", "Use Illustration Buffer")
    , propDebugFragmentLists_("debugFL", "Debug Fragment Lists")
    , debugFragmentLists_(false)
    , meshHasAdjacency_(false)
    , shader_("fancymeshrenderer.vert", "fancymeshrenderer.geom", "fancymeshrenderer.frag", false)
    , depthShader_("geometryrendering.vert", "depthonly.frag", false)
    , needsRecompilation_(true) {

    // query OpenGL Capability
    supportsFragmentLists_ = FragmentListRenderer::supportsFragmentLists();
    supportedIllustrationBuffer_ = FragmentListRenderer::supportsIllustrationBuffer();
    if (!supportsFragmentLists_) {
        LogProcessorWarn(
            "Fragment lists are not supported by the hardware -> use blending without sorting, may "
            "lead to errors");
    }
    if (!supportedIllustrationBuffer_) {
        LogProcessorWarn(
            "Illustration Buffer not supported by the hardware, screen-space silhouettes not "
            "available");
    }

    // input and output ports
    addPort(inport_);
    addPort(imageInport_).setOptional(true);
    addPort(outport_);

    inport_.onChange([this]() { updateMeshes(); });

    addProperties(camera_, lightingProperty_, trackball_, forceOpaque_, drawSilhouette_,
                  silhouetteColor_);

    if (supportedIllustrationBuffer_) {
        addProperties(propUseIllustrationBuffer_, illustrationBufferSettings_.container_);
    }

    addProperties(normalSource_, normalComputationMode_, alphaSettings_.container_,
                  edgeSettings_.container_, faceSettings_[0].container_,
                  faceSettings_[1].container_);

    camera_.setCollapsed(true);
    lightingProperty_.setCollapsed(true);
    trackball_.setCollapsed(true);

    silhouetteColor_.setSemantics(PropertySemantics::Color);

    // Callbacks
    shader_.onReload([this]() { invalidate(InvalidationLevel::InvalidResources); });
    const auto triggerRecompilation = [this]() {
        needsRecompilation_ = true;
        update();
    };

    const auto triggerMeshUpdate = [this]() {
        needsRecompilation_ = true;
        update();
        updateMeshes();
    };

    drawSilhouette_.onChange(triggerMeshUpdate);
    normalSource_.onChange(triggerMeshUpdate);
    normalComputationMode_.onChange(triggerMeshUpdate);

    forceOpaque_.onChange(triggerRecompilation);
    alphaSettings_.setCallbacks(triggerRecompilation);
    edgeSettings_.setCallbacks(triggerRecompilation);
    faceSettings_[0].setCallbacks(triggerRecompilation);
    faceSettings_[1].setCallbacks(triggerRecompilation);

    faceSettings_[1].frontPart_ = &faceSettings_[0];

    // DEBUG, in case we need debugging fragment lists at a later point again
    // addProperty(propDebugFragmentLists_);  // DEBUG, to be removed
    // propDebugFragmentLists_.onChange([this]() { debugFragmentLists_ = true; });

    // update visibility of properties
    update();

    // Compile depth-only shader
    // Why this is needed, see the end of process()
    depthShader_.build();
}

FancyMeshRenderer::AlphaSettings::AlphaSettings()
    : container_("alphaContainer", "Alpha")
    , enableUniform_("alphaUniform", "Uniform", true)
    , uniformScaling_("alphaUniformScaling", "Scaling", 0.5f, 0.f, 1.f, 0.01f)
    , enableAngleBased_("alphaAngleBased", "Angle-based", false)
    , angleBasedExponent_("alphaAngleBasedExponent", "Exponent", 1.f, 0.f, 5.f, 0.01f)
    , enableNormalVariation_("alphaNormalVariation", "Normal variation", false)
    , normalVariationExponent_("alphaNormalVariationExponent", "Exponent", 1.f, 0.f, 5.f, 0.01f)
    , enableDensity_("alphaDensity", "Density-based", false)
    , baseDensity_("alphaBaseDensity", "Base density", 1.f, 0.f, 2.f, 0.01f)
    , densityExponent_("alphaDensityExponent", "Exponent", 1.f, 0.f, 5.f, 0.01f)
    , enableShape_("alphaShape", "Shape-based", false)
    , shapeExponent_("alphaShapeExponent", "Exponent", 1.f, 0.f, 5.f, 0.01f) {
    container_.addProperties(enableUniform_, uniformScaling_, enableAngleBased_,
                             angleBasedExponent_, enableNormalVariation_, normalVariationExponent_,
                             enableDensity_, baseDensity_, densityExponent_, enableShape_,
                             shapeExponent_);
}

void FancyMeshRenderer::AlphaSettings::setCallbacks(
    const std::function<void()>& triggerRecompilation) {
    enableUniform_.onChange(triggerRecompilation);
    enableAngleBased_.onChange(triggerRecompilation);
    enableNormalVariation_.onChange(triggerRecompilation);
    enableDensity_.onChange(triggerRecompilation);
    enableShape_.onChange(triggerRecompilation);
}

void FancyMeshRenderer::AlphaSettings::update() {
    uniformScaling_.setVisible(enableUniform_.get());
    angleBasedExponent_.setVisible(enableAngleBased_.get());
    normalVariationExponent_.setVisible(enableNormalVariation_.get());
    baseDensity_.setVisible(enableDensity_.get());
    densityExponent_.setVisible(enableDensity_.get());
    shapeExponent_.setVisible(enableShape_.get());
}

FancyMeshRenderer::EdgeSettings::EdgeSettings()
    : container_("edges", "Edges")
    , edgeThickness_("edgesThickness", "Thickness", 2.f, 0.1f, 10.f, 0.1f)
    , depthDependent_("edgesDepth", "Depth dependent", false)
    , smoothEdges_("edgesSmooth", "Smooth edges", true) {
    container_.addProperties(edgeThickness_, depthDependent_, smoothEdges_);
}

void FancyMeshRenderer::EdgeSettings::setCallbacks(
    const std::function<void()>& triggerRecompilation) {
    depthDependent_.onChange(triggerRecompilation);
    smoothEdges_.onChange(triggerRecompilation);
}

void FancyMeshRenderer::EdgeSettings::update() {
    // do nothing
}

FancyMeshRenderer::HatchingSettings::HatchingSettings(const std::string& prefix)
    : mode_(prefix + "hatchingMode", "Hatching",
            {{"off", "Off", HatchingMode::Off},
             {"u", "U", HatchingMode::U},
             {"v", "V", HatchingMode::V},
             {"uv", "UV", HatchingMode::UV}})
    , container_(prefix + "hatchingContainer", "Hatching Settings")
    , steepness_(prefix + "hatchingSteepness", "Steepness", 5, 1, 10)
    , baseFrequencyU_(prefix + "hatchingFrequencyU", "U-Frequency", 3, 1, 10)
    , baseFrequencyV_(prefix + "hatchingFrequencyV", "V-Frequency", 3, 1, 10)
    , modulationMode_(prefix + "hatchingModulationMode", "Modulation",
                      {{"off", "Off", HatchingMode::Off},
                       {"u", "U", HatchingMode::U},
                       {"v", "V", HatchingMode::V},
                       {"uv", "UV", HatchingMode::UV}})
    , modulationAnisotropy_(prefix + "hatchingModulationAnisotropy", "Anisotropy", 0.5f, -1.f, 1.f,
                            0.01f)
    , modulationOffset_(prefix + "hatchingModulationOffset", "Offset", 0.f, 0.f, 1.f, 0.01f)
    , color_(prefix + "hatchingColor", "Color", {0.f, 0.f, 0.f})
    , strength_(prefix + "hatchingStrength", "Strength", 0.5f, 0.f, 1.f, 0.01f)
    , blendingMode_(prefix + "hatchingBlending", "Blending",
                    {{"mult", "Multiplicative", HatchingBlendingMode::Multiplicative},
                     {"add", "Additive", HatchingBlendingMode::Additive}}) {
    // init properties
    color_.setSemantics(PropertySemantics::Color);

    // add to container
    container_.addProperties(steepness_, baseFrequencyU_, baseFrequencyV_, modulationMode_,
                             modulationAnisotropy_, modulationOffset_, color_, strength_,
                             blendingMode_);
}  // namespace inviwo

FancyMeshRenderer::FaceRenderSettings::FaceRenderSettings(bool frontFace)
    : frontFace_(frontFace)
    , prefix_(frontFace ? "front" : "back")
    , container_(prefix_ + "container", frontFace ? "Front Face" : "Back Face")
    , show_(prefix_ + "show", "Show", true)
    , sameAsFrontFace_(prefix_ + "same", "Same as Front Face")
    , copyFrontToBack_(prefix_ + "copy", "Copy Front to Back")
    , transferFunction_(prefix_ + "tf", "Transfer Function")
    , externalColor_(prefix_ + "extraColor", "Color", {1.f, 0.3f, 0.01f})
    , colorSource_(prefix_ + "colorSource", "Color Source",
                   {{"vertexColor", "VertexColor", ColorSource::VertexColor},
                    {"tf", "Transfer Function", ColorSource::TransferFunction},
                    {"external", "Constant Color", ColorSource::ExternalColor}},
                   2)
    , separateUniformAlpha_(prefix_ + "separateUniformAlpha", "Separate Uniform Alpha")
    , uniformAlpha_(prefix_ + "uniformAlpha", "Uniform Alpha", 0.5f, 0.f, 1.f, 0.01f)
    , shadingMode_(prefix_ + "shadingMode", "Shading Mode",
                   {
                       {"off", "Off", ShadingMode::Off},
                       {"phong", "Phong", ShadingMode::Phong},
                       {"pbr", "PBR", ShadingMode::Pbr},
                   })
    , showEdges_(prefix_ + "showEdges", "Show Edges")
    , edgeColor_(prefix_ + "edgeColor", "Edge color", {0.f, 0.f, 0.f})
    , edgeOpacity_(prefix_ + "edgeOpacity", "Edge Opacity", 0.5f, 0.f, 2.f, 0.01f)
    , hatching_(prefix_) {
    // initialize combo boxes

    externalColor_.setSemantics(PropertySemantics::Color);
    edgeColor_.setSemantics(PropertySemantics::Color);

    // layouting, add the properties
    container_.addProperty(show_);
    if (!frontFace) {
        container_.addProperties(sameAsFrontFace_, copyFrontToBack_);
        copyFrontToBack_.onChange([this]() { copyFrontToBack(); });
    }
    container_.addProperties(colorSource_, transferFunction_, externalColor_, separateUniformAlpha_,
                             uniformAlpha_, shadingMode_, showEdges_, edgeColor_, edgeOpacity_,
                             hatching_.mode_, hatching_.container_);

    // set callbacks that will trigger update()
    auto triggerUpdate = [this]() { update(lastOpaque_); };
    show_.onChange(triggerUpdate);
    sameAsFrontFace_.onChange(triggerUpdate);
    colorSource_.onChange(triggerUpdate);
    separateUniformAlpha_.onChange(triggerUpdate);
    hatching_.mode_.onChange(triggerUpdate);
    hatching_.steepness_.onChange(triggerUpdate);
    hatching_.baseFrequencyU_.onChange(triggerUpdate);
    hatching_.baseFrequencyV_.onChange(triggerUpdate);
    hatching_.color_.onChange(triggerUpdate);
    hatching_.blendingMode_.onChange(triggerUpdate);
    hatching_.modulationMode_.onChange(triggerUpdate);
}

void FancyMeshRenderer::FaceRenderSettings::copyFrontToBack() {
    transferFunction_.set(frontPart_->transferFunction_.get());
    externalColor_.set(frontPart_->externalColor_.get());
    externalColor_.set(frontPart_->externalColor_.get());
    colorSource_.set(frontPart_->colorSource_.get());
    separateUniformAlpha_.set(frontPart_->separateUniformAlpha_.get());
    uniformAlpha_.set(frontPart_->uniformAlpha_.get());
    shadingMode_.set(frontPart_->shadingMode_.get());
    showEdges_.set(frontPart_->showEdges_.get());
    edgeColor_.set(frontPart_->edgeColor_.get());
    edgeOpacity_.set(frontPart_->edgeOpacity_.get());
    hatching_.mode_.set(frontPart_->hatching_.mode_.get());
    hatching_.steepness_.set(frontPart_->hatching_.steepness_.get());
    hatching_.baseFrequencyU_.set(frontPart_->hatching_.baseFrequencyU_.get());
    hatching_.baseFrequencyV_.set(frontPart_->hatching_.baseFrequencyV_.get());
    hatching_.modulationMode_.set(frontPart_->hatching_.modulationMode_.get());
    hatching_.modulationAnisotropy_.set(frontPart_->hatching_.modulationAnisotropy_.get());
    hatching_.modulationOffset_.set(frontPart_->hatching_.modulationOffset_.get());
    hatching_.color_.set(frontPart_->hatching_.color_.get());
    hatching_.blendingMode_.set(frontPart_->hatching_.blendingMode_.get());
}

void FancyMeshRenderer::FaceRenderSettings::update(bool opaque) {
    lastOpaque_ = opaque;
    // fetch properties
    bool show = show_.get();
    bool show2 = show && !sameAsFrontFace_.get();
    ColorSource colorSource = colorSource_.get();
    bool separateUniformAlpha = separateUniformAlpha_.get();
    bool showEdges = showEdges_.get();
    bool hatching = hatching_.mode_.get() != HatchingMode::Off;

    // set visibility
    sameAsFrontFace_.setVisible(show);
    copyFrontToBack_.setVisible(show);
    colorSource_.setVisible(show2);
    transferFunction_.setVisible(show2 && colorSource == ColorSource::TransferFunction);
    externalColor_.setVisible(show2 && colorSource == ColorSource::ExternalColor);
    separateUniformAlpha_.setVisible(show2 && !opaque);
    uniformAlpha_.setVisible(show2 && !opaque && separateUniformAlpha);
    shadingMode_.setVisible(show2);
    showEdges_.setVisible(show2);
    edgeColor_.setVisible(show2 && showEdges);
    edgeOpacity_.setVisible(show2 && showEdges);
    hatching_.mode_.setVisible(show2);
    hatching_.container_.setVisible(show2 && hatching);
    hatching_.baseFrequencyU_.setVisible(hatching_.mode_.get() != HatchingMode::V);
    hatching_.baseFrequencyV_.setVisible(hatching_.mode_.get() != HatchingMode::U);
    hatching_.modulationMode_.setVisible(hatching_.mode_.get() == HatchingMode::UV);
    hatching_.modulationAnisotropy_.setVisible(hatching_.mode_.get() == HatchingMode::UV &&
                                               hatching_.modulationMode_.get() !=
                                                   HatchingMode::Off);
    hatching_.modulationOffset_.setVisible(hatching_.mode_.get() == HatchingMode::UV &&
                                           hatching_.modulationMode_.get() != HatchingMode::Off);
}

void FancyMeshRenderer::FaceRenderSettings::setCallbacks(
    const std::function<void()>& triggerRecompilation) {
    showEdges_.onChange(triggerRecompilation);
    colorSource_.onChange(triggerRecompilation);
    hatching_.mode_.onChange(triggerRecompilation);
}

FancyMeshRenderer::IllustrationBufferSettings::IllustrationBufferSettings()
    : container_("illustrationBufferContainer", "Illustration Buffer Settings")
    , edgeColor_("illustrationBufferEdgeColor", "Edge Color", vec3(0.f, 0.f, 0.f))
    , edgeStrength_("illustrationBufferEdgeStrength", "Edge Strength", 0.5f, 0.f, 1.f, 0.01f)
    , haloStrength_("illustrationBufferHaloStrength", "Halo Strength", 0.5f, 0.f, 1.f, 0.01f)
    , smoothingSteps_("illustrationBufferSmoothingSteps", "Smoothing Steps", 3, 0, 50, 1)
    , edgeSmoothing_("illustrationBufferEdgeSmoothing", "Edge Smoothing", 0.8f, 0.f, 1.f, 0.01f)
    , haloSmoothing_("illustrationBufferHaloSmoothing", "Halo Smoothing", 0.8f, 0.f, 1.f, 0.01f) {
    edgeColor_.setSemantics(PropertySemantics::Color);
    container_.addProperties(edgeColor_, edgeStrength_, haloStrength_, smoothingSteps_,
                             edgeSmoothing_, haloSmoothing_);
}

void FancyMeshRenderer::initializeResources() {}

void FancyMeshRenderer::update() {
    // fetch all booleans
    bool opaque = forceOpaque_.get();

    // set top-level visibility
    alphaSettings_.container_.setVisible(!opaque);
    edgeSettings_.container_.setVisible(drawSilhouette_.get() ||
                                        faceSettings_[0].showEdges_.get() ||
                                        faceSettings_[1].showEdges_.get());

    // update nested settings
    alphaSettings_.update();
    edgeSettings_.update();
    faceSettings_[0].update(opaque);
    faceSettings_[1].update(opaque);
    propUseIllustrationBuffer_.setVisible(!opaque);
    illustrationBufferSettings_.container_.setVisible(!opaque && propUseIllustrationBuffer_.get());

    // update other
    silhouetteColor_.setVisible(drawSilhouette_.get());
    normalComputationMode_.setVisible(normalSource_.get() == NormalSource::GenerateVertex);
}

void FancyMeshRenderer::compileShader() {
    if (!needsRecompilation_) return;

    auto fso = shader_.getFragmentShaderObject();

    fso->addShaderExtension("GL_NV_gpu_shader5", true);
    fso->addShaderExtension("GL_EXT_shader_image_load_store", true);
    fso->addShaderExtension("GL_NV_shader_buffer_load", true);
    fso->addShaderExtension("GL_NV_shader_buffer_store", true);
    fso->addShaderExtension("GL_EXT_bindable_uniform", true);

    // shading defines
    utilgl::addShaderDefines(shader_, lightingProperty_);

    const std::array<std::pair<std::string, bool>, 15> defines = {
        {{"USE_FRAGMENT_LIST", !forceOpaque_},
         {"COLOR_LAYER", true},
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
         {"SEND_TEX_COORD", faceSettings_[0].hatching_.mode_ != HatchingMode::Off ||
                                faceSettings_[1].hatching_.mode_ != HatchingMode::Off},
         {"SEND_SCALAR", faceSettings_[0].colorSource_ == ColorSource::TransferFunction ||
                             faceSettings_[1].colorSource_ == ColorSource::TransferFunction},
         {"SEND_COLOR", faceSettings_[0].colorSource_ == ColorSource::VertexColor ||
                            faceSettings_[1].colorSource_ == ColorSource::VertexColor}}};

    for (auto&& [key, val] : defines) {
        for (auto&& so : shader_.getShaderObjects()) {
            so.setShaderDefine(key, val);
        }
    }

    shader_.build();

    needsRecompilation_ = false;
}

void FancyMeshRenderer::process() {
    // I have to call update here, otherwise, when you load a saved workspace,
    // the visibility of the properties is not updated on startup.
    update();

    utilgl::activateTargetAndClearOrCopySource(outport_, imageInport_);

    if (!faceSettings_[0].show_ && !faceSettings_[1].show_) {
        utilgl::deactivateCurrentTarget();
        return;  // everything is culled
    }
    const bool opaque = forceOpaque_.get();
    const bool fragmentLists = !opaque && supportsFragmentLists_;

    compileShader();

    // time measures
    glFinish();
    // auto start = std::chrono::steady_clock::now();

    // Loop: fragment list may need another try if not enough space for the pixels was available
    bool retry = false;
    do {
        retry = false;

        if (fragmentLists) {
            // prepare fragment list rendering
            flr_.prePass(outport_.getDimensions());
        }

        shader_.activate();

        // various OpenGL states: depth, blending, culling
        utilgl::GlBoolState depthTest(GL_DEPTH_TEST, opaque);
        utilgl::DepthMaskState depthMask(opaque ? GL_TRUE : GL_FALSE);
        utilgl::CullFaceState culling(
            !faceSettings_[0].show_ && faceSettings_[1].show_
                ? GL_FRONT
                : faceSettings_[0].show_ && !faceSettings_[1].show_ ? GL_BACK : GL_NONE);
        utilgl::BlendModeState blendModeStateGL(opaque ? GL_ONE : GL_SRC_ALPHA,
                                                opaque ? GL_ZERO : GL_ONE_MINUS_SRC_ALPHA);

        // general settings for camera, lighting, picking, mesh data
        utilgl::setUniforms(shader_, camera_, lightingProperty_);
        shader_.setUniform("halfScreenSize", ivec2(outport_.getDimensions()) / ivec2(2));

        // update face render settings
        std::array<TextureUnit, 2> transFuncUnit;
        for (int j = 0; j < faceSettings_.size(); ++j) {
            auto& face = faceSettings_[faceSettings_[1].sameAsFrontFace_.get() ? 0 : j];
            const std::string prefix = fmt::format("renderSettings[{}].", j);
            shader_.setUniform(prefix + "externalColor", vec4(face.externalColor_.get(), 1.0));
            shader_.setUniform(prefix + "colorSource", static_cast<int>(face.colorSource_.get()));
            shader_.setUniform(prefix + "separateUniformAlpha", face.separateUniformAlpha_.get());
            shader_.setUniform(prefix + "uniformAlpha", face.uniformAlpha_.get());
            shader_.setUniform(prefix + "shadingMode", static_cast<int>(face.shadingMode_.get()));
            shader_.setUniform(prefix + "showEdges", face.showEdges_.get());
            shader_.setUniform(prefix + "edgeColor",
                               vec4(face.edgeColor_.get(), face.edgeOpacity_.get()));
            if (face.hatching_.mode_.get() == HatchingMode::UV) {
                shader_.setUniform(prefix + "hatchingMode",
                                   3 + static_cast<int>(face.hatching_.modulationMode_.get()));
            } else {
                shader_.setUniform(prefix + "hatchingMode",
                                   static_cast<int>(face.hatching_.mode_.get()));
            }
            shader_.setUniform(prefix + "hatchingSteepness", face.hatching_.steepness_.get());
            shader_.setUniform(prefix + "hatchingFreqU",
                               face.hatching_.baseFrequencyU_.getMaxValue() -
                                   face.hatching_.baseFrequencyU_.get());
            shader_.setUniform(prefix + "hatchingFreqV",
                               face.hatching_.baseFrequencyV_.getMaxValue() -
                                   face.hatching_.baseFrequencyV_.get());
            shader_.setUniform(prefix + "hatchingModulationAnisotropy",
                               face.hatching_.modulationAnisotropy_.get());
            shader_.setUniform(prefix + "hatchingModulationOffset",
                               face.hatching_.modulationOffset_.get());
            shader_.setUniform(prefix + "hatchingColor",
                               vec4(face.hatching_.color_.get(), face.hatching_.strength_.get()));
            shader_.setUniform(prefix + "hatchingBlending",
                               static_cast<int>(face.hatching_.blendingMode_.get()));

            const Layer* tfLayer = face.transferFunction_->getData();
            const LayerGL* transferFunctionGL = tfLayer->getRepresentation<LayerGL>();
            transferFunctionGL->bindTexture(transFuncUnit[j].getEnum());
            shader_.setUniform(fmt::format("transferFunction{}", j),
                               transFuncUnit[j].getUnitNumber());
        }

        // update alpha settings
        shader_.setUniform("alphaSettings.uniformScale", alphaSettings_.uniformScaling_.get());
        shader_.setUniform("alphaSettings.angleExp", alphaSettings_.angleBasedExponent_.get());
        shader_.setUniform("alphaSettings.normalExp",
                           alphaSettings_.normalVariationExponent_.get());
        shader_.setUniform("alphaSettings.baseDensity", alphaSettings_.baseDensity_.get());
        shader_.setUniform("alphaSettings.densityExp", alphaSettings_.densityExponent_.get());
        shader_.setUniform("alphaSettings.shapeExp", alphaSettings_.shapeExponent_.get());

        // update other global fragment shader settings
        shader_.setUniform("silhouetteColor", silhouetteColor_.get());

        // update geometry shader settings
        shader_.setUniform("geomSettings.edgeWidth", edgeSettings_.edgeThickness_.get());
        shader_.setUniform("geomSettings.triangleNormal",
                           normalSource_.get() == NormalSource::GenerateTriangle);

        if (fragmentLists) {
            flr_.setShaderUniforms(shader_);  // set uniforms fragment list rendering
        }

        // Finally, draw it
        for (auto mesh : enhancedMeshes_) {
            MeshDrawerGL::DrawObject drawer{mesh->getRepresentation<MeshGL>(),
                                            mesh->getDefaultMeshInfo()};
            utilgl::setShaderUniforms(shader_, *mesh, "geometry");
            shader_.setUniform("pickingEnabled", meshutil::hasPickIDBuffer(mesh.get()));

            drawer.draw();
        }

        shader_.deactivate();

        if (fragmentLists) {
            // final processing of fragment list rendering
            const bool illustrationBuffer =
                propUseIllustrationBuffer_.get() && supportedIllustrationBuffer_;
            if (illustrationBuffer) {
                FragmentListRenderer::IllustrationBufferSettings settings;
                settings.edgeColor_ = illustrationBufferSettings_.edgeColor_.get();
                settings.edgeStrength_ = illustrationBufferSettings_.edgeStrength_.get();
                settings.haloStrength_ = illustrationBufferSettings_.haloStrength_.get();
                settings.smoothingSteps_ = illustrationBufferSettings_.smoothingSteps_.get();
                settings.edgeSmoothing_ = illustrationBufferSettings_.edgeSmoothing_.get();
                settings.haloSmoothing_ = illustrationBufferSettings_.haloSmoothing_.get();
                flr_.setIllustrationBufferSettings(settings);
            }
            retry = !flr_.postPass(illustrationBuffer, debugFragmentLists_);
            debugFragmentLists_ = false;
        }
    } while (retry);

    // report elapsed time
    glFinish();
    // auto finish = std::chrono::steady_clock::now();
    // double elapsed = std::chrono::duration_cast<std::chrono::duration<double> >(finish -
    // start).count() * 1000; LogProcessorInfo("Time: " << elapsed << "ms");

    // Workaround for a problem with the fragment lists:
    // The camera interaction requires the depth buffer for some reason to work,
    // otherwise, the rotation does not work.
    // My first idea was to set the depth in the fragment list's 'dispABufferLinkedList.frag'
    // using gl_FragDepth, but this don't work (yet).
    if (fragmentLists) {
        depthShader_.activate();
        utilgl::GlBoolState depthTest(GL_DEPTH_TEST, true);
        utilgl::DepthMaskState depthMask(GL_TRUE);
        utilgl::CullFaceState culling(
            !faceSettings_[0].show_ && faceSettings_[1].show_
                ? GL_FRONT
                : faceSettings_[0].show_ && !faceSettings_[1].show_ ? GL_BACK : GL_NONE);
        utilgl::BlendModeState blendModeStateGL(GL_ZERO, GL_ZERO);
        utilgl::setUniforms(depthShader_, camera_);

        for (auto mesh : enhancedMeshes_) {
            MeshDrawerGL::DrawObject drawer{mesh->getRepresentation<MeshGL>(),
                                            mesh->getDefaultMeshInfo()};
            utilgl::setShaderUniforms(depthShader_, *mesh, "geometry");
            drawer.draw();
        }

        depthShader_.deactivate();
    }

    utilgl::deactivateCurrentTarget();
}

void FancyMeshRenderer::updateMeshes() {
    enhancedMeshes_.clear();
    for (auto mesh : inport_) {
        std::shared_ptr<Mesh> copy = nullptr;

        if (drawSilhouette_) {
            copy = std::make_shared<Mesh>();
            for (auto&& [info, buffer] : mesh->getBuffers()) {
                copy->addBuffer(info, std::shared_ptr<BufferBase>(buffer->clone()));
            }

            // create adjacency information
            const auto halfEdges = HalfEdges{*mesh};

            // add new index buffer with adjacency information
            copy->addIndices(
                {DrawType::Triangles, ConnectivityType::Adjacency},
                std::make_shared<IndexBuffer>(halfEdges.createIndexBufferWithAdjacency()));

            meshHasAdjacency_ = true;

            LogProcessorInfo("Adjacency information created");
            LogProcessorInfo("draw mesh with adjacency information");
        } else {
            meshHasAdjacency_ = false;
        }

        if (normalSource_.get() == NormalSource::GenerateVertex) {
            if (!copy) copy = std::shared_ptr<Mesh>(mesh->clone());
            meshutil::calculateMeshNormals(*copy, normalComputationMode_);
        }

        enhancedMeshes_.push_back(copy ? copy : mesh);
    }

    // trigger shader recompilation
    // Geometry shader needs to know if it has adjacency or not
    needsRecompilation_ = true;
}

}  // namespace inviwo
