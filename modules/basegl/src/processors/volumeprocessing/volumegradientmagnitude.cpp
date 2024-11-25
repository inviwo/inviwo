/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2024 Inviwo Foundation
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

#include <inviwo/core/datastructures/datamapper.h>  // for DataMapper
#include <inviwo/core/datastructures/unitsystem.h>  // for Axis, Unit
#include <inviwo/core/ports/volumeport.h>           // for VolumeIn...
#include <inviwo/core/processors/processorinfo.h>   // for Processo...
#include <inviwo/core/processors/processorstate.h>  // for CodeState
#include <inviwo/core/processors/processortags.h>   // for Tags
#include <inviwo/core/properties/optionproperty.h>  // for OptionPr...
#include <inviwo/core/util/formats.h>               // for DataFloat32
#include <inviwo/core/util/glmvec.h>                // for dvec2
#include <inviwo/core/util/exception.h>
#include <modules/basegl/processors/volumeprocessing/volumeglprocessor.h>        // for VolumeGL...
#include <modules/basegl/processors/volumeprocessing/volumegradientmagnitude.h>  // for VolumeGr...
#include <modules/opengl/shader/shader.h>                                        // for Shader

#include <array>        // for array
#include <memory>       // for shared_ptr
#include <sstream>      // for stringst...
#include <string>       // for char_traits
#include <string_view>  // for string_view
#include <type_traits>  // for remove_e...

namespace inviwo {
class TextureUnitContainer;

const ProcessorInfo VolumeGradientMagnitude::processorInfo_{
    "org.inviwo.VolumeGradientMagnitude",  // Class identifier
    "Volume Gradient Magnitude",           // Display name
    "Volume Operation",                    // Category
    CodeState::Stable,                     // Code state
    Tags::GL,                              // Tags
};
const ProcessorInfo VolumeGradientMagnitude::getProcessorInfo() const { return processorInfo_; }

VolumeGradientMagnitude::VolumeGradientMagnitude()
    : VolumeGLProcessor{"volumegradientmagnitude.frag"}
    , channel_{"channel", "Channel", util::enumeratedOptions("Channel", 4), 0}
    , gradientScaling_{"gradientScaling", "Gradient Scaling", util::ordinalScale(1.0f)} {
    this->dataFormat_ = DataFloat32::get();

    channel_.addOption("Channel 1", "Channel 1", 0);
    channel_.setCurrentStateAsDefault();

    addProperties(channel_, gradientScaling_);
}

VolumeGradientMagnitude::~VolumeGradientMagnitude() {}

void VolumeGradientMagnitude::preProcess(TextureUnitContainer&) {
    auto volume = inport_.getData();
    if (channel_.getSelectedIndex() >= volume->getDataFormat()->getComponents()) {
        throw Exception(IVW_CONTEXT, "Channel is greater than the available channels {} >= {}",
                        channel_.getSelectedValue(), volume->getDataFormat()->getComponents());
    }

    shader_.setUniform("channel", static_cast<int>(channel_.getSelectedIndex()));
    shader_.setUniform("gradientScaling", gradientScaling_);
}

void VolumeGradientMagnitude::postProcess() {
    volume_->dataMap.valueAxis.name = "gradient magnitude";
    volume_->dataMap.valueAxis.unit =
        inport_.getData()->dataMap.valueAxis.unit / inport_.getData()->axes[0].unit;
    volume_->dataMap.dataRange = dvec2{0.0, 1.0};
    volume_->dataMap.valueRange = dvec2{0.0, 1.0};
}

void VolumeGradientMagnitude::afterInportChanged() {
    if (inport_.hasData()) {
        int channels = static_cast<int>(inport_.getData()->getDataFormat()->getComponents());

        if (channels == static_cast<int>(channel_.size())) return;

        channel_.clearOptions();
        for (int i = 0; i < channels; i++) {
            std::stringstream ss;
            ss << "Channel " << i;
            channel_.addOption(ss.str(), ss.str(), i);
        }
        channel_.setCurrentStateAsDefault();
    }
}

}  // namespace inviwo
