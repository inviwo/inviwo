/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015-2024 Inviwo Foundation
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

#pragma once

#include <inviwo/core/util/glmconvert.h>        // for glm_convert
#include <inviwo/core/util/glmutils.h>          // for Matrix
#include <inviwo/core/util/glmvec.h>            // for dvec3
#include <inviwo/core/util/spatialsampler.h>    // IWUY pragma: keep
#include <inviwo/core/util/spatial4dsampler.h>  // IWUY pragma: keep
#include <inviwo/core/util/typetraits.h>
#include <modules/vectorfieldvisualization/datastructures/integralline.h>        // for Integral...
#include <modules/vectorfieldvisualization/properties/integrallineproperties.h>  // for Integral...

#include <cstddef>        // for size_t
#include <limits>         // for numeric_...
#include <memory>         // for shared_ptr
#include <string>         // for string
#include <unordered_map>  // for unordere...
#include <utility>        // for pair
#include <vector>         // for vector

namespace inviwo {

template <typename SpatialSampler, bool TimeDependent>
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

    using SampleType = typename SpatialSampler::type;
    static constexpr auto SampleDim = util::extent_v<SampleType>;

    using SpatialVector = Vector<Sampler::SpatialDimensions, double>;
    using DataVector = Vector<SampleDim, double>;
    using DataHomogenousVector = Vector<SampleDim + 1, double>;
    using DataMatrix = Matrix<SampleDim, double>;
    using DataHomogeneousSpatialMatrix = Matrix<SampleDim + 1, double>;

    IntegralLineTracer(std::shared_ptr<const Sampler> sampler,
                       const IntegralLineProperties& properties);

    Result traceFrom(const SpatialVector& pIn) const;

    void addMetaDataSampler(const std::string& name, std::shared_ptr<const Sampler> sampler);

    const DataHomogeneousSpatialMatrix& getSeedTransformationMatrix() const;

private:
    struct StepResult {
        SpatialVector position;
        DataVector data;
        bool outOfBounds;
    };

    inline SpatialVector seedTransform(const SpatialVector& seed) const;

    StepResult step(const SpatialVector& oldPos, double stepSize) const;

    bool addPoint(IntegralLine& line, const SpatialVector& pos) const;
    bool addPoint(IntegralLine& line, const SpatialVector& pos,
                  const DataVector& worldVelocity) const;

    IntegralLine::TerminationReason integrate(size_t steps, SpatialVector pos, IntegralLine& line,
                                              bool fwd) const;

    IntegralLineProperties::IntegrationScheme integrationScheme_;

    int steps_;
    double stepSize_;
    IntegralLineProperties::Direction dir_;
    bool normalizeSamples_;

    std::shared_ptr<const Sampler> sampler_;
    std::unordered_map<std::string, std::shared_ptr<const Sampler>> metaSamplers_;

    DataMatrix invBasis_;
    DataHomogeneousSpatialMatrix seedTransformation_;
};

template <typename SpatialSampler, bool TimeDependent>
IntegralLineTracer<SpatialSampler, TimeDependent>::IntegralLineTracer(
    std::shared_ptr<const Sampler> sampler, const IntegralLineProperties& properties)
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
IntegralLineTracer<SpatialSampler, TimeDependent>::traceFrom(const SpatialVector& pIn) const {
    const SpatialVector p = seedTransform(pIn);
    Result res;
    IntegralLine& line = res.line;

    const auto [stepsBWD, stepsFWD] = [dir = dir_, steps = steps_,
                                       &line]() -> std::pair<size_t, size_t> {
        switch (dir) {
            case inviwo::IntegralLineProperties::Direction::Forward:
                line.setBackwardTerminationReason(IntegralLine::TerminationReason::StartPoint);
                return {1, steps + 1};
            case inviwo::IntegralLineProperties::Direction::Backward:
                line.setForwardTerminationReason(IntegralLine::TerminationReason::StartPoint);
                return {steps + 1, 1};
            default:
            case inviwo::IntegralLineProperties::Direction::Bidirectional: {
                return {steps / 2 + 1, steps - (steps / 2) + 1};
            }
        }
    }();

    line.getPositions().reserve(steps_ + 2);
    line.getMetaData<dvec3>("velocity", true).reserve(steps_ + 2);

    if constexpr (TimeDependent) {
        line.getMetaData<double>("timestamp", true).reserve(steps_ + 2);
    }

    for (auto& m : metaSamplers_) {
        line.getMetaData<typename Sampler::type>(m.first, true).reserve(steps_ + 2);
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
    const std::string& name, std::shared_ptr<const Sampler> sampler) {
    metaSamplers_[name] = sampler;
}

template <typename SpatialSampler, bool TimeDependent>
auto IntegralLineTracer<SpatialSampler, TimeDependent>::getSeedTransformationMatrix() const
    -> const DataHomogeneousSpatialMatrix& {
    return seedTransformation_;
}

template <typename SpatialSampler, bool TimeDependent>
inline auto IntegralLineTracer<SpatialSampler, TimeDependent>::seedTransform(
    const SpatialVector& seed) const -> SpatialVector {
    if constexpr (IsTimeDependent) {
        auto p = seedTransformation_ * SpatialVector(DataVector(seed), 1.0f);
        return SpatialVector(DataVector(p) / p[SampleDim], seed[SampleDim]);
    } else {
        auto p = seedTransformation_ * DataHomogenousVector(DataVector(seed), 1.0f);
        return SpatialVector(p) / p[SampleDim];
    }
}

template <typename SpatialSampler, bool TimeDependent>
auto IntegralLineTracer<SpatialSampler, TimeDependent>::step(const SpatialVector& oldPos,
                                                             double stepSize) const -> StepResult {
    auto normalize = [](const auto v) {
        auto l = glm::length(v);
        if (l == 0) return v;
        return v / l;
    };

    auto move = [&](const SpatialVector& pos, DataVector v, double stepsize) -> SpatialVector {
        if (normalizeSamples_) {
            v = normalize(v);
        }
        DataVector offset = (invBasis_ * (v * stepsize));
        if constexpr (TimeDependent) {
            return pos + SpatialVector(offset, stepsize);
        } else if constexpr (SampleDim == 3) {
            return pos + offset;
        } else if constexpr (SampleDim == 2) {
            return SpatialVector{DataVector{pos} + offset, 0.0};
        } else {
            static_assert(util::alwaysFalse<SpatialSampler>(),
                          "Unsupported number of DataDimensions");
        }
    };

    auto k1 = sampler_->sample(oldPos);

    switch (integrationScheme_) {
        case inviwo::IntegralLineProperties::IntegrationScheme::Euler:
            return {move(oldPos, k1, stepSize), k1, false};
        default:
            [[fallthrough]];
        case inviwo::IntegralLineProperties::IntegrationScheme::RK4: {
            SpatialVector pos = move(oldPos, k1, stepSize / 2);
            if (!sampler_->withinBounds(pos)) {
                return {oldPos, k1, true};
            }
            const DataVector k2 = sampler_->sample(pos);

            pos = move(oldPos, k2, stepSize / 2);
            if (!sampler_->withinBounds(pos)) {
                return {oldPos, k1, true};
            }
            const DataVector k3 = sampler_->sample(pos);

            pos = move(oldPos, k3, stepSize);
            if (!sampler_->withinBounds(pos)) {
                return {oldPos, k1, true};
            }
            const DataVector k4 = sampler_->sample(pos);

            const auto&& K = [n = normalizeSamples_, normalize, &k1, &k2, &k3, &k4]() {
                if (n) {
                    return normalize(k1 + k2 + k2 + k3 + k3 + k4);
                } else {
                    return (k1 + k2 + k2 + k3 + k3 + k4) / 6.0;
                }
            };
            return {move(oldPos, K(), stepSize), k1, false};
        }
    }
}

template <typename SpatialSampler, bool TimeDependent>
bool IntegralLineTracer<SpatialSampler, TimeDependent>::addPoint(IntegralLine& line,
                                                                 const SpatialVector& pos) const {
    return addPoint(line, pos, sampler_->sample(pos));
}

template <typename SpatialSampler, bool TimeDependent>
bool IntegralLineTracer<SpatialSampler, TimeDependent>::addPoint(
    IntegralLine& line, const SpatialVector& pos, const DataVector& worldVelocity) const {

    if (glm::length(worldVelocity) < std::numeric_limits<double>::epsilon()) {
        return false;
    }

    line.getPositions().emplace_back(util::glm_convert<dvec3>(pos));
    line.getMetaData<dvec3>("velocity").emplace_back(util::glm_convert<dvec3>(worldVelocity));

    if constexpr (TimeDependent) {
        line.getMetaData<double>("timestamp").emplace_back(pos[Sampler::SpatialDimensions - 1]);
    }

    for (auto& m : metaSamplers_) {
        line.getMetaData<typename Sampler::type>(m.first).emplace_back(
            util::glm_convert<dvec3>(m.second->sample(pos)));
    }
    return true;
}

template <typename SpatialSampler, bool TimeDependent>
IntegralLine::TerminationReason IntegralLineTracer<SpatialSampler, TimeDependent>::integrate(
    size_t steps, SpatialVector pos, IntegralLine& line, bool fwd) const {
    if (steps == 0) return IntegralLine::TerminationReason::StartPoint;
    for (size_t i = 0; i < steps; i++) {
        if (!sampler_->withinBounds(pos)) {
            return IntegralLine::TerminationReason::OutOfBounds;
        }
        StepResult result = step(pos, stepSize_ * (fwd ? 1.0 : -1.0));
        if (result.outOfBounds) {
            return IntegralLine::TerminationReason::OutOfBounds;
        }
        pos = result.position;

        if (!addPoint(line, result.position, result.data)) {
            return IntegralLine::TerminationReason::ZeroVelocity;
        }
    }
    return IntegralLine::TerminationReason::Steps;
}

using StreamLine2DTracer = IntegralLineTracer<SpatialSampler<dvec2>, false>;
using StreamLine3DTracer = IntegralLineTracer<SpatialSampler<dvec3>, false>;
using PathLine3DTracer = IntegralLineTracer<Spatial4DSampler<dvec3>, true>;

}  // namespace inviwo
