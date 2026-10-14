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

#include <modules/basegl/processors/raycasting/analyticvolumeraycaster.h>

#include <inviwo/core/algorithm/boundingbox.h>
#include <modules/basegl/shadercomponents/shadercomponentutil.h>

namespace inviwo {

const ProcessorInfo AnalyticVolumeRaycaster::processorInfo_{
    "org.inviwo.AnalyticVolumeRaycaster",         // Class identifier
    "Analytic Volume Raycaster",                  // Display name
    "Volume Rendering",                           // Category
    CodeState::Experimental,                      // Code state
    Tags::GL | Tag{"Volume"} | Tag{"Raycaster"},  // Tags
    R"(Processor for visualizing volumetric data by means of volume raycasting. Only one channel of
    the volume will be used. Unlike the Standard Volume Raycaster, no entry and exit point images
    are needed. Instead, the entry and exit points are computed analytically in the shader by
    intersecting the camera rays with either a parallelepiped or a sphere. The geometry can be
    defined in the Data, Model, or World space of the volume, and is optionally clipped against
    the bounding box of the volume.)"_unindentHelp,
};

const ProcessorInfo& AnalyticVolumeRaycaster::getProcessorInfo() const { return processorInfo_; }

AnalyticVolumeRaycaster::AnalyticVolumeRaycaster(std::string_view identifier,
                                                 std::string_view displayName)
    : VolumeRaycasterBase(identifier, displayName)
    , volume_{"volume", VolumeComponent::Gradients::Single,
              "input volume (Only one channel will be rendered)"_help}
    , camera_{"camera", util::boundingBox(volume_.volumePort)}
    , entryExit_{volume_.volumePort, camera_.getName()}
    , background_{*this}
    , isoTF_{&volume_.volumePort}
    , raycasting_{volume_.getName(), isoTF_.isoTFs[0]}
    , light_{&camera_.camera}
    , positionIndicator_{}
    , sampleTransform_{}
    , mask_{volume_.volumePort} {

    registerComponents(volume_, entryExit_, background_, raycasting_, isoTF_, camera_, light_,
                       positionIndicator_, sampleTransform_, mask_);
}

void AnalyticVolumeRaycaster::process() {
    util::checkValidChannel(raycasting_.selectedChannel(), volume_.channelsForVolume().value_or(0));

    mask_.preprocess();

    VolumeRaycasterBase::process();
}

}  // namespace inviwo
