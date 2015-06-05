/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2015 Inviwo Foundation
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

#ifndef IVW_VOLUMERAMHISTOGRAM_H
#define IVW_VOLUMERAMHISTOGRAM_H

#include <inviwo/core/datastructures/volume/volume.h>
#include <inviwo/core/datastructures/volume/volumeram.h>
#include <inviwo/core/datastructures/volume/volumeoperation.h>
#include <inviwo/core/datastructures/histogram.h>

namespace inviwo {

namespace util {

template <typename T, typename std::enable_if<util::rank<T>::value <= 1, int>::type = 0>
HistogramContainer calculateVolumeHistogram(const T* data, uvec3 dimensions,
                                                           dvec2 dataRange,
                                                           const bool& stop = false,
                                                           size_t bins = 2048,
                                                           uvec3 sampleRate = uvec3(1)) {
    // a double type with the same extent as T
    typedef typename util::same_extent<T, double>::type D;
    // a size_t type with same extent as T
    typedef typename util::same_extent<T, size_t>::type I;
    
    const size_t extent = util::rank<T>::value > 0 ? util::extent<T>::value : 1;
    
    // check whether number of bins exceeds the data range only if it is an integral type
    if (!util::is_floating_point<T>::value) {
        bins = std::min(bins, static_cast<std::size_t>(dataRange.y - dataRange.x + 1));
    }

    HistogramContainer histograms;
    for (size_t i = 0; i < extent; ++i) {
        histograms.add(new NormalizedHistogram(bins));
    }

    D min(std::numeric_limits<double>::max());
    D max(std::numeric_limits<double>::lowest());
    D sum(0);
    D sum2(0);
    double count(0);

    D val;
    I ind;
    size_t v;

    const D rangeMin(dataRange.x);
    const D rangeScaleFactor(static_cast<double>(bins - 1) / (dataRange.y - dataRange.x));

    uvec3 pos(0);
    // Column major data, so x is the fastest index.
    for (pos.z = 0; pos.z < dimensions.z; pos.z += sampleRate.z) {
        for (pos.y = 0; pos.y < dimensions.y; pos.y += sampleRate.y) {
            for (pos.x = 0; pos.x < dimensions.x; pos.x += sampleRate.x) {
                if (stop) return histograms;

                val = static_cast<D>(data[VolumeRAM::posToIndex(pos, dimensions)]);

                min = glm::min(min, val);
                max = glm::max(max, val);
                sum += val;
                sum2 += val * val;
                count++;

                ind = static_cast<I>((val - rangeMin) * rangeScaleFactor);

                for (size_t i = 0; i < extent; ++i) {
                    v = util::glmcomp(ind, i);
                    if (v < bins) {
                        histograms[i][v]++;
                    } 
                }
            }
        }
    }

    for (size_t i = 0; i < extent; ++i) {
        histograms[i].dataRange_ = dataRange;
        histograms[i].stats_.min = util::glmcomp(min, i);
        histograms[i].stats_.max = util::glmcomp(max, i);
        histograms[i].stats_.mean = util::glmcomp(sum, i) / count;
        histograms[i].stats_.standardDeviation = std::sqrt(
            (count * util::glmcomp(sum2, i) - util::glmcomp(sum, i) * util::glmcomp(sum, i)) /
            (count * (count - 1)));

        histograms[i].calculatePercentiles();
        histograms[i].performNormalization();
        histograms[i].calculateHistStats();
        histograms[i].setValid(true);
    }

    return histograms;
}

} // util

}  // namespace

#endif  // IVW_VOLUMERAMHISTOGRAM_H
