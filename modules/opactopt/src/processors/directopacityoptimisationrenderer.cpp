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

#include <modules/opactopt/processors/directopacityoptimisationrenderer.h>

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

#include <cstddef>      // for size_t
#include <functional>   // for __base
#include <memory>       // for shared_ptr, make_shared, shared_pt...
#include <string>       // for string
#include <string_view>  // for string_view
#include <type_traits>  // for remove_extent_t

namespace inviwo {

const ProcessorInfo DirectOpacityOptimisationRenderer::processorInfo_{
    "org.inviwo.OpactOpt",                   // Class identifier
    "Direct Opacity Optimisation Renderer",  // Display name
    "Mesh Rendering",                        // Category
    CodeState::Experimental,                 // Code state
    Tags::GL,                                // Tags
    "Directly performs opacity optimisation and renders mesh."_help};

const ProcessorInfo DirectOpacityOptimisationRenderer::getProcessorInfo() const {
    return processorInfo_;
}

DirectOpacityOptimisationRenderer::DirectOpacityOptimisationRenderer()
    : Processor()
    , inport_("geometry", "Input meshes"_help)
    , imageInport_("imageInport", "Background image (optional)"_help)
    , outport_("image",
               "Output image containing the rendered mesh and the optional input image"_help)
    , intermediateImage_({0, 0}, inviwo::DataFormat<glm::fvec4>::get())
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
    , pointShaders_{{"pointrenderer.vert", "opactopt/direct/point/projectimportance.frag",
                     Shader::Build::No},
                    {"pointrenderer.vert", "opactopt/direct/point/approximportancesum.frag",
                     Shader::Build::No},
                    {"pointrenderer.vert", "opactopt/direct/point/approxblending.frag",
                     Shader::Build::No}}
    , lineRenderer_(&lineSettings_)
    , smoothH_{"oit/simplequad.vert", "opactopt/approximate/smooth.frag", Shader::Build::No}
    , smoothV_{"oit/simplequad.vert", "opactopt/approximate/smooth.frag", Shader::Build::No}
    , clear_{"oit/simplequad.vert", "opactopt/direct/clear.frag", Shader::Build::No}
    , normalise_{"oit/simplequad.vert", "opactopt/direct/normalise.frag", Shader::Build::No}
    , background_{"oit/simplequad.vert", "opactopt/direct/background.frag", Shader::Build::No}
    , approximationProperties_{"approximationProperties", "Approximation Properties"}
    , q_{"q", "q", 1.0f, 0.0f, 1000.0f, 0.01f}
    , r_{"r", "r", 1.0f, 0.0f, 1000.0f, 0.01f}
    , lambda_{"lambda", "lambda", 0.5f, 0.001f, 10.0f, 0.01f}
    , approximationMethod_{"approximationMethod", "Approximation Method"}
    , importanceVolume_{"importanceVolume", "Optional scalar field with importance data"_help}
    , importanceSumCoefficients_{"importanceSumCoefficients", "Importance sum coefficients"}
    , opticalDepthCoefficients_{"opticalDepthCoefficients", "Optical depth coefficients"}
    , importanceSumCoeffs_{{size3_t(screenSize_.x, screenSize_.y, importanceSumCoefficients_),
                            GL_RED_INTEGER, GL_R32I, GL_INT, GL_NEAREST},
                           {size3_t(screenSize_.x, screenSize_.y, importanceSumCoefficients_),
                            GL_RED_INTEGER, GL_R32I, GL_INT, GL_NEAREST}}
    , opticalDepthCoeffs_{size3_t(screenSize_.x, screenSize_.y, opticalDepthCoefficients_),
                          GL_RED_INTEGER, GL_R32I, GL_INT, GL_NEAREST}
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

    addProperties(camera_, approximationProperties_, meshProperties_, lineSettings_,
                  pointProperties_, lightingProperty_, trackball_, layers_);

    approximationProperties_.addProperties(q_, r_, lambda_, approximationMethod_,
                                           importanceSumCoefficients_, opticalDepthCoefficients_,
                                           smoothing_);

    for (auto& isc : importanceSumCoeffs_) isc.initialize(nullptr);
    opticalDepthCoeffs_.initialize(nullptr);

    for (auto const& [key, val] : Approximations::approximations) {
        approximationMethod_.addOption(key.c_str(), val.name.c_str(), key);
    }
    approximationMethod_.setDefault("fourier");
    approximationMethod_.set("fourier");
    approximationMethod_.onChange([this]() {
        ap_ = &Approximations::approximations.at(approximationMethod_);
        importanceSumCoefficients_.setMinValue(ap_->minCoefficients);
        importanceSumCoefficients_.setMaxValue(ap_->maxCoefficients);
        opticalDepthCoefficients_.setMinValue(ap_->minCoefficients);
        opticalDepthCoefficients_.setMaxValue(ap_->maxCoefficients);
        initializeResources();
    });
    approximationMethod_.propertyModified();

    importanceSumCoefficients_.onChange([this]() { initializeResources(); });
    opticalDepthCoefficients_.onChange([this]() { initializeResources(); });

    smoothing_.addProperties(gaussianRadius_, gaussianSigma_);

    meshProperties_.addProperties(overrideColorBuffer_, overrideColor_);
    pointProperties_.addProperties(pointSize_, borderWidth_, borderColor_, antialising_);

    overrideColor_.setSemantics(PropertySemantics::Color)
        .visibilityDependsOn(overrideColorBuffer_, [](const BoolProperty p) { return p.get(); });

    layers_.addProperties(colorLayer_, texCoordLayer_, normalsLayer_, viewNormalsLayer_);

    for (auto& shader : meshShaders_) {
        shader.onReload([this]() { invalidate(InvalidationLevel::InvalidResources); });
    }
    for (auto& shader : pointShaders_) {
        shader.onReload([this]() { invalidate(InvalidationLevel::InvalidResources); });
    }
}

DirectOpacityOptimisationRenderer::~DirectOpacityOptimisationRenderer() = default;

void DirectOpacityOptimisationRenderer::initializeResources() {
    for (auto& shader : meshShaders_) {
        utilgl::addShaderDefines(shader, lightingProperty_);
        auto vert = shader.getVertexShaderObject();
        auto frag = shader.getFragmentShaderObject();

        vert->setShaderDefine("OVERRIDE_COLOR_BUFFER", overrideColorBuffer_);
        frag->setShaderDefine("COLOR_LAYER", colorLayer_);

        // add image load store extension
        frag->addShaderExtension("GL_EXT_shader_image_load_store", true);

        // set opacity optimisation defines
        frag->setShaderDefine("USE_IMPORTANCE_VOLUME", importanceVolume_.hasData());

        // set approximation defines
        frag->setShaderDefine("N_IMPORTANCE_SUM_COEFFICIENTS", true,
                              std::to_string(importanceSumCoefficients_).c_str());
        frag->setShaderDefine("N_OPTICAL_DEPTH_COEFFICIENTS", true,
                              std::to_string(opticalDepthCoefficients_).c_str());
        frag->setShaderDefine(ap_->shaderDefineName.c_str(), true);
        frag->setShaderDefine("INTEGER_COEFF_TEX", true);

        // account for background
        frag->setShaderDefine("BACKGROUND_AVAILABLE", imageInport_.hasData());

        shader.build();
    }

    // for (auto& shader : pointShaders_) {
    //     auto vert = shader.getVertexShaderObject();
    //     auto frag = shader.getFragmentShaderObject();

    //    // add image load store extension
    //    frag->addShaderExtension("GL_EXT_shader_image_load_store", true);

    //    // set opacity optimisation defines
    //    frag->setShaderDefine("USE_IMPORTANCE_VOLUME", importanceVolume_.hasData());

    //    // set approximation defines
    //    frag->setShaderDefine("N_IMPORTANCE_SUM_COEFFICIENTS", true,
    //                          std::to_string(importanceSumCoefficients_).c_str());
    //    frag->setShaderDefine("N_OPTICAL_DEPTH_COEFFICIENTS", true,
    //                          std::to_string(opticalDepthCoefficients_).c_str());
    //    frag->setShaderDefine(ap_->shaderDefineName.c_str(), true);
    //    frag->setShaderDefine("INTEGER_COEFF_TEX", true);

    //    // account for background
    //    frag->setShaderDefine("BACKGROUND_AVAILABLE", imageInport_.hasData());

    //    shader.build();
    //}

    {
        Shader& shader = clear_;

        auto vert = shader.getVertexShaderObject();
        auto frag = shader.getFragmentShaderObject();

        // add image load store extension
        frag->addShaderExtension("GL_EXT_shader_image_load_store", true);

        // set approximation defines
        frag->setShaderDefine("N_IMPORTANCE_SUM_COEFFICIENTS", true,
                              std::to_string(importanceSumCoefficients_).c_str());
        frag->setShaderDefine("N_OPTICAL_DEPTH_COEFFICIENTS", true,
                              std::to_string(opticalDepthCoefficients_).c_str());
        frag->setShaderDefine("INTEGER_COEFF_TEX", true);

        shader.build();
    }

    {
        Shader& shader = normalise_;

        auto vert = shader.getVertexShaderObject();
        auto frag = shader.getFragmentShaderObject();

        // add image load store extension
        frag->addShaderExtension("GL_EXT_shader_image_load_store", true);

        // set approximation defines
        frag->setShaderDefine("N_OPTICAL_DEPTH_COEFFICIENTS", true,
                              std::to_string(opticalDepthCoefficients_).c_str());
        frag->setShaderDefine(ap_->shaderDefineName.c_str(), true);
        frag->setShaderDefine("INTEGER_COEFF_TEX", true);
        shader.build();
    }

    {
        Shader& shader = background_;
        shader.build();
    }
}

void DirectOpacityOptimisationRenderer::process() {
    resizeBuffers(outport_.getDimensions());

    if (intermediateImage_.getDimensions() != outport_.getDimensions()) {
        intermediateImage_.setDimensions(outport_.getDimensions());
    }
    utilgl::activateAndClearTarget(intermediateImage_);

    utilgl::GlBoolState depthTest(GL_DEPTH_TEST, false);
    utilgl::CullFaceState culling(GL_NONE);

    // Bind textures
    importanceSumUnitMain_ = &textureUnits_.emplace_back();
    importanceSumUnitMain_->activate();
    importanceSumCoeffs_[0].bind();
    glBindImageTexture(importanceSumUnitMain_->getUnitNumber(), importanceSumCoeffs_[0].getID(), 0,
                       true, 0, GL_READ_WRITE, GL_R32F);

    importanceSumUnitSmooth_ = &textureUnits_.emplace_back();
    importanceSumUnitSmooth_->activate();
    importanceSumCoeffs_[1].bind();
    glBindImageTexture(importanceSumUnitSmooth_->getUnitNumber(), importanceSumCoeffs_[1].getID(),
                       0, true, 0, GL_READ_WRITE, GL_R32F);

    opticalDepthUnit_ = &textureUnits_.emplace_back();
    opticalDepthUnit_->activate();
    opticalDepthCoeffs_.bind();
    glBindImageTexture(opticalDepthUnit_->getUnitNumber(), opticalDepthCoeffs_.getID(), 0, true, 0,
                       GL_READ_WRITE, GL_R32F);

    // clear coefficient buffers
    clear_.activate();
    setUniforms(clear_);
    utilgl::singleDrawImagePlaneRect();
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    // Pass 1: Project importance
    renderGeometry(0);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    // Optional smoothing of importance coefficients
    if (smoothing_) {
        //// smoothing importance
        // gaussianKernel_.bindBase(8);

        //// horizontal pass
        // smoothH_.activate();
        // smoothH_.setUniform("radius", gaussianRadius_);
        // setUniforms(smoothH_);
        // utilgl::singleDrawImagePlaneRect();
        // glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

        //// vertical pass
        // smoothV_.activate();
        // smoothV_.setUniform("radius", gaussianRadius_);
        // setUniforms(smoothV_);
        // utilgl::singleDrawImagePlaneRect();
        // glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    }

    // Pass 2: Approximate importance and project opacities
    renderGeometry(1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    // Pass 3: Approximate blending, render to target
    utilgl::BlendModeState blending(GL_ONE, GL_ONE, GL_ONE, GL_ONE);
    depthTest = utilgl::GlBoolState(GL_DEPTH_TEST, false);
    renderGeometry(2);

    // normalise
    utilgl::activateAndClearTarget(outport_);
    normalise_.activate();
    utilgl::bindAndSetUniforms(normalise_, textureUnits_, intermediateImage_, "image",
                               ImageType::ColorOnly);
    normalise_.setUniform("opticalDepthCoeffs", opticalDepthUnit_->getUnitNumber());
    utilgl::singleDrawImagePlaneRect();

    // Add background if it exists
    if (imageInport_.hasData()) {
        blending = utilgl::BlendModeState(GL_ONE_MINUS_DST_ALPHA, GL_DST_ALPHA);
        background_.activate();
        utilgl::bindAndSetUniforms(background_, textureUnits_, *imageInport_.getData(), "bg",
                                   ImageType::ColorDepth);
        background_.setUniform("reciprocalDimensions", vec2(1) / vec2(screenSize_));
        utilgl::singleDrawImagePlaneRect();
    }

    glUseProgram(0);
    textureUnits_.clear();
    utilgl::deactivateCurrentTarget();
}

void DirectOpacityOptimisationRenderer::setUniforms(Shader& shader) {
    shader.setUniform("screenSize", ivec2(screenSize_));

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

void DirectOpacityOptimisationRenderer::renderGeometry(int pass) {
    // Mesh
    meshShaders_[pass].activate();
    utilgl::setUniforms(meshShaders_[pass], camera_, lightingProperty_, overrideColor_);
    setUniforms(meshShaders_[pass]);
    for (auto mesh : inport_) {
        // if (mesh->getDefaultMeshInfo().dt == DrawType::Triangles) {
        utilgl::setShaderUniforms(meshShaders_[pass], *mesh, "geometry");
        meshShaders_[pass].setUniform("pickingEnabled", meshutil::hasPickIDBuffer(mesh.get()));
        MeshDrawerGL::DrawObject drawer{mesh->getRepresentation<MeshGL>(),
                                        mesh->getDefaultMeshInfo()};
        drawer.draw();
        if (pass < 2) glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        //}
    }

    //    // Lines
    //    for (const auto& mesh : inport_) {
    //        lineRenderer_.render(*mesh, camera_.get(), outport_.getDimensions(), &lineSettings_,
    //        pass);
    //        if (pass < 2)
    //            glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    //    }
    //
    //    // Points
    //    utilgl::GlBoolState pointSprite(GL_PROGRAM_POINT_SIZE, true);
    //    utilgl::PolygonModeState polygon(GL_POINT, 1.0f, pointSize_.get());
    //    pointShaders_[pass].activate();
    //
    //    utilgl::setUniforms(pointShaders_[pass], camera_, lightingProperty_, pointSize_,
    //    borderWidth_,
    //                        borderColor_, antialising_);
    //    setUniforms(pointShaders_[pass]);
    //    for (auto elem : inport_) {
    //        if (elem->getDefaultMeshInfo().dt == DrawType::Points) {
    //            MeshDrawerGL::DrawObject drawer(elem->getRepresentation<MeshGL>(),
    //                                            elem->getDefaultMeshInfo());
    //            utilgl::setShaderUniforms(pointShaders_[pass], *elem, "geometry");
    //            drawer.draw(MeshDrawerGL::DrawMode::Points);
    //        if (pass < 2)
    //            glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    //        }
    //    }
}

void DirectOpacityOptimisationRenderer::resizeBuffers(size2_t screenSize) {
    if (screenSize != screenSize_) {
        screenSize_ = screenSize;
        importanceSumCoeffs_[0].uploadAndResize(
            nullptr, size3_t(screenSize.x, screenSize.y, importanceSumCoefficients_));
        importanceSumCoeffs_[1].uploadAndResize(
            nullptr, size3_t(screenSize.x, screenSize.y, importanceSumCoefficients_));
        opticalDepthCoeffs_.uploadAndResize(
            nullptr, size3_t(screenSize.x, screenSize.y, opticalDepthCoefficients_));
    }
}

}  // namespace inviwo
