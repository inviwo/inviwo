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

namespace inviwo {

    PathLineTracer::PathLineTracer(
        std::shared_ptr<const std::vector<std::shared_ptr<Volume>>> volumeVector,
        const IntegralLineProperties &properties)
        : IntegralLineTracer(properties)
        , sampler_(volumeVector)
        , dimensions_(0)
    
    {
        if (volumeVector->empty()) {
            LogWarn("Initializing PathLineTracer with an empty vector");
        }
        else {
            invBasis_ = dmat3(glm::inverse(volumeVector->at(0)->getBasis()));
            dimensions_ = volumeVector->at(0)->getDimensions();
        }

        sampler_.setVectorInterpolation(true);
    }

PathLineTracer::~PathLineTracer() {}

inviwo::IntegralLine PathLineTracer::traceFrom(const vec4 &p) {
    return traceFrom(dvec4(p));
}

inviwo::IntegralLine PathLineTracer::traceFrom(const dvec4 &p) {
    IntegralLine line;

    auto direction = dir_;
    bool fwd = direction == IntegralLineProperties::Direction::BOTH || direction == IntegralLineProperties::Direction::FWD;
    bool bwd = direction == IntegralLineProperties::Direction::BOTH || direction == IntegralLineProperties::Direction::BWD;
    bool both = fwd && bwd;

    line.positions_.reserve(steps_ + 2);
    line.metaData_["velocity"].reserve(steps_ + 2);

    if (bwd) {
        step(steps_ / (both ? 2 : 1), p, line,false);
    }
    if (both && !line.positions_.empty()) {
        std::reverse(line.positions_.begin(),
                     line.positions_.end());  // reverse is faster than insert first
        line.positions_.pop_back();           // dont repeat first step
    }
    if (fwd) {
        step(steps_ / (both ? 2 : 1), p, line , true);
    }

    return line;
}

void PathLineTracer::step(int steps, dvec4 curPos, IntegralLine &line, bool fwd) {
    for (int i = 0; i <= steps; i++) {
        if (curPos.x < 0) break;
        if (curPos.y < 0) break;
        if (curPos.z < 0) break;

        if (curPos.x > 1 - 1.0 / dimensions_.x) break;
        if (curPos.y > 1 - 1.0 / dimensions_.y) break;
        if (curPos.z > 1 - 1.0 / dimensions_.z) break;

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

        dvec3 worldVelocty = sampler_.sample(curPos).xyz();

        dvec3 velocity = invBasis_ * (v * stepSize_ * (fwd ? 1.0 : -1.0));

        line.positions_.push_back(curPos.xyz());
        line.metaData_["velocity"].push_back(worldVelocty);

        curPos += dvec4(velocity, stepSize_);
        while (curPos.a > 1) curPos.a -= 1;
        while (curPos.a < 0) curPos.a += 1;
    }
}

inviwo::dvec3 PathLineTracer::euler(const dvec4 &curPos) { return sampler_.sample(curPos).xyz(); }

inviwo::dvec3 PathLineTracer::rk4(const dvec4 &curPos,bool fwd) {
    auto h = stepSize_;
    if (!fwd) h = -h;
    auto h2 = h / 2;
    auto k1 = sampler_.sample(curPos).xyz();
    auto K1 = invBasis_ * k1;
    auto k2 = sampler_.sample(curPos + dvec4(K1 * h2, h2)).xyz();
    auto K2 = invBasis_ * k2;
    auto k3 = sampler_.sample(curPos + dvec4(K2 * h2, h2)).xyz();
    auto K3 = invBasis_ * k3;
    auto k4 = sampler_.sample(curPos + dvec4(K3 * h , h)).xyz();

    return (k1 + 2.0 * (k2 + k3) + k4) / 6.0;
}

}  // namespace
