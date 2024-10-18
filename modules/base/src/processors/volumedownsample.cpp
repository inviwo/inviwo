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

#include <modules/base/processors/volumedownsample.h>

#include <inviwo/core/datastructures/datamapper.h>
#include <inviwo/core/datastructures/volume/volume.h>
#include <inviwo/core/datastructures/volume/volumeram.h>
#include <inviwo/core/processors/poolprocessor.h>
#include <modules/base/algorithm/volume/volumeramdownsample.h>

#include <glm/common.hpp>

namespace inviwo {

const ProcessorInfo VolumeDownsample::processorInfo_{
    "org.inviwo.VolumeDownsample",                       // Class identifier
    "Volume Downsample",                                 // Display name
    "Volume Operation",                                  // Category
    CodeState::Stable,                                   // Code state
    Tags::CPU | Tag{"Downsampling"} | Tag{"Reduction"},  // Tags
    R"(Reduce the input volume by downsampling.
)"_unindentHelp,
};
const ProcessorInfo VolumeDownsample::getProcessorInfo() const { return processorInfo_; }

VolumeDownsample::VolumeDownsample()
    : PoolProcessor{}
    , inport_{"inputVolume", "Source volume"_help}
    , outport_{"outputVolume", "Downsampled source volume"_help}
    , enabled_{"enabled", "Enable Operation", true}
    , mode_{"mode", "Mode",
            OptionPropertyState<util::DownsamplingMode>{
                .options = {{"strided", "Strided", util::DownsamplingMode::Strided},
                            {"averaged", "Averaged", util::DownsamplingMode::Averaged}}}
                .setSelectedValue(util::DownsamplingMode::Averaged)}
    , uniform_{"uniform", "Uniform Subsampling",
               "If enabled, the value of the first stride will be used for all directions"_help,
               false}
    , strides_{
          "strides", "Strides",
          util::ordinalCount(ivec3{2}, ivec3{16})
              .setMin(ivec3{1})
              .set(
                  "The factors used to downsample the input volume along each corresponding axis"_help)} {

    addPort(inport_);
    addPort(outport_);

    addProperties(enabled_, mode_, uniform_, strides_);
}

void VolumeDownsample::process() {
    const size3_t strides = [&]() {
        size3_t s =
            uniform_ ? size3_t{static_cast<size_t>(strides_.get().x)} : size3_t{strides_.get()};
        return glm::clamp(s, size3_t{1}, inport_.getData()->getDimensions());
    }();

    if (!enabled_ || strides == size3_t(1, 1, 1)) {
        outport_.setData(inport_.getData());
    } else {
        outport_.clear();
        dispatchOne(
            [volume = inport_.getData(), strides, mode = mode_.getSelectedValue()]() {
                return downsample(volume, strides, mode);
            },
            [this](std::shared_ptr<Volume> result) {
                outport_.setData(result);
                newResults();
            });
    }
}

std::shared_ptr<Volume> VolumeDownsample::downsample(std::shared_ptr<const Volume> source,
                                                     size3_t strides, util::DownsamplingMode mode) {
    auto volumeRam = util::volumeDownsample(source->getRepresentation<VolumeRAM>(), strides, mode);
    auto volume = std::make_shared<Volume>(*source.get(), noData,
                                           VolumeConfig{.dimensions = volumeRam->getDimensions()});
    volume->addRepresentation(volumeRam);

    return volume;
}

}  // namespace inviwo
