/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015-2019 Inviwo Foundation
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

#include <inviwo/core/datastructures/volume/volumeram.h>
#include <inviwo/core/util/volumesequencesampler.h>

namespace inviwo {

VolumeSequenceSampler::VolumeSequenceSampler(
    std::shared_ptr<const std::vector<std::shared_ptr<Volume>>> volumeSequence, bool allowLooping)
    : Spatial4DSampler<3, double>(volumeSequence->front())
    , wrappers_()
    , allowLooping_(allowLooping)
    , timeRange_(0, 0)
    , totDuration_(0) {

    for (const auto &vol : (*volumeSequence.get())) {
        wrappers_.emplace_back(std::make_shared<Wrapper>(vol));
    }

    auto lastWrapper = wrappers_.back();

    auto infsTime = std::count_if(
        wrappers_.begin(), wrappers_.end(), [&](const std::shared_ptr<Wrapper> &w) -> bool {
            return w->timestamp_ == std::numeric_limits<double>::infinity();
        });

    auto infsDuration = std::count_if(
        wrappers_.begin(), wrappers_.end(), [&](const std::shared_ptr<Wrapper> &w) -> bool {
            return w->duration_ == std::numeric_limits<double>::infinity();
        });
    auto size = static_cast<decltype(infsTime)>(wrappers_.size());

    if (infsTime == 0) {  // all volumes has timestamps, make sure the volumes are in sorted order,
        std::sort(wrappers_.begin(), wrappers_.end(), [](auto a, auto b) { return *a < *b; });
    }

    if (!(infsTime == 0 || infsTime == size)) {
        LogWarn("Failed to create VolumeSequenceSampler due to missing data");
        LogInfo(infsTime << " volumes of " << size << " is missing a timestamp");
        return;
    }
    if (!(infsDuration == 0 || infsDuration == size)) {
        LogWarn("Failed to create VolumeSequenceSampler due to missing data");
        LogInfo(infsDuration << " volumes of " << size << " has unknown duration ");
        return;
    }

    bool firstAndLastAreSame = true;
    if (volumeSequence->size() >= 2) {
        auto firstVol = volumeSequence->front();
        auto lastVol = volumeSequence->back();
        auto firstVolRam = firstVol->getRepresentation<VolumeRAM>();
        auto firstData = static_cast<const char *>(firstVolRam->getData());
        auto lastData =
            static_cast<const char *>(lastVol->getRepresentation<VolumeRAM>()->getData());
        auto dataSize1 = firstVolRam->getNumberOfBytes();

        for (size_t i = 0; i < dataSize1 && firstAndLastAreSame; i++) {
            firstAndLastAreSame &= firstData[i] == lastData[i];
        }
    }

    auto prev = wrappers_.begin();
    for (auto it = prev + 1; it != wrappers_.end(); ++it) {
        prev->get()->next_ = *it;
        prev = it;
    }

    if (infsTime == size && infsDuration == size) {
        double dur = 1.0 / (size - 1.0);
        double t = 0;
        for (auto &w : wrappers_) {
            w->duration_ = dur;
            w->timestamp_ = t;
            t += dur;
        }
    } else if (infsTime == size && infsDuration == 0) {
        double t = 0;
        for (auto &w : wrappers_) {
            w->timestamp_ = t;
            t += w->duration_;
        }
    } else {  // timestamps are set

        if (infsDuration == size) {  // we do not have durations
            for (auto &w : wrappers_) {
                if (auto next = w->next_.lock()) {
                    w->duration_ = next->timestamp_ - w->timestamp_;
                }
            }
        }
    }

    if (firstAndLastAreSame && wrappers_.size() > 1) {
        wrappers_.pop_back();
    }

    totDuration_ = 0;
    for (auto &w : wrappers_) {
        totDuration_ += w->duration_;
    }

    timeRange_.x = wrappers_.front()->timestamp_;
    timeRange_.y = wrappers_.back()->timestamp_ + wrappers_.back()->duration_;
}

VolumeSequenceSampler::~VolumeSequenceSampler() {}

dvec3 VolumeSequenceSampler::sampleDataSpace(const dvec4 &pos) const {
    auto spatialPos = dvec3(pos);
    double t = pos.w;

    if (t < timeRange_.x || t > timeRange_.y) {
        if (!allowLooping_) {
            return dvec3(0);
        }
        while (t < timeRange_.x) {
            t += totDuration_;
        }
        while (t > timeRange_.y) {
            t -= totDuration_;
        }
    }

    auto it = std::upper_bound(
        wrappers_.begin(), wrappers_.end(), t,
        [](double t2, const std::shared_ptr<Wrapper> a) { return t2 < a->timestamp_; });
    --it;
    auto wrapper = *it;

    auto val0 = dvec3(wrapper->sampler_.sample(spatialPos));
    if (wrapper->next_.expired()) {
        return val0;
    }
    auto val1 = dvec3(wrapper->next_.lock()->sampler_.sample(spatialPos));

    double x = (t - wrapper->timestamp_) / wrapper->duration_;
    return Interpolation<dvec3>::linear(val0, val1, x);
}

bool VolumeSequenceSampler::withinBoundsDataSpace(const dvec4 &pos) const {
    // TODO check also time
    if (glm::any(glm::lessThan(dvec3(pos), dvec3(0.0)))) {
        return false;
    }
    if (glm::any(glm::greaterThan(dvec3(pos), dvec3(1.0)))) {
        return false;
    }
    return true;
}

}  // namespace inviwo
