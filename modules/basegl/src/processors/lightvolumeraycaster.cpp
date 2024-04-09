/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2024 Inviwo Foundation
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

#include <modules/basegl/processors/lightvolumeraycaster.h>

#include <inviwo/core/algorithm/boundingbox.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo LightVolumeRaycaster::processorInfo_{
    "org.inviwo.LightVolumeRaycaster",                                 // Class identifier
    "Light Volume Raycaster",                                          // Display name
    "Volume Rendering",                                                // Category
    CodeState::Experimental,                                           // Code state
    Tag::GL | Tag{"Volume"} | Tag{"Raycaster"} | Tag{"Light Volume"},  // Tags
    R"(
Processor for visualizing volumetric data by means of volume raycasting in combination with a
light volume. Only one channel of the volume will be used. Besides the volume data, entry and
exit point locations of the bounding box are required as well as a light volume. These can be
created with the `EntryExitPoints` processor and the `Light Volume` processor. The camera
properties between these this processor and the `EntryExitPoints` processor need to be linked.
)"_unindentHelp,
};
const ProcessorInfo LightVolumeRaycaster::getProcessorInfo() const { return processorInfo_; }

LightVolumeRaycaster::LightVolumeRaycaster(std::string_view identifier,
                                           std::string_view displayName)
    : VolumeRaycasterBase(identifier, displayName)
    , volume_{"volume"}
    , entryExit_{}
    , lightVolume_{*this, volume_.getName(), volume_.getGradientString()}
    , background_{*this}
    , isoTF_{volume_.volumePort}
    , raycasting_{volume_.getName(), isoTF_.isotfs[0]}
    , camera_{"camera", util::boundingBox(volume_.volumePort)}
    , positionIndicator_{}
    , sampleTransform_{} {

    volume_.volumePort.onChange([this]() {
        if (volume_.volumePort.hasData()) {
            const auto channels = volume_.volumePort.getData()->getDataFormat()->getComponents();
            raycasting_.setUsedChannels(channels);
            lightVolume_.setUsedChannels(channels);
        }
    });

    registerComponents(volume_, lightVolume_, entryExit_, background_, raycasting_, isoTF_, camera_,
                       positionIndicator_, sampleTransform_);
}

}  // namespace inviwo
