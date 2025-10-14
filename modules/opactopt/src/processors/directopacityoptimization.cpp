/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2024 Inviwo Foundation
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

#include <modules/opactopt/processors/directopacityoptimization.h>

#include <inviwo/core/algorithm/boundingbox.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/processors/processorinfo.h>
#include <inviwo/core/processors/processorstate.h>
#include <inviwo/core/processors/processortags.h>
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/properties/constraintbehavior.h>
#include <inviwo/core/properties/invalidationlevel.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/property.h>
#include <inviwo/core/properties/propertysemantics.h>
#include <modules/opengl/inviwoopengl.h>
#include <modules/opengl/volume/volumeutils.h>
#include <modules/opengl/rendering/meshdrawergl.h>
#include <modules/opengl/shader/shader.h>
#include <modules/opengl/shader/shaderutils.h>
#include <modules/opengl/texture/textureutils.h>
#include <modules/opactopt/utils/gaussian.h>

#include <cstddef>
#include <string>
#include <utility>
#include <ranges>

namespace inviwo {

namespace {
static constexpr std::string_view errorMsg =
    "The OpenGL extension for image load store operations "
    "(GL_EXT_shader_image_load_store) is required.";
}

const ProcessorInfo OpacityOptimization::processorInfo_{
    "org.inviwo.OpacityOptimization",  // Class identifier
    "Opacity Optimization",            // Display name
    "Mesh Rendering",                  // Category
    CodeState::Experimental,           // Code state
    Tags::GL | Tag{"GL4.2"},           // Tags
    R"(Performs approximate opacity optimization using a direct
    rendering approach. The processor takes a mesh as input,
    and optionally an importance volume and background
    texture. The output is an opacity optimized image.)"_unindentHelp,
};

const ProcessorInfo& OpacityOptimization::getProcessorInfo() const { return processorInfo_; }

OpacityOptimization::OpacityOptimization()
    : Processor()
    , inport_("geometry", "Input meshes"_help)
    , backgroundPort_("imageInport", "Background image (optional)"_help)
    , outport_("image", "Output image containing the opacity optimised mesh"_help)
    , intermediateImage_({0, 0}, inviwo::DataVec4Float32::get())
    , camera_("camera", "Camera", util::boundingBox(inport_))
    , meshProperties_("geometry", "Mesh Rendering Properties")
    , overrideColorBuffer_("overrideColorBuffer", "Override Color Buffer", false,
                           InvalidationLevel::InvalidResources)
    , overrideColor_("overrideColor", "Override Color", util::ordinalColor(0.75f, 0.75f, 0.75f))
    , lineSettings_("lineSettings", "Line Rendering Properties")
    , pointProperties_("points", "Point Rendering Properties")
    , pointSize_("pointSize", "Point Size (pixel)", 1.0f, 0.00001f, 50.0f, 0.1f)
    , borderWidth_("borderWidth", "Border Width (pixel)", 2.0f, 0.0f, 50.0f, 0.1f)
    , borderColor_("borderColor", "Border Color", vec4(0.0f, 0.0f, 0.0f, 1.0f), vec4(0.0f),
                   vec4(1.0f), vec4(0.01f), InvalidationLevel::InvalidOutput,
                   PropertySemantics::Color)
    , antialising_("antialising", "Antialising (pixel)", 1.5f, 0.0f, 10.0f, 0.1f)
    , lightingProperty_("lighting", "Lighting", &camera_)
    , trackball_(&camera_)
    , meshShaders_{{{"meshrendering.vert", "opactopt/mesh/projectimportance.frag",
                     Shader::Build::No},
                    {"meshrendering.vert", "opactopt/mesh/approximportancesum.frag",
                     Shader::Build::No},
                    {"meshrendering.vert", "opactopt/mesh/approxblending.frag", Shader::Build::No}}}
    , lineShaders_{{{"opactopt/line/linerenderer.vert", "linerenderer.geom",
                     "opactopt/line/projectimportance.frag", Shader::Build::No},
                    {"opactopt/line/linerenderer.vert", "linerenderer.geom",
                     "opactopt/line/approximportancesum.frag", Shader::Build::No},
                    {"opactopt/line/linerenderer.vert", "linerenderer.geom",
                     "opactopt/line/approxblending.frag", Shader::Build::No}}}
    , lineAdjacencyShaders_{{{"opactopt/line/linerenderer.vert", "linerenderer.geom",
                              "opactopt/line/projectimportance.frag", Shader::Build::No},
                             {"opactopt/line/linerenderer.vert", "linerenderer.geom",
                              "opactopt/line/approximportancesum.frag", Shader::Build::No},
                             {"opactopt/line/linerenderer.vert", "linerenderer.geom",
                              "opactopt/line/approxblending.frag", Shader::Build::No}}}
    , pointShaders_{{{"opactopt/point/basicpointrenderer.vert",
                      "opactopt/point/projectimportance.frag", Shader::Build::No},
                     {"opactopt/point/basicpointrenderer.vert",
                      "opactopt/point/approximportancesum.frag", Shader::Build::No},
                     {"opactopt/point/basicpointrenderer.vert",
                      "opactopt/point/approxblending.frag", Shader::Build::No}}}
    , smooth_{{{"minimal.vert", "opactopt/smooth.frag", Shader::Build::No},
               {"minimal.vert", "opactopt/smooth.frag", Shader::Build::No}}}
    , clear_{"minimal.vert", "opactopt/clear.frag", Shader::Build::No}
    , normalize_{"minimal.vert", "opactopt/normalise.frag", Shader::Build::No}
    , importanceVolume_{"importanceVolume", "Scalar field with importance data(optional)"_help}
    , occlusionReduction_{"occlusionReduction",
                          "Occlusion Reduction",
                          "Reduces occlusion in front of important data"_help,
                          0.0f,
                          std::make_pair(0.0f, ConstraintBehavior::Immutable),
                          std::make_pair(1000.0f, ConstraintBehavior::Ignore),
                          0.01f,
                          InvalidationLevel::InvalidOutput}
    , clutterReduction_{"clutterReduction",
                        "Clutter Reduction",
                        "Reduces clutter behind important data"_help,
                        0.0f,
                        std::make_pair(0.0f, ConstraintBehavior::Immutable),
                        std::make_pair(1000.0f, ConstraintBehavior::Ignore),
                        0.01f,
                        InvalidationLevel::InvalidOutput}
    , lambda_{"lambda",
              "Importance λ",
              "Controls emphasis of important structures"_help,
              1.0f,
              std::make_pair(0.01f, ConstraintBehavior::Editable),
              std::make_pair(100.0f, ConstraintBehavior::Editable),
              0.01f,
              InvalidationLevel::InvalidOutput}
    , approximationProperties_{"approximationProperties", "Approximation Properties",
                               "Controls approximation method, number of coefficients and "
                               "smoothing of coefficients"_help}
    , approximationMethod_{"approximationMethod",
                           "Approximation Method",
                           R"(Method used to approximate importance sum and optical depth function.
                              * Fourier approximation is usually the most balanced between 
                                performance and accuracy
                              * Piecewise is the fastest method.
                              * Different methods may work better on different datasets.
                           )"_unindentHelp,
                           approximations::generateApproximationStringOptions(),
                           0,
                           InvalidationLevel::InvalidResources}
    , importanceSumCoefficients_{"importanceSumCoefficients",
                                 "Importance sum coefficients",
                                 "Number of importance sum coefficients, more importance sum "
                                 "coefficients optimizes opacity more accurately"_help,
                                 5,
                                 std::make_pair(approximations::get(approximationMethod_).min,
                                                ConstraintBehavior::Immutable),
                                 std::make_pair(approximations::get(approximationMethod_).max,
                                                ConstraintBehavior::Immutable),
                                 approximations::get(approximationMethod_).inc,
                                 InvalidationLevel::InvalidResources}
    , opticalDepthCoefficients_{"opticalDepthCoefficients",
                                "Optical depth coefficients",
                                "Number of optical depth coefficients, more optical depth "
                                "coefficients gives more accurate blending"_help,
                                5,
                                std::make_pair(approximations::get(approximationMethod_).min,
                                               ConstraintBehavior::Immutable),
                                std::make_pair(approximations::get(approximationMethod_).max,
                                               ConstraintBehavior::Immutable),
                                approximations::get(approximationMethod_).inc,
                                InvalidationLevel::InvalidResources}
    , normalizedBlending_{"normalizedBlending", "Normalized blending",
                          "Normalize the approximate blending, "
                          "this reduces over- and under-saturation"_help,
                          true, InvalidationLevel::InvalidResources}
    , coeffTexFixedPointFactor_{"coeffTexFixedPointFactor",
                                "Coefficient texture fixed point factor",
                                "Fixed point multiplier "
                                "(only used if GL_NV_shader_atomic_float not supported)"_help,
                                1e6f,
                                std::make_pair(32.0f, ConstraintBehavior::Editable),
                                std::make_pair(1e10f, ConstraintBehavior::Editable),
                                1.0f,
                                InvalidationLevel::InvalidResources,
                                PropertySemantics::SpinBox}
    , imageFormat_{GLFormats::getGLFormat(
          OpenGLCapabilities::isExtensionSupported("GL_NV_shader_atomic_float") ? GL_FLOAT : GL_INT,
          1)}
    , importanceSumTexture_{{{size3_t(outport_.getDimensions(), importanceSumCoefficients_),
                              imageFormat_, GL_NEAREST},
                             {size3_t(outport_.getDimensions(), importanceSumCoefficients_),
                              imageFormat_, GL_NEAREST}}}
    , opticalDepthTexture_{size3_t(outport_.getDimensions(), opticalDepthCoefficients_),
                           imageFormat_, GL_NEAREST}
    , gaussianKernel_{static_cast<size_t>(gaussianKernelMaxRadius_ +
                                          1),  // allocate max possible size
                      GLFormats::getGLFormat(GL_FLOAT, 1), GL_NEAREST}
    , smoothing_{"smoothing", "Smoothing",
                 "Smooth the importance sum coefficients, this reduces clutter around important "
                 "structures"_help,
                 false}
    , gaussianRadius_{"gaussianKernelRadius", "Gaussian kernel radius", 3,
                      std::make_pair(1, ConstraintBehavior::Immutable),
                      std::make_pair(gaussianKernelMaxRadius_, ConstraintBehavior::Immutable)

      }
    , gaussianSigma_{"gaussianKernelSigma", "Gaussian kernel sigma", 1.0f, 0.001f, 50.0f}
    , legendreCoeffs_{[]() {
                          const auto max = static_cast<size_t>(
                              approximations::get(approximations::Type::Legendre).max);
                          return max * (max + 1) / 2;
                      }(),
                      GLFormats::getGLFormat(GL_FLOAT, 1), GL_NEAREST}
    , momentSettings_{2, GLFormats::getGLFormat(GL_FLOAT, 4), GL_NEAREST} {

    if (!OpenGLCapabilities::isExtensionSupported("GL_EXT_shader_image_load_store")) {
        isReady_.setUpdate([]() -> ProcessorStatus { return {ProcessorStatus::Error, errorMsg}; });
    }

    addPort(inport_);
    addPort(backgroundPort_).setOptional(true);
    addPort(importanceVolume_).setOptional(true);
    addPort(outport_);

    backgroundPort_.onConnect([&]() { invalidate(InvalidationLevel::InvalidResources); });
    backgroundPort_.onDisconnect([&]() { invalidate(InvalidationLevel::InvalidResources); });

    importanceVolume_.onConnect([&]() { invalidate(InvalidationLevel::InvalidResources); });
    importanceVolume_.onDisconnect([&]() { invalidate(InvalidationLevel::InvalidResources); });

    addProperties(camera_, occlusionReduction_, clutterReduction_, lambda_,
                  approximationProperties_, meshProperties_, lineSettings_, pointProperties_,
                  lightingProperty_, trackball_);

    approximationProperties_.addProperties(approximationMethod_, importanceSumCoefficients_,
                                           opticalDepthCoefficients_);
    if (!OpenGLCapabilities::isExtensionSupported("GL_NV_shader_atomic_float")) {
        approximationProperties_.addProperty(coeffTexFixedPointFactor_);
    }
    approximationProperties_.addProperties(normalizedBlending_, smoothing_);

    approximationMethod_.onChange([this]() {
        const auto& ap = approximations::get(approximationMethod_);
        importanceSumCoefficients_.setMinValue(ap.min);
        importanceSumCoefficients_.setMaxValue(ap.max);
        importanceSumCoefficients_.setIncrement(ap.inc);
        opticalDepthCoefficients_.setMinValue(ap.min);
        opticalDepthCoefficients_.setMaxValue(ap.max);
        opticalDepthCoefficients_.setIncrement(ap.inc);
    });

    smoothing_.addProperties(gaussianRadius_, gaussianSigma_);
    meshProperties_.addProperties(overrideColorBuffer_, overrideColor_);
    pointProperties_.addProperties(pointSize_, borderWidth_, borderColor_, antialising_);

    overrideColor_.setSemantics(PropertySemantics::Color)
        .visibilityDependsOn(overrideColorBuffer_, [](const BoolProperty& p) { return p.get(); });

    for (auto& shader : meshShaders_) {
        shader.onReload([this]() { invalidate(InvalidationLevel::InvalidResources); });
    }
    for (auto& shader : lineShaders_) {
        shader.onReload([this]() { invalidate(InvalidationLevel::InvalidResources); });
    }
    for (auto& shader : lineAdjacencyShaders_) {
        shader.onReload([this]() { invalidate(InvalidationLevel::InvalidResources); });
    }
    for (auto& shader : pointShaders_) {
        shader.onReload([this]() { invalidate(InvalidationLevel::InvalidResources); });
    }
    for (auto& shader : smooth_) {
        shader.onReload([this]() { invalidate(InvalidationLevel::InvalidResources); });
    }
    clear_.onReload([this]() { invalidate(InvalidationLevel::InvalidResources); });
    normalize_.onReload([this]() { invalidate(InvalidationLevel::InvalidResources); });
}

OpacityOptimization::~OpacityOptimization() = default;

void OpacityOptimization::setNetwork(ProcessorNetwork* network) {
    // Report the error in setNetwork, to avoid printing it when just constructing the processor
    if (!OpenGLCapabilities::isExtensionSupported("GL_EXT_shader_image_load_store")) {
        log::report(LogLevel::Error, errorMsg);
    }
    Processor::setNetwork(network);
}

void OpacityOptimization::initializeResources() {

    for (auto& ist : importanceSumTexture_) {
        ist.initialize(nullptr);
    }
    opticalDepthTexture_.initialize(nullptr);
    gaussianKernel_.initialize(nullptr);

    buildShaders();
    generateAndUploadGaussianKernel();
    generateAndUploadLegendreCoefficients();
    generateAndUploadMomentSettings();
}

void OpacityOptimization::buildShaders() {
    // Configure all the shaders with correct extensions and defines

    const auto clearAndAddExtensionsAndDefines = [&](ShaderObject* frag) {
        frag->clearShaderExtensions();
        frag->clearShaderDefines();

        frag->addShaderExtension("GL_EXT_shader_image_load_store", true);
        if (OpenGLCapabilities::isExtensionSupported("GL_NV_shader_atomic_float")) {
            frag->addShaderExtension("GL_NV_shader_atomic_float", true);
            frag->setShaderDefine("COEFF_TEX_ATOMIC_FLOAT", true, "");
        } else {
            frag->setShaderDefine("COEFF_TEX_FIXED_POINT_FACTOR", true,
                                  std::to_string(coeffTexFixedPointFactor_));
        }

        frag->setShaderDefine(approximations::get(approximationMethod_).define, true, "");

        frag->setShaderDefine("N_IMPORTANCE_SUM_COEFFICIENTS", true,
                              std::to_string(importanceSumCoefficients_));
        frag->setShaderDefine("N_OPTICAL_DEPTH_COEFFICIENTS", true,
                              std::to_string(opticalDepthCoefficients_));
    };

    for (auto& shader : meshShaders_) {
        auto* vert = shader.getVertexShaderObject();
        auto* frag = shader.getFragmentShaderObject();
        clearAndAddExtensionsAndDefines(frag);

        utilgl::addShaderDefines(shader, lightingProperty_);

        vert->setShaderDefine("OVERRIDE_COLOR_BUFFER", overrideColorBuffer_);
        frag->setShaderDefine("USE_IMPORTANCE_VOLUME", importanceVolume_.hasData());

        shader.build();
    }

    for (auto& shader : lineShaders_) {
        auto* vert = shader.getVertexShaderObject();
        auto* frag = shader.getFragmentShaderObject();
        clearAndAddExtensionsAndDefines(frag);

        utilgl::addShaderDefines(shader, lineSettings_.getStippling().getMode());

        vert->setShaderDefine("HAS_COLOR", true);
        frag->setShaderDefine("ENABLE_PSEUDO_LIGHTING", lineSettings_.getPseudoLighting());
        frag->setShaderDefine("ENABLE_ROUND_DEPTH_PROFILE", lineSettings_.getRoundDepthProfile());
        frag->setShaderDefine("USE_IMPORTANCE_VOLUME", importanceVolume_.hasData());

        shader.build();
    }

    for (auto& shader : lineAdjacencyShaders_) {
        auto* vert = shader.getVertexShaderObject();
        auto* geom = shader.getGeometryShaderObject();
        auto* frag = shader.getFragmentShaderObject();
        clearAndAddExtensionsAndDefines(frag);

        utilgl::addShaderDefines(shader, lineSettings_.getStippling().getMode());

        vert->setShaderDefine("HAS_COLOR", true);
        geom->setShaderDefine("ENABLE_ADJACENCY", true, "1");

        frag->setShaderDefine("ENABLE_PSEUDO_LIGHTING", lineSettings_.getPseudoLighting());
        frag->setShaderDefine("ENABLE_ROUND_DEPTH_PROFILE", lineSettings_.getRoundDepthProfile());
        frag->setShaderDefine("USE_IMPORTANCE_VOLUME", importanceVolume_.hasData());

        shader.build();
    }

    for (auto& shader : pointShaders_) {
        auto* frag = shader.getFragmentShaderObject();
        clearAndAddExtensionsAndDefines(frag);

        frag->setShaderDefine("USE_IMPORTANCE_VOLUME", importanceVolume_.hasData());

        shader.build();
    }

    {
        Shader& shader = clear_;
        auto* frag = shader.getFragmentShaderObject();
        clearAndAddExtensionsAndDefines(frag);
        shader.build();
    }

    {
        Shader& shader = normalize_;
        auto* frag = shader.getFragmentShaderObject();
        clearAndAddExtensionsAndDefines(frag);
        frag->setShaderDefine("NORMALISE", normalizedBlending_);
        frag->setShaderDefine("BACKGROUND_AVAILABLE", backgroundPort_.hasData());

        shader.build();
    }

    for (auto&& [shader, horizontal] : std::views::zip(smooth_, std::array{"1", "0"})) {
        auto* frag = shader.getFragmentShaderObject();
        clearAndAddExtensionsAndDefines(frag);
        frag->setShaderDefine("HORIZONTAL", true, horizontal);
        shader.build();
    }
}

void OpacityOptimization::process() {
    resizeImportanceSumTextures(outport_.getDimensions(), importanceSumCoefficients_);
    resizeOpticalDepthTexture(outport_.getDimensions(), opticalDepthCoefficients_);

    if (intermediateImage_.getDimensions() != outport_.getDimensions()) {
        intermediateImage_.setDimensions(outport_.getDimensions());
    }

    if (gaussianSigma_.isModified() || gaussianRadius_.isModified()) {
        generateAndUploadGaussianKernel();
    }

    // Bind textures
    Units units;

    units.importanceSumMain.activate();
    importanceSumTexture_[0].bind();
    glBindImageTexture(units.importanceSumMain.getUnitNumber(), importanceSumTexture_[0].getID(), 0,
                       true, 0, GL_READ_WRITE, imageFormat_.internalFormat);

    units.opticalDepth.activate();
    opticalDepthTexture_.bind();
    glBindImageTexture(units.opticalDepth.getUnitNumber(), opticalDepthTexture_.getID(), 0, true, 0,
                       GL_READ_WRITE, imageFormat_.internalFormat);

    if (smoothing_) {
        units.importanceSumSmooth.emplace();
        units.importanceSumSmooth->activate();
        importanceSumTexture_[1].bind();
        glBindImageTexture(units.importanceSumSmooth->getUnitNumber(),
                           importanceSumTexture_[1].getID(), 0, true, 0, GL_READ_WRITE,
                           imageFormat_.internalFormat);

        units.gaussianKernel.emplace();
        units.gaussianKernel->activate();
        gaussianKernel_.bind();
    }

    if (approximationMethod_ == Legendre) {
        units.legendreCoeffs.emplace();
        units.legendreCoeffs->activate();
        legendreCoeffs_.bind();
    }

    if (approximationMethod_ == PowerMoments || approximationMethod_ == TrigonometricMoments) {
        units.momentSettings.emplace();
        units.momentSettings->activate();
        momentSettings_.bind();
    }

    if (importanceVolume_.hasData()) {
        units.importanceVolume.emplace();
        utilgl::bindTexture(importanceVolume_, *units.importanceVolume);
    }

    utilgl::activateAndClearTarget(intermediateImage_);

    const utilgl::CullFaceState culling(GL_NONE);
    const utilgl::DepthFuncState depthFuncState(GL_LEQUAL);
    const utilgl::DepthMaskState depthMaskState(GL_TRUE);

    {
        // use first pass to write to depth buffer...
        const utilgl::GlBoolState depthTest(GL_DEPTH_TEST, GL_TRUE);

        // clear coefficient buffers
        clear_.activate();
        setUniforms(clear_, units);
        utilgl::singleDrawImagePlaneRect();
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

        // Pass 1: Project importance
        renderGeometry(Pass::ProjectImportance, units);
    }

    // ... then turn off depth test
    const utilgl::GlBoolState depthTest(GL_DEPTH_TEST, GL_FALSE);

    // Optional smoothing of importance coefficients
    if (units.gaussianKernel) {
        // horizontal pass and then vertical pass
        for (auto& shader : smooth_) {
            shader.activate();
            shader.setUniform("radius", gaussianRadius_);
            shader.setUniform("gaussianKernel", *units.gaussianKernel);
            setUniforms(shader, units);
            utilgl::singleDrawImagePlaneRect();
            glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        }
    }

    // Pass 2: Approximate importance and project opacities
    renderGeometry(Pass::ApproximateImportance, units);

    // Pass 3: Approximate blending, render to target
    const utilgl::BlendModeState blending(GL_ONE, GL_ONE, GL_ONE, GL_ONE);
    renderGeometry(Pass::ApproximateBlending, units);

    // normalize
    utilgl::activateAndClearTarget(outport_);
    glBlendFuncSeparate(GL_ONE, GL_ZERO, GL_ONE, GL_ZERO);

    normalize_.activate();
    TextureUnitContainer textureUnits;
    utilgl::bindAndSetUniforms(normalize_, textureUnits, intermediateImage_, "image",
                               ImageType::ColorDepth);
    if (backgroundPort_.hasData()) {
        utilgl::bindAndSetUniforms(normalize_, textureUnits, *backgroundPort_.getData(), "bg",
                                   ImageType::ColorDepthPicking);
    }
    setUniforms(normalize_, units);
    utilgl::singleDrawImagePlaneRect();

    glUseProgram(0);
    utilgl::deactivateCurrentTarget();
}

void OpacityOptimization::setUniforms(Shader& shader, Units& units) {
    shader.setUniform("screenSize", ivec2(outport_.getDimensions()));
    shader.setUniform("reciprocalDimensions", vec2(1.0f) / vec2(outport_.getDimensions()));

    shader.setUniform("q", occlusionReduction_);
    shader.setUniform("r", clutterReduction_);
    shader.setUniform("lambda", lambda_);

    shader.setUniform("importanceSumCoeffs[0]", units.importanceSumMain);
    shader.setUniform("opticalDepthCoeffs", units.opticalDepth);

    if (units.importanceSumSmooth) {
        shader.setUniform("importanceSumCoeffs[1]", *units.importanceSumSmooth);
    }
    if (units.legendreCoeffs) {
        shader.setUniform("legendreCoeffs", *units.legendreCoeffs);
    }
    if (units.momentSettings) {
        shader.setUniform("momentSettings", *units.momentSettings);
    }
    if (units.importanceVolume) {
        shader.setUniform(importanceVolume_.getIdentifier(), *units.importanceVolume);
        utilgl::setShaderUniforms(shader, importanceVolume_,
                                  StrBuffer{"{}Parameters", importanceVolume_.getIdentifier()});
    }
}

void OpacityOptimization::renderGeometry(const Pass pass, Units& units) {

    const auto draw = [&](Shader& shader, auto&& shouldDraw) {
        setUniforms(shader, units);

        for (const auto& mesh : inport_) {
            utilgl::setShaderUniforms(shader, *mesh, "geometry");
            shader.setUniform("pickingEnabled", meshutil::hasPickIDBuffer(mesh.get()));
            MeshDrawerGL::DrawObject drawer(*mesh);

            if (mesh->getNumberOfIndices() > 0) {
                for (size_t i = 0; i < mesh->getNumberOfIndices(); ++i) {
                    if (!shouldDraw(mesh->getIndexMeshInfo(i))) continue;

                    drawer.draw(i);
                    if (pass < Pass::ApproximateBlending) {
                        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
                    }
                }
            } else if (shouldDraw(mesh->getDefaultMeshInfo())) {
                drawer.draw();
                if (pass < Pass::ApproximateBlending) {
                    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
                }
            }
        }
    };

    const auto lineSettings = [&](Shader& shader) {
        utilgl::setUniforms(shader, camera_, lineSettings_.lineWidth, lineSettings_.antialiasing,
                            lineSettings_.miterLimit, lineSettings_.roundCaps,
                            lineSettings_.defaultColor);

        // Stippling settings
        shader.setUniform("stippling.length", lineSettings_.getStippling().getLength());
        shader.setUniform("stippling.spacing", lineSettings_.getStippling().getSpacing());
        shader.setUniform("stippling.offset", lineSettings_.getStippling().getOffset());
        shader.setUniform("stippling.worldScale", lineSettings_.getStippling().getWorldScale());
    };

    {  // Triangles
        auto& shader = meshShaders_[std::to_underlying(pass)];
        shader.activate();
        utilgl::setUniforms(shader, camera_, lightingProperty_, overrideColor_);
        draw(shader, [](const Mesh::MeshInfo& mi) { return mi.dt == DrawType::Triangles; });
    }

    {  // Lines
        auto& shader = lineShaders_[std::to_underlying(pass)];
        shader.activate();
        lineSettings(shader);
        draw(shader, [](const Mesh::MeshInfo& mi) {
            return mi.dt == DrawType::Lines && mi.ct == ConnectivityType::None;
        });
    }
    {  // Line adjacency
        auto& shader = lineAdjacencyShaders_[std::to_underlying(pass)];
        shader.activate();
        lineSettings(shader);
        draw(shader, [](const Mesh::MeshInfo& mi) {
            return mi.dt == DrawType::Lines && (mi.ct == ConnectivityType::Adjacency ||
                                                mi.ct == ConnectivityType::StripAdjacency);
        });
    }

    {  // Points
        auto& shader = pointShaders_[std::to_underlying(pass)];
        const utilgl::GlBoolState pointSprite(GL_PROGRAM_POINT_SIZE, true);
        const utilgl::PolygonModeState polygon(GL_POINT, 1.0f, pointSize_.get());
        shader.activate();
        utilgl::setUniforms(shader, camera_, lightingProperty_, pointSize_, borderWidth_,
                            borderColor_, antialising_);

        draw(shader, [](const Mesh::MeshInfo& mi) { return mi.dt == DrawType::Points; });
    }
    LGL_ERROR_CLASS;
}

void OpacityOptimization::resizeTexture(Texture2DArray& texture, size2_t size, size_t depth) {
    if (texture.getDimensions() != size3_t{size, depth}) {
        texture.uploadAndResize(nullptr, size3_t{size, depth});
    }
}

void OpacityOptimization::resizeImportanceSumTextures(const size2_t screenSize,
                                                      size_t importanceSumCoefficients) {
    resizeTexture(importanceSumTexture_[0], screenSize, importanceSumCoefficients);
    resizeTexture(importanceSumTexture_[1], screenSize, importanceSumCoefficients);
}

void OpacityOptimization::resizeOpticalDepthTexture(const size2_t screenSize,
                                                    size_t opticalDepthCoefficients) {
    resizeTexture(opticalDepthTexture_, screenSize, opticalDepthCoefficients);
}

void OpacityOptimization::generateAndUploadGaussianKernel() {
    std::vector<float> k = util::generateGaussianKernel(gaussianRadius_, gaussianSigma_);
    k.resize(gaussianKernelMaxRadius_ + 1);
    gaussianKernel_.upload(k.data());
}

void OpacityOptimization::generateAndUploadLegendreCoefficients() {
    std::vector<float> coeffs = approximations::generateLegendreCoefficients();
    legendreCoeffs_.initialize(coeffs.data());
}

void OpacityOptimization::generateAndUploadMomentSettings() {
    static_assert(sizeof(approximations::MomentSettingsGL) == 2 * 4 * sizeof(float));
    approximations::MomentSettingsGL ms = approximations::generateMomentSettings();

    IVW_ASSERT(momentSettings_.getNumberOfValues() * momentSettings_.getSizeInBytes() ==
                   sizeof(approximations::MomentSettingsGL),
               "Size missmatch");

    momentSettings_.initialize(&ms);
}

}  // namespace inviwo
