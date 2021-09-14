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
#include <modules/discretedata/util/arrayutil.h>
namespace inviwo {
namespace discretedata {

// template <unsigned int Dim>
// Interpolant<Dim>* Interpolant<Dim>::copy() const {
//     return new Interpolant<Dim>();
// }

template <unsigned int Dim>
bool Interpolant<Dim>::supportsInterpolationType(InterpolationType type) const {
    return (type == InterpolationType::Ignore || type == InterpolationType::Nearest ||
            type == InterpolationType::SquaredDistance);
}

template <unsigned int Dim>
bool Interpolant<Dim>::getWeights(InterpolationType type,
                                  const std::vector<std::array<float, Dim>>& coordinates,
                                  std::vector<double>& weights,
                                  const std::array<float, Dim>& position) const {
    switch (type) {
        case InterpolationType::Ignore:
            return true;
        case InterpolationType::Nearest:
            [[fallthrough]];
        case InterpolationType::SquaredDistance:
            weights.resize(coordinates.size());
            for (size_t c = 0; c < weights.size(); ++c) {
                weights[c] = 0;
                for (size_t d = 0; d < Dim; ++d) {
                    weights[c] += std::pow((coordinates[c])[d] - position[d], 2);
                }
            }
            if (type == InterpolationType::Nearest) {
                auto min = std::min_element(weights.begin(), weights.end());
                std::fill(weights.begin(), weights.end(), 0.f);
                *min = 1.f;
            } else {
                float sum = std::accumulate(weights.begin(), weights.end(), 0.f);
                for (auto& weight : weights) {
                    weight /= sum;
                }
            }
            return true;
        default:
            return false;
    }
}

template <unsigned int Dim>
bool SkewedBoxInterpolant<Dim>::supportsInterpolationType(InterpolationType type) const {
    return (type == InterpolationType::Ignore || type == InterpolationType::Nearest ||
            type == InterpolationType::SquaredDistance ||
            (type == InterpolationType::Linear && Dim >= 2));
}

template <unsigned int Dim>
bool SkewedBoxInterpolant<Dim>::getWeights(InterpolationType type,
                                           const std::vector<std::array<float, Dim>>& coordinates,
                                           std::vector<double>& weights,
                                           const std::array<float, Dim>& pos) const {

    static const bool debugOutput = false;
    if (debugOutput)
        std::cout << "% Doing the skewed box weights! Dim " << Dim
                  << ", num coordinates: " << coordinates.size() << std::endl;
    if (Dim < 2 || coordinates.size() != (1 << Dim)) return false;
    if (debugOutput) std::cout << "%   Now for real!" << std::endl;
    weights.resize(5);

    // The important part: the relative position within the cell.
    double u, v;

    // Blindly assume all faces are planar.
    auto a = coordinates[0];                        // TODO: Is this the order?
    auto b = dd_util::arrMinus(coordinates[1], a);  // b--d
    auto c = dd_util::arrMinus(coordinates[2], a);  // |  |
    auto d = dd_util::arrMinus(coordinates[3], a);  // a--c
    auto p = dd_util::arrMinus(pos, a);
    a = {0};
    if (debugOutput)
        std::cout << fmt::format(
                         "Relative positions:\n\ta = ({}, {})\n\tb = ({}, {})\n\tc = ({}, {})\n\td "
                         "= ({}, "
                         "{})\n\tp = ({}, {})",
                         a[0], a[1], b[0], b[1], c[0], c[1], d[0], d[1], p[0], p[1])
                  << std::endl;

    bool parallelX = dd_util::arrParallel(b, dd_util::arrMinus(d, c));
    bool parallelY = dd_util::arrParallel(c, dd_util::arrMinus(d, b));
    // double dotProdX = dd_util::arrDotProduct(b, dd_util::arrMinus(d, c));
    // double dotProdY = dd_util::arrDotProduct(c, dd_util::arrMinus(d, b));
    if (parallelX && parallelY) {
        // std::cerr << "Interpolant says both parallel!" << std::endl;
        // std::cout << fmt::format("u = {} = ({} - {}) / ({} - {})", u, p[1] * c[0], p[0] * c[1],
        //                          b[1] * c[0], b[0] * c[1])
        //           << std::endl;
        // std::cout << fmt::format("v = {} = ({} - {}) / ({} - {})", v, p[1] * b[0], p[0] * b[1],
        //                          b[0] * c[1], b[1] * c[0])
        //           << std::endl;
        u = (p[1] * c[0] - p[0] * c[1]) / (b[1] * c[0] - b[0] * c[1]);
        v = (p[1] * b[0] - p[0] * b[1]) / (c[1] * b[0] - c[0] * b[1]);
        // std::cout << "u: " << u << " - v: " << v << std::endl;

    } else {
        double lx = d[0] - b[0];
        double ly = d[1] - b[1];
        // double t = double(a[1] - b[1]) / (ly * (1.0 - (lx * c[1]) / (ly * c[0])));
        double s = (b[0] / lx - b[1] / ly) / (c[0] / lx - c[1] / ly);
        double fx = s * c[0];
        double fy = s * c[1];
        double jx = p[0] - fx;
        double jy = p[1] - fy;
        u = (fx / jx - fy / jy) / (b[0] / jx - b[1] / jy);
        v = (p[0] - u * b[0]) / (c[0] - u * b[0] - u * c[0] + u * d[0]);

        // DBG
        // weights.resize(3);
        // static bool first = true;
        // std::cerr << "more complicated case!" << std::endl;
        // first = false;
        // TODO!
        // return false;
    }
    // Check if outside.
    if (u < 0 || u > 1 || v < 0 || v > 1) {
        // static bool firstUV = true;
        // if (firstUV)
        if (debugOutput) std::cout << fmt::format("% ({}, {}) not inside [0,1]", u, v) << std::endl;
        // firstUV = false;

        return false;
    }
    if (debugOutput) std::cerr << "% Point inside cell" << std::endl;

    switch (type) {
        case InterpolationType::Ignore:
            std::cout << "Ignored interpolation" << std::endl;
            return true;
        case InterpolationType::Nearest:
            [[fallthrough]];
        case InterpolationType::SquaredDistance:
            // std::cout << "Nearest/squares interpolation" << std::endl;
            return Interpolant<Dim>::getWeights(type, coordinates, weights, pos);
        case InterpolationType::Linear:
            // TODO: Do interpolation!
            weights.resize(coordinates.size());

            // ONLY 2D FOR NOW!
            weights[0] = (1.0 - u) * (1.0 - v);
            weights[1] = u * (1 - v);
            weights[2] = (1.0 - u) * v;
            weights[3] = u * v;
            // std::cout << "Some nice weights!" << std::endl;
            return true;
        default:
            std::cout << "...what interpolation?!" << std::endl;
            return false;
    }
}

template <unsigned int Dim>
Interpolant<Dim>* SkewedBoxInterpolant<Dim>::copy() const {
    return new SkewedBoxInterpolant<Dim>();
}

// template <GridPrimitive Dim>
// bool SkewedBoxInterpolant<Dim>::isInside(const std::vector<std::array<float, Dim>>& coordinates,
//                                          const std::array<float, Dim>& position) const {
//     return false;
// }

}  // namespace discretedata
}  // namespace inviwo