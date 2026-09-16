/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2026 Inviwo Foundation
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

#include <modules/hdf5/processors/hdf5volumesource.h>

#include <inviwo/core/datastructures/unitsystem.h>
#include <inviwo/core/datastructures/volume/volumeram.h>
#include <inviwo/core/io/datareader.h>
#include <inviwo/core/io/datareaderexception.h>
#include <inviwo/core/network/networklock.h>
#include <inviwo/core/util/concat.h>
#include <inviwo/core/util/glm.h>

#include <modules/hdf5/datastructures/hdf5handle.h>
#include <modules/hdf5/hdf5read.h>
#include <modules/hdf5/datastructures/hdf5path.h>
#include <modules/hdf5/hdf5utils.h>

#include <modules/base/algorithm/dataminmax.h>

#include <algorithm>
#include <functional>
#include <numeric>
#include <limits>

namespace inviwo {

namespace hdf5 {

const ProcessorInfo HDF5ToVolume::processorInfo_{
    "org.inviwo.hdf5.ToVolume",               // Class identifier
    "HDF5 To Volume",                         // Display name
    "Data Input",                             // Category
    CodeState::Stable,                        // Code state
    Tags::CPU | Tag{"HDF5"} | Tag{"Volume"},  // Tags
    "Load a volume from a HDF5 file handle."_help,
};
const ProcessorInfo& HDF5ToVolume::getProcessorInfo() const { return processorInfo_; }

HDF5ToVolume::HDF5ToVolume()
    : Processor()
    , inport_("inport")
    , outport_("outport")

    , volumeSelection_("volumeSelection", "Volume")

    , basisGroup_("basisGroup", "Basis")
    , basisSelection_("basisSelection", "Source")
    , basis_("basis", "Matrix", mat4(1.0f), inviwo::util::filled<mat4>(-1000.f),
             inviwo::util::filled<mat4>(1000.f))
    , spacing_("spacing", "Spacing", vec3(0.01f), vec3(0.0f), vec3(1.0f))
    , information_("Information", "Data information")
    , outputGroup_("outputGroup", "Operations")
    , datatype_("convertType", "Convert to type", util::conversionOptions(), 0)
    , adjustBasis_("adjustBasis", "Automatically adjust basis", true)
    , adjustOffset_("adjustOffset", "Automatically adjust offset", true)
    , selection_("selection", "Selection", 6)
    , cache_{} {

    addPort(inport_);
    addPort(outport_);

    volumeSelection_.setSerializationMode(PropertySerializationMode::All);

    basisGroup_.addProperties(basisSelection_, spacing_, basis_);
    basis_.readonlyDependsOn(basisSelection_, [](auto& p) { return p.getSelectedIndex() != 0; });
    spacing_.visibilityDependsOn(basisSelection_,
                                 [](auto& p) { return p.getSelectedIndex() == 1; });
    basisSelection_.setSerializationMode(PropertySerializationMode::All);

    outputGroup_.addProperties(datatype_, adjustBasis_, adjustOffset_, selection_);

    addProperties(volumeSelection_, basisGroup_, information_, outputGroup_);
}

HDF5ToVolume::~HDF5ToVolume() = default;

void HDF5ToVolume::process() try {
    const std::scoped_lock lock{Handle::globalMutex()};

    const auto data = inport_.getData();

    if (inport_.isChanged()) {

        const auto metadata = util::getDataSets(*data);

        volumeMatches_.assign_range(metadata | std::views::filter([](const DataSetInfo& info) {
                                        return info.dimensions.size() >= 3ull &&
                                               std::ranges::fold_left(info.dimensions, size_t{1},
                                                                      std::multiplies{}) > 50000ull;
                                    }));
        basisMatches_.assign_range(metadata | std::views::filter([](const DataSetInfo& info) {
                                       auto dims = info.getColumnMajorDimensions();
                                       static constexpr std::array<size_t, 2> basis{3, 3};
                                       static constexpr std::array<size_t, 2> basisAndOffset{4, 4};
                                       return std::ranges::equal(dims, basis) ||
                                              std::ranges::equal(dims, basisAndOffset);
                                   }));
        // Update Volume Selection
        volumeSelection_.replaceOptions(volumeMatches_ |
                                        std::views::transform(util::dataSetInfoToOption));
        volumeSelection_.setCurrentStateAsDefault();

        // Update Basis Selection
        const std::array<OptionPropertyStringOption, 2> basisOptions{
            {{"user_basis", "User defined basis", "user_basis"},
             {"user_spacing", "User defined spacing", "user_spacing"}}};

        basisSelection_.replaceOptions(views::concat(
            basisOptions, basisMatches_ | std::views::transform(util::dataSetInfoToOption)));
        basisSelection_.setCurrentStateAsDefault();
    }

    if (volumeMatches_.empty()) {
        outport_.clear();
        return;
    }

    const auto& volumeInfo = volumeMatches_[volumeSelection_.getSelectedIndex()];
    selection_.update(volumeInfo);

    const auto* format = util::conversionFormat(datatype_.getSelectedIndex());
    auto volume = std::shared_ptr<Volume>(getVolumeAtPathAsType(
        *data + volumeInfo.path, selection_.getSelection(), format, std::ref(cache_)));

    const auto [rangeMin, rangeMax] =
        ::inviwo::util::volumeMinMax(volume->getRepresentation<VolumeRAM>());
    volume->dataMap.dataRange = {glm::compMin(rangeMin), glm::compMax(rangeMax)};
    volume->dataMap.valueRange = volume->dataMap.dataRange;

    information_.updateForNewVolume(*volume, deserialized_ ? inviwo::util::OverwriteState::Yes
                                                           : inviwo::util::OverwriteState::No);
    deserialized_ = false;

    information_.updateVolume(*volume);

    switch (basisSelection_.getSelectedIndex()) {
        case 0: {  // User defined basis
            break;
        }
        case 1: {  // User defined spacing
            basis_.set(util::createBasis(volume->getDimensions(), spacing_.get()));
            break;
        }
        default: {
            const auto basisInfo = basisMatches_[basisSelection_.getSelectedIndex() - 2];
            basis_.set(getBasis(*data + basisInfo.path));
            break;
        }
    }

    volume->setModelMatrix(util::adjustBasis(basis_, selection_.getSelection(),
                                             volumeInfo.getColumnMajorDimensions(),
                                             adjustBasis_.get(), adjustOffset_.get()));

    outport_.setData(volume);

} catch (H5::Exception& e) {
    throw Exception(SourceContext{}, "Error reading HDF5 data: {}", e.getDetailMsg());
}

void HDF5ToVolume::deserialize(Deserializer& d) {
    Processor::deserialize(d);
    deserialized_ = true;
}

}  // namespace hdf5

}  // namespace inviwo
