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
#pragma once

#include <modules/base/basemoduledefine.h>

#include <inviwo/core/datastructures/volume/volume.h>
#include <inviwo/core/datastructures/volume/volumeconfig.h>

#include <memory>   // for shared_ptr, make_shared
#include <utility>  // for pair
#include <vector>   // for vector

namespace inviwo {

/**
 * @brief Provides a cache for reusing Volume objects with a specific configuration.
 *
 * The VolumeReuseCache class manages a pool of Volume objects that share the same
 * VolumeConfig. This allows for efficient reuse of Volume instances, reducing the
 * overhead of frequent allocations and deallocations. The cache is initialized with
 * a given VolumeConfig, which can be updated at runtime. When a Volume is requested
 * via get(), the cache will return an existing Volume if available, or create a new
 * one if the cache is empty.
 *
 * Copy and move operations are disabled to ensure unique ownership of the cache.
 *
 * Example usage:
 * @code
 * VolumeReuseCache cache(config);
 * auto volume = cache.get();
 * @endcode
 *
 * @see Volume
 * @see VolumeConfig
 */
class IVW_MODULE_BASE_API VolumeReuseCache {
public:
    explicit VolumeReuseCache(VolumeConfig config = {});
    VolumeReuseCache(const VolumeReuseCache&) = delete;
    VolumeReuseCache(VolumeReuseCache&&) = delete;
    VolumeReuseCache& operator=(const VolumeReuseCache&) = delete;
    VolumeReuseCache& operator=(VolumeReuseCache&&) = delete;
    ~VolumeReuseCache() = default;

    /**
     * @brief Returns the current VolumeConfig used by the cache.
     */
    const VolumeConfig& getConfig() const;

    /**
     * @brief Sets a new VolumeConfig for the cache.
     * 
     * Updates the configuration used for creating new Volume objects.
     * If the existing config does not match the new config,
     * existing volumes will be discarded and new Volumes will be created as needed.
     *
     * @param config The new VolumeConfig to use.
     */
    void setConfig(const VolumeConfig& config);

    /**
     * @brief Retrieves a Volume from the cache or creates a new one if none are available.
     * 
     * Returns a shared pointer to a Volume object matching the current configuration.
     * If the cache contains available Volumes, one is reused; otherwise, a new Volume is created.
     * After the volume is reused, call Volume::discardHistograms() to have them recalculated.
     * 
     * @return std::shared_ptr<Volume> A shared pointer to a Volume instance.
     */
    std::shared_ptr<Volume> get();

private:
    VolumeConfig config_;
    std::vector<std::shared_ptr<Volume>> cache_;
};

}  // namespace inviwo
