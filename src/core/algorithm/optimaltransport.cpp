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
#include <vector>

namespace inviwo::algorithm {

namespace {

// Numerical tolerance for floating-point comparisons.
constexpr double eps = 1e-12;

// ---------------------------------------------------------------------------
// Internal data structures
// ---------------------------------------------------------------------------

// A point on the cumulative distribution function.
struct CdfPoint {
    double pos = 0.0;    // Position along the TF domain.
    double mass = 0.0;   // Cumulative mass up to this position.
    double alpha = 0.0;  // Alpha value at this position.
    vec3 color = vec3{0.0f};
};

// Piecewise-linear CDF built from a transfer function's alpha channel.
struct Cdf {
    std::vector<CdfPoint> points;
    double totalMass = 0.0;
};

// A point on the interpolated quantile function.
struct QuantilePoint {
    double pos = 0.0;  // Interpolated position.
    double q = 0.0;    // Quantile level in [0, 1].
    vec3 color = vec3{0.0f};
};

// ---------------------------------------------------------------------------
// TF evaluation helpers
// ---------------------------------------------------------------------------

// Extract non-negative alpha from a TF primitive.
double alphaOf(const TFPrimitiveData& p) { return std::max(0.0, static_cast<double>(p.color.a)); }

// Sort and deduplicate TF primitives by position. A piecewise-linear function cannot
// represent vertical discontinuities, so duplicate positions are collapsed (last wins).
std::vector<TFPrimitiveData> sanitize(std::span<const TFPrimitiveData> tf) {
    std::vector<TFPrimitiveData> points(tf.begin(), tf.end());
    std::sort(points.begin(), points.end(),
              [](const TFPrimitiveData& a, const TFPrimitiveData& b) { return a.pos < b.pos; });

    std::vector<TFPrimitiveData> result;
    result.reserve(points.size());
    for (const auto& p : points) {
        if (result.empty() || std::abs(p.pos - result.back().pos) > eps) {
            result.push_back(p);
        } else {
            result.back() = p;
        }
    }
    return result;
}

// Evaluate a piecewise-linear TF at position x via linear interpolation.
vec4 evaluate(std::span<const TFPrimitiveData> tf, double x) {
    if (tf.empty()) return vec4{0.0f};
    if (x <= tf.front().pos) return tf.front().color;
    if (x >= tf.back().pos) return tf.back().color;

    auto it = std::upper_bound(tf.begin(), tf.end(), x,
                               [](double v, const TFPrimitiveData& p) { return v < p.pos; });
    const auto& p0 = *(it - 1);
    const auto& p1 = *it;

    const double dx = p1.pos - p0.pos;
    if (std::abs(dx) < eps) return glm::mix(p0.color, p1.color, 0.5f);

    const auto u = static_cast<float>((x - p0.pos) / dx);
    return glm::mix(p0.color, p1.color, u);
}

// ---------------------------------------------------------------------------
// Fallback: simple pointwise linear blend
// ---------------------------------------------------------------------------

// Collect all unique positions from both TFs.
std::vector<double> mergedPositions(std::span<const TFPrimitiveData> a,
                                    std::span<const TFPrimitiveData> b) {
    std::vector<double> positions;
    positions.reserve(a.size() + b.size());
    for (const auto& p : a) positions.push_back(p.pos);
    for (const auto& p : b) positions.push_back(p.pos);

    std::sort(positions.begin(), positions.end());
    positions.erase(std::unique(positions.begin(), positions.end(),
                                [](double x, double y) { return std::abs(x - y) < eps; }),
                    positions.end());
    return positions;
}

// Pointwise linear interpolation, used as fallback when OT is not applicable
// (e.g., when one TF has zero mass).
std::vector<TFPrimitiveData> linearBlend(std::span<const TFPrimitiveData> a,
                                         std::span<const TFPrimitiveData> b, double t) {
    const auto positions = mergedPositions(a, b);
    std::vector<TFPrimitiveData> result;
    result.reserve(positions.size());
    for (double x : positions) {
        const vec4 ca = evaluate(a, x);
        const vec4 cb = evaluate(b, x);
        result.push_back({x, glm::mix(ca, cb, static_cast<float>(t))});
    }
    return result;
}

// ---------------------------------------------------------------------------
// CDF construction and inversion
// ---------------------------------------------------------------------------

// Build the CDF by integrating the piecewise-linear alpha channel (trapezoidal rule).
// Each CdfPoint stores the cumulative mass from the TF start up to that position.
Cdf computeCdf(std::span<const TFPrimitiveData> tf) {
    Cdf cdf;
    if (tf.empty()) return cdf;

    cdf.points.reserve(tf.size());
    cdf.points.push_back(
        CdfPoint{tf.front().pos, 0.0, alphaOf(tf.front()), vec3{tf.front().color}});

    double cumulative = 0.0;
    for (std::size_t i = 1; i < tf.size(); ++i) {
        const double x0 = tf[i - 1].pos;
        const double x1 = tf[i].pos;
        const double dx = x1 - x0;
        if (dx <= eps) continue;

        const double a0 = alphaOf(tf[i - 1]);
        const double a1 = alphaOf(tf[i]);
        cumulative += 0.5 * (a0 + a1) * dx;

        cdf.points.push_back(CdfPoint{x1, cumulative, a1, vec3{tf[i].color}});
    }

    cdf.totalMass = cumulative;
    return cdf;
}

// Find the leftmost position where the CDF begins to increase (start of support).
double supportMin(const Cdf& cdf) {
    if (cdf.points.empty()) return 0.0;
    for (std::size_t i = 1; i < cdf.points.size(); ++i) {
        if (cdf.points[i].mass - cdf.points[i - 1].mass > eps) {
            return cdf.points[i - 1].pos;
        }
    }
    return cdf.points.front().pos;
}

// Find the rightmost position where the CDF stops increasing (end of support).
double supportMax(const Cdf& cdf) {
    if (cdf.points.empty()) return 0.0;
    for (std::size_t i = cdf.points.size(); i-- > 1;) {
        if (cdf.points[i].mass - cdf.points[i - 1].mass > eps) {
            return cdf.points[i].pos;
        }
    }
    return cdf.points.back().pos;
}

// Compute normalized quantile [0,1] at a CDF point.
double quantileAtPoint(const Cdf& cdf, const CdfPoint& p) {
    if (cdf.totalMass <= eps) return 0.0;
    return std::clamp(p.mass / cdf.totalMass, 0.0, 1.0);
}

// Solve for offset u within a linear-alpha segment [0, dx] where the integral equals localMass.
// The segment has alpha(x) = a0 + (a1-a0)/dx * x, and we solve:
//   a0*u + 0.5*((a1-a0)/dx)*u^2 = localMass
double solveSegmentInverse(double a0, double a1, double dx, double localMass) {
    if (localMass <= eps) return 0.0;

    const double slope = (a1 - a0) / dx;

    if (std::abs(slope) < eps) {
        // Constant alpha: linear inverse.
        if (a0 <= eps) return 0.0;
        return std::clamp(localMass / a0, 0.0, dx);
    }

    // Quadratic formula: 0.5*slope*u^2 + a0*u - localMass = 0
    const double A = 0.5 * slope;
    const double B = a0;
    const double C = -localMass;
    const double discriminant = std::max(0.0, B * B - 4.0 * A * C);
    const double sqrtD = std::sqrt(discriminant);
    const double denom = 2.0 * A;

    const double r0 = (-B + sqrtD) / denom;
    const double r1 = (-B - sqrtD) / denom;

    const auto valid = [dx](double u) { return u >= -eps && u <= dx + eps; };
    if (valid(r0)) return std::clamp(r0, 0.0, dx);
    if (valid(r1)) return std::clamp(r1, 0.0, dx);
    return std::clamp(r0, 0.0, dx);
}

// Invert the CDF: find position x such that CDF(x) / totalMass = q.
double invertCdf(const Cdf& cdf, double q) {
    if (cdf.points.empty()) return 0.0;
    if (cdf.totalMass <= eps) return cdf.points.front().pos;

    q = std::clamp(q, 0.0, 1.0);
    if (q <= 0.0) return supportMin(cdf);
    if (q >= 1.0) return supportMax(cdf);

    const double targetMass = q * cdf.totalMass;

    auto it = std::lower_bound(cdf.points.begin(), cdf.points.end(), targetMass,
                               [](const CdfPoint& p, double mass) { return p.mass < mass; });

    if (it == cdf.points.begin()) return cdf.points.front().pos;
    if (it == cdf.points.end()) return supportMax(cdf);

    const auto& p1 = *it;
    const auto& p0 = *(it - 1);

    const double segmentMass = p1.mass - p0.mass;
    if (segmentMass <= eps) return p0.pos;

    const double dx = p1.pos - p0.pos;
    if (dx <= eps) return p0.pos;

    const double localMass = targetMass - p0.mass;
    return p0.pos + solveSegmentInverse(p0.alpha, p1.alpha, dx, localMass);
}

// ---------------------------------------------------------------------------
// Quantile sampling
// ---------------------------------------------------------------------------

// Add uniformly-spaced quantile levels within each CDF segment.
void addQuantileLevels(const Cdf& cdf, std::size_t samplesPerSegment, std::vector<double>& levels) {
    if (cdf.points.empty() || cdf.totalMass <= eps) {
        levels.push_back(0.0);
        levels.push_back(1.0);
        return;
    }

    samplesPerSegment = std::max<std::size_t>(1, samplesPerSegment);

    for (std::size_t i = 1; i < cdf.points.size(); ++i) {
        const double q0 = quantileAtPoint(cdf, cdf.points[i - 1]);
        const double q1 = quantileAtPoint(cdf, cdf.points[i]);

        levels.push_back(q0);
        if (q1 - q0 > eps) {
            for (std::size_t s = 1; s < samplesPerSegment; ++s) {
                const double u = static_cast<double>(s) / static_cast<double>(samplesPerSegment);
                levels.push_back((1.0 - u) * q0 + u * q1);
            }
        }
        levels.push_back(q1);
    }

    levels.push_back(0.0);
    levels.push_back(1.0);
}

// Build a merged set of quantile levels from both CDFs, ensuring both distributions
// are adequately sampled. Contains sub-samples within each CDF segment of both TFs.
std::vector<double> mergedQuantileLevels(const Cdf& a, const Cdf& b,
                                         std::size_t samplesPerSegment) {
    std::vector<double> levels;
    levels.reserve((a.points.size() + b.points.size()) * (samplesPerSegment + 1) + 2);

    addQuantileLevels(a, samplesPerSegment, levels);
    addQuantileLevels(b, samplesPerSegment, levels);

    std::sort(levels.begin(), levels.end());
    levels.erase(std::unique(levels.begin(), levels.end(),
                             [](double x, double y) { return std::abs(x - y) < eps; }),
                 levels.end());

    if (levels.empty() || levels.front() > 0.0) levels.insert(levels.begin(), 0.0);
    if (levels.back() < 1.0) levels.push_back(1.0);
    return levels;
}

// Evaluate the quantile function: invert the CDF and sample the TF color at that position.
QuantilePoint quantilePoint(std::span<const TFPrimitiveData> tf, const Cdf& cdf, double q) {
    const double x = invertCdf(cdf, q);
    const vec4 color = evaluate(tf, x);
    return QuantilePoint{x, q, vec3{color}};
}

// ---------------------------------------------------------------------------
// Structural vertex insertion
// ---------------------------------------------------------------------------

// For each knot of a source TF, find its quantile in the source CDF and compute the
// corresponding interpolated position. This ensures that structurally important positions
// (peak starts, peaks, peak ends) are always represented as output vertices, regardless of
// how uniform quantile sampling distributes points. Without this, peaks with steep edges
// can have sparse vertices at their boundaries, causing reconstruction errors.
void addStructuralVertices(std::span<const TFPrimitiveData> aspan,
                           std::span<const TFPrimitiveData> bspan, const Cdf& cdfA, const Cdf& cdfB,
                           double t, std::vector<QuantilePoint>& interpolatedCdf) {

    auto addKnotsFrom = [&](std::span<const TFPrimitiveData> span, const Cdf& cdf) {
        for (const auto& p : span) {
            // Find the quantile of this knot by interpolating within its CDF segment.
            double q = 0.0;
            bool found = false;
            for (std::size_t si = 0; si + 1 < cdf.points.size(); ++si) {
                if (p.pos >= cdf.points[si].pos - eps && p.pos <= cdf.points[si + 1].pos + eps) {
                    const double dx = cdf.points[si + 1].pos - cdf.points[si].pos;
                    const double q0 = cdf.points[si].mass / cdf.totalMass;
                    const double q1 = cdf.points[si + 1].mass / cdf.totalMass;
                    if (dx > eps) {
                        const double frac = std::clamp((p.pos - cdf.points[si].pos) / dx, 0.0, 1.0);
                        q = q0 + frac * (q1 - q0);
                    } else {
                        q = q1;
                    }
                    found = true;
                    break;
                }
            }
            if (!found || q <= 0.0 || q >= 1.0) continue;

            // Compute the OT-interpolated position for this quantile.
            const auto qa = quantilePoint(aspan, cdfA, q);
            const auto qb = quantilePoint(bspan, cdfB, q);
            const double x = (1.0 - t) * qa.pos + t * qb.pos;
            const vec3 color = glm::mix(qa.color, qb.color, static_cast<float>(t));
            interpolatedCdf.push_back(QuantilePoint{x, q, color});
        }
    };

    addKnotsFrom(aspan, cdfA);
    addKnotsFrom(bspan, cdfB);
}

// ---------------------------------------------------------------------------
// Output TF construction helper
// ---------------------------------------------------------------------------

// Append a zero-alpha boundary point with interpolated color.
void appendZeroAlphaBoundary(std::vector<TFPrimitiveData>& result,
                             std::span<const TFPrimitiveData> a, std::span<const TFPrimitiveData> b,
                             double x, double t) {
    const vec3 color = glm::mix(vec3{evaluate(a, x)}, vec3{evaluate(b, x)}, static_cast<float>(t));
    result.push_back({x, vec4{color, 0.0f}});
}

}  // namespace

// ===========================================================================
// Public API
// ===========================================================================

std::vector<TFPrimitiveData> optimalTransportInterpolation(std::span<const TFPrimitiveData> tfA,
                                                           std::span<const TFPrimitiveData> tfB,
                                                           double t,
                                                           std::size_t samplesPerSegment) {
    t = std::clamp(t, 0.0, 1.0);

    auto a = sanitize(tfA);
    auto b = sanitize(tfB);

    const std::span<const TFPrimitiveData> aspan{a.data(), a.size()};
    const std::span<const TFPrimitiveData> bspan{b.data(), b.size()};

    if (a.empty()) return b;
    if (b.empty()) return a;
    if (t <= 0.0) return a;
    if (t >= 1.0) return b;

    // --- Step 1: Build CDFs from alpha channels ---
    const Cdf cdfA = computeCdf(aspan);
    const Cdf cdfB = computeCdf(bspan);

    const double massA = cdfA.totalMass;
    const double massB = cdfB.totalMass;
    const double targetMass = (1.0 - t) * massA + t * massB;

    // OT requires normalized distributions. Fall back to pointwise blend if either
    // TF has zero alpha mass.
    if (massA <= eps || massB <= eps || targetMass <= eps) {
        return linearBlend(aspan, bspan, t);
    }

    // --- Step 2: Sample the interpolated quantile function ---
    // For each quantile level q in [0,1], compute the displacement-interpolated position:
    //   x_t(q) = (1-t) * Q_A(q) + t * Q_B(q)
    // where Q_A, Q_B are the quantile functions (inverse CDFs) of A and B.
    const auto levels = mergedQuantileLevels(cdfA, cdfB, samplesPerSegment);
    if (levels.size() < 2) return linearBlend(aspan, bspan, t);

    std::vector<QuantilePoint> interpolatedCdf;
    interpolatedCdf.reserve(levels.size() + aspan.size() + bspan.size());

    for (double q : levels) {
        const auto qa = quantilePoint(aspan, cdfA, q);
        const auto qb = quantilePoint(bspan, cdfB, q);
        const double x = (1.0 - t) * qa.pos + t * qb.pos;
        const vec3 color = glm::mix(qa.color, qb.color, static_cast<float>(t));
        interpolatedCdf.push_back(QuantilePoint{x, q, color});
    }

    // --- Step 3: Insert structural vertices ---
    // Add vertices at the interpolated positions of the original TF knots. This ensures
    // that key structural positions (peak boundaries, peak apexes) are always represented,
    // preventing reconstruction artifacts at steep transitions where quantile sampling
    // would otherwise produce sparse vertices.
    addStructuralVertices(aspan, bspan, cdfA, cdfB, t, interpolatedCdf);

    // Sort by quantile (monotonically related to position for valid transport maps).
    std::sort(interpolatedCdf.begin(), interpolatedCdf.end(),
              [](const QuantilePoint& lhs, const QuantilePoint& rhs) { return lhs.q < rhs.q; });

    // --- Step 4: Deduplicate ---
    // Remove vertices that collapse to the same position (zero-width intervals).
    std::vector<QuantilePoint> vertices;
    vertices.reserve(interpolatedCdf.size());
    for (const auto& p : interpolatedCdf) {
        if (!vertices.empty() && std::abs(p.pos - vertices.back().pos) <= eps) {
            vertices.back() = p;  // Keep the higher-quantile vertex.
        } else {
            vertices.push_back(p);
        }
    }

    const std::size_t n = vertices.size();
    if (n < 2) return linearBlend(aspan, bspan, t);

    // --- Step 5: Reconstruct piecewise-linear alpha ---
    //
    // The interpolated quantile function defines a piecewise-linear CDF. Between consecutive
    // vertices (x_i, q_i) and (x_{i+1}, q_{i+1}), the density is constant:
    //
    //     d_i = targetMass * (q_{i+1} - q_i) / (x_{i+1} - x_i)
    //
    // To obtain smooth vertex alpha values, we assign each interior vertex the width-weighted
    // average of its adjacent interval densities:
    //
    //     alpha_i = (d_{i-1} * dx_{i-1} + d_i * dx_i) / (dx_{i-1} + dx_i)
    //
    // This produces a smooth (non-oscillating) result. A per-component mass rescaling step
    // afterward corrects any mass discrepancy introduced by the approximation.

    // Compute piecewise-constant interval densities.
    std::vector<double> density(n - 1, 0.0);
    for (std::size_t i = 0; i + 1 < n; ++i) {
        const double dx = vertices[i + 1].pos - vertices[i].pos;
        const double dq = vertices[i + 1].q - vertices[i].q;
        if (dx > eps && dq > eps) {
            density[i] = targetMass * dq / dx;
        }
    }

    // Identify gaps: intervals with negligible density separate disjoint peaks.
    double maxDensity = 0.0;
    for (const auto& d : density) {
        maxDensity = std::max(maxDensity, d);
    }
    const double gapThreshold = maxDensity * 1e-6;

    std::vector<double> alpha(n, 0.0);

    // Check whether original TFs have zero alpha at the global support boundaries.
    // This determines boundary conditions for the outermost components.
    const double alphaAtStartA = static_cast<double>(evaluate(aspan, supportMin(cdfA)).a);
    const double alphaAtStartB = static_cast<double>(evaluate(bspan, supportMin(cdfB)).a);
    const bool zeroAtStart = (alphaAtStartA <= eps) || (alphaAtStartB <= eps);

    const double alphaAtEndA = static_cast<double>(evaluate(aspan, supportMax(cdfA)).a);
    const double alphaAtEndB = static_cast<double>(evaluate(bspan, supportMax(cdfB)).a);
    const bool zeroAtEnd = (alphaAtEndA <= eps) || (alphaAtEndB <= eps);

    // Process each connected component (contiguous run of non-gap intervals) independently.
    std::size_t i = 0;
    while (i < n - 1) {
        if (density[i] <= gapThreshold) {
            ++i;
            continue;
        }

        // Find the extent of this component.
        const std::size_t compStart = i;
        while (i < n - 1 && density[i] > gapThreshold) {
            ++i;
        }
        const std::size_t compEnd = i;
        const std::size_t compLen = compEnd - compStart + 1;

        // Boundary alpha: zero at gap edges, or the local density at global TF boundaries.
        const double bc0 = (compStart == 0 && !zeroAtStart) ? density[compStart] : 0.0;
        const double bcN = (compEnd == n - 1 && !zeroAtEnd) ? density[compEnd - 1] : 0.0;

        if (compLen == 2) {
            // Single interval: assign interval density to both endpoints.
            const double dx = vertices[compStart + 1].pos - vertices[compStart].pos;
            const double dq = vertices[compStart + 1].q - vertices[compStart].q;
            if (dx > eps) {
                const double d = targetMass * dq / dx;
                alpha[compStart] = std::max(bc0, d);
                alpha[compEnd] = std::max(bcN, d);
            }
        } else {
            // Multi-interval component: width-weighted average of adjacent densities.
            for (std::size_t j = 0; j < compLen; ++j) {
                const std::size_t idx = compStart + j;
                const bool hasLeft = (j > 0);
                const bool hasRight = (j + 1 < compLen);

                if (hasLeft && hasRight) {
                    const double dxL = vertices[idx].pos - vertices[idx - 1].pos;
                    const double dxR = vertices[idx + 1].pos - vertices[idx].pos;
                    const double dL = density[idx - 1];
                    const double dR = density[idx];
                    const double totalDx = dxL + dxR;
                    if (totalDx > eps) {
                        alpha[idx] = (dL * dxL + dR * dxR) / totalDx;
                    }
                } else if (hasRight) {
                    alpha[idx] = bc0;  // Left boundary of component.
                } else {
                    alpha[idx] = bcN;  // Right boundary of component.
                }
            }
        }

        // Clamp negative values from numerical noise.
        for (std::size_t j = compStart; j <= compEnd; ++j) {
            alpha[j] = std::max(0.0, alpha[j]);
        }

        // Rescale this component so its piecewise-linear integral matches the target mass.
        double compTargetMass = 0.0;
        double compActualMass = 0.0;
        for (std::size_t j = compStart; j < compEnd; ++j) {
            const double dx = vertices[j + 1].pos - vertices[j].pos;
            if (dx <= eps) continue;
            const double dq = vertices[j + 1].q - vertices[j].q;
            compTargetMass += targetMass * dq;
            compActualMass += 0.5 * (alpha[j] + alpha[j + 1]) * dx;
        }
        if (compActualMass > eps) {
            const double scale = compTargetMass / compActualMass;
            for (std::size_t j = compStart; j <= compEnd; ++j) {
                alpha[j] *= scale;
            }
        }
    }

    // Final safety clamp.
    for (auto& a : alpha) {
        a = std::max(0.0, a);
    }

    // --- Step 6: Build the output TF ---
    const double domainMin = (1.0 - t) * a.front().pos + t * b.front().pos;
    const double domainMax = (1.0 - t) * a.back().pos + t * b.back().pos;

    std::vector<TFPrimitiveData> result;
    result.reserve(n + 2);

    // Add zero-alpha point at domain start if the first vertex is interior.
    if (vertices.front().pos - domainMin > eps) {
        appendZeroAlphaBoundary(result, aspan, bspan, domainMin, t);
    }

    for (std::size_t idx = 0; idx < n; ++idx) {
        const auto alphaVal = static_cast<float>(alpha[idx]);
        result.push_back({vertices[idx].pos, vec4{vertices[idx].color, alphaVal}});
    }

    // Add zero-alpha point at domain end if the last vertex is interior.
    if (domainMax - vertices.back().pos > eps) {
        appendZeroAlphaBoundary(result, aspan, bspan, domainMax, t);
    }

    if (result.empty()) {
        return linearBlend(aspan, bspan, t);
    }

    return result;
}

double earthMoversDistance(std::span<const TFPrimitiveData> tfA,
                           std::span<const TFPrimitiveData> tfB, std::size_t samplesPerSegment) {
    auto a = sanitize(tfA);
    auto b = sanitize(tfB);

    const std::span<const TFPrimitiveData> aspan{a.data(), a.size()};
    const std::span<const TFPrimitiveData> bspan{b.data(), b.size()};

    if (a.empty() || b.empty()) return 0.0;

    const Cdf cdfA = computeCdf(aspan);
    const Cdf cdfB = computeCdf(bspan);

    if (cdfA.totalMass <= eps || cdfB.totalMass <= eps) return 0.0;

    // EMD in 1D equals the L1 distance between quantile functions:
    //   EMD = integral_0^1 |Q_A(q) - Q_B(q)| dq
    // Approximated via trapezoidal quadrature over the merged quantile levels.
    const auto levels = mergedQuantileLevels(cdfA, cdfB, samplesPerSegment);
    if (levels.size() < 2) return 0.0;

    double distance = 0.0;
    double prevQ = levels.front();
    double prevDiff = std::abs(invertCdf(cdfA, prevQ) - invertCdf(cdfB, prevQ));

    for (std::size_t i = 1; i < levels.size(); ++i) {
        const double q = levels[i];
        const double diff = std::abs(invertCdf(cdfA, q) - invertCdf(cdfB, q));
        const double dq = q - prevQ;
        distance += 0.5 * (prevDiff + diff) * dq;
        prevQ = q;
        prevDiff = diff;
    }

    return distance;
}

}  // namespace inviwo::algorithm
