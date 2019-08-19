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

#ifndef IVW_INTEGRALLINETRACER_H
#define IVW_INTEGRALLINETRACER_H

#include <modules/vectorfieldvisualization/vectorfieldvisualizationmoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/util/spatialsampler.h>
#include <inviwo/core/util/spatial4dsampler.h>
#include <inviwo/core/util/bufferutils.h>
#include <modules/vectorfieldvisualization/properties/integrallineproperties.h>
#include <modules/vectorfieldvisualization/datastructures/integralline.h>

#include <unordered_map>

namespace inviwo {

/**
 * \class IntegralLineTracer
 * \brief VERY_BRIEFLY_DESCRIBE_THE_CLASS
 * DESCRIBE_THE_CLASS
 */
template <typename SpatialSampler,
          bool TimeDependent = SpatialSampler::SpatialDimensions != SpatialSampler::DataDimensions>
class IntegralLineTracer {
public:
    using Sampler = SpatialSampler;

    struct Result {
        IntegralLine line;
        size_t seedIndex{0};
        operator IntegralLine() const { return line; }
    };

    const static bool IsTimeDependent = TimeDependent;

    /*
     * Various types used within this class
     */
    using SpatialVector = Vector<SpatialSampler::SpatialDimensions, double>;
    using DataVector = Vector<SpatialSampler::DataDimensions, double>;
    using DataHomogenousVector = Vector<SpatialSampler::DataDimensions + 1, double>;
    using SpatialMatrix = Matrix<SpatialSampler::SpatialDimensions, double>;
    using DataMatrix = Matrix<SpatialSampler::DataDimensions, double>;
    using DataHomogenouSpatialMatrixrix = Matrix<SpatialSampler::DataDimensions + 1, double>;

    IntegralLineTracer(std::shared_ptr<const Sampler> sampler,
                       const IntegralLineProperties &properties);

    Result traceFrom(const SpatialVector &pIn);

    void addMetaDataSampler(const std::string &name, std::shared_ptr<const Sampler> sampler);

    const DataHomogenouSpatialMatrixrix &getSeedTransformationMatrix() const;

private:
    inline SpatialVector seedTransform(const SpatialVector &seed) const;

    std::pair<SpatialVector, DataVector> step(const SpatialVector &oldPos, const double stepSize);

    bool addPoint(IntegralLine &line, const SpatialVector &pos);
    bool addPoint(IntegralLine &line, const SpatialVector &pos, const DataVector &worldVelocity);

    IntegralLine::TerminationReason integrate(size_t steps, SpatialVector pos, IntegralLine &line,
                                              bool fwd);

    IntegralLineProperties::IntegrationScheme integrationScheme_;

    int steps_;
    double stepSize_;
    IntegralLineProperties::Direction dir_;
    bool normalizeSamples_;

    std::shared_ptr<const Sampler> sampler_;
    std::unordered_map<std::string, std::shared_ptr<const Sampler>> metaSamplers_;

    DataMatrix invBasis_;
    DataHomogenouSpatialMatrixrix seedTransformation_;
};

template <typename SpatialSampler, bool TimeDependent>
IntegralLineTracer<SpatialSampler, TimeDependent>::IntegralLineTracer(
    std::shared_ptr<const Sampler> sampler, const IntegralLineProperties &properties)
    : integrationScheme_(properties.getIntegrationScheme())
    , steps_(properties.getNumberOfSteps())
    , stepSize_(properties.getStepSize())
    , dir_(properties.getStepDirection())
    , normalizeSamples_(properties.getNormalizeSamples())
    , sampler_(sampler)
    , invBasis_(glm::inverse(DataMatrix(sampler->getModelMatrix())))
    , seedTransformation_(
          properties.getSeedPointTransformationMatrix(sampler->getCoordinateTransformer())) {}

template <typename SpatialSampler, bool TimeDependent>
typename IntegralLineTracer<SpatialSampler, TimeDependent>::Result
IntegralLineTracer<SpatialSampler, TimeDependent>::traceFrom(const SpatialVector &pIn) {
    const SpatialVector p = seedTransform(pIn);
    Result res;
    IntegralLine &line = res.line;

    const auto [stepsBWD, stepsFWD] = [dir = dir_, steps = steps_,
                                       &line]() -> std::pair<size_t, size_t> {
        switch (dir) {
            case inviwo::IntegralLineProperties::Direction::FWD:
                line.setBackwardTerminationReason(IntegralLine::TerminationReason::StartPoint);
                return {1, steps + 1};
            case inviwo::IntegralLineProperties::Direction::BWD:
                line.setForwardTerminationReason(IntegralLine::TerminationReason::StartPoint);
                return {steps + 1, 1};
            default:
            case inviwo::IntegralLineProperties::Direction::BOTH: {
                return {steps / 2 + 1, steps - (steps / 2) + 1};
            }
        }
    }();

    line.getPositions().reserve(steps_ + 2);
    line.getMetaData<dvec3>("velocity", true).reserve(steps_ + 2);

    if constexpr (TimeDependent) {
        line.getMetaData<double>("timestamp", true).reserve(steps_ + 2);
    }

    for (auto &m : metaSamplers_) {
        line.getMetaData<typename Sampler::ReturnType>(m.first, true).reserve(steps_ + 2);
    }

    if (!addPoint(line, p)) {
        return res;  // Zero velocity at seed point
    }

    line.setBackwardTerminationReason(integrate(stepsBWD, p, line, false));

    if (line.getPositions().size() > 1) {
        line.reverse();
        res.seedIndex = line.getPositions().size() - 1;
    }

    line.setForwardTerminationReason(integrate(stepsFWD, p, line, true));
    return res;
}

template <typename SpatialSampler, bool TimeDependent>
void IntegralLineTracer<SpatialSampler, TimeDependent>::addMetaDataSampler(
    const std::string &name, std::shared_ptr<const Sampler> sampler) {
    metaSamplers_[name] = sampler;
}

template <typename SpatialSampler, bool TimeDependent>
const typename IntegralLineTracer<SpatialSampler, TimeDependent>::DataHomogenouSpatialMatrixrix &
IntegralLineTracer<SpatialSampler, TimeDependent>::getSeedTransformationMatrix() const {
    return seedTransformation_;
}

template <typename SpatialSampler, bool TimeDependent>
inline typename IntegralLineTracer<SpatialSampler, TimeDependent>::SpatialVector
IntegralLineTracer<SpatialSampler, TimeDependent>::seedTransform(const SpatialVector &seed) const {
    if constexpr (IsTimeDependent) {
        using V = DataVector;
        auto p = seedTransformation_ * SpatialVector(V(seed), 1.0f);
        return SpatialVector(V(p) / p[SpatialSampler::DataDimensions],
                             seed[SpatialSampler::DataDimensions]);
    } else {
        using H = DataHomogenousVector;
        auto p = seedTransformation_ * H(seed, 1.0f);
        return SpatialVector(p) / p[SpatialSampler::DataDimensions];
    }
}

template <typename SpatialSampler, bool TimeDependent>
std::pair<typename IntegralLineTracer<SpatialSampler, TimeDependent>::SpatialVector,
          typename IntegralLineTracer<SpatialSampler, TimeDependent>::DataVector>
IntegralLineTracer<SpatialSampler, TimeDependent>::step(const SpatialVector &oldPos,
                                                        const double stepSize) {
    auto normalize = [](const auto v) {
        auto l = glm::length(v);
        if (l == 0) return v;
        return v / l;
    };

    auto move = [&](const auto &pos, auto v, const auto stepsize) {
        if (normalizeSamples_) {
            v = normalize(v);
        }
        auto offset = (invBasis_ * (v * stepsize));
        if constexpr (TimeDependent) {
            return pos + SpatialVector(offset, stepsize);
        } else {
            return pos + offset;
        }
    };

    auto k1 = sampler_->sample(oldPos);

    switch (integrationScheme_) {
        case inviwo::IntegralLineProperties::IntegrationScheme::Euler:
            return {move(oldPos, k1, stepSize), k1};
        default:
            [[fallthrough]];
        case inviwo::IntegralLineProperties::IntegrationScheme::RK4: {
            const auto k2 = sampler_->sample(move(oldPos, k1, stepSize / 2));
            const auto k3 = sampler_->sample(move(oldPos, k2, stepSize / 2));
            const auto k4 = sampler_->sample(move(oldPos, k3, stepSize));
            const auto &&K = [n = normalizeSamples_, normalize, &k1, &k2, &k3, &k4]() {
                if (n) {
                    return normalize(k1 + k2 + k2 + k3 + k3 + k4);
                } else {
                    return (k1 + k2 + k2 + k3 + k3 + k4) * (1.0 / 6.0);
                }
            };
            return {move(oldPos, K(), stepSize), k1};
        }
    }
}

template <typename SpatialSampler, bool TimeDependent>
bool IntegralLineTracer<SpatialSampler, TimeDependent>::addPoint(IntegralLine &line,
                                                                 const SpatialVector &pos) {
    return addPoint(line, pos, sampler_->sample(pos));
}

template <typename SpatialSampler, bool TimeDependent>
bool IntegralLineTracer<SpatialSampler, TimeDependent>::addPoint(IntegralLine &line,
                                                                 const SpatialVector &pos,
                                                                 const DataVector &worldVelocity) {

    if (glm::length(worldVelocity) < std::numeric_limits<double>::epsilon()) {
        return false;
    }

    line.getPositions().emplace_back(util::glm_convert<dvec3>(pos));
    line.getMetaData<dvec3>("velocity").emplace_back(util::glm_convert<dvec3>(worldVelocity));

    if constexpr (TimeDependent) {
        line.getMetaData<double>("timestamp").emplace_back(pos[Sampler::SpatialDimensions - 1]);
    }

    for (auto &m : metaSamplers_) {
        line.getMetaData<typename Sampler::ReturnType>(m.first).emplace_back(
            util::glm_convert<dvec3>(m.second->sample(pos)));
    }
    return true;
}

template <typename SpatialSampler, bool TimeDependent>
IntegralLine::TerminationReason IntegralLineTracer<SpatialSampler, TimeDependent>::integrate(
    size_t steps, SpatialVector pos, IntegralLine &line, bool fwd) {
    if (steps == 0) return IntegralLine::TerminationReason::StartPoint;
    for (size_t i = 0; i < steps; i++) {
        if (!sampler_->withinBounds(pos)) {
            return IntegralLine::TerminationReason::OutOfBounds;
        }
        auto res = step(pos, stepSize_ * (fwd ? 1.0 : -1.0));
        pos = res.first;

        if (!addPoint(line, pos, res.second)) {
            return IntegralLine::TerminationReason::ZeroVelocity;
        }
    }
    return IntegralLine::TerminationReason::Steps;
}

using StreamLine2DTracer = IntegralLineTracer<SpatialSampler<2, 2, double>>;
using StreamLine3DTracer = IntegralLineTracer<SpatialSampler<3, 3, double>>;
using PathLine3DTracer = IntegralLineTracer<Spatial4DSampler<3, double>>;

}  // namespace inviwo

#endif  // IVW_INTEGRALLINETRACER_H
