/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2025 Inviwo Foundation
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

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/util/threadutil.h>
#include <inviwo/core/datastructures/volume/volumeram.h>

#include <vector>
#include <future>

namespace inviwo {

namespace util {

template <typename C>
void forEachVoxel(const size3_t dims, C callback) {
    size3_t pos;
    for (pos.z = 0; pos.z < dims.z; ++pos.z) {
        for (pos.y = 0; pos.y < dims.y; ++pos.y) {
            for (pos.x = 0; pos.x < dims.x; ++pos.x) {
                callback(pos);
            }
        }
    }
}

template <typename C>
void forEachVoxel(const VolumeRAM& v, C callback) {
    forEachVoxel(v.getDimensions(), callback);
}

template <typename C>
void forEachVoxelParallel(const size3_t dims, C callback, size_t jobs = 0) {
    const size_t poolSize = util::getPoolSize();
    if (jobs == 0) {
        jobs = 4 * poolSize;
    }
    if ((jobs == 0) || (poolSize == 0)) {
        // fallback to serial version
        forEachVoxel(dims, callback);
        return;
    }

    std::vector<std::future<void>> futures;
    for (size_t job = 0; job < jobs; ++job) {
        size3_t start = size3_t(0, 0, job * dims.z / jobs);
        size3_t stop = size3_t(dims.x, dims.y, std::min(dims.z, (job + 1) * dims.z / jobs));

        futures.push_back(util::dispatchPool([&callback, start, stop]() {
            size3_t pos{0};

            for (pos.z = start.z; pos.z < stop.z; ++pos.z) {
                for (pos.y = start.y; pos.y < stop.y; ++pos.y) {
                    for (pos.x = start.x; pos.x < stop.x; ++pos.x) {
                        callback(pos);
                    }
                }
            }
        }));
    }

    for (const auto& e : futures) {
        e.wait();
    }
}
template <typename C>
void forEachVoxelParallel(const VolumeRAM& v, C callback, size_t jobs = 0) {
    forEachVoxelParallel(v.getDimensions(), callback, jobs);
}

}  // namespace util

}  // namespace inviwo
