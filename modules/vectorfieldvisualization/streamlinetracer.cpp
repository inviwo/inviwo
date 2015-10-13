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

#include "streamlinetracer.h"
#include <inviwo/core/util/volumesampler.h>
#include <inviwo/core/datastructures/volume/volume.h>
#include <inviwo/core/datastructures/volume/volumeram.h>

namespace inviwo {

StreamLineTracer::StreamLineTracer(const Volume *vol, IntegrationScheme integrationScheme)
    : volumeSampler_(vol->getRepresentation<VolumeRAM>())
    , invBasis_(dmat3(glm::inverse(vol->getBasis())))
    , dimensions_(vol->getDimensions())
    , integrationScheme_(integrationScheme)
{}

StreamLineTracer::~StreamLineTracer() {}

void StreamLineTracer::addMetaVolume(const std::string &name, const VolumeRAM *vol) {
    metaVolumes_.insert(std::make_pair(name, VolumeSampler(vol)));
}

inviwo::IntegralLine StreamLineTracer::traceFrom(const dvec3 &p, int steps, double stepSize,
                                                 StreamLineTracer::Direction dir,
                                                 bool normalzieSample) {
    IntegralLine line;

    auto direction = dir;
    bool fwd = direction == Direction::BOTH || direction == Direction::FWD;
    bool bwd = direction == Direction::BOTH || direction == Direction::BWD;
    bool both = fwd && bwd;

    line.positions_.reserve(steps + 2);
    line.metaData_["velocity"].reserve(steps + 2);
    for (auto m : metaVolumes_) {
        line.metaData_[m.first].reserve(steps + 2);
    }

    if (bwd) {
        step(steps, p, line, -stepSize / (both ? 2 : 1), normalzieSample);
    }
    if (both && !line.positions_.empty()) {
        std::reverse(line.positions_.begin(),
                     line.positions_.end());  // reverse is faster than insert first
        line.positions_.pop_back();           // dont repeat first step
        for (auto &m : line.metaData_) {
            std::reverse(m.second.begin(), m.second.end());  // reverse is faster than insert first
            m.second.pop_back();                             // dont repeat first step
        }
    }
    if (fwd) {
        step(steps, p, line, stepSize / (both ? 2 : 1), normalzieSample);
    }

    return line;
}

inviwo::IntegralLine StreamLineTracer::traceFrom(const vec3 &p, int steps, double stepSize,
                                                 Direction dir, bool normalzieSample) {
    return traceFrom(dvec3(p), steps, stepSize, dir, normalzieSample);
}

void StreamLineTracer::step(int steps, dvec3 curPos, IntegralLine &line, double stepSize,
                            bool normalzieSample) {
    for (int i = 0; i <= steps; i++) {
        if (curPos.x < 0) break;
        if (curPos.y < 0) break;
        if (curPos.z < 0) break;

        if (curPos.x > 1 - 1.0 / dimensions_.x) break;
        if (curPos.y > 1 - 1.0 / dimensions_.y) break;
        if (curPos.z > 1 - 1.0 / dimensions_.z) break;

        dvec3 v;
        switch (integrationScheme_)
        {
        case inviwo::StreamLineTracer::IntegrationScheme::RK4:
            v = rk4(curPos,stepSize,normalzieSample, invBasis_);
            break;
        case inviwo::StreamLineTracer::IntegrationScheme::Euler:
        default:
            v = euler(curPos);
            break;
        }
        
        if (glm::length(v) == 0) {
            break;
        }

        
        dvec3 worldVelocty = volumeSampler_.sample(curPos).xyz();


        if (normalzieSample) v = glm::normalize(v);
        dvec3 velocity = invBasis_ * (v * stepSize);
        line.positions_.push_back(curPos);
        line.metaData_["velocity"].push_back(worldVelocty);
        for (auto m : metaVolumes_) {
            line.metaData_[m.first].push_back(m.second.sample(curPos).xyz());
        }

        curPos += velocity;
    }
}

dvec3 StreamLineTracer::euler(const dvec3 &curPos) {
    return volumeSampler_.sample(curPos).xyz();
}

dvec3 StreamLineTracer::rk4(const dvec3 &curPos, double stepSize, bool normalzieSample , dmat3 m ) {
    auto h = stepSize / 2;
    auto k1 = volumeSampler_.sample(curPos).xyz();
    if (normalzieSample) k1 = glm::normalize(k1);
    auto K1 = m * k1;
    auto k2 = volumeSampler_.sample(curPos + K1 * h).xyz();
    if (normalzieSample) k2 = glm::normalize(k2);
    auto K2 = m * k2;
    auto k3 = volumeSampler_.sample(curPos + K2 * h).xyz();
    if (normalzieSample) k3 = glm::normalize(k3);
    auto K3 = m * k3;
    auto k4 = volumeSampler_.sample(curPos + K3 * stepSize).xyz();
    if (normalzieSample) k4 = glm::normalize(k4);
    /*
    auto k1 = volumeSampler_.sample(glm::clamp(curPos, 0.0, 1.0)).xyz();
    auto k2 = volumeSampler_.sample(glm::clamp(curPos + k1 * h, 0.0, 1.0)).xyz();
    auto k3 = volumeSampler_.sample(glm::clamp(curPos + k2 * h, 0.0, 1.0)).xyz();
    auto k4 = volumeSampler_.sample(glm::clamp(curPos + k3 * stepSize, 0.0, 1.0)).xyz(); */
    return (k1+2.0*(k2+k3)+k4 )/6.0;
}

}  // namespace
