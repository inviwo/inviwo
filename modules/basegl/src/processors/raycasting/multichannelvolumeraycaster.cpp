/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2021-2024 Inviwo Foundation
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

#include <modules/basegl/processors/raycasting/multichannelvolumeraycaster.h>

#include <inviwo/core/algorithm/boundingbox.h>                          // for boundingBox
#include <inviwo/core/datastructures/histogram.h>                       // for HistogramSelection
#include <inviwo/core/datastructures/representationconverter.h>         // for RepresentationCon...
#include <inviwo/core/datastructures/representationconverterfactory.h>  // for RepresentationCon...
#include <inviwo/core/ports/volumeport.h>                               // for VolumeInport
#include <inviwo/core/processors/processorinfo.h>                       // for ProcessorInfo
#include <inviwo/core/processors/processorstate.h>                      // for CodeState, CodeSt...
#include <inviwo/core/processors/processortags.h>                       // for Tag, Tag::GL, Tags
#include <inviwo/core/properties/isotfproperty.h>                       // for IsoTFProperty
#include <inviwo/core/util/formats.h>                                   // for DataFormatBase
#include <inviwo/core/util/stdextensions.h>                             // for make_array
#include <inviwo/core/util/zip.h>                                       // for zipper, enumerate
#include <modules/basegl/processors/raycasting/volumeraycasterbase.h>   // for VolumeRaycasterBase
#include <modules/basegl/shadercomponents/cameracomponent.h>            // for CameraComponent
#include <modules/basegl/shadercomponents/isotfcomponent.h>             // for IsoTFComponent
#include <modules/basegl/shadercomponents/raycastingcomponent.h>        // for MultiRaycastingCo...
#include <modules/basegl/shadercomponents/volumecomponent.h>            // for VolumeComponent

#include <bitset>       // for bitset<>::reference
#include <functional>   // for ref, __base
#include <string>       // for string
#include <type_traits>  // for remove_extent_t

#include <fmt/core.h>  // for basic_string_view

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo MultiChannelVolumeRaycaster::processorInfo_{
    "org.inviwo.MultichannelVolumeRaycaster",    // Class identifier
    "Multichannel Volume Raycaster",             // Display name
    "Volume Rendering",                          // Category
    CodeState::Experimental,                     // Code state
    Tags::GL | Tag{"Volume"} | Tag{"Raycaster"}  // Tags
};
const ProcessorInfo MultiChannelVolumeRaycaster::getProcessorInfo() const { return processorInfo_; }

MultiChannelVolumeRaycaster::MultiChannelVolumeRaycaster(std::string_view identifier,
                                                         std::string_view displayName)
    : VolumeRaycasterBase(identifier, displayName)
    , volume_{"volume", VolumeComponent::Gradients::All}
    , entryExit_{}
    , background_{*this}
    , isoTFs_{volume_.volumePort}
    , raycasting_{volume_.getName(),
                  util::make_array<4>([&](auto i) { return std::ref(isoTFs_.isotfs[i]); })}
    , camera_{"camera", util::boundingBox(volume_.volumePort)}
    , light_{&camera_.camera}
    , positionIndicator_{}
    , sampleTransform_{} {

    for (auto&& [i, isotf] : util::enumerate(isoTFs_.isotfs)) {
        HistogramSelection selection{};
        selection[i] = true;
        isotf.setHistogramSelection(selection).setCurrentStateAsDefault();
    }

    volume_.volumePort.onChange([this]() {
        const auto channels = volume_.volumePort.hasData()
                                  ? volume_.volumePort.getData()->getDataFormat()->getComponents()
                                  : 4;
        if (raycasting_.setUsedChannels(channels)) {
            // The port onchange callback is invoked while evaluating the network
            // hence it is safe to call initializeResources here.
            // Also calls to invalidate will be ignored
            initializeResources();
        }
        for (auto&& [i, isotf] : util::enumerate(isoTFs_.isotfs)) {
            isotf.setVisible(i < channels);
        }
    });

    registerComponents(volume_, entryExit_, background_, isoTFs_, raycasting_, camera_, light_,
                       positionIndicator_, sampleTransform_);
}

}  // namespace inviwo
