/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2023 Inviwo Foundation
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

#include <inviwo/pathtracing/processors/volumepathtracer.h>
#include <modules/opengl/openglmodule.h>
#include <modules/opengl/shader/shadermanager.h>
#include <modules/opengl/inviwoopengl.h>

#include <inviwo/core/datastructures/image/image.h>
#include <inviwo/core/datastructures/image/layer.h>
#include <modules/opengl/texture/texture2d.h>
#include <modules/opengl/image/layergl.h>
#include <modules/opengl/image/imagegl.h>

#include <modules/opengl/texture/textureutils.h>
#include <modules/opengl/texture/textureunit.h>
#include <modules/opengl/shader/shaderutils.h>
#include <modules/opengl/volume/volumegl.h>     // IWYU pragma: keep
#include <modules/opengl/volume/volumeutils.h>  // for bindAndSetUniforms

#include <inviwo/core/properties/cameraproperty.h>             // for CameraProperty
#include <inviwo/core/properties/raycastingproperty.h>         // for RaycastingProperty
#include <inviwo/core/properties/volumeindicatorproperty.h>    // for VolumeIndicatorPr...
#include <inviwo/core/properties/optionproperty.h>             // for OptionPropertyOption
#include <inviwo/core/algorithm/boundingbox.h>                 // for boundingBox
#include <inviwo/core/datastructures/light/baselightsource.h>  // for Lights
#include <inviwo/core/ports/bufferport.h>                      // for Lights

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo VolumePathTracer::processorInfo_{
    "org.inviwo.VolumePathTracer",  // Class identifier
    "Volume Path Tracer",           // Display name
    "Volume Rendering",             // Category
    CodeState::Experimental,        // Code state
    Tags::GL,                       // Tags
};

const ProcessorInfo VolumePathTracer::getProcessorInfo() const { return processorInfo_; }

VolumePathTracer::VolumePathTracer()
    : Processor()
    , volumePort_("Volume")
    , entryPort_("entry")
    , exitPort_("exit")
    , outport_("outport", DataVec4Float32::get())
    , shader_({{ShaderType::Compute, "bidirectionalvolumepathtracer.comp"}}, Shader::Build::No)
    , shaderUniform_({{ShaderType::Compute, "bidirectionalvolumepathtraceruniform.comp"}},
                     Shader::Build::No)
    , channel_("channel", "Render Channel", {{"Channel 1", "Channel 1", 0}}, 0)
    , raycasting_("raycaster", "Raycasting")
    , transferFunction_("transferFunction", "Transfer Function", &volumePort_)
    , camera_("camera", "Camera", util::boundingBox(volumePort_))
    , positionIndicator_("positionindicator", "Position Indicator")
    , light_("light", "Light", &camera_)
    
    , transmittanceMethod_(
          "transmittanceMethod", "Transmittance method",
          {
              {"Woodcock", "Woodcock", TransmittanceMethod::Woodcock},
              {"RatioTracking", "Ratio Tracking", TransmittanceMethod::RatioTracking},
              {"ResidualRatioTracking", "Residual Ratio Tracking",
               TransmittanceMethod::ResidualRatioTracking},
              {"PoissonRatioTracking", "Poisson Ratio Tracking",
               TransmittanceMethod::PoissonRatioTracking},
              {"PoissonResidualRatioTracking", "Poisson Residual Ratio Tracking",
               TransmittanceMethod::PoissonResidualRatioTracking},
              {"IndependentPoissonTracking", "Independent Poisson Tracking",
               TransmittanceMethod::IndependentPoissonTracking},
              {"DependentMultiPoissonTracking", "Dependent Multi Poisson Tracking",
               TransmittanceMethod::IndependentPoissonTracking},
              {"GeometricResidualTracking", "Geometric Residual Ratio Tracking",
               TransmittanceMethod::GeometricResidualTracking},
          })
    , iterateRender_("iterate", "Iterate render")
    , enableProgressiveRefinement_("enableRefinement", "Enable progressive refinement", false)
    , invalidateRender_("invalidate", "Invalidate render", [this]() { invalidateProgressiveRendering(); })
    , accelerate_("accelerate", "Accelerate", true, InvalidationLevel::InvalidResources)
    , volumeRegionSize_("region", "Region size", 8, 1, 100)
    , minMaxAvgShader_({{ShaderType::Compute, "volume/regionminmaxavg.comp"}})
    , progressiveTimer_(Timer::Milliseconds(0), std::bind(&VolumePathTracer::onTimerEvent, this)) {

    addPort(volumePort_, "VolumePortGroup");
    addPort(entryPort_, "ImagePortGroup1");
    addPort(exitPort_, "ImagePortGroup1");
    addPort(outport_, "ImagePortGroup1");

    volumePort_.onChange([this]() { invalidateProgressiveRendering(); });
    entryPort_.onChange([this]() { invalidateProgressiveRendering(); });
    exitPort_.onChange([this]() { invalidateProgressiveRendering(); });

    channel_.setSerializationMode(PropertySerializationMode::All);

    auto updateTFHistSel = [this]() {
        HistogramSelection selection{};
        selection[channel_] = true;
        transferFunction_.setHistogramSelection(selection);
    };
    updateTFHistSel();
    channel_.onChange(updateTFHistSel);

    volumePort_.onChange([this]() {
        if (volumePort_.hasData()) {
            size_t channels = volumePort_.getData()->getDataFormat()->getComponents();

            if (channels == channel_.size()) return;

            std::vector<OptionPropertyIntOption> channelOptions;
            for (size_t i = 0; i < channels; i++) {
                channelOptions.emplace_back("Channel " + toString(i + 1),
                                            "Channel " + toString(i + 1), static_cast<int>(i));
            }
            channel_.replaceOptions(channelOptions);
            channel_.setCurrentStateAsDefault();
        }
    });

    accelerate_.addProperty(volumeRegionSize_);


    raycasting_.gradientComputation_.onChange([this]() {
        if (channel_.size() == 4) {
            if (raycasting_.gradientComputation_.get() ==
                RaycastingProperty::GradientComputation::PrecomputedXYZ) {
                channel_.set(3);
            } else if (raycasting_.gradientComputation_.get() ==
                       RaycastingProperty::GradientComputation::PrecomputedYZW) {
                channel_.set(0);
            }
        }
    });

    shader_.getComputeShaderObject()->addShaderExtension(
        "GL_NV_compute_shader_derivatives", ShaderObject::ExtensionBehavior::Enable);
    shaderUniform_.getComputeShaderObject()->addShaderExtension(
        "GL_NV_compute_shader_derivatives", ShaderObject::ExtensionBehavior::Enable);
    shader_.onReload([this]() {
        invalidate(InvalidationLevel::InvalidResources);
        invalidateProgressiveRendering();
    });
    shaderUniform_.onReload([this]() {
        invalidate(InvalidationLevel::InvalidResources);
        invalidateProgressiveRendering();
    });
    minMaxAvgShader_.onReload([this]() {
        invalidate(InvalidationLevel::InvalidResources);
        invalidateProgressiveRendering();
    });

    transmittanceMethod_.onChange([this]() {
        invalidate(InvalidationLevel::InvalidOutput);
        invalidateProgressiveRendering();
    });

    // Used for determining uniform float t_ms
    timeStart_ = std::chrono::high_resolution_clock::now();

    addProperties(channel_, raycasting_, transferFunction_, camera_, positionIndicator_, light_,
                  accelerate_, transmittanceMethod_, iterateRender_,
                  enableProgressiveRefinement_, invalidateRender_);

    transferFunction_.onChange([this]() { invalidateProgressiveRendering(); });
    light_.onChange([this]() { invalidateProgressiveRendering(); });

    enableProgressiveRefinement_.onChange([this]() { progressiveRefinementChanged(); });

    progressiveRefinementChanged();
}

void VolumePathTracer::initializeResources() {
    invalidateProgressiveRendering();

    if (accelerate_) {
        activeShader_ = &shaderUniform_;
    } else {
        activeShader_ = &shader_;
    }
 
    utilgl::addShaderDefines(*activeShader_, raycasting_);
    utilgl::addShaderDefines(*activeShader_, camera_);
    utilgl::addShaderDefines(*activeShader_, light_);
    activeShader_->build();
}

void VolumePathTracer::process() {
    auto inputVolume = volumePort_.getData();
    if (transferFunction_.isModified() && accelerate_) {
        
        volumeRegionMinMaxAvg(inputVolume, volumeRegionSize_.get());
    }
    // Partial seeding for random values
    timeNow_ = std::chrono::high_resolution_clock::now();
    using FpMilliseconds = std::chrono::duration<float, std::chrono::milliseconds::period>;
    float MSSinceStart_ = FpMilliseconds(timeNow_ - timeStart_).count();

    if (iteration_ == 0) {
        // Copy depth and picking
        Image* outImage = outport_.getEditableData().get();
        ImageGL* outImageGL = outImage->getEditableRepresentation<ImageGL>();
        entryPort_.getData()->getRepresentation<ImageGL>()->copyRepresentationsTo(outImageGL);
    }
    activeShader_->activate();
    activeShader_->setUniform("time_ms", MSSinceStart_);
    activeShader_->setUniform("iteration", iteration_);
    activeShader_->setUniform("transmittanceMethod",
                              static_cast<int>(transmittanceMethod_.getSelectedIndex()));
    TextureUnitContainer units;
    /*
    Compute Shader specific impl to read and/or write to textures using GLSL imageND() function
    call: utilgl::bindAndSetUniforms(shader_, units, outport_, ImageType::ColorDepthPicking);
    */
    {
        TextureUnit unit1, unit2, unit3;
        auto image = outport_.getEditableData();
        auto colorLayerGL = image->getColorLayer()->getEditableRepresentation<LayerGL>();
        colorLayerGL->bindImageTexture(unit1, GL_READ_WRITE);
        auto depthLayerGL = image->getDepthLayer()->getEditableRepresentation<LayerGL>();

        GLenum texUnit_ = unit2.getEnum();
        glActiveTexture(texUnit_);
        glBindImageTexture(unit2.getUnitNumber(), depthLayerGL->getTexture()->getID(), 0, GL_FALSE,
                           0, GL_READ_WRITE,
                           GL_R32F /*depthLayerGL->getTexture()->getInternalFormat()*/);
        glActiveTexture(GL_TEXTURE0);

        auto pickingLayerGL = image->getPickingLayer()->getEditableRepresentation<LayerGL>();
        pickingLayerGL->bindImageTexture(unit3, GL_WRITE_ONLY);

        activeShader_->setUniform("outportColor", unit1);
        activeShader_->setUniform("outportDepth", unit2);
        activeShader_->setUniform("outportPicking", unit3);

        units.push_back(std::move(unit1));
        units.push_back(std::move(unit2));
        units.push_back(std::move(unit3));

        StrBuffer buff;

        utilgl::setShaderUniforms(*activeShader_, *image,
                                  buff.replace("{}Parameters", outport_.getIdentifier()));
    }

    utilgl::bindAndSetUniforms(*activeShader_, units, entryPort_, ImageType::ColorDepthPicking);

    utilgl::bindAndSetUniforms(*activeShader_, units, exitPort_, ImageType::ColorDepth);
    utilgl::bindAndSetUniforms(*activeShader_, units, *volumePort_.getData(), "volume");
    if (accelerate_) {
        utilgl::bindAndSetUniforms(*activeShader_, units, *optimizedEntryPort_.getData(), "entry",
                                   ImageType::ColorOnly);
        utilgl::bindAndSetUniforms(*activeShader_, units, *regionMinMaxVolume_,
                                   "minMaxOpacity");
        utilgl::bindAndSetUniforms(*activeShader_, units, *regionAvgOpacityVolume_,
                                   "avgOpacity");
    }

    utilgl::bindAndSetUniforms(*activeShader_, units, transferFunction_);
    const auto* tfLayer = transferFunction_.getRepresentation<LayerGL>();
    activeShader_->setUniform("tfSize", static_cast<int>(tfLayer->getDimensions().x));

    utilgl::setUniforms(*activeShader_, camera_, raycasting_, positionIndicator_, light_, channel_);

    activeShader_->setUniform("cellDim", ivec3(volumeRegionSize_));

    // Start render
    glDispatchCompute(glm::ceil(static_cast<float>(outport_.getDimensions().x) / 16.f),
                      glm::ceil(static_cast<float>(outport_.getDimensions().y) / 16.f), 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    activeShader_->deactivate();

    ++iteration_;
}


void VolumePathTracer::volumeRegionMinMaxAvg(std::shared_ptr<const inviwo::Volume>& inputVolume,
                                              const int regionSize) {
    const auto dim = inputVolume->getDimensions();
    const size3_t outDim{glm::ceil(vec3(dim) / static_cast<float>(regionSize))};
    if (!regionMinMaxVolume_ || glm::any(glm::notEqual(outDim, regionMinMaxVolume_->getDimensions()))) {
        regionMinMaxVolume_ =
            std::make_shared<Volume>(outDim, DataVec4Float32::get(), swizzlemasks::redGreen,
                                           InterpolationType::Nearest, inputVolume->getWrapping());
        // Opacity is defined between [0 1]
        regionMinMaxVolume_->dataMap.dataRange = dvec2(0, 1);
        regionMinMaxVolume_->dataMap.valueRange = dvec2(0, 1);

        regionMinMaxVolume_->setModelMatrix(inputVolume->getModelMatrix());
        regionMinMaxVolume_->setWorldMatrix(inputVolume->getWorldMatrix());

        regionAvgOpacityVolume_ =
            std::make_shared<Volume>(outDim, DataFloat32::get(), swizzlemasks::luminance,
                                     InterpolationType::Linear, inputVolume->getWrapping());
        // Opacity is defined between [0 1]
        regionAvgOpacityVolume_->dataMap.dataRange = dvec2(0, 1);
        regionAvgOpacityVolume_->dataMap.valueRange = dvec2(0, 1);

        regionAvgOpacityVolume_->setModelMatrix(inputVolume->getModelMatrix());
        regionAvgOpacityVolume_->setWorldMatrix(inputVolume->getWorldMatrix());
    }

    // Compute Shader set up
    minMaxAvgShader_.activate();

    TextureUnitContainer units;

    minMaxAvgShader_.setUniform("regionSize", inviwo::ivec3(regionSize));

    utilgl::bindAndSetUniforms(minMaxAvgShader_, units, *inputVolume, "volumeData");
    // I need to write to it, and i dont think bindandset lets me do that

    /*utilgl::bindAndSetUniforms(shader_, units, *stats, "opacityData"); // with write capabilites*/
    {
        TextureUnit unit;
        auto statsGL = regionMinMaxVolume_->getEditableRepresentation<VolumeGL>();
        statsGL->bindImageTexture(unit.getEnum(), unit.getUnitNumber(), GL_WRITE_ONLY);

        minMaxAvgShader_.setUniform("regionMinMaxOpacity", unit);

        units.push_back(std::move(unit));
        StrBuffer buff;

        TextureUnit avgOpacityUnit;
        auto avgOpacityGL = regionAvgOpacityVolume_->getEditableRepresentation<VolumeGL>();
        avgOpacityGL->bindImageTexture(avgOpacityUnit.getEnum(), avgOpacityUnit.getUnitNumber(),
                                       GL_WRITE_ONLY);

        minMaxAvgShader_.setUniform("regionAvgOpacity", avgOpacityUnit);

        units.push_back(std::move(avgOpacityUnit));

    }

    utilgl::bindAndSetUniforms(minMaxAvgShader_, units, transferFunction_);
    minMaxAvgShader_.setUniform("tfSize", static_cast<int>(transferFunction_.getLookUpTableSize()));

    // dispatch size? The workload can be anything from 1 to 150^3
    // 150/regionsize in all dimensions? We can start with a 'naive' (stupid) implementation and let
    // the dispatch be for stats.dim. Each compute shader will do the inner 3loops work.

    glDispatchCompute(outDim.x, outDim.y, outDim.z);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    minMaxAvgShader_.deactivate();
}
void VolumePathTracer::updateLightSources() {}

// Progressive refinement
void VolumePathTracer::invalidateProgressiveRendering() { iteration_ = 0; }

void VolumePathTracer::evaluateProgressiveRefinement() {
    invalidate(InvalidationLevel::InvalidOutput);
}

void VolumePathTracer::progressiveRefinementChanged() {
    if (enableProgressiveRefinement_.get()) {
        progressiveTimer_.start(Timer::Milliseconds(0));
    } else {
        progressiveTimer_.stop();
    }
}

void VolumePathTracer::onTimerEvent() { iterateRender_.pressButton(); }

}  // namespace inviwo
