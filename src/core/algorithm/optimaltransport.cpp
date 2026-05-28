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

// Knot-induced quantile levels: sorted union of normalized CDF values F(x)/m at
// every input knot of A and B. Equivalent to mergedQuantileLevels(..., 1).
std::vector<double> knotInducedQuantileLevels(const Cdf& a, const Cdf& b) {
    return mergedQuantileLevels(a, b, 1);
}

void insertQuantileLevel(std::vector<double>& levels, double q) {
    q = std::clamp(q, 0.0, 1.0);
    auto it = std::lower_bound(levels.begin(), levels.end(), q);
    if (it != levels.end() && std::abs(*it - q) <= eps) return;
    levels.insert(it, q);
}

// ---------------------------------------------------------------------------
// Shared OT interpolation context and pipeline stages
// ---------------------------------------------------------------------------

struct TransportContext {
    std::span<const TFPrimitiveData> aspan;
    std::span<const TFPrimitiveData> bspan;
    const Cdf& cdfA;
    const Cdf& cdfB;
    double t = 0.0;
    double targetMass = 0.0;
};

QuantilePoint transportedQuantilePoint(const TransportContext& ctx, double q) {
    const auto qa = quantilePoint(ctx.aspan, ctx.cdfA, q);
    const auto qb = quantilePoint(ctx.bspan, ctx.cdfB, q);
    const double x = (1.0 - ctx.t) * qa.pos + ctx.t * qb.pos;
    const vec3 color = glm::mix(qa.color, qb.color, static_cast<float>(ctx.t));
    return QuantilePoint{x, q, color};
}

// ---------------------------------------------------------------------------
// Structural vertex insertion
// ---------------------------------------------------------------------------

// For each knot of a source TF, find its quantile in the source CDF and compute the
// corresponding interpolated position. This ensures that structurally important positions
// (peak starts, peaks, peak ends) are always represented as output vertices, regardless of
// how uniform quantile sampling distributes points. Without this, peaks with steep edges
// can have sparse vertices at their boundaries, causing reconstruction errors.
//
// Plateau handling: a run of consecutive knots with zero alpha (a transport gap) all share
// the same quantile q, so the quantile function Q is multi-valued there. invertCdf
// resolves the tie to the left edge, which would collapse the right edges of every
// zero-alpha plateau into duplicates and silently drop them. We avoid that by (a) using
// the knot's own position directly on the side it came from, and (b) pairing it with the
// matching knot on the other side by plateau index: the k-th knot of A at quantile q is
// paired with the k-th knot of B at the same q (if any), falling back to invertCdf
// otherwise. With this pairing tfA == tfB reproduces the input exactly.
void addStructuralVertices(std::span<const TFPrimitiveData> aspan,
                           std::span<const TFPrimitiveData> bspan, const Cdf& cdfA, const Cdf& cdfB,
                           double t, std::vector<QuantilePoint>& interpolatedCdf) {

    // Returns the position of the plateauIndex-th CDF point of `other` whose normalized
    // mass equals q (within eps). Returns NaN if no such point exists, signalling a
    // fall-back to invertCdf.
    const auto matchPlateauKnot = [](const Cdf& other, double q,
                                     std::size_t plateauIndex) -> double {
        if (other.totalMass <= eps) return std::numeric_limits<double>::quiet_NaN();
        std::size_t count = 0;
        for (const auto& pt : other.points) {
            const double pq = pt.mass / other.totalMass;
            if (std::abs(pq - q) <= eps) {
                if (count == plateauIndex) return pt.pos;
                ++count;
            }
        }
        return std::numeric_limits<double>::quiet_NaN();
    };

    const auto addKnotsFrom = [&](std::span<const TFPrimitiveData> selfSpan, const Cdf& selfCdf,
                                  std::span<const TFPrimitiveData> otherSpan, const Cdf& otherCdf,
                                  double weightSelf, double weightOther) {
        if (selfCdf.totalMass <= eps) return;
        // Knot k of selfSpan corresponds to selfCdf.points[k] when both have been sanitized
        // (computeCdf preserves the knot ordering modulo zero-width duplicates that
        // sanitize already removed).
        const std::size_t n = std::min(selfSpan.size(), selfCdf.points.size());
        // Plateau-index counter keyed by the quantile of the previous knot, so duplicates
        // in q get successive indices 0, 1, 2, ...
        double prevQ = std::numeric_limits<double>::quiet_NaN();
        std::size_t plateauIndex = 0;
        for (std::size_t k = 0; k < n; ++k) {
            const double q = selfCdf.points[k].mass / selfCdf.totalMass;
            if (!std::isnan(prevQ) && std::abs(q - prevQ) <= eps) {
                ++plateauIndex;
            } else {
                plateauIndex = 0;
            }
            prevQ = q;
            if (q <= 0.0 || q >= 1.0) continue;

            const double xSelf = selfSpan[k].pos;
            double xOther = matchPlateauKnot(otherCdf, q, plateauIndex);
            if (std::isnan(xOther)) xOther = invertCdf(otherCdf, q);

            const double x = weightSelf * xSelf + weightOther * xOther;
            const vec3 color =
                glm::mix(vec3{evaluate(selfSpan, xSelf)}, vec3{evaluate(otherSpan, xOther)},
                         static_cast<float>(weightOther));
            interpolatedCdf.push_back(QuantilePoint{x, q, color});
        }
    };

    addKnotsFrom(aspan, cdfA, bspan, cdfB, 1.0 - t, t);
    addKnotsFrom(bspan, cdfB, aspan, cdfA, t, 1.0 - t);
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

// Sample X_t(q) at every level in L, add structural knot vertices, sort and deduplicate in x.
std::vector<QuantilePoint> buildInterpolatedVertices(const TransportContext& ctx,
                                                     const std::vector<double>& levels) {
    std::vector<QuantilePoint> interpolatedCdf;
    interpolatedCdf.reserve(levels.size() + ctx.aspan.size() + ctx.bspan.size());

    for (double q : levels) {
        interpolatedCdf.push_back(transportedQuantilePoint(ctx, q));
    }

    addStructuralVertices(ctx.aspan, ctx.bspan, ctx.cdfA, ctx.cdfB, ctx.t, interpolatedCdf);

    std::sort(interpolatedCdf.begin(), interpolatedCdf.end(),
              [](const QuantilePoint& lhs, const QuantilePoint& rhs) { return lhs.q < rhs.q; });

    std::vector<QuantilePoint> vertices;
    vertices.reserve(interpolatedCdf.size());
    for (const auto& p : interpolatedCdf) {
        if (!vertices.empty() && std::abs(p.pos - vertices.back().pos) <= eps) {
            vertices.back() = p;
        } else {
            vertices.push_back(p);
        }
    }
    return vertices;
}

struct AlphaReconstruction {
    std::vector<double> density;
    std::vector<double> alpha;
    double gapThreshold = 0.0;
};

// Reconstruct piecewise-linear alpha from quantile vertices via secant interval
// densities d_k = m_t * Delta q / Delta x and width-weighted vertex averaging.
AlphaReconstruction reconstructAlpha(const TransportContext& ctx,
                                     const std::vector<QuantilePoint>& vertices) {
    AlphaReconstruction result;
    const std::size_t n = vertices.size();
    if (n < 2) return result;

    result.density.assign(n - 1, 0.0);
    for (std::size_t i = 0; i + 1 < n; ++i) {
        const double dx = vertices[i + 1].pos - vertices[i].pos;
        const double dq = vertices[i + 1].q - vertices[i].q;
        if (dx > eps && dq > eps) {
            result.density[i] = ctx.targetMass * dq / dx;
        }
    }

    double maxDensity = 0.0;
    for (const auto& d : result.density) {
        maxDensity = std::max(maxDensity, d);
    }
    result.gapThreshold = maxDensity * 1e-6;

    result.alpha.assign(n, 0.0);

    const double alphaAtStartA = static_cast<double>(evaluate(ctx.aspan, supportMin(ctx.cdfA)).a);
    const double alphaAtStartB = static_cast<double>(evaluate(ctx.bspan, supportMin(ctx.cdfB)).a);
    const bool zeroAtStart = (alphaAtStartA <= eps) || (alphaAtStartB <= eps);

    const double alphaAtEndA = static_cast<double>(evaluate(ctx.aspan, supportMax(ctx.cdfA)).a);
    const double alphaAtEndB = static_cast<double>(evaluate(ctx.bspan, supportMax(ctx.cdfB)).a);
    const bool zeroAtEnd = (alphaAtEndA <= eps) || (alphaAtEndB <= eps);

    std::size_t i = 0;
    while (i < n - 1) {
        if (result.density[i] <= result.gapThreshold) {
            ++i;
            continue;
        }

        const std::size_t compStart = i;
        while (i < n - 1 && result.density[i] > result.gapThreshold) {
            ++i;
        }
        const std::size_t compEnd = i;
        const std::size_t compLen = compEnd - compStart + 1;

        const double bc0 = (compStart == 0 && !zeroAtStart) ? result.density[compStart] : 0.0;
        const double bcN = (compEnd == n - 1 && !zeroAtEnd) ? result.density[compEnd - 1] : 0.0;

        if (compLen == 2) {
            const double dx = vertices[compStart + 1].pos - vertices[compStart].pos;
            const double dq = vertices[compStart + 1].q - vertices[compStart].q;
            if (dx > eps) {
                const double d = ctx.targetMass * dq / dx;
                result.alpha[compStart] = std::max(bc0, d);
                result.alpha[compEnd] = std::max(bcN, d);
            }
        } else {
            for (std::size_t j = 0; j < compLen; ++j) {
                const std::size_t idx = compStart + j;
                const bool hasLeft = (j > 0);
                const bool hasRight = (j + 1 < compLen);

                if (hasLeft && hasRight) {
                    const double dxL = vertices[idx].pos - vertices[idx - 1].pos;
                    const double dxR = vertices[idx + 1].pos - vertices[idx].pos;
                    const double dL = result.density[idx - 1];
                    const double dR = result.density[idx];
                    const double totalDx = dxL + dxR;
                    if (totalDx > eps) {
                        result.alpha[idx] = (dL * dxL + dR * dxR) / totalDx;
                    }
                } else if (hasRight) {
                    result.alpha[idx] = bc0;
                } else {
                    result.alpha[idx] = bcN;
                }
            }
        }

        for (std::size_t j = compStart; j <= compEnd; ++j) {
            result.alpha[j] = std::max(0.0, result.alpha[j]);
        }

        double compTargetMass = 0.0;
        double compActualMass = 0.0;
        for (std::size_t j = compStart; j < compEnd; ++j) {
            const double dx = vertices[j + 1].pos - vertices[j].pos;
            if (dx <= eps) continue;
            const double dq = vertices[j + 1].q - vertices[j].q;
            compTargetMass += ctx.targetMass * dq;
            compActualMass += 0.5 * (result.alpha[j] + result.alpha[j + 1]) * dx;
        }
        if (compActualMass > eps) {
            const double scale = compTargetMass / compActualMass;
            for (std::size_t j = compStart; j <= compEnd; ++j) {
                result.alpha[j] *= scale;
            }
        }
    }

    for (auto& aVal : result.alpha) {
        aVal = std::max(0.0, aVal);
    }
    return result;
}

std::vector<TFPrimitiveData> buildOutputTransferFunction(const TransportContext& ctx,
                                                         const std::vector<QuantilePoint>& vertices,
                                                         const AlphaReconstruction& reconstruction,
                                                         double domainMin, double domainMax) {
    const std::size_t n = vertices.size();
    std::vector<TFPrimitiveData> result;
    result.reserve(n + 2);

    if (vertices.front().pos - domainMin > eps) {
        appendZeroAlphaBoundary(result, ctx.aspan, ctx.bspan, domainMin, ctx.t);
    }

    for (std::size_t idx = 0; idx < n; ++idx) {
        const auto alphaVal = static_cast<float>(reconstruction.alpha[idx]);
        result.push_back({vertices[idx].pos, vec4{vertices[idx].color, alphaVal}});
    }

    if (domainMax - vertices.back().pos > eps) {
        appendZeroAlphaBoundary(result, ctx.aspan, ctx.bspan, domainMax, ctx.t);
    }
    return result;
}

// Shared pipeline: sample quantile levels, reconstruct alpha, emit output TF.
std::vector<TFPrimitiveData> optimalTransportInterpolationFromLevels(
    const TransportContext& ctx, const std::vector<double>& levels, double domainMin,
    double domainMax) {
    if (levels.size() < 2) {
        return linearBlend(ctx.aspan, ctx.bspan, ctx.t);
    }

    const auto vertices = buildInterpolatedVertices(ctx, levels);
    if (vertices.size() < 2) {
        return linearBlend(ctx.aspan, ctx.bspan, ctx.t);
    }

    const auto reconstruction = reconstructAlpha(ctx, vertices);
    auto result = buildOutputTransferFunction(ctx, vertices, reconstruction, domainMin, domainMax);
    if (result.empty()) {
        return linearBlend(ctx.aspan, ctx.bspan, ctx.t);
    }
    return result;
}

TransportContext makeTransportContext(std::span<const TFPrimitiveData> aspan,
                                      std::span<const TFPrimitiveData> bspan, const Cdf& cdfA,
                                      const Cdf& cdfB, double t) {
    const double massA = cdfA.totalMass;
    const double massB = cdfB.totalMass;
    return TransportContext{aspan, bspan, cdfA, cdfB, t, (1.0 - t) * massA + t * massB};
}

// ---------------------------------------------------------------------------
// Closed-form inversion of X_t on a single quantile sub-interval
// ---------------------------------------------------------------------------
//
// On each sub-interval between consecutive quantile levels, the source quantile
// Q_A(q) lies entirely on one piecewise-linear segment of A and Q_B(q) on one
// segment of B. CdfSegment caches the segment data needed to evaluate Q_X(q),
// alpha_X(Q_X(q)), X_t'(q), and to invert X_t(q) = x (a quadratic when both
// segment slopes are non-zero, otherwise linear or linear-plus-square-root).

struct CdfSegment {
    double xStart = 0.0;
    double xEnd = 0.0;
    double qStart = 0.0;  // F(xStart) / m
    double qEnd = 0.0;    // F(xEnd)   / m
    double alphaStart = 0.0;
    double alphaEnd = 0.0;
    double slope = 0.0;       // s = (alphaEnd - alphaStart) / (xEnd - xStart)
    double mass = 0.0;        // total CDF mass m
    double cdfAtStart = 0.0;  // F(xStart), un-normalized
};

CdfSegment makeSegment(const Cdf& cdf, std::size_t i) {
    const auto& p0 = cdf.points[i];
    const auto& p1 = cdf.points[i + 1];
    const double dx = p1.pos - p0.pos;
    const double m = std::max(cdf.totalMass, eps);
    const double slope = (dx > eps) ? (p1.alpha - p0.alpha) / dx : 0.0;
    return CdfSegment{p0.pos,   p1.pos, p0.mass / m, p1.mass / m, p0.alpha,
                      p1.alpha, slope,  m,           p0.mass};
}

// alpha_X(Q_X(q)) = sqrt(alpha_i^2 + 2 s (q m - F_i)) (with the s = 0 limit handled).
double alphaAtQuantile(const CdfSegment& s, double q) {
    if (std::abs(s.slope) <= eps) return s.alphaStart;
    const double P = s.alphaStart * s.alphaStart + 2.0 * s.slope * (q * s.mass - s.cdfAtStart);
    return std::sqrt(std::max(0.0, P));
}

double quantileOnSegment(const CdfSegment& s, double q) {
    if (std::abs(s.slope) <= eps) {
        if (s.alphaStart <= eps) return s.xStart;
        return s.xStart + (q * s.mass - s.cdfAtStart) / s.alphaStart;
    }
    return s.xStart + (alphaAtQuantile(s, q) - s.alphaStart) / s.slope;
}

double xtOnSegment(const CdfSegment& segA, const CdfSegment& segB, double t, double q) {
    return (1.0 - t) * quantileOnSegment(segA, q) + t * quantileOnSegment(segB, q);
}

// Exact transported opacity alpha_t(X_t(q)) = m_t / X_t'(q), with
// X_t'(q) = (1-t) m_A / alpha_A(Q_A(q)) + t m_B / alpha_B(Q_B(q)).
double densityAtQuantile(const CdfSegment& segA, const CdfSegment& segB, double t, double q,
                         double targetMass) {
    const double aA = alphaAtQuantile(segA, q);
    const double aB = alphaAtQuantile(segB, q);
    if (aA <= eps || aB <= eps) return 0.0;
    const double xtPrime = (1.0 - t) * segA.mass / aA + t * segB.mass / aB;
    if (xtPrime <= eps) return 0.0;
    return targetMass / xtPrime;
}

// Solve a*q^2 + b*q + c = 0 numerically robustly and return both roots (NaN if none).
struct QuadraticRoots {
    double r0 = std::numeric_limits<double>::quiet_NaN();
    double r1 = std::numeric_limits<double>::quiet_NaN();
    bool linear = false;
};

QuadraticRoots solveQuadratic(double a, double b, double c) {
    QuadraticRoots out;
    const double scale = std::max({std::abs(a), std::abs(b), std::abs(c), 1.0});
    if (std::abs(a) <= 1e-14 * scale) {
        out.linear = true;
        if (std::abs(b) > eps) out.r0 = -c / b;
        return out;
    }
    const double disc = b * b - 4.0 * a * c;
    if (disc < -1e-12 * scale * scale) return out;
    const double sqrtDisc = std::sqrt(std::max(0.0, disc));
    // Use the form that avoids subtraction of nearly equal quantities.
    const double q = -0.5 * (b + std::copysign(sqrtDisc, b));
    if (std::abs(q) > eps) {
        out.r0 = q / a;
        out.r1 = c / q;
    } else {
        out.r0 = (-b + sqrtDisc) / (2.0 * a);
        out.r1 = (-b - sqrtDisc) / (2.0 * a);
    }
    return out;
}

// Solve X_t(q) = x on a sub-interval where Q_A lies on segA and Q_B on segB.
// When both segment slopes are non-zero, substituting the segment-local quantile
// inverses yields a quadratic in q; a zero slope reduces the problem to a
// linear or linear-plus-square-root equation. Returns NaN if no root lies in
// [qLo, qHi].
double invertXtOnSegment(const CdfSegment& segA, const CdfSegment& segB, double t, double x) {
    const double oneMt = 1.0 - t;
    const double qLo = std::max(segA.qStart, segB.qStart);
    const double qHi = std::min(segA.qEnd, segB.qEnd);
    const double qTol = 1e-9 + 1e-9 * std::max(1.0, std::abs(qHi - qLo));

    const auto clampToRange = [qLo, qHi](double q) { return std::clamp(q, qLo, qHi); };

    const auto pickBranch = [&](double r0, double r1, const auto& residual) {
        double best = std::numeric_limits<double>::quiet_NaN();
        double bestRes = std::numeric_limits<double>::infinity();
        for (double r : {r0, r1}) {
            if (std::isnan(r)) continue;
            if (r < qLo - qTol || r > qHi + qTol) continue;
            const double res = std::abs(residual(r));
            if (res < bestRes) {
                bestRes = res;
                best = r;
            }
        }
        return std::isnan(best) ? best : clampToRange(best);
    };

    const bool sAZero = std::abs(segA.slope) <= eps;
    const bool sBZero = std::abs(segB.slope) <= eps;

    // Both segments flat: X_t is linear in q on this sub-interval.
    if (sAZero && sBZero) {
        const double aA = segA.alphaStart;
        const double aB = segB.alphaStart;
        if (aA <= eps || aB <= eps) return std::numeric_limits<double>::quiet_NaN();
        const double c =
            oneMt * (segA.xStart - segA.cdfAtStart / aA) + t * (segB.xStart - segB.cdfAtStart / aB);
        const double slopeQ = oneMt * segA.mass / aA + t * segB.mass / aB;
        if (std::abs(slopeQ) <= eps) return std::numeric_limits<double>::quiet_NaN();
        return clampToRange((x - c) / slopeQ);
    }

    // General case: at least one slope is non-zero. Substitute the segment-local
    // square-root quantile laws, isolate the cross term, and reduce to a quadratic.
    if (!sAZero && !sBZero) {
        const double A = oneMt / segA.slope;
        const double B = t / segB.slope;
        const double a0 = segA.alphaStart * segA.alphaStart - 2.0 * segA.slope * segA.cdfAtStart;
        const double a1 = 2.0 * segA.slope * segA.mass;
        const double b0 = segB.alphaStart * segB.alphaStart - 2.0 * segB.slope * segB.cdfAtStart;
        const double b1 = 2.0 * segB.slope * segB.mass;
        const double cConst = oneMt * (segA.xStart - segA.alphaStart / segA.slope) +
                              t * (segB.xStart - segB.alphaStart / segB.slope);
        const double d = x - cConst;

        // Isolate sqrt(P_A * P_B) and square: 2 A B sqrt(P_A P_B) = d^2 - A^2 P_A - B^2 P_B.
        // E(q) = E_0 + E_1 q with E_0, E_1 as below; second squaring yields the quadratic.
        const double E0 = d * d - A * A * a0 - B * B * b0;
        const double E1 = -(A * A * a1 + B * B * b1);
        const double Ac = (A * A * a1 - B * B * b1) * (A * A * a1 - B * B * b1);
        const double Bc = 2.0 * E0 * E1 - 4.0 * A * A * B * B * (a0 * b1 + a1 * b0);
        const double Cc = E0 * E0 - 4.0 * A * A * B * B * a0 * b0;

        const auto roots = solveQuadratic(Ac, Bc, Cc);
        const auto residual = [&](double q) {
            const double pa = std::max(0.0, a0 + a1 * q);
            const double pb = std::max(0.0, b0 + b1 * q);
            return A * std::sqrt(pa) + B * std::sqrt(pb) - d;
        };
        if (roots.linear) {
            return std::isnan(roots.r0) ? std::numeric_limits<double>::quiet_NaN()
                                        : clampToRange(roots.r0);
        }
        return pickBranch(roots.r0, roots.r1, residual);
    }

    // Exactly one slope is zero: X_t(q) is a linear term plus one square-root term.
    // Rearrange to isolate the radical, then square once to obtain a quadratic in q.
    const CdfSegment& segLin = sAZero ? segA : segB;
    const CdfSegment& segRoot = sAZero ? segB : segA;
    const double wLin = sAZero ? oneMt : t;
    const double wRoot = sAZero ? t : oneMt;

    if (segLin.alphaStart <= eps) return std::numeric_limits<double>::quiet_NaN();

    const double c0Lin = segLin.xStart - segLin.cdfAtStart / segLin.alphaStart;
    const double c1Lin = segLin.mass / segLin.alphaStart;
    const double c0Root = segRoot.xStart - segRoot.alphaStart / segRoot.slope;
    const double C = wRoot / segRoot.slope;
    const double p0 =
        segRoot.alphaStart * segRoot.alphaStart - 2.0 * segRoot.slope * segRoot.cdfAtStart;
    const double p1 = 2.0 * segRoot.slope * segRoot.mass;

    const double d0 = x - wLin * c0Lin - wRoot * c0Root;
    const double d1 = wLin * c1Lin;

    const double Ac = d1 * d1;
    const double Bc = -2.0 * d0 * d1 - C * C * p1;
    const double Cc = d0 * d0 - C * C * p0;

    const auto roots = solveQuadratic(Ac, Bc, Cc);
    const auto residual = [&](double q) {
        const double p = std::max(0.0, p0 + p1 * q);
        return C * std::sqrt(p) - (d0 - d1 * q);
    };
    if (roots.linear) {
        return std::isnan(roots.r0) ? std::numeric_limits<double>::quiet_NaN()
                                    : clampToRange(roots.r0);
    }
    return pickBranch(roots.r0, roots.r1, residual);
}

// Sub-interval between consecutive quantile levels: q in [qLo, qHi], x in
// [xLo, xHi] under X_t, with the paired (A, B) knot segments held fixed.
struct SubInterval {
    double qLo = 0.0;
    double qHi = 0.0;
    double xLo = 0.0;
    double xHi = 0.0;
    CdfSegment segA;
    CdfSegment segB;
};

std::vector<SubInterval> buildSubIntervalsFromLevels(const TransportContext& ctx,
                                                     const std::vector<double>& levels);

// Build sub-intervals from a sorted quantile level set. Each consecutive pair
// (q_k, q_{k+1}) lies entirely inside one (A_i, B_j) knot-segment pairing; on
// that pairing X_t(q) is a sum of at most two square-root terms in q.
std::vector<SubInterval> buildSubIntervals(const TransportContext& ctx) {
    return buildSubIntervalsFromLevels(ctx, knotInducedQuantileLevels(ctx.cdfA, ctx.cdfB));
}

// Closed-form vertex opacity: m_t / X_t'(q_k), averaged from the left and right
// sub-intervals that meet at q_k (they agree when alpha is continuous at knots).
double vertexDensityClosedForm(const std::vector<SubInterval>& intervals, double q,
                               double targetMass, double t) {
    double left = 0.0;
    double right = 0.0;
    double wL = 0.0;
    double wR = 0.0;
    for (const auto& s : intervals) {
        if (std::abs(q - s.qHi) <= eps) {
            left = densityAtQuantile(s.segA, s.segB, t, q, targetMass);
            wL = s.qHi - s.qLo;
        }
        if (std::abs(q - s.qLo) <= eps) {
            right = densityAtQuantile(s.segA, s.segB, t, q, targetMass);
            wR = s.qHi - s.qLo;
        }
    }
    const double w = wL + wR;
    if (w <= eps) return 0.0;
    return (left * wL + right * wR) / w;
}

// Closed-form reconstruction: alpha_k = m_t / X_t'(q_k) at every quantile vertex,
// then per-component trapezoidal mass rescaling so the piecewise-linear output
// integrates to the same mass share m_t * sum(Delta q) as the continuous model.
AlphaReconstruction reconstructAlphaClosedForm(const TransportContext& ctx,
                                               const std::vector<QuantilePoint>& vertices,
                                               const std::vector<SubInterval>& intervals) {
    AlphaReconstruction result;
    const std::size_t n = vertices.size();
    if (n < 2) return result;

    result.density.assign(n - 1, 0.0);
    for (std::size_t i = 0; i + 1 < n; ++i) {
        const double dx = vertices[i + 1].pos - vertices[i].pos;
        const double dq = vertices[i + 1].q - vertices[i].q;
        if (dx > eps && dq > eps) {
            result.density[i] = ctx.targetMass * dq / dx;
        }
    }
    double maxDensity = 0.0;
    for (const auto& d : result.density) maxDensity = std::max(maxDensity, d);
    result.gapThreshold = maxDensity * 1e-6;

    result.alpha.assign(n, 0.0);
    for (std::size_t i = 0; i < n; ++i) {
        result.alpha[i] = vertexDensityClosedForm(intervals, vertices[i].q, ctx.targetMass, ctx.t);
    }

    // Boundary conditions: zero out if either endpoint of either TF has zero alpha.
    const double alphaAtStartA = static_cast<double>(evaluate(ctx.aspan, supportMin(ctx.cdfA)).a);
    const double alphaAtStartB = static_cast<double>(evaluate(ctx.bspan, supportMin(ctx.cdfB)).a);
    const bool zeroAtStart = (alphaAtStartA <= eps) || (alphaAtStartB <= eps);
    const double alphaAtEndA = static_cast<double>(evaluate(ctx.aspan, supportMax(ctx.cdfA)).a);
    const double alphaAtEndB = static_cast<double>(evaluate(ctx.bspan, supportMax(ctx.cdfB)).a);
    const bool zeroAtEnd = (alphaAtEndA <= eps) || (alphaAtEndB <= eps);

    // Identify connected components separated by near-zero density intervals; force
    // zero alpha at component boundaries that are gap edges (matching the secant-path
    // semantics so that a peak does not bleed across a transport gap).
    std::size_t i = 0;
    while (i < n - 1) {
        if (result.density[i] <= result.gapThreshold) {
            result.alpha[i] = 0.0;
            result.alpha[i + 1] = 0.0;
            ++i;
            continue;
        }
        const std::size_t compStart = i;
        while (i < n - 1 && result.density[i] > result.gapThreshold) ++i;
        const std::size_t compEnd = i;
        const std::size_t compLen = compEnd - compStart + 1;

        // Boundary conditions on this component's endpoints.
        const bool zeroStartHere = (compStart > 0) || zeroAtStart;
        const bool zeroEndHere = (compEnd < n - 1) || zeroAtEnd;

        if (compLen == 2) {
            // A single-sub-interval component cannot have its two endpoints both forced
            // to zero without losing all of its mass: the trapezoidal integral would be
            // 0 and the per-component rescaling has no interior alpha to redistribute
            // mass onto. Fall back to the secant density d_k on the component (mass-
            // preserving by construction) and apply the boundary clamps on top, mirroring
            // the compLen == 2 branch of reconstructAlpha.
            const double dx = vertices[compEnd].pos - vertices[compStart].pos;
            const double dq = vertices[compEnd].q - vertices[compStart].q;
            const double dSec = (dx > eps) ? ctx.targetMass * dq / dx : 0.0;
            result.alpha[compStart] = zeroStartHere ? 0.0 : std::max(result.alpha[compStart], dSec);
            result.alpha[compEnd] = zeroEndHere ? 0.0 : std::max(result.alpha[compEnd], dSec);
            // If both endpoints are clamped to zero on a 2-vertex component, the
            // component is irrecoverable as a piecewise-linear TF (no interior knot to
            // carry mass). Insert dSec into both so its trapezoidal mass equals the
            // target: this preserves total integral even if it slightly violates the
            // zero-edge cosmetic.
            if (zeroStartHere && zeroEndHere && dSec > 0.0) {
                result.alpha[compStart] = dSec;
                result.alpha[compEnd] = dSec;
            }
            continue;
        }

        if (zeroStartHere) result.alpha[compStart] = 0.0;
        if (zeroEndHere) result.alpha[compEnd] = 0.0;

        // Mass-preserving rescaling on this component: the trapezoidal mass of the
        // output piecewise-linear TF differs from the exact integral of alpha_t because
        // alpha_t is nonlinear within each sub-interval. Scaling all alphas uniformly
        // forces the trapezoidal mass to match the target m_t * sum(dq).
        double compTargetMass = 0.0;
        double compActualMass = 0.0;
        for (std::size_t j = compStart; j < compEnd; ++j) {
            const double dx = vertices[j + 1].pos - vertices[j].pos;
            if (dx <= eps) continue;
            const double dq = vertices[j + 1].q - vertices[j].q;
            compTargetMass += ctx.targetMass * dq;
            compActualMass += 0.5 * (result.alpha[j] + result.alpha[j + 1]) * dx;
        }
        if (compActualMass > eps) {
            const double scale = compTargetMass / compActualMass;
            for (std::size_t j = compStart; j <= compEnd; ++j) result.alpha[j] *= scale;
        }
    }

    for (auto& aVal : result.alpha) aVal = std::max(0.0, aVal);
    return result;
}

// Build a per-interval (segA, segB) lookup keyed by qLo. levels is assumed sorted
// strictly ascending and to contain every breakpoint of L plus any refinements;
// each consecutive pair lies entirely inside one (A_i, B_j) pairing.
std::vector<SubInterval> buildSubIntervalsFromLevels(const TransportContext& ctx,
                                                     const std::vector<double>& levels) {
    std::vector<SubInterval> intervals;
    intervals.reserve(levels.size());

    const auto locateSegment = [](const Cdf& cdf, double q) -> std::size_t {
        if (cdf.points.size() < 2) return 0;
        const double targetMass = q * cdf.totalMass;
        auto it = std::lower_bound(cdf.points.begin(), cdf.points.end(), targetMass,
                                   [](const CdfPoint& p, double mass) { return p.mass < mass; });
        if (it == cdf.points.begin()) return 0;
        if (it == cdf.points.end()) return cdf.points.size() - 2;
        return static_cast<std::size_t>(it - cdf.points.begin()) - 1;
    };

    for (std::size_t k = 0; k + 1 < levels.size(); ++k) {
        const double qLo = levels[k];
        const double qHi = levels[k + 1];
        if (qHi - qLo <= eps) continue;
        const double qMid = 0.5 * (qLo + qHi);
        const std::size_t iA = locateSegment(ctx.cdfA, qMid);
        const std::size_t iB = locateSegment(ctx.cdfB, qMid);
        SubInterval sub;
        sub.qLo = qLo;
        sub.qHi = qHi;
        sub.segA = makeSegment(ctx.cdfA, iA);
        sub.segB = makeSegment(ctx.cdfB, iB);
        sub.xLo = xtOnSegment(sub.segA, sub.segB, ctx.t, qLo);
        sub.xHi = xtOnSegment(sub.segA, sub.segB, ctx.t, qHi);
        intervals.push_back(sub);
    }
    return intervals;
}

// Refine the quantile level set by bisecting sub-intervals where the straight line
// between closed-form endpoint opacities deviates from the closed-form opacity at
// the sub-interval midpoint by more than relTol (relative error, measured in x).
// As relTol -> 0 the piecewise-linear knot output converges to the continuous alpha_t.
//
// Sub-intervals where the A and B segment slopes have opposite signs are refined
// first in practice: X_t'(q) can have an interior extremum there, producing an
// interior opacity minimum or maximum that a coarse level set cannot represent.
void refineQuantileLevelsByClosedForm(const TransportContext& ctx, std::vector<double>& levels,
                                      double relTol, std::size_t maxLevels,
                                      std::size_t maxIterations) {
    if (relTol <= 0.0) return;

    for (std::size_t iter = 0; iter < maxIterations; ++iter) {
        if (levels.size() >= maxLevels) break;
        const auto intervals = buildSubIntervalsFromLevels(ctx, levels);

        double maxRelError = 0.0;
        double bestSplit = 0.0;
        bool found = false;

        for (const auto& s : intervals) {
            const double qMid = 0.5 * (s.qLo + s.qHi);
            const double aLo = densityAtQuantile(s.segA, s.segB, ctx.t, s.qLo, ctx.targetMass);
            const double aHi = densityAtQuantile(s.segA, s.segB, ctx.t, s.qHi, ctx.targetMass);
            const double aMid = densityAtQuantile(s.segA, s.segB, ctx.t, qMid, ctx.targetMass);
            if (aMid <= 0.0 && aLo <= 0.0 && aHi <= 0.0) continue;

            const double xMid = xtOnSegment(s.segA, s.segB, ctx.t, qMid);
            const double dx = s.xHi - s.xLo;
            if (dx <= eps) continue;
            const double aMidLinear = aLo + (aHi - aLo) * (xMid - s.xLo) / dx;

            const double absErr = std::abs(aMid - aMidLinear);
            const double scale = std::max({std::abs(aMid), std::abs(aMidLinear), eps});
            const double relErr = absErr / scale;
            if (relErr > maxRelError) {
                maxRelError = relErr;
                bestSplit = qMid;
                found = true;
            }
        }

        if (!found || maxRelError <= relTol) break;
        insertQuantileLevel(levels, bestSplit);
    }
}

// Closed-form pipeline: sample quantile levels, evaluate alpha_k = m_t / X_t'(q_k),
// and emit a piecewise-linear output TF (same assembly as the secant-based path).
std::vector<TFPrimitiveData> optimalTransportInterpolationClosedFormFromLevels(
    const TransportContext& ctx, const std::vector<double>& levels, double domainMin,
    double domainMax) {
    if (levels.size() < 2) return linearBlend(ctx.aspan, ctx.bspan, ctx.t);
    const auto vertices = buildInterpolatedVertices(ctx, levels);
    if (vertices.size() < 2) return linearBlend(ctx.aspan, ctx.bspan, ctx.t);

    const auto intervals = buildSubIntervalsFromLevels(ctx, levels);
    const auto reconstruction = reconstructAlphaClosedForm(ctx, vertices, intervals);
    auto result = buildOutputTransferFunction(ctx, vertices, reconstruction, domainMin, domainMax);
    if (result.empty()) return linearBlend(ctx.aspan, ctx.bspan, ctx.t);
    return result;
}

}  // namespace

// ===========================================================================
// Public API
// ===========================================================================
//
// Two quantile-sampling strategies are provided for comparison:
//
//   optimalTransportInterpolation        — legacy uniform sampling with
//                                          samplesPerSegment sub-divisions per
//                                          CDF segment (unchanged behaviour).
//
//   optimalTransportInterpolationClosedForm — knot-induced quantile levels,
//                                             closed-form vertex opacities
//                                             alpha_k = m_t / X_t'(q_k), and
//                                             optional midpoint-error refinement.

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

    const Cdf cdfA = computeCdf(aspan);
    const Cdf cdfB = computeCdf(bspan);

    if (cdfA.totalMass <= eps || cdfB.totalMass <= eps ||
        (1.0 - t) * cdfA.totalMass + t * cdfB.totalMass <= eps) {
        return linearBlend(aspan, bspan, t);
    }

    const TransportContext ctx = makeTransportContext(aspan, bspan, cdfA, cdfB, t);

    // Legacy uniform quantile sampling: sub-divide each CDF segment into samplesPerSegment
    // equal steps in q. Kept unchanged for comparison with the adaptive path below.
    const auto levels = mergedQuantileLevels(cdfA, cdfB, samplesPerSegment);

    const double domainMin = (1.0 - t) * a.front().pos + t * b.front().pos;
    const double domainMax = (1.0 - t) * a.back().pos + t * b.back().pos;

    return optimalTransportInterpolationFromLevels(ctx, levels, domainMin, domainMax);
}

std::vector<TFPrimitiveData> optimalTransportInterpolationClosedForm(
    std::span<const TFPrimitiveData> tfA, std::span<const TFPrimitiveData> tfB, double t,
    const ClosedFormRefinementOptions& opts) {
    t = std::clamp(t, 0.0, 1.0);

    auto a = sanitize(tfA);
    auto b = sanitize(tfB);

    const std::span<const TFPrimitiveData> aspan{a.data(), a.size()};
    const std::span<const TFPrimitiveData> bspan{b.data(), b.size()};

    if (a.empty()) return b;
    if (b.empty()) return a;
    if (t <= 0.0) return a;
    if (t >= 1.0) return b;

    const Cdf cdfA = computeCdf(aspan);
    const Cdf cdfB = computeCdf(bspan);

    if (cdfA.totalMass <= eps || cdfB.totalMass <= eps ||
        (1.0 - t) * cdfA.totalMass + t * cdfB.totalMass <= eps) {
        return linearBlend(aspan, bspan, t);
    }

    const TransportContext ctx = makeTransportContext(aspan, bspan, cdfA, cdfB, t);

    // Knot-induced quantile levels, then bisect sub-intervals where the piecewise-linear
    // chord between endpoint opacities deviates from the closed-form opacity at the
    // midpoint (relative tolerance, measured in x). This targets sub-intervals where
    // X_t'(q) curves most strongly, e.g. when the paired segment slopes have opposite sign.
    auto levels = knotInducedQuantileLevels(cdfA, cdfB);
    refineQuantileLevelsByClosedForm(ctx, levels, opts.relativeTolerance, opts.maxQuantileLevels,
                                     opts.maxRefinementIterations);

    const double domainMin = (1.0 - t) * a.front().pos + t * b.front().pos;
    const double domainMax = (1.0 - t) * a.back().pos + t * b.back().pos;

    return optimalTransportInterpolationClosedFormFromLevels(ctx, levels, domainMin, domainMax);
}

double evaluateInterpolatedAlpha(std::span<const TFPrimitiveData> tfA,
                                 std::span<const TFPrimitiveData> tfB, double t, double x) {
    t = std::clamp(t, 0.0, 1.0);

    auto a = sanitize(tfA);
    auto b = sanitize(tfB);

    const std::span<const TFPrimitiveData> aspan{a.data(), a.size()};
    const std::span<const TFPrimitiveData> bspan{b.data(), b.size()};

    if (a.empty() || b.empty()) return 0.0;
    if (t <= 0.0) return static_cast<double>(evaluate(aspan, x).a);
    if (t >= 1.0) return static_cast<double>(evaluate(bspan, x).a);

    const Cdf cdfA = computeCdf(aspan);
    const Cdf cdfB = computeCdf(bspan);
    if (cdfA.totalMass <= eps || cdfB.totalMass <= eps) return 0.0;

    const TransportContext ctx = makeTransportContext(aspan, bspan, cdfA, cdfB, t);
    const auto intervals = buildSubIntervals(ctx);
    if (intervals.empty()) return 0.0;

    // Outside the union of widget supports of the interpolated TF: alpha = 0.
    if (x <= intervals.front().xLo + eps || x >= intervals.back().xHi - eps) return 0.0;

    // Binary search the sub-interval that brackets x in output coordinates. The xLo
    // values are strictly increasing because X_t is monotone (it is a sum of two
    // monotone-in-q quantile functions).
    auto it = std::upper_bound(intervals.begin(), intervals.end(), x,
                               [](double xv, const SubInterval& s) { return xv < s.xHi; });
    if (it == intervals.end()) it = intervals.end() - 1;
    const SubInterval& sub = *it;

    const double q = invertXtOnSegment(sub.segA, sub.segB, t, x);
    if (std::isnan(q)) return 0.0;

    return densityAtQuantile(sub.segA, sub.segB, t, q, ctx.targetMass);
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
