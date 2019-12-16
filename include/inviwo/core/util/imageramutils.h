/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2019 Inviwo Foundation
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

#ifndef IVW_IMAGERAMUTILS_H
#define IVW_IMAGERAMUTILS_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/datastructures/image/layerram.h>

#include <inviwo/core/datastructures/image/imageram.h>
#include <inviwo/core/datastructures/image/image.h>
#include <inviwo/core/util/settings/systemsettings.h>

namespace inviwo {

namespace util {

template <typename C>
void forEachPixel(const size2_t dims, C callback) {
    size2_t pos;
    for (pos.y = 0; pos.y < dims.y; pos.y++) {
        for (pos.x = 0; pos.x < dims.x; pos.x++) {
            callback(pos);
        }
    }
}

template <typename C>
void forEachPixel(const LayerRAM &layer, C callback) {
    forEachPixel(layer.getDimensions(), callback);
}

template <typename C>
void forEachPixelParallel(const size2_t dims, C callback, size_t jobs = 0) {
    if (jobs == 0) {
        auto settings = InviwoApplication::getPtr()->getSettingsByType<SystemSettings>();
        jobs = 4 * settings->poolSize_.get();
        if (jobs == 0) {  // if poolsize is zero
            forEachPixel(dims, callback);
            return;
        }
    }

    std::vector<std::future<void>> futures;
    for (size_t job = 0; job < jobs; ++job) {
        size2_t start = size2_t(0, job * dims.y / jobs);
        size2_t stop = size2_t(dims.x, std::min(dims.y, (job + 1) * dims.y / jobs));

        futures.push_back(dispatchPool([&callback, start, stop]() {
            size2_t pos{0};

            for (pos.y = start.y; pos.y < stop.y; ++pos.y) {
                for (pos.x = start.x; pos.x < stop.x; ++pos.x) {
                    callback(pos);
                }
            }
        }));
    }

    for (const auto &e : futures) {
        e.wait();
    }
}

template <typename C>
void forEachPixelParallel(const LayerRAM &layer, C callback, size_t jobs = 0) {
    forEachPixelParallel(layer.getDimensions(), callback, jobs);
}

IVW_CORE_API std::shared_ptr<Image> readImageFromDisk(std::string filename);

}  // namespace util

}  // namespace inviwo

#endif  // IVW_IMAGERAMUTILS_H
