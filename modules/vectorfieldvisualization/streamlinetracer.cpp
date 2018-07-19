/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015-2018 Inviwo Foundation
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

#include "streamlinetracer.h"
#include <inviwo/core/util/volumesampler.h>
#include <inviwo/core/datastructures/volume/volume.h>
#include <inviwo/core/datastructures/volume/volumeram.h>

namespace inviwo {

StreamLineTracer::StreamLineTracer(std::shared_ptr<const SpatialSampler<3, 3, double>> sampler,
                                   const StreamLineProperties &properties)
    : IntegralLineTracer(properties)

    , invBasis_(dmat3(glm::inverse(sampler->getBasis())))
    , volumeSampler_(sampler)
    , normalizeSample_(properties.getNormalizeSamples()) {}

StreamLineTracer::~StreamLineTracer() {}

void StreamLineTracer::addMetaVolume(const std::string &name, std::shared_ptr<const Volume> vol) {
    metaSamplers_.insert(std::make_pair(name, std::make_shared<VolumeDoubleSampler<3>>(vol)));
}

void StreamLineTracer::addMetaSampler(const std::string &name,
                                      std::shared_ptr<const SpatialSampler<3, 3, double>> sampler) {
    metaSamplers_.insert(std::make_pair(name, sampler));
}

inviwo::IntegralLine StreamLineTracer::traceFrom(const dvec3 &p) {
    IntegralLine line;
    auto &positions = line.getPositions();

    bool fwd = dir_ == IntegralLineProperties::Direction::BOTH ||
               dir_ == IntegralLineProperties::Direction::FWD;
    bool bwd = dir_ == IntegralLineProperties::Direction::BOTH ||
               dir_ == IntegralLineProperties::Direction::BWD;
    bool both = fwd && bwd;

    positions.reserve(steps_ + 2);
    line.getMetaData("velocity").reserve(steps_ + 2);
    for (auto &m : metaSamplers_) {
        line.getMetaData(m.first).reserve(steps_ + 2);
    }

    if (bwd) {
        step(steps_ / (both ? 2 : 1), p, line, false);
    }
    if (both && !positions.empty()) {
        std::reverse(positions.begin(),
                     positions.end());  // reverse is faster than insert first
        positions.pop_back();           // dont repeat first step
        auto keys = line.getMetaDataKeys();
        for (auto &key : keys) {
            auto m = line.getMetaData(key);
            std::reverse(m.begin(), m.end());  // reverse is faster than insert first
            m.pop_back();                      // dont repeat first step
        }
    }
    if (fwd) {
        step(steps_ / (both ? 2 : 1), p, line, true);
    }

    return line;
}

inviwo::IntegralLine StreamLineTracer::traceFrom(const vec3 &p) { return traceFrom(dvec3(p)); }

void StreamLineTracer::step(int steps, dvec3 curPos, IntegralLine &line, bool fwd) {
    auto &positions = line.getPositions();

    for (int i = 0; i <= steps; i++) {
        if (!volumeSampler_->withinBounds(curPos)) {
            line.setTerminationReason(IntegralLine::TerminationReason::OutOfBounds);
            break;
        }

        dvec3 v;
        switch (integrationScheme_) {
            case IntegralLineProperties::IntegrationScheme::RK4:
                v = rk4(curPos, invBasis_, fwd);
                break;
            case IntegralLineProperties::IntegrationScheme::Euler:
            default:
                v = euler(curPos);
                break;
        }

        if (glm::length(v) < std::numeric_limits<double>::epsilon()) {
            line.setTerminationReason(IntegralLine::TerminationReason::ZeroVelocity);
            break;
        }

        const dvec3 worldVelocty{volumeSampler_->sample(curPos)};

        if (normalizeSample_) v = glm::normalize(v);
        dvec3 velocity = invBasis_ * (v * stepSize_ * (fwd ? 1.0 : -1.0));
        positions.push_back(curPos);
        line.getMetaData("velocity").push_back(worldVelocty);
        for (auto &m : metaSamplers_) {
            line.getMetaData(m.first).push_back(m.second->sample(curPos));
        }

        curPos += velocity;
    }
}

dvec3 StreamLineTracer::euler(const dvec3 &curPos) { return dvec3(volumeSampler_->sample(curPos)); }

dvec3 StreamLineTracer::rk4(const dvec3 &curPos, const dmat3 &m, bool fwd) {
    auto h = stepSize_;
    if (!fwd) h = -h;
    auto h2 = h / 2;

    auto normalize = [](dvec3 k) {
        auto l = glm::length(k);
        if (l == 0) {
            return k;
        } else {
            return k / l;
        }
    };

    auto k1 = dvec3(volumeSampler_->sample(curPos));
    if (normalizeSample_) k1 = normalize(k1);
    auto K1 = m * k1;
    auto k2 = dvec3(volumeSampler_->sample(curPos + K1 * h2));
    if (normalizeSample_) k2 = normalize(k2);
    auto K2 = m * k2;
    auto k3 = dvec3(volumeSampler_->sample(curPos + K2 * h2));
    if (normalizeSample_) k3 = normalize(k3);
    auto K3 = m * k3;
    auto k4 = dvec3(volumeSampler_->sample(curPos + K3 * h));
    if (normalizeSample_) k4 = normalize(k4);

    return (k1 + 2.0 * (k2 + k3) + k4) / 6.0;
}

}  // namespace inviwo
