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
    "VolumePathTracer",             // Display name
    "Volume",                       // Category
    CodeState::Experimental,        // Code state
    Tags::GL,                       // Tags
    R"(<Explanation of how to use the processor.>)"_unindentHelp};

const ProcessorInfo VolumePathTracer::getProcessorInfo() const { return processorInfo_; }

VolumePathTracer::VolumePathTracer()
    : Processor()
    , volumePort_("Volume")
    , entryPort_("entry")
    , exitPort_("exit")
    , outport_("outport")
    , shader_({{ShaderType::Compute, "bidirectionalvolumepathtracer.comp"}})
    , channel_("channel", "Render Channel", {{"Channel 1", "Channel 1", 0}}, 0)
    , raycasting_("raycaster", "Raycasting")
    , transferFunction_("transferFunction", "Transfer Function", &volumePort_)
    , camera_("camera", "Camera", util::boundingBox(volumePort_))
    , positionIndicator_("positionindicator", "Position Indicator")
    , light_("light", "Light", &camera_)
    , invalidateRendering_("iterate", "Invalidate rendering")
    , enableProgressiveRefinement_("enableRefinement", "Enable progressive refinement", false)
    , progressiveTimer_(Timer::Milliseconds(100),
                        std::bind(&VolumePathTracer::onTimerEvent, this)) {

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

    // Used for determining uniform float t_ms
    time_start = std::chrono::high_resolution_clock::now();

    addProperty(channel_);
    addProperty(raycasting_);
    addProperty(transferFunction_);
    transferFunction_.onChange([this]() { invalidateProgressiveRendering(); });

    addProperty(camera_);
    addProperty(positionIndicator_);

    addProperty(light_);
    light_.onChange([this]() { invalidateProgressiveRendering(); });

    addProperty(invalidateRendering_);
    addProperty(enableProgressiveRefinement_);
    enableProgressiveRefinement_.onChange([this]() { progressiveRefinementChanged(); });

    shader_.onReload([this]() {
        invalidate(InvalidationLevel::InvalidOutput);
        invalidateProgressiveRendering();
    });
    progressiveRefinementChanged();
}

void VolumePathTracer::process() {

    shader_.activate();

    // Partial seeding for random values
    time_now = std::chrono::high_resolution_clock::now();
    using FpMilliseconds = std::chrono::duration<float, std::chrono::milliseconds::period>;
    float t_ms = FpMilliseconds(time_now - time_start).count();

    if (iteration_ == 0) {
        // Copy depth and picking
        Image* outImage = outport_.getEditableData().get();
        ImageGL* outImageGL = outImage->getEditableRepresentation<ImageGL>();
        entryPort_.getData()->getRepresentation<ImageGL>()->copyRepresentationsTo(outImageGL);
    }

    TextureUnitContainer units;
    /*
    Stand in for:
    utilgl::bindAndSetUniforms(shader_, units, outport_, ImageType::ColorDepthPicking);
    */
    {
        TextureUnit unit1, unit2, unit3;
        auto image = outport_.getEditableData();
        auto colorLayerGL = image->getColorLayer()->getEditableRepresentation<LayerGL>();
        colorLayerGL->bindImageTexture(unit1, GL_READ_WRITE);
        auto depthLayerGL = image->getDepthLayer()->getEditableRepresentation<LayerGL>();
        depthLayerGL->bindImageTexture(unit2, GL_WRITE_ONLY);
        auto pickingLayerGL = image->getPickingLayer()->getEditableRepresentation<LayerGL>();
        pickingLayerGL->bindImageTexture(unit3, GL_WRITE_ONLY);

        shader_.setUniform("outportColor", unit1);
        shader_.setUniform("outportDepth", unit2);
        shader_.setUniform("outportPicking", unit3);

        units.push_back(std::move(unit1));
        units.push_back(std::move(unit2));
        units.push_back(std::move(unit3));

        StrBuffer buff;
        utilgl::setShaderUniforms(shader_, *image,
                                  buff.replace("{}Parameters", outport_.getIdentifier()));
    }

    utilgl::bindAndSetUniforms(shader_, units, entryPort_, ImageType::ColorDepthPicking);
    utilgl::bindAndSetUniforms(shader_, units, exitPort_, ImageType::ColorDepth);
    utilgl::bindAndSetUniforms(shader_, units, *volumePort_.getData(), "volume");
    utilgl::bindAndSetUniforms(shader_, units, transferFunction_);
    shader_.setUniform("time_ms", t_ms);
    shader_.setUniform("iteration", iteration_);

    utilgl::setUniforms(shader_, camera_, raycasting_, positionIndicator_, light_, channel_);

    // Start render
    glDispatchCompute(outport_.getDimensions().x / 16, outport_.getDimensions().y / 16, 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    shader_.deactivate();

    ++iteration_;
}

void VolumePathTracer::updateLightSources() {}

// Progressive refinement
void VolumePathTracer::invalidateProgressiveRendering() { iteration_ = 0; }

void VolumePathTracer::evaluateProgressiveRefinement() {
    invalidate(InvalidationLevel::InvalidOutput);
}

void VolumePathTracer::progressiveRefinementChanged() {
    if (enableProgressiveRefinement_.get()) {
        progressiveTimer_.start(Timer::Milliseconds(100));
    } else {
        progressiveTimer_.stop();
    }
}

void VolumePathTracer::onTimerEvent() { invalidateRendering_.pressButton(); }

}  // namespace inviwo
