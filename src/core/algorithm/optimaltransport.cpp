/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2026 Inviwo Foundation
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

#include <inviwo/core/algorithm/optimaltransport.h>

#include <algorithm>
#include <cmath>
#include <numeric>

namespace inviwo {

namespace algorithm {

namespace detail {

namespace {

// A point on the CDF curve: position, cumulative value, and the interpolated RGB color at that
// position in the original TF.
struct CdfPoint {
    double pos;  // position along the TF domain
    double cdf;  // cumulative alpha-mass up to this position (normalized to [0,1])
    vec3 color;  // RGB color at this position (interpolated from the TF)
};

// Evaluate the piecewise linear alpha (density) at position x
double evaluateAlpha(std::span<const TFPrimitiveData> tf, double x) {
    if (tf.empty()) return 0.0;
    if (x <= tf.front().pos) return static_cast<double>(tf.front().color.a);
    if (x >= tf.back().pos) return static_cast<double>(tf.back().color.a);

    // Binary search for the segment
    const auto it = std::lower_bound(tf.begin(), tf.end(), x,
                               [](const TFPrimitiveData& p, double val) { return p.pos < val; });

    if (it == tf.begin()) return static_cast<double>(tf.front().color.a);

    const auto prev = std::prev(it);
    const double dx = it->pos - prev->pos;
    if (dx < 1e-15) return static_cast<double>(it->color.a);

    const double t = (x - prev->pos) / dx;
    return glm::mix(static_cast<double>(prev->color.a), static_cast<double>(it->color.a), t);
}

// Evaluate the piecewise linear RGB color at position x
vec3 evaluateColor(std::span<const TFPrimitiveData> tf, double x) {
    if (tf.empty()) return vec3(0.0f);
    if (x <= tf.front().pos) return vec3(tf.front().color);
    if (x >= tf.back().pos) return vec3(tf.back().color);

    const auto it = std::lower_bound(tf.begin(), tf.end(), x,
                               [](const TFPrimitiveData& p, double val) { return p.pos < val; });

    if (it == tf.begin()) return vec3(tf.front().color);

    const auto prev = std::prev(it);
    const double dx = it->pos - prev->pos;
    if (dx < 1e-15) return vec3(it->color);

    const float t = static_cast<float>((x - prev->pos) / dx);
    return glm::mix(vec3(prev->color), vec3(it->color), t);
}

// Compute the CDF of the alpha channel (treated as a piecewise linear density).
// Returns CDF points at each TF primitive position.
std::vector<CdfPoint> computeCdf(std::span<const TFPrimitiveData> tf) {
    std::vector<CdfPoint> cdf;
    cdf.reserve(tf.size());

    double cumulative = 0.0;
    cdf.push_back({tf.front().pos, 0.0, vec3(tf.front().color)});

    for (size_t i = 1; i < tf.size(); ++i) {
        const double dx = tf[i].pos - tf[i - 1].pos;
        // Trapezoidal integration of alpha
        const double area =
            0.5 * (static_cast<double>(tf[i - 1].color.a) + static_cast<double>(tf[i].color.a)) *
            dx;
        cumulative += area;
        cdf.push_back({tf[i].pos, cumulative, vec3(tf[i].color)});
    }

    // Normalize
    const double totalMass = cumulative;
    if (totalMass > 1e-15) {
        for (auto& p : cdf) {
            p.cdf /= totalMass;
        }
    }

    return cdf;
}

// Compute total alpha mass of a TF (integral of alpha over position)
double totalMass(std::span<const TFPrimitiveData> tf) {
    double mass = 0.0;
    for (size_t i = 1; i < tf.size(); ++i) {
        const double dx = tf[i].pos - tf[i - 1].pos;
        mass += 0.5 *
                (static_cast<double>(tf[i - 1].color.a) + static_cast<double>(tf[i].color.a)) * dx;
    }
    return mass;
}

// Invert the CDF: given a quantile q in [0,1], find the position x where CDF(x) = q.
double invertCdf(const std::vector<CdfPoint>& cdf, double q) {
    q = std::clamp(q, 0.0, 1.0);

    if (q <= cdf.front().cdf) return cdf.front().pos;
    if (q >= cdf.back().cdf) return cdf.back().pos;

    const auto it = std::lower_bound(cdf.begin(), cdf.end(), q,
                               [](const CdfPoint& p, double val) { return p.cdf < val; });

    if (it == cdf.begin()) return cdf.front().pos;

    const auto prev = std::prev(it);
    const double dy = it->cdf - prev->cdf;
    if (std::abs(dy) < 1e-15) return prev->pos;

    const double t = (q - prev->cdf) / dy;
    return prev->pos + t * (it->pos - prev->pos);
}

// Interpolate RGB color at a given quantile level from a CDF
vec3 colorAtQuantile(const std::vector<CdfPoint>& cdf, double q) {
    q = std::clamp(q, 0.0, 1.0);

    if (q <= cdf.front().cdf) return cdf.front().color;
    if (q >= cdf.back().cdf) return cdf.back().color;

    const auto it = std::lower_bound(cdf.begin(), cdf.end(), q,
                               [](const CdfPoint& p, double val) { return p.cdf < val; });

    if (it == cdf.begin()) return cdf.front().color;

    const auto prev = std::prev(it);
    const double dy = it->cdf - prev->cdf;
    if (std::abs(dy) < 1e-15) return prev->color;

    const float t = static_cast<float>((q - prev->cdf) / dy);
    return glm::mix(prev->color, it->color, t);
}

// Merge all quantile levels (CDF breakpoints) from both CDFs, ensuring all features are captured.
std::vector<double> mergeQuantileLevels(const std::vector<CdfPoint>& cdfA,
                                        const std::vector<CdfPoint>& cdfB) {
    std::vector<double> levels;
    levels.reserve(cdfA.size() + cdfB.size());

    for (const auto& p : cdfA) levels.push_back(p.cdf);
    for (const auto& p : cdfB) levels.push_back(p.cdf);

    std::sort(levels.begin(), levels.end());
    levels.erase(std::unique(levels.begin(), levels.end(),
                             [](double a, double b) { return std::abs(a - b) < 1e-15; }),
                 levels.end());

    return levels;
}

}
}

std::vector<TFPrimitiveData> optimalTransportInterpolation(std::span<const TFPrimitiveData> tfA,
                                                           std::span<const TFPrimitiveData> tfB,
                                                           double t) {
    if (tfA.empty()) return std::vector<TFPrimitiveData>(tfB.begin(), tfB.end());
    if (tfB.empty()) return std::vector<TFPrimitiveData>(tfA.begin(), tfA.end());

    t = std::clamp(t, 0.0, 1.0);

    // Handle trivial endpoints
    if (t <= 0.0) return std::vector<TFPrimitiveData>(tfA.begin(), tfA.end());
    if (t >= 1.0) return std::vector<TFPrimitiveData>(tfB.begin(), tfB.end());

    // 1. Compute CDFs
    const auto cdfA = detail::computeCdf(tfA);
    const auto cdfB = detail::computeCdf(tfB);

    // 2. Compute total masses and interpolated target mass
    const double massA = detail::totalMass(tfA);
    const double massB = detail::totalMass(tfB);
    const double targetMass = (1.0 - t) * massA + t * massB;

    // Handle zero-mass cases
    if (massA < 1e-15 && massB < 1e-15) {
        // Both are zero mass - just linearly interpolate colors at merged positions
        std::vector<TFPrimitiveData> result;
        for (const auto& p : tfA) {
            const vec4 colorA = p.color;
            const vec3 colorB = detail::evaluateColor(tfB, p.pos);
            const float alphaB = static_cast<float>(detail::evaluateAlpha(tfB, p.pos));
            const vec4 blended = glm::mix(colorA, vec4(colorB, alphaB), static_cast<float>(t));
            result.push_back({p.pos, blended});
        }
        return result;
    }

    // 3. Gather all quantile levels from both CDFs
    const std::vector<double> levels = detail::mergeQuantileLevels(cdfA, cdfB);

    // 4. Build interpolated points: for each quantile q,
    //    - interpolate position: x_t = (1-t)*Q_A(q) + t*Q_B(q)
    //    - interpolate color: c_t = (1-t)*c_A(q) + t*c_B(q)
    struct InterpolatedPoint {
        double pos;
        double cdf;
        vec3 color;
    };

    std::vector<InterpolatedPoint> interpolatedCdf;
    interpolatedCdf.reserve(levels.size());

    for (const double q : levels) {
        const double xA = detail::invertCdf(cdfA, q);
        const double xB = detail::invertCdf(cdfB, q);
        const double xInterp = (1.0 - t) * xA + t * xB;

        const vec3 cA = detail::colorAtQuantile(cdfA, q);
        const vec3 cB = detail::colorAtQuantile(cdfB, q);
        const vec3 cInterp = glm::mix(cA, cB, static_cast<float>(t));

        interpolatedCdf.push_back({xInterp, q, cInterp});
    }

    // 5. Differentiate the interpolated CDF to recover the alpha (density).
    //    density(x) = dCDF/dx at each point.
    std::vector<TFPrimitiveData> result;
    result.reserve(interpolatedCdf.size());

    for (size_t i = 0; i < interpolatedCdf.size(); ++i) {
        double density = 0.0;

        if (interpolatedCdf.size() < 2) {
            density = 0.0;
        } else if (i == 0) {
            const double dx = interpolatedCdf[1].pos - interpolatedCdf[0].pos;
            const double dq = interpolatedCdf[1].cdf - interpolatedCdf[0].cdf;
            density = (std::abs(dx) > 1e-15) ? dq / dx : 0.0;
        } else if (i == interpolatedCdf.size() - 1) {
            const double dx = interpolatedCdf[i].pos - interpolatedCdf[i - 1].pos;
            const double dq = interpolatedCdf[i].cdf - interpolatedCdf[i - 1].cdf;
            density = (std::abs(dx) > 1e-15) ? dq / dx : 0.0;
        } else {
            // Average of left and right finite differences for smoother result
            const double dxL = interpolatedCdf[i].pos - interpolatedCdf[i - 1].pos;
            const double dqL = interpolatedCdf[i].cdf - interpolatedCdf[i - 1].cdf;
            const double dxR = interpolatedCdf[i + 1].pos - interpolatedCdf[i].pos;
            const double dqR = interpolatedCdf[i + 1].cdf - interpolatedCdf[i].cdf;

            const double dL = (std::abs(dxL) > 1e-15) ? dqL / dxL : 0.0;
            const double dR = (std::abs(dxR) > 1e-15) ? dqR / dxR : 0.0;
            density = 0.5 * (dL + dR);
        }

        // Density is in normalized CDF units; scale by target mass to get actual alpha
        float alpha = static_cast<float>(std::max(0.0, density * targetMass));
        alpha = std::clamp(alpha, 0.0f, 1.0f);

        const vec4 color(interpolatedCdf[i].color, alpha);
        result.push_back({interpolatedCdf[i].pos, color});
    }

    return result;
}

double earthMoversDistance(std::span<const TFPrimitiveData> tfA,
                           std::span<const TFPrimitiveData> tfB) {
    if (tfA.empty() || tfB.empty()) return 0.0;

    // Merge all positions from both TFs
    std::vector<double> positions;
    positions.reserve(tfA.size() + tfB.size());
    for (const auto& p : tfA) positions.push_back(p.pos);
    for (const auto& p : tfB) positions.push_back(p.pos);
    std::sort(positions.begin(), positions.end());
    positions.erase(std::unique(positions.begin(), positions.end(),
                                [](double a, double b) { return std::abs(a - b) < 1e-15; }),
                    positions.end());

    // Compute CDFs at merged positions
    const auto cdfA = detail::computeCdf(tfA);
    const auto cdfB = detail::computeCdf(tfB);

    const double massA = detail::totalMass(tfA);
    const double massB = detail::totalMass(tfB);

    // EMD = integral |F_A(x) - F_B(x)| dx  (for normalized distributions)
    // We approximate using the trapezoidal rule on the merged position grid
    if (massA < 1e-15 && massB < 1e-15) return 0.0;

    double emd = 0.0;
    double prevDiff = 0.0;
    double prevPos = positions.front();

    for (size_t i = 0; i < positions.size(); ++i) {
        const double x = positions[i];

        // Evaluate normalized CDFs at x by finding where x falls in each CDF
        // For CDF A: linearly interpolate in the cdfA structure
        double fA = 0.0;
        {
            if (x <= cdfA.front().pos)
                fA = 0.0;
            else if (x >= cdfA.back().pos)
                fA = 1.0;
            else {
                const auto it = std::lower_bound(
                    cdfA.begin(), cdfA.end(), x,
                    [](const detail::CdfPoint& p, double val) { return p.pos < val; });
                if (it == cdfA.begin())
                    fA = 0.0;
                else {
                    const auto prev = std::prev(it);
                    const double dx = it->pos - prev->pos;
                    const double tt = (dx > 1e-15) ? (x - prev->pos) / dx : 0.0;
                    fA = prev->cdf + tt * (it->cdf - prev->cdf);
                }
            }
        }

        double fB = 0.0;
        {
            if (x <= cdfB.front().pos)
                fB = 0.0;
            else if (x >= cdfB.back().pos)
                fB = 1.0;
            else {
                const auto it = std::lower_bound(
                    cdfB.begin(), cdfB.end(), x,
                    [](const detail::CdfPoint& p, double val) { return p.pos < val; });
                if (it == cdfB.begin())
                    fB = 0.0;
                else {
                    const auto prev = std::prev(it);
                    const double dx = it->pos - prev->pos;
                    const double tt = (dx > 1e-15) ? (x - prev->pos) / dx : 0.0;
                    fB = prev->cdf + tt * (it->cdf - prev->cdf);
                }
            }
        }

        const double diff = std::abs(fA - fB);
        if (i > 0) {
            const double dx = x - prevPos;
            emd += 0.5 * (prevDiff + diff) * dx;
        }
        prevDiff = diff;
        prevPos = x;
    }

    return emd;
}

}  // namespace algorithm

}  // namespace inviwo
