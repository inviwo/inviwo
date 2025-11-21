/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2025 Inviwo Foundation
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

#include <modules/base/datastructures/volumereusecache.h>

#include <algorithm>

namespace inviwo {

VolumeReuseCache::VolumeReuseCache() = default;

const VolumeConfig& VolumeReuseCache::getConfig() const { return config_; }

auto VolumeReuseCache::get(const VolumeConfig& config)
    -> std::pair<std::shared_ptr<Volume>, Status> {

    const std::scoped_lock lock{mutex_};

    // only check for dim/format
    auto status = Status::NoChange;
    if (config.dimensions != config_.dimensions || config.format != config_.format) {
        cache_.clear();
        status = Status::ClearedCache;
    }
    config_ = config;

    auto it = std::ranges::find_if(cache_, [](const auto& elem) { return elem.use_count() == 1; });
    if (it != cache_.end()) {
        auto volume = *it;
        volume->getMetaDataMap()->removeAll();

        volume->setSwizzleMask(config_.swizzleMask.value_or(VolumeConfig::defaultSwizzleMask));
        volume->setInterpolation(
            config_.interpolation.value_or(VolumeConfig::defaultInterpolation));
        volume->setWrapping(config_.wrapping.value_or(VolumeConfig::defaultWrapping));

        volume->axes[0] = config_.xAxis.value_or(VolumeConfig::defaultYAxis);
        volume->axes[1] = config_.yAxis.value_or(VolumeConfig::defaultYAxis);
        volume->axes[2] = config_.zAxis.value_or(VolumeConfig::defaultZAxis);
        volume->dataMap = config_.dataMap();
        volume->setModelMatrix(config_.model.value_or(VolumeConfig::defaultModel));
        volume->setWorldMatrix(config_.world.value_or(VolumeConfig::defaultWorld));

        if (*volume != config_.reprConfig()) {
            throw Exception("Unexpected changes found in cache");
        }

        return {volume, status};
    } else {
        auto volume = std::make_shared<Volume>(config_);
        cache_.push_back(volume);
        return {volume, status};
    }
}

}  // namespace inviwo
