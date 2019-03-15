/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2019 Inviwo Foundation
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

#include <modules/base/processors/volumecreator.h>
#include <modules/base/algorithm/volume/volumegeneration.h>
#include <inviwo/core/util/formatdispatching.h>

namespace inviwo {

namespace {
struct Helper {
    template <typename Format>
    void operator()(std::vector<OptionPropertyOption<DataFormatId>>& formats) {
        formats.emplace_back(Format::str(), Format::str(), Format::id());
    }
};

struct Creator {
    template <typename Result, typename Format>
    Result operator()(VolumeCreator::Type type, size3_t size, size_t index) {
        switch (type) {
            case VolumeCreator::Type::SingleVoxel:
                return std::shared_ptr<Volume>(
                    util::makeSingleVoxelVolume<typename Format::type>(size));
            case VolumeCreator::Type::Sphere:
                return std::shared_ptr<Volume>(
                    util::makeSphericalVolume<typename Format::type>(size));
            case VolumeCreator::Type::Ripple:
                return std::shared_ptr<Volume>(util::makeRippleVolume<typename Format::type>(size));
            case VolumeCreator::Type::MarchingCube:
                return std::shared_ptr<Volume>(
                    util::makeMarchingCubeVolume<typename Format::type>(index));
            default:
                return std::shared_ptr<Volume>{};
        }
    }
};
}  // namespace

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo VolumeCreator::processorInfo_{
    "org.inviwo.VolumeCreator",  // Class identifier
    "Volume Creator",            // Display name
    "Data Creation",             // Category
    CodeState::Stable,           // Code state
    Tags::CPU,                   // Tags
};
const ProcessorInfo VolumeCreator::getProcessorInfo() const { return processorInfo_; }

VolumeCreator::VolumeCreator()
    : Processor()
    , outport_("volume")
    , type_{"type",
            "Type",
            {{"singleVoxel", "Single Voxel", Type::SingleVoxel},
             {"sphere", "Sphere", Type::Sphere},
             {"ripple", "Ripple", Type::Ripple},
             {"marchingCube", "Marching Cube", Type::MarchingCube}}}
    , format_{"format", "Format",
              [&]() {
                  std::vector<OptionPropertyOption<DataFormatId>> formats;
                  util::for_each_type<DefaultDataFormats>{}(Helper{}, formats);
                  return formats;
              }(),
              1}
    , dimensions_("dimensions", "Dimensions", size3_t(10), size3_t(0), size3_t(512))
    , index_("index", "Index", 5, 0, 255) {

    addPort(outport_);
    addProperty(type_);
    addProperty(format_);
    addProperty(dimensions_);
    addProperty(index_);
}

void VolumeCreator::process() {
    auto volume = dispatching::dispatch<std::shared_ptr<Volume>, dispatching::filter::All>(
        format_.get(), Creator{}, type_.get(), dimensions_.get(), index_.get());
    outport_.setData(volume);
}

}  // namespace inviwo
