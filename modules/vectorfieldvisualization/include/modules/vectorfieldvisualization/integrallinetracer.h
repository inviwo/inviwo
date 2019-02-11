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

namespace detail {

template <typename SpatialSampler, typename V, typename H, typename M, typename P>
P seedTransform(std::true_type, const M &m, const P &pIn) {
    auto p = m * P(V(pIn), 1.0f);
    return P(V(p) / p[SpatialSampler::DataDimensions], pIn[SpatialSampler::DataDimensions]);
}

template <typename SpatialSampler, typename V, typename H, typename M, typename P>
P seedTransform(std::false_type, const M &m, const P &pIn) {
    auto p = m * H(pIn, 1.0f);
    return P(p) / p[SpatialSampler::DataDimensions];
}

template <typename P, typename V, typename F>
P moveHelper(std::true_type, const P &pos, const V &offset, F stepsize) {
    return pos + P(offset, stepsize);
}

template <typename P, typename V, typename F>
P moveHelper(std::false_type, const P &pos, const V &offset, F /*stepsize*/) {
    return pos + offset;
}

template <bool TimeDependent, typename SpatialVector, typename DataVector, typename Sampler,
          typename F, typename DataMatrix>
std::pair<SpatialVector, DataVector> step(
    const SpatialVector &oldPos, IntegralLineProperties::IntegrationScheme integrationScheme,
    F stepSize, const DataMatrix &invBasis, bool normalizeSamples, const Sampler &sampler) {

    auto normalize = [](auto v) {
        auto l = glm::length(v);
        if (l == 0) return v;
        return v / l;
    };

    auto move = [&](auto pos, auto v, auto stepsize) {
        if (normalizeSamples) {
            v = normalize(v);
        }
        auto offset = (invBasis * (v * stepsize));

        return moveHelper(typename std::integral_constant<bool, TimeDependent>::type(), pos, offset,
                          stepsize);
    };

    auto k1 = sampler.sample(oldPos);

    if (integrationScheme == IntegralLineProperties::IntegrationScheme::RK4) {
        auto k2 = sampler.sample(move(oldPos, k1, stepSize / 2));
        auto k3 = sampler.sample(move(oldPos, k2, stepSize / 2));
        auto k4 = sampler.sample(move(oldPos, k3, stepSize));
        auto K = k1 + k2 + k2 + k3 + k3 + k4;
        if (normalizeSamples) {
            K = normalize(K);
        } else {
            K = K / 6.0;
        }

        return {move(oldPos, K, stepSize), k1};
    } else {
        return {move(oldPos, k1, stepSize), k1};
    }
}
}  // namespace detail

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

    void setTransformOutputToWorldSpace(bool transform);
    bool isTransformingOutputToWorldSpace() const;

private:
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
    DataHomogenouSpatialMatrixrix toWorld_;
    bool transformOutputToWorldSpace_;
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
          properties.getSeedPointTransformationMatrix(sampler->getCoordinateTransformer()))
    , toWorld_(sampler->getCoordinateTransformer().getDataToWorldMatrix())
    , transformOutputToWorldSpace_{false} {}

template <typename SpatialSampler, bool TimeDependent>
typename IntegralLineTracer<SpatialSampler, TimeDependent>::Result
IntegralLineTracer<SpatialSampler, TimeDependent>::traceFrom(const SpatialVector &pIn) {
    SpatialVector p = detail::seedTransform<Sampler, DataVector, DataHomogenousVector>(
        typename std::integral_constant<bool, TimeDependent>::type(), seedTransformation_, pIn);

    Result res;
    IntegralLine &line = res.line;

    bool both = dir_ == IntegralLineProperties::Direction::BOTH;
    bool fwd = both || dir_ == IntegralLineProperties::Direction::FWD;
    bool bwd = both || dir_ == IntegralLineProperties::Direction::BWD;

    size_t stepsFWD = 0;
    size_t stepsBWD = 0;

    if (both) {
        stepsBWD = steps_ / 2;
        stepsFWD = steps_ - stepsBWD;
    } else if (fwd) {
        line.setBackwardTerminationReason(IntegralLine::TerminationReason::StartPoint);
        stepsFWD = steps_;
    } else if (bwd) {
        line.setForwardTerminationReason(IntegralLine::TerminationReason::StartPoint);
        stepsBWD = steps_;
    }

    stepsBWD++;  // for adjendency info
    stepsFWD++;

    line.getPositions().reserve(steps_ + 2);
    line.getMetaData<dvec3>("velocity", true).reserve(steps_ + 2);

    if (TimeDependent) {
        line.getMetaData<double>("timestamp", true).reserve(steps_ + 2);
    }

    for (auto &m : metaSamplers_) {
        line.getMetaData<typename Sampler::ReturnType>(m.first, true).reserve(steps_ + 2);
    }

    if (!addPoint(line, p)) {
        return res;  // Zero velocity at seed point
    }

    line.setBackwardTerminationReason(integrate(stepsBWD, p, line, false));

    if (!line.getPositions().empty()) {
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
void IntegralLineTracer<SpatialSampler, TimeDependent>::setTransformOutputToWorldSpace(
    bool transform) {
    transformOutputToWorldSpace_ = transform;
}

template <typename SpatialSampler, bool TimeDependent>
bool IntegralLineTracer<SpatialSampler, TimeDependent>::isTransformingOutputToWorldSpace() const {
    return transformOutputToWorldSpace_;
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

    if (transformOutputToWorldSpace_) {
        SpatialVector worldPos = detail::seedTransform<Sampler, DataVector, DataHomogenousVector>(
            typename std::integral_constant<bool, TimeDependent>::type(), toWorld_, pos);

        line.getPositions().emplace_back(util::glm_convert<dvec3>(worldPos));
    } else {
        line.getPositions().emplace_back(util::glm_convert<dvec3>(pos));
    }

    line.getMetaData<dvec3>("velocity").emplace_back(util::glm_convert<dvec3>(worldVelocity));

    if (TimeDependent) {
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

        auto res = detail::step<TimeDependent, SpatialVector, DataVector>(
            pos, integrationScheme_, stepSize_ * (fwd ? 1.0 : -1.0), invBasis_, normalizeSamples_,
            *sampler_);
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
