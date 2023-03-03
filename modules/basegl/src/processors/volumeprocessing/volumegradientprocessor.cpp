/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2023 Inviwo Foundation
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

#include <inviwo/core/datastructures/datamapper.h>                               // for DataMapper
#include <inviwo/core/datastructures/unitsystem.h>                               // for Axis, Unit
#include <inviwo/core/ports/volumeport.h>                                        // for VolumeIn...
#include <inviwo/core/processors/processorinfo.h>                                // for Processo...
#include <inviwo/core/processors/processorstate.h>                               // for CodeState
#include <inviwo/core/processors/processortags.h>                                // for Tags
#include <inviwo/core/properties/boolproperty.h>                                 // for BoolProp...
#include <inviwo/core/properties/invalidationlevel.h>                            // for Invalida...
#include <inviwo/core/properties/optionproperty.h>                               // for OptionPr...
#include <inviwo/core/util/formats.h>                                            // for DataFormat
#include <inviwo/core/util/glmvec.h>                                             // for dvec2
#include <modules/basegl/processors/volumeprocessing/volumeglprocessor.h>        // for VolumeGL...
#include <modules/basegl/processors/volumeprocessing/volumegradientprocessor.h>  // for VolumeGr...
#include <modules/opengl/shader/shader.h>                                        // for Shader
#include <modules/opengl/shader/shaderobject.h>                                  // for ShaderOb...

#include <array>        // for array
#include <functional>   // for __base
#include <memory>       // for shared_ptr
#include <sstream>      // for stringst...
#include <string>       // for char_traits
#include <string_view>  // for string_view
#include <type_traits>  // for remove_e...

namespace inviwo {
class TextureUnitContainer;

const ProcessorInfo VolumeGradientProcessor::processorInfo_{
    "org.inviwo.VolumeGradient",  // Class identifier
    "Volume Gradient",            // Display name
    "Volume Operation",           // Category
    CodeState::Stable,            // Code state
    Tags::GL,                     // Tags
};
const ProcessorInfo VolumeGradientProcessor::getProcessorInfo() const { return processorInfo_; }

VolumeGradientProcessor::VolumeGradientProcessor()
    : VolumeGLProcessor("volume_gradient.frag")
    , channel_("channel", "Channel")
    , dataInChannel4_("dataInChannel4_", "Store Input Data in Alpha", false,
                      InvalidationLevel::InvalidResources) {
    this->dataFormat_ = DataVec3Float32::get();

    channel_.addOption("Channel 1", "Channel 1", 0);
    channel_.setCurrentStateAsDefault();

    inport_.onChange([this]() {
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
    });

    addProperty(channel_);
    addProperty(dataInChannel4_);
}

VolumeGradientProcessor::~VolumeGradientProcessor() = default;

void VolumeGradientProcessor::preProcess(TextureUnitContainer&) {
    shader_.setUniform("channel", channel_.getSelectedValue());
}

void VolumeGradientProcessor::postProcess() {
    volume_->dataMap_.valueAxis.name = "gradient";
    volume_->dataMap_.valueAxis.unit =
        inport_.getData()->dataMap_.valueAxis.unit / inport_.getData()->axes[0].unit;
    volume_->dataMap_.dataRange = dvec2(-1.0, 1.0);
    volume_->dataMap_.valueRange = dvec2(-1.0, 1.0);
}

void VolumeGradientProcessor::initializeResources() {
    if (dataInChannel4_.get()) {
        shader_.getFragmentShaderObject()->addShaderDefine("ADD_DATA_CHANNEL");
        this->dataFormat_ = DataVec4Float32::get();
    } else {
        shader_.getFragmentShaderObject()->removeShaderDefine("ADD_DATA_CHANNEL");
        this->dataFormat_ = DataVec3Float32::get();
    }
    shader_.build();
    internalInvalid_ = true;
}

}  // namespace inviwo
