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

#include <modules/hdf5/hdf5temporalvolumeloader.h>
#include <modules/hdf5/hdf5read.h>

#include <inviwo/core/datastructures/volume/volume.h>
#include <inviwo/core/util/exception.h>

#include <warn/push>
#include <warn/ignore/all>
#include <H5Cpp.h>
#include <warn/pop>

namespace inviwo::hdf5 {

HDF5TemporalVolumeLoader::HDF5TemporalVolumeLoader(Handle handle, std::vector<Selection> selection,
                                                   size_t timeDimension,
                                                   const DataFormatBase* format, const dmat4& basis,
                                                   double dt)
    : handle_{std::move(handle)}
    , selection_{std::move(selection)}
    , timeDimension_{timeDimension}
    , format_{format}
    , basis_{basis} {

    if (selection_.size() < 4) {
        throw Exception(SourceContext{},
                        "HDF5TemporalVolumeLoader requires a selection of rank >= 4, got {}",
                        selection_.size());
    }
    // the trailing three entries are always the X/Y/Z volume axes
    const size_t numExtraDims = selection_.size() - 3;
    if (timeDimension_ >= numExtraDims) {
        throw Exception(SourceContext{},
                        "Invalid time dimension {}, must designate one of the {} leading "
                        "(non X/Y/Z) dimensions",
                        timeDimension_, numExtraDims);
    }

    const auto dataset = handle_.open();
    const H5::DataSpace dataSpace = dataset.getSpace();
    const auto rank = static_cast<size_t>(dataSpace.getSimpleExtentNdims());
    if (selection_.size() != rank) {
        throw Exception(SourceContext{}, "Selection rank {} does not match dataset rank {}",
                        selection_.size(), rank);
    }
    std::vector<hsize_t> dims(rank);
    dataSpace.getSimpleExtentDims(dims.data());

    // column major index i corresponds to HDF5 (row major) storage dim (rank - 1 - i)
    const auto colMajorDimSize = [&](size_t i) { return static_cast<size_t>(dims[rank - 1 - i]); };

    for (size_t i = 0; i < numExtraDims; ++i) {
        if (i == timeDimension_) continue;
        const auto clamped = clamp(selection_[i], colMajorDimSize(i));
        if (clamped.count != 1) {
            throw Exception(SourceContext{},
                            "Extra dimension {} must resolve to a single index (got count {}); "
                            "did you mean to select it as the time dimension?",
                            i, clamped.count);
        }
    }

    const auto timeSelection = clamp(selection_[timeDimension_], colMajorDimSize(timeDimension_));
    rawStart_ = timeSelection.start;
    rawStride_ = timeSelection.stride;
    if (timeSelection.count == 0) {
        throw Exception(SourceContext{}, "Time dimension selection resulted in zero frames");
    }

    times_.reserve(timeSelection.count);
    for (size_t i = 0; i < timeSelection.count; ++i) {
        times_.emplace_back(static_cast<double>(i) * dt);
    }

    prototype_ = getVolumeConfig(handle_, selection_, format_);
    prototype_.model = basis_;
}

std::shared_ptr<Volume> HDF5TemporalVolumeLoader::readFrame(size_t rawIndex,
                                                            std::shared_ptr<Volume> reuse) const {
    auto sel = selection_;
    sel[timeDimension_] = Selection{.start = rawIndex, .count = 1, .stride = 1};

    auto volume = getVolumeAtPathAsType(handle_, sel, format_, [&](const VolumeConfig& cfg) {
        const auto dims = cfg.dimensions.value_or(VolumeConfig::defaultDimensions);
        const auto* format = cfg.format ? cfg.format : VolumeConfig::defaultFormat;
        if (reuse && reuse->getDimensions() == dims && reuse->getDataFormat() == format) {
            reuse->getMetaDataMap()->removeAll();
            return reuse;
        }
        return std::make_shared<Volume>(cfg);
    });
    volume->setModelMatrix(basis_);
    return volume;
}

std::shared_ptr<Volume> HDF5TemporalVolumeLoader::load(size_t index,
                                                       std::shared_ptr<Volume> reuse) {
    if (index >= times_.size()) {
        throw RangeException(SourceContext{}, "Frame index {} out of range [0, {})", index,
                             times_.size());
    }
    const std::scoped_lock lock{mutex_};
    return readFrame(rawStart_ + index * rawStride_, std::move(reuse));
}

size_t HDF5TemporalVolumeLoader::size() const { return times_.size(); }

std::span<const Seconds> HDF5TemporalVolumeLoader::times() const { return times_; }

VolumeConfig HDF5TemporalVolumeLoader::prototype() const { return prototype_; }


}  // namespace inviwo::hdf5
