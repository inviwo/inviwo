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
                                                   size_t timeDimension, Seconds dt,
                                                   VolumeConfig config)
    : handle_{std::move(handle)}
    , selection_{std::move(selection)}
    , timeDimension_{timeDimension}
    , timeSelection_{}
    , dt_{dt}
    , prototype_{config} {

    const std::scoped_lock lock{Handle::globalMutex()};
    std::tie(prototype_, timeSelection_) =
        getTemporalVolumeConfig(handle_, selection_, timeDimension_, prototype_);
}

std::shared_ptr<Volume> HDF5TemporalVolumeLoader::load(size_t index,
                                                       std::shared_ptr<Volume> reuse) {

    if (index >= size()) {
        throw Exception(SourceContext{}, "Frame index {} out of range [0, {})", index, size());
    }

    const std::scoped_lock lock{Handle::globalMutex()};
    auto sel = selection_;
    sel[timeDimension_] = Selection{
        .start = timeSelection_.start + timeSelection_.stride * index, .count = 1, .stride = 1};

    auto volume = getVolumeAtPathAsType(handle_, sel, prototype_, [&](const VolumeConfig& cfg) {
        const auto dims = cfg.dimensions.value_or(VolumeConfig::defaultDimensions);
        const auto* format = cfg.format ? cfg.format : VolumeConfig::defaultFormat;
        if (reuse && reuse->getDimensions() == dims && reuse->getDataFormat() == format) {
            reuse->getMetaDataMap()->removeAll();

            reuse->setSwizzleMask(cfg.swizzleMask.value_or(VolumeConfig::defaultSwizzleMask));
            reuse->setInterpolation(cfg.interpolation.value_or(VolumeConfig::defaultInterpolation));
            reuse->setWrapping(cfg.wrapping.value_or(VolumeConfig::defaultWrapping));

            reuse->axes[0] = cfg.xAxis.value_or(VolumeConfig::defaultXAxis);
            reuse->axes[1] = cfg.yAxis.value_or(VolumeConfig::defaultYAxis);
            reuse->axes[2] = cfg.zAxis.value_or(VolumeConfig::defaultZAxis);
            reuse->dataMap = cfg.dataMap();
            reuse->setModelMatrix(cfg.model.value_or(VolumeConfig::defaultModel));
            reuse->setWorldMatrix(cfg.world.value_or(VolumeConfig::defaultWorld));

            return reuse;
        }
        return std::make_shared<Volume>(cfg);
    });
    return volume;
}

size_t HDF5TemporalVolumeLoader::size() const { return timeSelection_.count; }

Seconds HDF5TemporalVolumeLoader::time(size_t index) const { return index * dt_; }

VolumeConfig HDF5TemporalVolumeLoader::prototype() const { return prototype_; }

}  // namespace inviwo::hdf5
