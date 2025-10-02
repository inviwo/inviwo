/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2025 Inviwo Foundation
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

#include <modules/basegl/processors/raycasting/acceleratedvolumeraycaster.h>

#include <inviwo/core/algorithm/boundingbox.h>                          // for boundingBox
#include <inviwo/core/datastructures/representationconverter.h>         // for RepresentationCon...
#include <inviwo/core/datastructures/representationconverterfactory.h>  // for RepresentationCon...
#include <inviwo/core/ports/volumeport.h>                               // for VolumeInport
#include <inviwo/core/processors/processorinfo.h>                       // for ProcessorInfo
#include <inviwo/core/processors/processorstate.h>                      // for CodeState, CodeSt...
#include <inviwo/core/processors/processortags.h>                       // for Tag, Tags::GL, Tags
#include <inviwo/core/properties/isotfproperty.h>                       // for IsoTFProperty
#include <inviwo/core/util/formats.h>                                   // for DataFormatBase
#include <inviwo/core/util/zip.h>                                       // for zipper
#include <modules/basegl/processors/raycasting/volumeraycasterbase.h>   // for VolumeRaycasterBase
#include <modules/basegl/shadercomponents/cameracomponent.h>            // for CameraComponent
#include <modules/basegl/shadercomponents/isotfcomponent.h>             // for IsoTFComponent
#include <modules/basegl/shadercomponents/raycastingcomponent.h>        // for RaycastingComponent
#include <modules/basegl/shadercomponents/volumecomponent.h>            // for VolumeComponent

#include <functional>   // for __base
#include <string>       // for string
#include <type_traits>  // for remove_extent_t

#include <fmt/core.h>  // for basic_string_view

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo AcceleratedVolumeRaycaster::processorInfo_{
    "org.inviwo.AcceleratedVolumeRaycaster",      // Class identifier
    "Accelerated Volume Raycaster",               // Display name
    "Volume Rendering",                           // Category
    CodeState::Experimental,                      // Code state
    Tags::GL | Tag{"Volume"} | Tag{"Raycaster"},  // Tags
    R"(
    Processor for visualizing volumetric data by means of volume raycasting. Only one channel of the
    volume will be used. Besides the volume data, entry and exit point locations of the bounding box
    are required. These can be created with the EntryExitPoints processor. The camera properties
    between these two processors need to be linked.
    )"_unindentHelp,
};

const ProcessorInfo& AcceleratedVolumeRaycaster::getProcessorInfo() const { return processorInfo_; }

AcceleratedVolumeRaycaster::AcceleratedVolumeRaycaster(std::string_view identifier,
                                                       std::string_view displayName)
    : VolumeRaycasterBase(identifier, displayName)
    , volume_{"volume"}
    , entryExit_{}
    , background_{*this}
    , isoTF_{volume_.volumePort}
    , accelerate_{volume_.volumePort, isoTF_.isotfs[0]}
    , raycasting_{volume_.getName(), isoTF_.isotfs[0]}
    , camera_{"camera", util::boundingBox(volume_.volumePort)}
    , light_{&camera_.camera}
    , positionIndicator_{}
    , sampleTransform_{} {

    volume_.volumePort.onChange([this]() {
        if (volume_.volumePort.hasData()) {
            const auto channels = volume_.volumePort.getData()->getDataFormat()->getComponents();
            raycasting_.setUsedChannels(channels);
        }
    });

    registerComponents(volume_, entryExit_, background_, raycasting_, isoTF_, accelerate_, camera_,
                       light_, positionIndicator_, sampleTransform_);

    addPort(accelerate_.accVol);
}

AcceleratedVolumeRaycaster::~AcceleratedVolumeRaycaster() = default;

void AcceleratedVolumeRaycaster::process() {
    auto data = outport_.getEditableData();
    if (data->getNumberOfColorLayers() < 2) {
        auto layer0 = data->getColorLayer(0);
        auto layer1 = std::make_shared<Layer>(layer0->config());
        data->addColorLayer(layer1);
    }

    accelerate_.preprocess();
    VolumeRaycasterBase::process();
}

}  // namespace inviwo
