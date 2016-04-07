/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015 Inviwo Foundation
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

#include <inviwo/core/util/volumesequenceutils.h>

#include <inviwo/core/datastructures/volume/volume.h>

namespace inviwo {
namespace util {
bool hasTimestamps(const VoumeSequence &seq, bool checkfirstonly) {
    if (seq.empty()) {
        return false;
    }
    if (checkfirstonly) {
        return seq.at(0)->hasMetaData<DoubleMetaData>("timestamp");
    }
    for (auto v : seq) {
        if (!v->hasMetaData<DoubleMetaData>("timestamp")) return false;
    }
    return true;
}

std::pair<double, double> getTimestampRange(const VoumeSequence &seq, bool sorted) {
    if (!hasTimestamps(seq)) {
        return std::make_pair(0.0, 1.0);
    }
    double start, end;
    start = getTimestamp(seq.front());
    if (sorted) {
        end = getTimestamp(seq.back());
    } else {
        end = start;
        for (auto v : seq) {
            auto t = getTimestamp(v);
            start = std::min(start, t);
            end = std::max(end, t);
        }
    }
    return std::make_pair(start, end);
}

bool isSorted(const VoumeSequence &seq) {
    if (seq.size() <= 1) {
        return true;
    }
    auto prev = getTimestamp(seq.front());
    auto it = seq.begin() + 1;
    for (; it < seq.end(); ++it) {
        auto t = getTimestamp(*it);
        if (prev >= t) return false;
        prev = t;
    }
    return true;
}

VoumeSequence sortSequence(const VoumeSequence &seq) {
    VoumeSequence sorted = seq;
    std::sort(sorted.begin(), sorted.end(), [](const SharedVolume &a, const SharedVolume &b) {
        auto t1 = getTimestamp(a);
        auto t2 = getTimestamp(b);
        return t1 < t2;
    });
    return sorted;
}

std::pair<SharedVolume, SharedVolume> getVolumesForTimestep(
    const VoumeSequence &seq, double t, bool sorted) {
    if (seq.size() == 1) {
        return std::make_pair(seq.front(), seq.front());
    }
    else if (!hasTimestamps(seq)) { 
        if (t < 0) {
            return std::make_pair(seq.front(), seq.front());
        }
        size_t i = static_cast<size_t>((seq.size() - 1) * t);
        i = glm::clamp(i, static_cast<size_t>(0), seq.size() - 1);
        auto i2 = i + 1;
        if (i2 >= seq.size() - 1u) {
            i2 = i;
        }
        return std::make_pair(seq[i], seq[i2]);
    }
    else if (sorted) {
        // find first volume with timestamp greater than t
        auto it = std::find_if(seq.begin(), seq.end(), [t](const SharedVolume &v) {
            auto vt = getTimestamp(v);
            return t < vt;
        });

        if (it == seq.end()) {
            // t > time stamp of last volume
            return std::make_pair(seq.back(), seq.back());
        } else if (it== seq.begin()) {
            // t < time stamp of first volume
            return std::make_pair(*it, *it);
        }
        
        //return predecessor and current iterator to enclose t
        return std::make_pair(*std::prev(it), *it);
    } else {
        return getVolumesForTimestep(sortSequence(seq), t, true);
    }
}

double getTimestamp(SharedVolume vol) {
    return vol->getMetaData<DoubleMetaData>("timestamp")->get();
}

}  // namespace
}  // namespace
