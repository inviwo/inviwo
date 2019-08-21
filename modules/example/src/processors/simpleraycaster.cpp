/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2019 Inviwo Foundation
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

#include <modules/example/processors/simpleraycaster.h>

#include <inviwo/core/datastructures/volume/volume.h>
#include <inviwo/core/util/rendercontext.h>
#include <modules/opengl/volume/volumegl.h>
#include <modules/opengl/texture/textureunit.h>
#include <modules/opengl/texture/textureutils.h>
#include <modules/opengl/shader/shaderutils.h>
#include <modules/opengl/volume/volumeutils.h>
#include <inviwo/core/algorithm/boundingbox.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo SimpleRaycaster::processorInfo_{
    "org.inviwo.SimpleRaycaster",    // Class identifier
    "Simple Raycaster",              // Display name
    "Volume Rendering",              // Category
    CodeState::Stable,               // Code state
    "GL, DVR, Raycasting, Example",  // Tags
};
const ProcessorInfo SimpleRaycaster::getProcessorInfo() const { return processorInfo_; }

SimpleRaycaster::SimpleRaycaster()
    : Processor()
    , shader_("simpleraycasting.frag")
    , volumePort_("volume")
    , entryPort_("entry")
    , exitPort_("exit")
    , outport_("outport")
    , channel_("channel", "Render Channel", {{"channel1", "Channel 1", 0}})
    , transferFunction_("transferFunction", "Transfer Function", &volumePort_)
    , samplingRate_("samplingRate", "Sampling Rate", 2.0f, 1.0f, 10.0f)
    , camera_("camera", "Camera", util::boundingBox(volumePort_)) {

    shader_.onReload([this]() { invalidate(InvalidationLevel::InvalidResources); });

    addPort(volumePort_, "VolumePortGroup");
    addPort(entryPort_, "ImagePortGroup1");
    addPort(exitPort_, "ImagePortGroup1");
    addPort(outport_, "ImagePortGroup1");

    channel_.setSerializationMode(PropertySerializationMode::All);

    // adjust option property for available channels when the input volume changes
    volumePort_.onChange([this]() {
        if (volumePort_.hasData()) {
            size_t channels = volumePort_.getData()->getDataFormat()->getComponents();

            if (channels == channel_.size()) return;

            std::vector<OptionPropertyIntOption> channelOptions;
            for (size_t i = 0; i < channels; i++) {
                channelOptions.emplace_back("channel" + toString(i + 1),
                                            "Channel " + toString(i + 1), static_cast<int>(i));
            }
            channel_.replaceOptions(channelOptions);
            channel_.setCurrentStateAsDefault();
        }
    });
    addProperty(channel_);
    addProperty(transferFunction_);
    addProperty(samplingRate_);
    addProperty(camera_);
}

void SimpleRaycaster::process() {
    if (volumePort_.isChanged()) {
        auto newVolume = volumePort_.getData();

        // ensure that the volume has a GL representation
        // if not, create it in a background thread
        if (newVolume->hasRepresentation<VolumeGL>()) {
            loadedVolume_ = newVolume;
        } else {
            dispatchPool([this, newVolume]() {
                RenderContext::getPtr()->activateLocalRenderContext();
                newVolume->getRep<kind::GL>();
                glFinish();
                dispatchFront([this, newVolume]() {
                    loadedVolume_ = newVolume;
                    invalidate(InvalidationLevel::InvalidOutput);
                });
            });
        }
    }
    if (!loadedVolume_) return;

    // clear output image
    utilgl::activateAndClearTarget(outport_);

    // activate volume rendering shader
    shader_.activate();

    TextureUnitContainer units;
    // bind textures for volume, transfer function, and entry-/exit points
    utilgl::bindAndSetUniforms(shader_, units, *loadedVolume_, "volume");
    utilgl::bindAndSetUniforms(shader_, units, transferFunction_);
    utilgl::bindAndSetUniforms(shader_, units, entryPort_, ImageType::ColorDepthPicking);
    utilgl::bindAndSetUniforms(shader_, units, exitPort_, ImageType::ColorDepth);

    utilgl::setUniforms(shader_, outport_, camera_, channel_, samplingRate_);

    // render a full-screen quad to trigger the volume rendering
    utilgl::singleDrawImagePlaneRect();

    shader_.deactivate();
    utilgl::deactivateCurrentTarget();
}

}  // namespace inviwo
