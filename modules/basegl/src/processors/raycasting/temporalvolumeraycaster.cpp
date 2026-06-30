/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2026 Inviwo Foundation
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

#include <modules/basegl/processors/raycasting/temporalvolumeraycaster.h>

#include <inviwo/core/algorithm/boundingbox.h>
#include <inviwo/core/datastructures/volume/temporalvolume.h>
#include <modules/basegl/shadercomponents/shadercomponentutil.h>

#include <optional>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo TemporalVolumeRaycaster::processorInfo_{
    "org.inviwo.TemporalVolumeRaycaster",  // Class identifier
    "Temporal Volume Raycaster",           // Display name
    "Volume Rendering",                    // Category
    CodeState::Experimental,               // Code state
    Tags::GL | Tag{"Volume"} | Tag{"Raycaster"} | Tag{"Temporal"},
    R"(Raycaster for time-dependent volumetric data. Consumes a TemporalVolume and performs the
    time interpolation on the GPU: the two frames bracketing the current time are uploaded as 3D
    textures and blended in the shader, giving smooth transitions between time steps. Besides the
    volume data, entry and exit point locations of the bounding box are required. These can be
    created with the EntryExitPoints processor.)"_unindentHelp,
};

const ProcessorInfo& TemporalVolumeRaycaster::getProcessorInfo() const { return processorInfo_; }

namespace {

std::function<std::optional<dmat4>()> temporalBoundingBox(const TemporalVolumeInport& port) {
    return [&port]() -> std::optional<dmat4> {
        if (auto data = port.getData(); data && !data->empty()) {
            return util::calcBoundingBox(data->prototype());
        }
        return std::nullopt;
    };
}

}  // namespace

TemporalVolumeRaycaster::TemporalVolumeRaycaster(std::string_view identifier,
                                                 std::string_view displayName)
    : VolumeRaycasterBase(identifier, displayName)
    , volume_{"volume", TemporalVolumeComponent::Gradients::Single,
              "input temporal volume (Only one channel will be rendered)"_help}
    , entryExit_{}
    , background_{*this}
    , raycasting_{volume_.getName(), volume_.isoTF()}
    , camera_{"camera", temporalBoundingBox(volume_.volumePort)}
    , light_{&camera_.camera}
    , positionIndicator_{}
    , sampleTransform_{} {

    registerComponents(volume_, entryExit_, background_, raycasting_, camera_, light_,
                       positionIndicator_, sampleTransform_);
}

void TemporalVolumeRaycaster::process() {
    util::checkValidChannel(raycasting_.selectedChannel(), volume_.channelsForVolume().value_or(0));

    VolumeRaycasterBase::process();
}

}  // namespace inviwo
