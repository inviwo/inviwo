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

#include "pathlinetracer.h"
#include <inviwo/core/util/volumesequenceutils.h>
#include <inviwo/core/util/interpolation.h>

namespace inviwo {

PathLineTracer::PathLineTracer(
    std::shared_ptr<const std::vector<std::shared_ptr<Volume>>> volumeSequence,
    const PathLineProperties &properties)
    : IntegralLineTracer(properties)
    , volumeSequence_(*volumeSequence)
    , dimensions_(0)
    , allowLooping_(properties.isLoopingAllowed())
    , timespan_(1.)
    , timeRange_(0., 1.)
    , hasTimestamps_(false) {
    
    if (volumeSequence_.empty()) {
        LogWarn("Initializing PathLineTracer with an empty vector");
    } else {
        invBasis_ = dmat3(glm::inverse(volumeSequence_.at(0)->getBasis()));
        dimensions_ = volumeSequence_.at(0)->getDimensions();
        if ((hasTimestamps_ = util::hasTimestamps(volumeSequence_, false))) {
            if (util::isSorted(volumeSequence_)) {
                volumeSequence_ = util::sortSequence(volumeSequence_);
            }
            timeRange_ = util::getTimestampRange(volumeSequence_);
            timespan_ = timeRange_.second - timeRange_.first;
        }
    }

    for (auto &vol : volumeSequence_) {
        samplers_.emplace(vol.get(), VolumeDoubleSampler<3>(vol));
    }
}

PathLineTracer::~PathLineTracer() {}

inviwo::IntegralLine PathLineTracer::traceFrom(const vec4 &p) { return traceFrom(dvec4(p)); }

inviwo::IntegralLine PathLineTracer::traceFrom(const dvec4 &p) {
    IntegralLine line;

    auto direction = dir_;
    bool fwd = direction == IntegralLineProperties::Direction::BOTH ||
               direction == IntegralLineProperties::Direction::FWD;
    bool bwd = direction == IntegralLineProperties::Direction::BOTH ||
               direction == IntegralLineProperties::Direction::BWD;
    bool both = fwd && bwd;

    line.positions_.reserve(steps_ + 2);
    line.metaData_["velocity"].reserve(steps_ + 2);
    line.metaData_["timestamp"].reserve(steps_ + 2);

    if (bwd) {
        step(steps_ / (both ? 2 : 1), p, line, false);
    }
    if (both && !line.positions_.empty()) {
        std::reverse(line.positions_.begin(),
                     line.positions_.end());  // reverse is faster than insert first
        line.positions_.pop_back();           // dont repeat first step
    }
    if (fwd) {
        step(steps_ / (both ? 2 : 1), p, line, true);
    }

    return line;
}

void PathLineTracer::step(int steps, dvec4 curPos, IntegralLine &line, bool fwd) {
    for (int i = 0; i <= steps; i++) {
        if (curPos.x < 0) break;
        if (curPos.y < 0) break;
        if (curPos.z < 0) break;

        if (curPos.x > 1) break;
        if (curPos.y > 1) break;
        if (curPos.z > 1) break;

        dvec3 v;
        switch (integrationScheme_) {
            case IntegralLineProperties::IntegrationScheme::RK4:
                v = rk4(curPos, fwd);
                break;
            case IntegralLineProperties::IntegrationScheme::Euler:
            default:
                v = euler(curPos);
                break;
        }

        if (glm::length(v) == 0) {
            break;
        }

        dvec3 worldVelocty = sample(curPos);

        dvec3 velocity = invBasis_ * (v * stepSize_ * (fwd ? 1.0 : -1.0));

        line.positions_.push_back(curPos.xyz());
        line.metaData_["velocity"].push_back(worldVelocty);
        line.metaData_["timestamp"].push_back(dvec3(curPos.a));

        curPos += dvec4(velocity, stepSize_);
        if (allowLooping_ && std::abs(timespan_) > 0.0) {
            while (curPos.a > timeRange_.second) curPos.a -= timespan_;
            while (curPos.a < timeRange_.first) curPos.a += timespan_;
        } else {
            if (curPos.a > timeRange_.second || curPos.a < timeRange_.first) {
                break;
            }
        }
    }
}

dvec3 PathLineTracer::sample(const dvec4 &pos) {
    double t = pos.w;
    dvec3 pos3 = pos.xyz();
    auto vols = util::getVolumesForTimestep(volumeSequence_, t);
    auto vol0 = vols.first.get();
    auto vol1 = vols.second.get();
    if (vol0 == vol1) {
        auto it = samplers_.find(vol0);
        return it->second.sample(pos3);
    }
    auto s0 = samplers_.find(vol0)->second;
    auto s1 = samplers_.find(vol1)->second;

    auto v0 = s0.sample(pos3);
    auto v1 = s1.sample(pos3);

    double t0, t1;

    if (hasTimestamps_) {
        t0 = util::getTimestamp(vols.first);
        t1 = util::getTimestamp(vols.second);
    } else {
        auto i0 = std::find(volumeSequence_.begin(), volumeSequence_.end(), vols.first) -
                  volumeSequence_.begin();
        auto i1 = std::find(volumeSequence_.begin(), volumeSequence_.end(), vols.second) -
                  volumeSequence_.begin();
        t0 = i0 / (volumeSequence_.size() - 1.0);
        t1 = i1 / (volumeSequence_.size() - 1.0);
    }

    auto x = (t - t0) / (t1 - t0);
    return Interpolation<dvec3>::linear(v0, v1, x);
}

inviwo::dvec3 PathLineTracer::euler(const dvec4 &curPos) { return sample(curPos); }

inviwo::dvec3 PathLineTracer::rk4(const dvec4 &curPos, bool fwd) {
    auto h = stepSize_;
    if (!fwd) h = -h;
    auto h2 = h / 2;
    auto k1 = sample(curPos);
    auto K1 = invBasis_ * k1;
    auto k2 = sample(curPos + dvec4(K1 * h2, h2));
    auto K2 = invBasis_ * k2;
    auto k3 = sample(curPos + dvec4(K2 * h2, h2));
    auto K3 = invBasis_ * k3;
    auto k4 = sample(curPos + dvec4(K3 * h, h));

    return (k1 + 2.0 * (k2 + k3) + k4) / 6.0;
}

}  // namespace
