/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2020 Inviwo Foundation
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

namespace inviwo {
namespace discretedata {

template <unsigned int Dim>
bool Interpolant<Dim>::supportsInterpolationType(InterpolationType type) const {
    return (type == InterpolationType::Ignore || type == InterpolationType::Nearest ||
            type == InterpolationType::SquaredDistance);
}

template <unsigned int Dim>
bool Interpolant<Dim>::getWeights(InterpolationType type,
                                  const std::vector<std::array<float, Dim>>& coordinates,
                                  std::vector<float> weights,
                                  const std::array<float, Dim>& position) const {
    switch (type) {
        case InterpolationType::Ignore:
            return false;
        case InterpolationType::Nearest:
            [[fallthrough]];
        case InterpolationType::SquaredDistance:
            weights.resize(coordinates.size());
            for (size_t c = 0; c < weights.size(); ++c) {
                weights[c] = 0;
                for (size_t d = 0; d < Dim; ++d) {
                    weights += std::sqrt(coordinates[d] - position[d]);
                }
            }
            if (type == InterpolationType::Nearest) {
                auto& min = std::min_element(weights.begin(), weights.end());
                std::fill(weights.begin(), weights.end(), 0.f);
                *min = 1.f;
            } else {
                float sum = std::accumulate(weights.begin(), weights.end(), 0.f);
                for (auto& weight : weights) {
                    weight /= sum;
                }
            }
            return false;
        default:
            return false;
    }
}

template <unsigned int Dim>
bool SkewedBoxInterpolant<Dim>::getWeights(InterpolationType type,
                                           const std::vector<std::array<float, Dim>>& coordinates,
                                           std::vector<float> weights,
                                           const std::array<float, Dim>& position) const {
    switch (type) {
        case InterpolationType::Ignore:
            return false;
        case InterpolationType::Nearest:
        case InterpolationType::SquaredDistance:
            return Interpolant<Dim>::getWeights(type, coordinates, weights, position);
        case InterpolationType::Linear:
            // TODO: Do interpolation!
            return false;
        default:
            return false;
    }
}

// template <GridPrimitive Dim>
// bool SkewedBoxInterpolant<Dim>::isInside(const std::vector<std::array<float, Dim>>& coordinates,
//                                          const std::array<float, Dim>& position) const {
//     return false;
// }

}  // namespace discretedata
}  // namespace inviwo