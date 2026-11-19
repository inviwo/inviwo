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

#include <modules/hdf5/hdf5multifiletemporalvolumeloader.h>
#include <modules/hdf5/datastructures/hdf5handle.h>
#include <modules/hdf5/hdf5read.h>

#include <inviwo/core/datastructures/volume/volume.h>
#include <inviwo/core/util/exception.h>
#include <inviwo/core/util/formats.h>
#include <inviwo/core/util/glmfmt.h>

#include <algorithm>
#include <iterator>

#include <fmt/format.h>
#include <fmt/std.h>

#include <warn/push>
#include <warn/ignore/all>
#include <H5Cpp.h>
#include <warn/pop>

namespace inviwo::hdf5 {

HDF5MultiFileTemporalVolumeLoader::HDF5MultiFileTemporalVolumeLoader(
    std::vector<std::filesystem::path> files, Path path, std::vector<Selection> selection,
    size_t timeDimension, Seconds dt, VolumeConfig config)
    : files_{std::move(files)}
    , path_{std::move(path)}
    , selection_{std::move(selection)}
    , timeDimension_{timeDimension}
    , timeSelections_{}
    , frameOffsets_{}
    , dt_{dt}
    , prototype_{std::move(config)} {

    if (files_.empty()) {
        throw Exception{SourceContext{}, "No HDF5 files given"};
    }

    const std::scoped_lock lock{Handle::globalMutex()};

    timeSelections_.reserve(files_.size());
    frameOffsets_.reserve(files_.size() + 1);
    frameOffsets_.push_back(0);

    // Only the dataspace of each dataset is read here, no volume data.
    for (size_t i = 0; i < files_.size(); ++i) {
        const auto& file = files_[i];
        try {
            const Handle handle = Handle{file} + path_;
            auto [cfg, timeSelection] =
                getTemporalVolumeConfig(handle, selection_, timeDimension_, prototype_);
            if (i == 0) {
                prototype_ = std::move(cfg);
            }
            frameOffsets_.push_back(frameOffsets_.back() + timeSelection.count);
            timeSelections_.push_back(timeSelection);
        } catch (H5::Exception& e) {
            throw Exception{SourceContext{}, "Error reading dataset '{}' in HDF5 file '{}': {}",
                            path_.toString(), file, e.getDetailMsg()};
        }
    }
}

std::pair<size_t, size_t> HDF5MultiFileTemporalVolumeLoader::locate(size_t index) const {
    if (index >= size()) {
        throw Exception(SourceContext{}, "Frame index {} out of range [0, {})", index, size());
    }
    // frameOffsets_ is sorted and starts with 0, so the file containing `index` is the last
    // one whose offset is <= index.
    const auto it = std::ranges::upper_bound(frameOffsets_, index);
    const auto fileIndex = static_cast<size_t>(std::distance(frameOffsets_.begin(), it)) - 1;
    return {fileIndex, index - frameOffsets_[fileIndex]};
}

std::shared_ptr<Volume> HDF5MultiFileTemporalVolumeLoader::load(size_t index,
                                                                std::shared_ptr<Volume> reuse,
                                                                std::stop_token stop) const {
    const auto [fileIndex, localIndex] = locate(index);

    if (stop.stop_requested()) return nullptr;
    const std::scoped_lock lock{Handle::globalMutex()};
    if (stop.stop_requested()) return nullptr;

    const auto& file = files_[fileIndex];
    const auto& timeSelection = timeSelections_[fileIndex];

    auto sel = selection_;
    sel[timeDimension_] = Selection{
        .start = timeSelection.start + timeSelection.stride * localIndex, .count = 1, .stride = 1};

    try {
        const Handle handle = Handle{file} + path_;
        return getVolumeAtPathAsType(handle, sel, prototype_, [&](const VolumeConfig& cfg) {
            const auto expectedDims =
                prototype_.dimensions.value_or(VolumeConfig::defaultDimensions);
            const auto expectedFormat = prototype_.formatId.value_or(VolumeConfig::defaultFormatId);
            const auto dims = cfg.dimensions.value_or(VolumeConfig::defaultDimensions);
            const auto formatId = cfg.formatId.value_or(VolumeConfig::defaultFormatId);
            if (dims != expectedDims || formatId != expectedFormat) {
                throw Exception{SourceContext{},
                                "Dataset '{}' in HDF5 file '{}' has dimensions {} and format {}, "
                                "expected {} and {} as in '{}'",
                                path_.toString(),
                                file,
                                dims,
                                DataFormatBase::get(formatId)->getString(),
                                expectedDims,
                                DataFormatBase::get(expectedFormat)->getString(),
                                files_.front()};
            }
            return reuseOrCreateVolume(cfg, reuse);
        });
    } catch (H5::Exception& e) {
        throw Exception{SourceContext{}, "Error reading dataset '{}' in HDF5 file '{}': {}",
                        path_.toString(), file, e.getDetailMsg()};
    }
}

size_t HDF5MultiFileTemporalVolumeLoader::size() const { return frameOffsets_.back(); }

Seconds HDF5MultiFileTemporalVolumeLoader::time(size_t index) const { return index * dt_; }

VolumeConfig HDF5MultiFileTemporalVolumeLoader::prototype() const { return prototype_; }

}  // namespace inviwo::hdf5
