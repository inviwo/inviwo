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

#include <modules/opactopt/processors/directopacityoptimisation.h>

#include <inviwo/core/algorithm/boundingbox.h>         // for boundingBox
#include <inviwo/core/algorithm/markdown.h>            // for operator""_help
#include <inviwo/core/datastructures/geometry/mesh.h>  // for hasPickIDBuffer, Mesh
#include <inviwo/core/datastructures/image/image.h>    // for Image
#include <inviwo/core/datastructures/image/layer.h>    // for Layer
#include <inviwo/core/ports/imageport.h>               // for ImageOutport, BaseImageInport, Ima...
#include <inviwo/core/ports/inportiterable.h>          // for InportIterable<>::const_iterator
#include <inviwo/core/ports/meshport.h>                // for MeshFlatMultiInport
#include <inviwo/core/processors/processor.h>          // for Processor
#include <inviwo/core/processors/processorinfo.h>      // for ProcessorInfo
#include <inviwo/core/processors/processorstate.h>     // for CodeState, CodeState::Stable
#include <inviwo/core/processors/processortags.h>      // for Tags, Tags::GL
#include <inviwo/core/properties/boolproperty.h>       // for BoolProperty
#include <inviwo/core/properties/cameraproperty.h>     // for CameraProperty
#include <inviwo/core/properties/compositeproperty.h>  // for CompositeProperty
#include <inviwo/core/properties/invalidationlevel.h>  // for InvalidationLevel, InvalidationLev...
#include <inviwo/core/properties/optionproperty.h>     // for OptionPropertyOption, OptionProper...
#include <inviwo/core/properties/ordinalproperty.h>    // for ordinalColor, FloatVec4Property
#include <inviwo/core/properties/property.h>           // for Property
#include <inviwo/core/properties/propertysemantics.h>  // for PropertySemantics, PropertySemanti...
#include <modules/opengl/geometry/meshgl.h>            // for MeshGL
#include <modules/opengl/inviwoopengl.h>               // for GL_BACK, GL_DEPTH_TEST, GL_FRONT
#include <modules/opengl/openglutils.h>                // for BlendModeState, CullFaceState, GlB...
#include <modules/opengl/rendering/meshdrawergl.h>     // for MeshDrawerGL::DrawObject, MeshDraw...
#include <modules/opengl/shader/shader.h>              // for Shader, Shader::Build
#include <modules/opengl/shader/shaderobject.h>        // for ShaderObject
#include <modules/opengl/shader/shaderutils.h>         // for addShaderDefines, setShaderUniforms
#include <modules/opengl/texture/textureutils.h>       // for activateTargetAndClearOrCopySource
#include <modules/opactopt/utils/gaussian.h>

#include <cstddef>      // for size_t
#include <functional>   // for __base
#include <memory>       // for shared_ptr, make_shared, shared_pt...
#include <string>       // for string
#include <string_view>  // for string_view
#include <type_traits>  // for remove_extent_t
#include <chrono>

namespace inviwo {

const ProcessorInfo DirectOpacityOptimisation::processorInfo_{
    "org.inviwo.DirectOpacityOptimisation",  // Class identifier
    "Direct Opacity Optimisation",           // Display name
    "Mesh Rendering",                        // Category
    CodeState::Experimental,                 // Code state
    Tags::GL,                                // Tags
    "Performs approximate opacity optimisation using a direct"
    " rendering approach.The processor takes a mesh as input,"
    " and optionally an importance volume and background"
    "texture. The output is an opacity optimised image."_help};

const ProcessorInfo& DirectOpacityOptimisation::getProcessorInfo() const { return processorInfo_; }

DirectOpacityOptimisation::DirectOpacityOptimisation()
    : Processor()
    , inport_("geometry", "Input meshes"_help)
    , imageInport_("imageInport", "Background image (optional)"_help)
    , outport_("image",
               "Output image containing the rendered mesh and the optional input image"_help)
    , intermediateImage_({0, 0}, inviwo::DataVec4Float32::get())
    , screenSize_{0, 0}
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
    , layers_("layers", "Output Layers")
    , colorLayer_("colorLayer", "Color", "Toggle output of color layer"_help, true,
                  InvalidationLevel::InvalidResources)
    , texCoordLayer_("texCoordLayer", "Texture Coordinates",
                     "Toggle output of texture coordinates"_help, false,
                     InvalidationLevel::InvalidResources)
    , normalsLayer_("normalsLayer", "Normals (World Space)",
                    "Toggle output of normals (world space)"_help, false,
                    InvalidationLevel::InvalidResources)
    , viewNormalsLayer_("viewNormalsLayer", "Normals (View space)",
                        "Toggle output of view space normals"_help, false,
                        InvalidationLevel::InvalidResources)
    , meshShaders_{{"meshrendering.vert", "opactopt/direct/mesh/projectimportance.frag",
                    Shader::Build::No},
                   {"meshrendering.vert", "opactopt/direct/mesh/approximportancesum.frag",
                    Shader::Build::No},
                   {"meshrendering.vert", "opactopt/direct/mesh/approxblending.frag",
                    Shader::Build::No}}
    , lineShaders_{{"opactopt/direct/line/linerenderer.vert", "linerenderer.geom",
                    "opactopt/direct/line/projectimportance.frag", Shader::Build::No},
                   {"opactopt/direct/line/linerenderer.vert", "linerenderer.geom",
                    "opactopt/direct/line/approximportancesum.frag", Shader::Build::No},
                   {"opactopt/direct/line/linerenderer.vert", "linerenderer.geom",
                    "opactopt/direct/line/approxblending.frag", Shader::Build::No}}
    , lineAdjacencyShaders_{{"opactopt/direct/line/linerenderer.vert", "linerenderer.geom",
                             "opactopt/direct/line/projectimportance.frag", Shader::Build::No},
                            {"opactopt/direct/line/linerenderer.vert", "linerenderer.geom",
                             "opactopt/direct/line/approximportancesum.frag", Shader::Build::No},
                            {"opactopt/direct/line/linerenderer.vert", "linerenderer.geom",
                             "opactopt/direct/line/approxblending.frag", Shader::Build::No}}
    , pointShaders_{{"pointrenderer.vert", "opactopt/direct/point/projectimportance.frag",
                     Shader::Build::No},
                    {"pointrenderer.vert", "opactopt/direct/point/approximportancesum.frag",
                     Shader::Build::No},
                    {"pointrenderer.vert", "opactopt/direct/point/approxblending.frag",
                     Shader::Build::No}}
    , smoothH_{"oit/simplequad.vert", "opactopt/approximate/smooth.frag", Shader::Build::No}
    , smoothV_{"oit/simplequad.vert", "opactopt/approximate/smooth.frag", Shader::Build::No}
    , clear_{"oit/simplequad.vert", "opactopt/direct/clear.frag", Shader::Build::No}
    , normalise_{"oit/simplequad.vert", "opactopt/direct/normalise.frag", Shader::Build::No}
    , approximationProperties_{"approximationProperties", "Approximation Properties"}
    , q_{"q",
         "q",
         1.0f,
         0.0f,
         10000.0f,
         0.01f,
         InvalidationLevel::InvalidOutput,
         PropertySemantics::SpinBox}
    , r_{"r",
         "r",
         1.0f,
         0.0f,
         10000.0f,
         0.01f,
         InvalidationLevel::InvalidOutput,
         PropertySemantics::SpinBox}
    , lambda_{"lambda",
              "lambda",
              1.0f,
              0.0f,
              100.0f,
              0.01f,
              InvalidationLevel::InvalidOutput,
              PropertySemantics::SpinBox}
    , approximationMethod_{"approximationMethod", "Approximation Method",
                           Approximations::generateApproximationStringOptions(), 0}
    , importanceVolume_{"importanceVolume", "Optional scalar field with importance data"_help}
    , importanceSumCoefficients_{"importanceSumCoefficients",
                                 "Importance sum coefficients",
                                 5,
                                 Approximations::approximations.at(approximationMethod_)
                                     .minCoefficients,
                                 Approximations::approximations.at(approximationMethod_)
                                     .maxCoefficients,
                                 Approximations::approximations.at(approximationMethod_).increment}
    , opticalDepthCoefficients_{"opticalDepthCoefficients",
                                "Optical depth coefficients",
                                5,
                                Approximations::approximations.at(approximationMethod_)
                                    .minCoefficients,
                                Approximations::approximations.at(approximationMethod_)
                                    .maxCoefficients,
                                Approximations::approximations.at(approximationMethod_).increment}
    , normalisedBlending_{"normalisedBlending", "Normalised blending", true}
    , coeffTexFixedPointFactor_{"coeffTexFixedPointFactor",
                                "Fixed point multiplier",
                                1e6,
                                32,
                                1e10,
                                1.0,
                                InvalidationLevel::InvalidOutput,
                                PropertySemantics::SpinBox}
    , importanceSumTexture_{{size3_t(screenSize_.x, screenSize_.y, importanceSumCoefficients_),
                             imageFormat_, GL_NEAREST},
                            {size3_t(screenSize_.x, screenSize_.y, importanceSumCoefficients_),
                             imageFormat_, GL_NEAREST}}
    , opticalDepthTexture_{size3_t(screenSize_.x, screenSize_.y, opticalDepthCoefficients_),
                           imageFormat_, GL_NEAREST}
    , gaussianKernel_{128 * sizeof(float),                  // allocate max possible size
                      GLFormats::getGLFormat(GL_FLOAT, 1),  // dummy format
                      GL_STATIC_DRAW, GL_SHADER_STORAGE_BUFFER}
    , smoothing_{"smoothing", "Smoothing", false}
    , gaussianRadius_{"gaussianKernelRadius", "Gaussian kernel radius", 3, 1, 50}
    , gaussianSigma_{"gaussianKernelSigma", "Gaussian kernel sigma", 1, 0.001, 50}
    , legendreCoefficients_{
          Approximations::approximations.at("legendre").maxCoefficients * sizeof(int),
          GLFormats::getGLFormat(GL_INT, 1),  // dummy format
          GL_STATIC_DRAW, GL_SHADER_STORAGE_BUFFER} {

    addPort(inport_);
    addPort(imageInport_).setOptional(true);
    addPort(importanceVolume_).setOptional(true);
    addPort(outport_);

    imageInport_.onChange([this]() { initializeResources(); });
    importanceVolume_.onChange([this]() { initializeResources(); });

    addProperties(camera_, q_, r_, lambda_, approximationProperties_, meshProperties_,
                  lineSettings_, pointProperties_, lightingProperty_, trackball_, layers_);

    approximationProperties_.addProperties(approximationMethod_, importanceSumCoefficients_,
                                           opticalDepthCoefficients_);
    if (!OpenGLCapabilities::isExtensionSupported("GL_NV_shader_atomic_float"))
        approximationProperties_.addProperty(coeffTexFixedPointFactor_);
    approximationProperties_.addProperties(normalisedBlending_, smoothing_);

    for (auto& ist : importanceSumTexture_) ist.initialize(nullptr);
    opticalDepthTexture_.initialize(nullptr);

    approximationMethod_.setDefault("fourier");
    approximationMethod_.set("fourier");
    approximationMethod_.onChange([this]() {
        ap_ = &Approximations::approximations.at(approximationMethod_);
        importanceSumCoefficients_.setMinValue(ap_->minCoefficients);
        importanceSumCoefficients_.setMaxValue(ap_->maxCoefficients);
        importanceSumCoefficients_.setIncrement(ap_->increment);
        opticalDepthCoefficients_.setMinValue(ap_->minCoefficients);
        opticalDepthCoefficients_.setMaxValue(ap_->maxCoefficients);
        opticalDepthCoefficients_.setIncrement(ap_->increment);
        initializeResources();
    });
    approximationMethod_.propertyModified();

    importanceSumCoefficients_.onChange([this]() {
        importanceSumTexture_[0].uploadAndResize(
            nullptr, size3_t(screenSize_.x, screenSize_.y, importanceSumCoefficients_));
        importanceSumTexture_[1].uploadAndResize(
            nullptr, size3_t(screenSize_.x, screenSize_.y, importanceSumCoefficients_));
        initializeResources();
    });
    opticalDepthCoefficients_.onChange([this]() {
        opticalDepthTexture_.uploadAndResize(
            nullptr, size3_t(screenSize_.x, screenSize_.y, opticalDepthCoefficients_));
        initializeResources();
    });
    normalisedBlending_.onChange([this]() { initializeResources(); });
    if (!OpenGLCapabilities::isExtensionSupported("GL_NV_shader_atomic_float"))
        coeffTexFixedPointFactor_.onChange([this]() { initializeResources(); });

    smoothing_.addProperties(gaussianRadius_, gaussianSigma_);

    gaussianRadius_.onChange([this]() { generateAndUploadGaussianKernel(); });
    gaussianSigma_.onChange([this]() { generateAndUploadGaussianKernel(); });

    meshProperties_.addProperties(overrideColorBuffer_, overrideColor_);
    pointProperties_.addProperties(pointSize_, borderWidth_, borderColor_, antialising_);

    overrideColor_.setSemantics(PropertySemantics::Color)
        .visibilityDependsOn(overrideColorBuffer_, [](const BoolProperty p) { return p.get(); });

    layers_.addProperties(colorLayer_, texCoordLayer_, normalsLayer_, viewNormalsLayer_);

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

    for (auto& shader : {&smoothH_, &smoothV_, &normalise_}) {
        shader->onReload([this]() { invalidate(InvalidationLevel::InvalidResources); });
    }

    generateAndUploadGaussianKernel();
    generateAndUploadLegendreCoefficients();
    legendreCoefficients_.bindBase(9);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
}

DirectOpacityOptimisation::~DirectOpacityOptimisation() = default;

void DirectOpacityOptimisation::initializeResources() {
    // Configure all the shaders with correct extensions and defines
    for (auto& shader : meshShaders_) {
        auto vert = shader.getVertexShaderObject();
        auto frag = shader.getFragmentShaderObject();

        frag->clearShaderExtensions();
        frag->clearShaderDefines();

        utilgl::addShaderDefines(shader, lightingProperty_);

        vert->setShaderDefine("OVERRIDE_COLOR_BUFFER", overrideColorBuffer_);
        frag->setShaderDefine("COLOR_LAYER", colorLayer_);

        // add image load store extension
        frag->addShaderExtension("GL_EXT_shader_image_load_store", true);
        if (OpenGLCapabilities::isExtensionSupported("GL_NV_shader_atomic_float"))
            frag->addShaderExtension("GL_NV_shader_atomic_float", true);

        // set opacity optimisation defines
        frag->setShaderDefine("USE_IMPORTANCE_VOLUME", importanceVolume_.hasData());

        // set approximation defines
        frag->setShaderDefine("N_IMPORTANCE_SUM_COEFFICIENTS", true,
                              std::to_string(importanceSumCoefficients_).c_str());
        frag->setShaderDefine("N_OPTICAL_DEPTH_COEFFICIENTS", true,
                              std::to_string(opticalDepthCoefficients_).c_str());
        frag->setShaderDefine(ap_->shaderDefineName.c_str(), true);
        if (OpenGLCapabilities::isExtensionSupported("GL_NV_shader_atomic_float"))
            frag->setShaderDefine("COEFF_TEX_ATOMIC_FLOAT", true);
        else
            frag->setShaderDefine("COEFF_TEX_FIXED_POINT_FACTOR", true,
                                  std::to_string(coeffTexFixedPointFactor_));

        shader.build();
    }

    for (auto& shader : lineShaders_) {
        auto vert = shader.getVertexShaderObject();
        auto frag = shader.getFragmentShaderObject();

        frag->clearShaderExtensions();
        frag->clearShaderDefines();

        shader[ShaderType::Fragment]->setShaderDefine("ENABLE_PSEUDO_LIGHTING",
                                                      lineSettings_.getPseudoLighting());
        shader[ShaderType::Fragment]->setShaderDefine("ENABLE_ROUND_DEPTH_PROFILE",
                                                      lineSettings_.getRoundDepthProfile());

        utilgl::addShaderDefines(shader, lineSettings_.getStippling().getMode());

        vert->setShaderDefine("HAS_COLOR", true);

        // add image load store extension
        frag->addShaderExtension("GL_EXT_shader_image_load_store", true);
        if (OpenGLCapabilities::isExtensionSupported("GL_NV_shader_atomic_float"))
            frag->addShaderExtension("GL_NV_shader_atomic_float", true);

        // set opacity optimisation defines
        frag->setShaderDefine("USE_IMPORTANCE_VOLUME", importanceVolume_.hasData());

        // set approximation defines
        frag->setShaderDefine("N_IMPORTANCE_SUM_COEFFICIENTS", true,
                              std::to_string(importanceSumCoefficients_).c_str());
        frag->setShaderDefine("N_OPTICAL_DEPTH_COEFFICIENTS", true,
                              std::to_string(opticalDepthCoefficients_).c_str());
        frag->setShaderDefine(ap_->shaderDefineName.c_str(), true);
        if (OpenGLCapabilities::isExtensionSupported("GL_NV_shader_atomic_float"))
            frag->setShaderDefine("COEFF_TEX_ATOMIC_FLOAT", true);
        else
            frag->setShaderDefine("COEFF_TEX_FIXED_POINT_FACTOR", true,
                                  std::to_string(coeffTexFixedPointFactor_));

        shader.build();
    }

    for (auto& shader : lineAdjacencyShaders_) {
        auto vert = shader.getVertexShaderObject();
        auto geom = shader.getGeometryShaderObject();
        auto frag = shader.getFragmentShaderObject();

        frag->clearShaderExtensions();
        frag->clearShaderDefines();

        shader[ShaderType::Fragment]->setShaderDefine("ENABLE_PSEUDO_LIGHTING",
                                                      lineSettings_.getPseudoLighting());
        shader[ShaderType::Fragment]->setShaderDefine("ENABLE_ROUND_DEPTH_PROFILE",
                                                      lineSettings_.getRoundDepthProfile());

        utilgl::addShaderDefines(shader, lineSettings_.getStippling().getMode());

        vert->setShaderDefine("HAS_COLOR", true);

        geom->setShaderDefine("ENABLE_ADJACENCY", true, "1");

        // add image load store extension
        frag->addShaderExtension("GL_EXT_shader_image_load_store", true);
        if (OpenGLCapabilities::isExtensionSupported("GL_NV_shader_atomic_float"))
            frag->addShaderExtension("GL_NV_shader_atomic_float", true);

        // set opacity optimisation defines
        frag->setShaderDefine("USE_IMPORTANCE_VOLUME", importanceVolume_.hasData());

        // set approximation defines
        frag->setShaderDefine("N_IMPORTANCE_SUM_COEFFICIENTS", true,
                              std::to_string(importanceSumCoefficients_).c_str());
        frag->setShaderDefine("N_OPTICAL_DEPTH_COEFFICIENTS", true,
                              std::to_string(opticalDepthCoefficients_).c_str());
        frag->setShaderDefine(ap_->shaderDefineName.c_str(), true);
        if (OpenGLCapabilities::isExtensionSupported("GL_NV_shader_atomic_float"))
            frag->setShaderDefine("COEFF_TEX_ATOMIC_FLOAT", true);
        else
            frag->setShaderDefine("COEFF_TEX_FIXED_POINT_FACTOR", true,
                                  std::to_string(coeffTexFixedPointFactor_));

        shader.build();
    }

    for (auto& shader : pointShaders_) {
        auto vert = shader.getVertexShaderObject();
        auto frag = shader.getFragmentShaderObject();

        frag->clearShaderExtensions();
        frag->clearShaderDefines();

        // add image load store extension
        frag->addShaderExtension("GL_EXT_shader_image_load_store", true);
        if (OpenGLCapabilities::isExtensionSupported("GL_NV_shader_atomic_float"))
            frag->addShaderExtension("GL_NV_shader_atomic_float", true);

        // set opacity optimisation defines
        frag->setShaderDefine("USE_IMPORTANCE_VOLUME", importanceVolume_.hasData());

        // set approximation defines
        frag->setShaderDefine("N_IMPORTANCE_SUM_COEFFICIENTS", true,
                              std::to_string(importanceSumCoefficients_).c_str());
        frag->setShaderDefine("N_OPTICAL_DEPTH_COEFFICIENTS", true,
                              std::to_string(opticalDepthCoefficients_).c_str());
        frag->setShaderDefine(ap_->shaderDefineName.c_str(), true);
        if (OpenGLCapabilities::isExtensionSupported("GL_NV_shader_atomic_float"))
            frag->setShaderDefine("COEFF_TEX_ATOMIC_FLOAT", true);
        else
            frag->setShaderDefine("COEFF_TEX_FIXED_POINT_FACTOR", true,
                                  std::to_string(coeffTexFixedPointFactor_));

        shader.build();
    }

    {
        Shader& shader = clear_;

        auto vert = shader.getVertexShaderObject();
        auto frag = shader.getFragmentShaderObject();

        frag->clearShaderExtensions();
        frag->clearShaderDefines();

        // add image load store extension
        frag->addShaderExtension("GL_EXT_shader_image_load_store", true);

        // set approximation defines
        frag->setShaderDefine("N_IMPORTANCE_SUM_COEFFICIENTS", true,
                              std::to_string(importanceSumCoefficients_).c_str());
        frag->setShaderDefine("N_OPTICAL_DEPTH_COEFFICIENTS", true,
                              std::to_string(opticalDepthCoefficients_).c_str());

        shader.build();
    }

    {
        Shader& shader = normalise_;

        auto vert = shader.getVertexShaderObject();
        auto frag = shader.getFragmentShaderObject();

        frag->clearShaderExtensions();
        frag->clearShaderDefines();

        // add image load store extension
        frag->addShaderExtension("GL_EXT_shader_image_load_store", true);
        if (OpenGLCapabilities::isExtensionSupported("GL_NV_shader_atomic_float"))
            frag->addShaderExtension("GL_NV_shader_atomic_float", true);

        // set approximation defines
        frag->setShaderDefine("N_IMPORTANCE_SUM_COEFFICIENTS", true,
                              std::to_string(importanceSumCoefficients_).c_str());
        frag->setShaderDefine("N_OPTICAL_DEPTH_COEFFICIENTS", true,
                              std::to_string(opticalDepthCoefficients_).c_str());
        frag->setShaderDefine(ap_->shaderDefineName.c_str(), true);
        if (OpenGLCapabilities::isExtensionSupported("GL_NV_shader_atomic_float"))
            frag->setShaderDefine("COEFF_TEX_ATOMIC_FLOAT", true);
        else
            frag->setShaderDefine("COEFF_TEX_FIXED_POINT_FACTOR", true,
                                  std::to_string(coeffTexFixedPointFactor_));
        frag->setShaderDefine("NORMALISE", normalisedBlending_);
        frag->setShaderDefine("BACKGROUND_AVAILABLE", imageInport_.hasData());

        shader.build();
    }

    bool horizontal = true;
    for (auto& shader : {&smoothH_, &smoothV_}) {

        auto vert = shader->getVertexShaderObject();
        auto frag = shader->getFragmentShaderObject();

        frag->clearShaderExtensions();
        frag->clearShaderDefines();

        // add image load store extension
        frag->addShaderExtension("GL_EXT_shader_image_load_store", true);

        // set approximation defines
        frag->setShaderDefine("N_IMPORTANCE_SUM_COEFFICIENTS", true,
                              std::to_string(importanceSumCoefficients_).c_str());
        frag->setShaderDefine(ap_->shaderDefineName.c_str(), true);
        if (OpenGLCapabilities::isExtensionSupported("GL_NV_shader_atomic_float"))
            frag->setShaderDefine("COEFF_TEX_ATOMIC_FLOAT", true);
        else
            frag->setShaderDefine("COEFF_TEX_FIXED_POINT_FACTOR", true,
                                  std::to_string(coeffTexFixedPointFactor_));
        frag->setShaderDefine("HORIZONTAL", true, std::to_string(int(horizontal)));
        frag->setShaderDefine("USE_PICK_IMAGE", true);
        shader->build();

        horizontal = false;
    }
}

void DirectOpacityOptimisation::process() {
    resizeBuffers(outport_.getDimensions());

    if (intermediateImage_.getDimensions() != outport_.getDimensions()) {
        intermediateImage_.setDimensions(outport_.getDimensions());
    }
    utilgl::activateAndClearTarget(intermediateImage_);

    utilgl::GlBoolState depthTest(GL_DEPTH_TEST, GL_TRUE);
    utilgl::DepthMaskState depthMask(GL_FALSE);
    utilgl::CullFaceState culling(GL_NONE);

    // Bind textures
    importanceSumUnitMain_ = &textureUnits_.emplace_back();
    importanceSumUnitMain_->activate();
    importanceSumTexture_[0].bind();
    glBindImageTexture(importanceSumUnitMain_->getUnitNumber(), importanceSumTexture_[0].getID(), 0,
                       true, 0, GL_READ_WRITE, imageFormat_.internalFormat);

    if (smoothing_) {
        importanceSumUnitSmooth_ = &textureUnits_.emplace_back();
        importanceSumUnitSmooth_->activate();
        importanceSumTexture_[1].bind();
        glBindImageTexture(importanceSumUnitSmooth_->getUnitNumber(),
                           importanceSumTexture_[1].getID(), 0, true, 0, GL_READ_WRITE,
                           imageFormat_.internalFormat);
    }

    opticalDepthUnit_ = &textureUnits_.emplace_back();
    opticalDepthUnit_->activate();
    opticalDepthTexture_.bind();
    glBindImageTexture(opticalDepthUnit_->getUnitNumber(), opticalDepthTexture_.getID(), 0, true, 0,
                       GL_READ_WRITE, imageFormat_.internalFormat);

    // clear coefficient buffers
    clear_.activate();
    setUniforms(clear_);
    utilgl::singleDrawImagePlaneRect();
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    // Pass 1: Project importance
    renderGeometry(0);

    // Optional smoothing of importance coefficients
    utilgl::activateTarget(
        intermediateImage_,
        inviwo::ImageType::ColorOnly);  // bind color only so we can use picking texture
    // as texture
    if (smoothing_) {
        // smoothing importance
        gaussianKernel_.bindBase(8);

        // horizontal pass
        smoothH_.activate();
        smoothH_.setUniform("radius", gaussianRadius_);
        setUniforms(smoothH_);
        utilgl::bindAndSetUniforms(smoothH_, textureUnits_, intermediateImage_, "image",
                                   ImageType::ColorPicking);
        utilgl::singleDrawImagePlaneRect();
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

        // vertical pass
        smoothV_.activate();
        smoothV_.setUniform("radius", gaussianRadius_);
        setUniforms(smoothV_);
        utilgl::bindAndSetUniforms(smoothV_, textureUnits_, intermediateImage_, "image",
                                   ImageType::ColorPicking);
        utilgl::singleDrawImagePlaneRect();
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    }

    // Pass 2: Approximate importance and project opacities
    renderGeometry(1);

    // Pass 3: Approximate blending, render to target
    utilgl::BlendModeState blending(GL_ONE, GL_ONE, GL_ONE, GL_ONE);
    renderGeometry(2);

    // normalise
    utilgl::activateAndClearTarget(outport_);
    blending = utilgl::BlendModeState(GL_ONE, GL_ZERO);

    normalise_.activate();
    utilgl::bindAndSetUniforms(normalise_, textureUnits_, intermediateImage_, "image",
                               ImageType::ColorPicking);
    if (imageInport_.hasData())
        utilgl::bindAndSetUniforms(normalise_, textureUnits_, *imageInport_.getData(), "bg",
                                   ImageType::ColorDepth);
    setUniforms(normalise_);
    utilgl::singleDrawImagePlaneRect();

    glUseProgram(0);
    textureUnits_.clear();
    utilgl::deactivateCurrentTarget();
}

void DirectOpacityOptimisation::setUniforms(Shader& shader) {
    shader.setUniform("screenSize", ivec2(screenSize_));
    shader.setUniform("reciprocalDimensions", vec2(1) / vec2(screenSize_));

    shader.setUniform("q", q_);
    shader.setUniform("r", r_);
    shader.setUniform("lambda", lambda_);

    shader.setUniform("importanceSumCoeffs[0]", importanceSumUnitMain_->getUnitNumber());
    if (smoothing_)
        shader.setUniform("importanceSumCoeffs[1]", importanceSumUnitSmooth_->getUnitNumber());
    shader.setUniform("opticalDepthCoeffs", opticalDepthUnit_->getUnitNumber());

    if (importanceVolume_.hasData())
        utilgl::bindAndSetUniforms(shader, textureUnits_, importanceVolume_);
}

void DirectOpacityOptimisation::renderGeometry(const int pass) {
    // Mesh
    meshShaders_[pass].activate();
    utilgl::setUniforms(meshShaders_[pass], camera_, lightingProperty_, overrideColor_);
    setUniforms(meshShaders_[pass]);
    for (auto mesh : inport_) {
        utilgl::setShaderUniforms(meshShaders_[pass], *mesh, "geometry");
        meshShaders_[pass].setUniform("pickingEnabled", meshutil::hasPickIDBuffer(mesh.get()));
        MeshDrawerGL::DrawObject drawer(*mesh);

        if (mesh->getNumberOfIndicies() > 0) {
            for (size_t i = 0; i < mesh->getNumberOfIndicies(); ++i) {
                if (mesh->getIndexMeshInfo(i).dt != DrawType::Triangles) continue;
                drawer.draw(i);
                if (pass < 2) glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
            }
        } else if (mesh->getDefaultMeshInfo().dt == DrawType::Triangles) {
            drawer.draw();
            if (pass < 2) glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        }
    }

    // Lines
    for (const auto& mesh : inport_) {
        lineShaders_[pass].activate();

        lineShaders_[pass].setUniform("screenDim", vec2(screenSize_));
        utilgl::setShaderUniforms(lineShaders_[pass], camera_, "camera");
        utilgl::setUniforms(lineShaders_[pass], lineSettings_.lineWidth,
                            lineSettings_.antialiasing, lineSettings_.miterLimit,
                            lineSettings_.roundCaps, lineSettings_.defaultColor);

        // Stippling settings
        lineShaders_[pass].setUniform("stippling.length", lineSettings_.getStippling().getLength());
        lineShaders_[pass].setUniform("stippling.spacing",
                                      lineSettings_.getStippling().getSpacing());
        lineShaders_[pass].setUniform("stippling.offset", lineSettings_.getStippling().getOffset());
        lineShaders_[pass].setUniform("stippling.worldScale",
                                      lineSettings_.getStippling().getWorldScale());

        utilgl::setShaderUniforms(lineShaders_[pass], *mesh, "geometry");
        setUniforms(lineShaders_[pass]);
        for (auto mesh : inport_) {
            utilgl::setShaderUniforms(lineShaders_[pass], *mesh, "geometry");
            MeshDrawerGL::DrawObject drawer(*mesh);

            if (mesh->getNumberOfIndicies() > 0) {
                for (size_t i = 0; i < mesh->getNumberOfIndicies(); ++i) {
                    if (mesh->getIndexMeshInfo(i).dt != DrawType::Lines ||
                        mesh->getIndexMeshInfo(0).ct != ConnectivityType::None)
                        continue;

                    drawer.draw(i);
                    if (pass < 2) glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
                }
            } else if (mesh->getDefaultMeshInfo().dt == DrawType::Lines &&
                       mesh->getDefaultMeshInfo().ct == ConnectivityType::None) {
                drawer.draw();
                if (pass < 2) glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
            }
        }
    }

    // Line adjacency
    for (const auto& mesh : inport_) {
        lineAdjacencyShaders_[pass].activate();

        lineAdjacencyShaders_[pass].setUniform("screenDim", vec2(screenSize_));
        utilgl::setShaderUniforms(lineAdjacencyShaders_[pass], camera_, "camera");
        utilgl::setUniforms(lineAdjacencyShaders_[pass], lineSettings_.lineWidth,
                            lineSettings_.antialiasing, lineSettings_.miterLimit,
                            lineSettings_.roundCaps, lineSettings_.defaultColor);

        // Stippling settings
        lineAdjacencyShaders_[pass].setUniform("stippling.length",
                                               lineSettings_.getStippling().getLength());
        lineAdjacencyShaders_[pass].setUniform("stippling.spacing",
                                               lineSettings_.getStippling().getSpacing());
        lineAdjacencyShaders_[pass].setUniform("stippling.offset",
                                               lineSettings_.getStippling().getOffset());
        lineAdjacencyShaders_[pass].setUniform("stippling.worldScale",
                                               lineSettings_.getStippling().getWorldScale());

        utilgl::setShaderUniforms(lineAdjacencyShaders_[pass], *mesh, "geometry");
        setUniforms(lineAdjacencyShaders_[pass]);
        for (auto mesh : inport_) {
            utilgl::setShaderUniforms(lineAdjacencyShaders_[pass], *mesh, "geometry");
            MeshDrawerGL::DrawObject drawer(*mesh);

            if (mesh->getNumberOfIndicies() > 0) {
                for (size_t i = 0; i < mesh->getNumberOfIndicies(); ++i) {
                    if (mesh->getIndexMeshInfo(i).dt != DrawType::Lines ||
                        mesh->getIndexMeshInfo(0).ct < ConnectivityType::Adjacency)
                        continue;

                    drawer.draw(i);
                    if (pass < 2) glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
                }
            } else if (mesh->getDefaultMeshInfo().dt == DrawType::Lines &&
                       mesh->getDefaultMeshInfo().ct >= ConnectivityType::Adjacency) {
                drawer.draw();
                if (pass < 2) glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
            }
        }
    }

    // Points
    utilgl::GlBoolState pointSprite(GL_PROGRAM_POINT_SIZE, true);
    utilgl::PolygonModeState polygon(GL_POINT, 1.0f, pointSize_.get());
    pointShaders_[pass].activate();

    utilgl::setUniforms(pointShaders_[pass], camera_, lightingProperty_, pointSize_, borderWidth_,
                        borderColor_, antialising_);
    setUniforms(pointShaders_[pass]);
    for (auto mesh : inport_) {
        utilgl::setShaderUniforms(pointShaders_[pass], *mesh, "geometry");
        MeshDrawerGL::DrawObject drawer(*mesh);

        if (mesh->getNumberOfIndicies() > 0) {
            for (size_t i = 0; i < mesh->getNumberOfIndicies(); ++i) {
                if (mesh->getIndexMeshInfo(i).dt != DrawType::Points) continue;

                drawer.draw(i);
                if (pass < 2) glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
            }
        } else if (mesh->getDefaultMeshInfo().dt == DrawType::Points ||
                   mesh->getDefaultMeshInfo().dt == DrawType::NotSpecified) {
            drawer.draw();
            if (pass < 2) glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        }
    }

    LGL_ERROR_CLASS;
}

void DirectOpacityOptimisation::resizeBuffers(const size2_t screenSize) {
    if (screenSize != screenSize_) {
        screenSize_ = screenSize;
        importanceSumTexture_[0].uploadAndResize(
            nullptr, size3_t(screenSize.x, screenSize.y, importanceSumCoefficients_));
        importanceSumTexture_[1].uploadAndResize(
            nullptr, size3_t(screenSize.x, screenSize.y, importanceSumCoefficients_));
        opticalDepthTexture_.uploadAndResize(
            nullptr, size3_t(screenSize.x, screenSize.y, opticalDepthCoefficients_));
    }
}

void DirectOpacityOptimisation::generateAndUploadGaussianKernel() {
    std::vector<float> k = util::generateGaussianKernel(gaussianRadius_, gaussianSigma_);
    gaussianKernel_.upload(&k[0], k.size() * sizeof(float));
    gaussianKernel_.unbind();
}

void DirectOpacityOptimisation::generateAndUploadLegendreCoefficients() {
    std::vector<float> coeffs = Approximations::generateLegendreCoefficients();
    legendreCoefficients_.upload(&coeffs[0], coeffs.size() * sizeof(int));
    legendreCoefficients_.unbind();
}

}  // namespace inviwo
