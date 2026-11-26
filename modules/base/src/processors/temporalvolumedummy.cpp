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

#include <modules/base/processors/temporalvolumedummy.h>

#include <inviwo/core/datastructures/volume/volume.h>
#include <inviwo/core/datastructures/volume/volumeram.h>
#include <inviwo/core/datastructures/volume/volumeconfig.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo TemporalVolumeDummy::processorInfo_{
    "org.inviwo.TemporalVolumeDummy",  // Class identifier
    "Temporal Volume Dummy",           // Display name
    "Volume Operation",                // Category
    CodeState::Experimental,           // Code state
    Tags::CPU | Tag{"Volume"},         // Tags
    R"(Converts the prototype volume, which provides data format, data range, basis, etc., of
       of a Temporal Volume into a corresponding single-voxel dummy Volume.

       Note: the resulting dummy volume can only be used for accessing the meta information but 
       not the actual volume data.)"_unindentHelp,
};

const ProcessorInfo& TemporalVolumeDummy::getProcessorInfo() const { return processorInfo_; }

TemporalVolumeDummy::TemporalVolumeDummy()
    : Processor{}
    , inport_{"inport", "Temporal Volume"_help}
    , outport_{"outport", "Single-voxel dummy Volume"_help} {

    addPorts(inport_, outport_);
}

void TemporalVolumeDummy::process() {
    inport_.getData();

    auto volume = dispatching::singleDispatch<std::shared_ptr<Volume>, dispatching::filter::All>(
        inport_.getData()->prototype().formatId.value_or(VolumeConfig::defaultFormatId),
        []<typename DataType>(const TemporalVolume& temporalVolume) -> std::shared_ptr<Volume> {
            VolumeConfig config = temporalVolume.prototype();
            config.dimensions = size3_t{1u};

            const auto volumeRam =
                std::make_shared<VolumeRAMPrecision<DataType>>(config.reprConfig());
            volumeRam->getView().front() = DataType{0};

            auto volume = std::make_shared<Volume>(config);
            volume->addRepresentation(volumeRam);
            return volume;
        },
        *inport_.getData());

    outport_.setData(volume);
}

}  // namespace inviwo
