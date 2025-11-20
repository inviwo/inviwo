/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2025 Inviwo Foundation
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
    "Computes the gradient of one channel of a 3D scalar field. The data of the input channel "
    "can optionally be stored in the fourth channel of the output volume along with the gradient."_help,
};
const ProcessorInfo& VolumeGradientProcessor::getProcessorInfo() const { return processorInfo_; }

VolumeGradientProcessor::VolumeGradientProcessor()
    : VolumeGLProcessor("volume_gradient.frag")
    , channel_("channel", "Channel", "Selects the channel used for the gradient computation"_help,
               util::enumeratedOptions("Channel", 4))
    , dataInChannel4_(
          "dataInChannel4_", "Store Input Data in Alpha",
          "Toggles whether the input data is saved in the alpha channel of the output"_help, false,
          InvalidationLevel::InvalidResources) {

    addProperties(channel_, dataInChannel4_, calculateDataRange_, dataRange_);
}

VolumeGradientProcessor::~VolumeGradientProcessor() = default;

void VolumeGradientProcessor::initializeShader(Shader& shader) {
    shader.getFragmentShaderObject()->setShaderDefine("ADD_DATA_CHANNEL", dataInChannel4_.get());
}

void VolumeGradientProcessor::preProcess([[maybe_unused]] TextureUnitContainer& cont,
                                         Shader& shader, VolumeConfig& config) {

    if (dataInChannel4_.get()) {
        config.format = DataVec4Float32::get();
    } else {
        config.format = DataVec3Float32::get();
    }

    IVW_ASSERT(inport_.has_value(), "Inport should be constructed");
    const auto& data = inport_->getData();

    const int channels = static_cast<int>(data->getDataFormat()->getComponents());
    if (channel_.getSelectedValue() >= channels) {
        throw Exception(SourceContext{}, "Selected channel out of bounds {} of {}",
                        channel_.getSelectedValue(), channels);
    }

    if (!dataInChannel4_) {
        config.valueAxis =
            Axis{.name = "Gradient", .unit = data->dataMap.valueAxis.unit / data->axes[0].unit};
    }

    config.swizzleMask = swizzlemasks::defaultData(3);
    shader.setUniform("channel", channel_.getSelectedValue());
}

}  // namespace inviwo
