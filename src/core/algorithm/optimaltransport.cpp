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
#include <limits>
#include <utility>
#include <vector>

namespace inviwo::algorithm {

namespace {

// Numerical tolerance for floating-point comparisons.
constexpr double eps = 1e-12;

// Density below maxDensity * gapThresholdFactor marks a transport gap (near-zero
// alpha) that separates connected components in the reconstructed TF.
constexpr double gapThresholdFactor = 1e-6;

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
    double supportMinVal = 0.0;
    double supportMaxVal = 0.0;
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

// Make clamp-to-edge extrapolation explicit in the control-point data. Sampling a TF (see
// evaluate) holds the boundary opacity/color constant outside [front.pos, back.pos], but
// computeCdf only integrates mass *between* control points. That inconsistency makes the OT
// machinery transport a compactly supported distribution while the result renders with
// clamp-to-edge flats, so the two disagree on [0, front.pos] and [back.pos, 1]. Materializing
// the implied flat regions as boundary vertices at the domain edges (0 and 1) reconciles the
// views: computeCdf, the domain bounds (domainMin/domainMax), structural-vertex insertion and
// evaluate then all agree on a common [0, 1] support. Inputs already reaching an edge (or with
// control points outside [0, 1]) are left untouched, so full-range TFs are unaffected.
std::vector<TFPrimitiveData> extendToClampSupport(std::vector<TFPrimitiveData> tf) {
    if (tf.empty()) return tf;
    if (tf.front().pos > 0) {
        tf.insert(tf.begin(), TFPrimitiveData{0.0, tf.front().color});
    }
    if (tf.back().pos < 1.0) {
        tf.push_back(TFPrimitiveData{1.0, tf.back().color});
    }
    return tf;
}

// Sort and deduplicate TF primitives by position, then make clamp-to-edge support explicit
// (see extendToClampSupport). A piecewise-linear function cannot represent vertical
// discontinuities, so duplicate positions are collapsed (last wins).
std::vector<TFPrimitiveData> sanitize(std::span<const TFPrimitiveData> tf) {
    if (tf.empty()) return {};

    // Check if it is already sorted strictly (with no eps duplicates).
    bool processSorted = true;
    for (std::size_t i = 1; i < tf.size(); ++i) {
        if (tf[i].pos <= tf[i - 1].pos || std::abs(tf[i].pos - tf[i - 1].pos) <= eps) {
            processSorted = false;
            break;
        }
    }

    if (processSorted) {
        return extendToClampSupport(std::vector<TFPrimitiveData>(tf.begin(), tf.end()));
    }

    std::vector<TFPrimitiveData> points(tf.begin(), tf.end());
    std::sort(points.begin(), points.end(),
              [](const TFPrimitiveData& a, const TFPrimitiveData& b) { return a.pos < b.pos; });

    std::vector<TFPrimitiveData> result;
    result.reserve(points.size() + 2);
    for (const auto& p : points) {
        if (result.empty() || std::abs(p.pos - result.back().pos) > eps) {
            result.push_back(p);
        } else {
            result.back() = p;
        }
    }
    return extendToClampSupport(std::move(result));
}

// The position span of the TF. Returns a NaN pair if input is empty.
std::pair<double, double> minMaxPos(std::span<const TFPrimitiveData> tf) {
    if (tf.empty()) {
        const double nan = std::numeric_limits<double>::quiet_NaN();
        return {nan, nan};
    }
    double lo = tf.front().pos;
    double hi = tf.front().pos;
    for (const auto& p : tf) {
        lo = std::min(lo, p.pos);
        hi = std::max(hi, p.pos);
    }
    return {lo, hi};
}

// Remove the synthetic boundary vertex that extendToClampSupport inserts at x=0 (and/or x=1)
// so the returned TF spans only the original input support again. The removal is gated and
// safe-by-construction:
//   * supportMin > eps confirms the original input did not already reach the edge (so the
//     vertex at 0 is synthetic, not a genuine control point the caller supplied);
//   * the vertex is dropped only when it is collinear with its neighbour in both alpha and
//     color, which is always true for the constant flat edge extendToClampSupport creates but
//     not for a real ramp, so a one-sided extension over genuine structure is never cut.
// Because the dropped span is constant, clamp-to-edge sampling of the trimmed TF reproduces it
// exactly, and the (also-extended) oracle returns the same constant there: render- and
// accuracy-neutral. At most one vertex is removed per end, and at least two are kept.
std::vector<TFPrimitiveData> trimSyntheticEdges(std::vector<TFPrimitiveData> tf, double supportMin,
                                                double supportMax) {
    const auto collinear = [](const TFPrimitiveData& a, const TFPrimitiveData& b) {
        constexpr float colorEps = 1e-6f;
        return std::abs(a.color.r - b.color.r) <= colorEps &&
               std::abs(a.color.g - b.color.g) <= colorEps &&
               std::abs(a.color.b - b.color.b) <= colorEps &&
               std::abs(a.color.a - b.color.a) <= colorEps;
    };

    if (tf.size() >= 3 && std::isfinite(supportMin) && supportMin > eps &&
        tf.front().pos <= eps && collinear(tf[0], tf[1])) {
        tf.erase(tf.begin());
    }
    if (tf.size() >= 3 && std::isfinite(supportMax) && supportMax < 1.0 - eps &&
        tf.back().pos >= 1.0 - eps && collinear(tf[tf.size() - 1], tf[tf.size() - 2])) {
        tf.pop_back();
    }
    return tf;
}

// Evaluate a piecewise-linear TF at position x via linear interpolation.
// Fast path: reuse search hints or state when available.
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

// Optimized local sequential search/evaluation where search advances or is near.
struct EvaluationCursor {
    std::size_t idx = 0;
};

vec4 evaluateCursor(std::span<const TFPrimitiveData> tf, double x, EvaluationCursor& cursor) {
    if (tf.empty()) return vec4{0.0f};
    if (x <= tf.front().pos) return tf.front().color;
    if (x >= tf.back().pos) return tf.back().color;

    // Shift cursor forward or backward appropriately to find the interval enclosing x.
    if (cursor.idx >= tf.size() - 1) {
        cursor.idx = tf.size() - 2;
    }
    // Advance forward
    while (cursor.idx + 1 < tf.size() && tf[cursor.idx + 1].pos <= x) {
        ++cursor.idx;
    }
    // Rewind backward
    while (cursor.idx > 0 && tf[cursor.idx].pos > x) {
        --cursor.idx;
    }

    const auto& p0 = tf[cursor.idx];
    const auto& p1 = tf[cursor.idx + 1];

    const double dx = p1.pos - p0.pos;
    if (std::abs(dx) < eps) return glm::mix(p0.color, p1.color, 0.5f);

    const auto u = static_cast<float>((x - p0.pos) / dx);
    return glm::mix(p0.color, p1.color, u);
}

// ---------------------------------------------------------------------------
// Fallback: simple pointwise linear blend
// ---------------------------------------------------------------------------;

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

    // Cache supportMin and supportMax
    if (cdf.points.empty()) {
        cdf.supportMinVal = 0.0;
        cdf.supportMaxVal = 0.0;
    } else {
        bool foundMin = false;
        cdf.supportMinVal = cdf.points.front().pos;
        for (std::size_t i = 1; i < cdf.points.size(); ++i) {
            if (cdf.points[i].mass - cdf.points[i - 1].mass > eps) {
                cdf.supportMinVal = cdf.points[i - 1].pos;
                foundMin = true;
                break;
            }
        }
        if (!foundMin) {
            cdf.supportMinVal = cdf.points.front().pos;
        }

        bool foundMax = false;
        cdf.supportMaxVal = cdf.points.back().pos;
        for (std::size_t i = cdf.points.size(); i-- > 1;) {
            if (cdf.points[i].mass - cdf.points[i - 1].mass > eps) {
                cdf.supportMaxVal = cdf.points[i].pos;
                foundMax = true;
                break;
            }
        }
        if (!foundMax) {
            cdf.supportMaxVal = cdf.points.back().pos;
        }
    }

    return cdf;
}

// Find the leftmost position where the CDF begins to increase (start of support).
double supportMin(const Cdf& cdf) {
    return cdf.supportMinVal;
}

// Find the rightmost position where the CDF stops increasing (end of support).
double supportMax(const Cdf& cdf) {
    return cdf.supportMaxVal;
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
// Fast path: use a search cursor if iterating/sweeping sequentially.
struct InvertCdfCursor {
    std::size_t idx = 0;
};

double invertCdfCursor(const Cdf& cdf, double q, InvertCdfCursor& cursor) {
    if (cdf.points.empty()) return 0.0;
    if (cdf.totalMass <= eps) return cdf.points.front().pos;

    q = std::clamp(q, 0.0, 1.0);
    if (q <= 0.0) return supportMin(cdf);
    if (q >= 1.0) return supportMax(cdf);

    const double targetMass = q * cdf.totalMass;

    if (cursor.idx >= cdf.points.size()) {
        cursor.idx = cdf.points.size() - 1;
    }

    // Advance forward
    while (cursor.idx < cdf.points.size() && cdf.points[cursor.idx].mass < targetMass) {
        ++cursor.idx;
    }
    // Rewind backward
    while (cursor.idx > 0 && cdf.points[cursor.idx - 1].mass >= targetMass) {
        --cursor.idx;
    }

    if (cursor.idx == 0) return cdf.points.front().pos;
    if (cursor.idx == cdf.points.size()) return supportMax(cdf);

    const auto& p1 = cdf.points[cursor.idx];
    const auto& p0 = cdf.points[cursor.idx - 1];

    const double segmentMass = p1.mass - p0.mass;
    if (segmentMass <= eps) return p0.pos;

    const double dx = p1.pos - p0.pos;
    if (dx <= eps) return p0.pos;

    const double localMass = targetMass - p0.mass;
    return p0.pos + solveSegmentInverse(p0.alpha, p1.alpha, dx, localMass);
}

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

    // Initial point
    levels.push_back(quantileAtPoint(cdf, cdf.points.front()));

    for (std::size_t i = 1; i < cdf.points.size(); ++i) {
        const double q0 = quantileAtPoint(cdf, cdf.points[i - 1]);
        const double q1 = quantileAtPoint(cdf, cdf.points[i]);

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
    std::vector<double> levelsA;
    levelsA.reserve(a.points.size() * (samplesPerSegment + 1) + 2);
    addQuantileLevels(a, samplesPerSegment, levelsA);
    std::sort(levelsA.begin(), levelsA.end());
    levelsA.erase(std::unique(levelsA.begin(), levelsA.end(),
                               [](double x, double y) { return std::abs(x - y) < eps; }),
                  levelsA.end());

    std::vector<double> levelsB;
    levelsB.reserve(b.points.size() * (samplesPerSegment + 1) + 2);
    addQuantileLevels(b, samplesPerSegment, levelsB);
    std::sort(levelsB.begin(), levelsB.end());
    levelsB.erase(std::unique(levelsB.begin(), levelsB.end(),
                               [](double x, double y) { return std::abs(x - y) < eps; }),
                  levelsB.end());

    std::vector<double> levels;
    levels.resize(levelsA.size() + levelsB.size());
    std::merge(levelsA.begin(), levelsA.end(), levelsB.begin(), levelsB.end(), levels.begin());

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

QuantilePoint transportedQuantilePointCursor(const TransportContext& ctx, double q,
                                              InvertCdfCursor& cursorA, InvertCdfCursor& cursorB,
                                              EvaluationCursor& evalCursorA, EvaluationCursor& evalCursorB) {
    const double xA = invertCdfCursor(ctx.cdfA, q, cursorA);
    const vec4 colorA = evaluateCursor(ctx.aspan, xA, evalCursorA);
    const QuantilePoint qa{xA, q, vec3{colorA}};

    const double xB = invertCdfCursor(ctx.cdfB, q, cursorB);
    const vec4 colorB = evaluateCursor(ctx.bspan, xB, evalCursorB);
    const QuantilePoint qb{xB, q, vec3{colorB}};

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
        EvaluationCursor selfCursor;
        EvaluationCursor otherCursor;
        InvertCdfCursor invertCursor;
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
            if (std::isnan(xOther)) xOther = invertCdfCursor(otherCdf, q, invertCursor);

            const double x = weightSelf * xSelf + weightOther * xOther;
            const vec3 color =
                glm::mix(vec3{evaluateCursor(selfSpan, xSelf, selfCursor)}, vec3{evaluateCursor(otherSpan, xOther, otherCursor)},
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

    InvertCdfCursor cursorA;
    InvertCdfCursor cursorB;
    EvaluationCursor evalCursorA;
    EvaluationCursor evalCursorB;

    for (double q : levels) {
        interpolatedCdf.push_back(transportedQuantilePointCursor(ctx, q, cursorA, cursorB, evalCursorA, evalCursorB));
    }

    // The transported levels are already sorted by q (levels is sorted ascending and
    // transportedQuantilePointCursor preserves q). addStructuralVertices appends a much
    // smaller, unsorted batch of knot vertices. Rather than re-sorting the whole array in
    // O(M log M), sort only the appended suffix and merge it into the sorted prefix in
    // O(M), which dominates for large samplesPerSegment.
    const std::size_t sortedPrefix = interpolatedCdf.size();

    addStructuralVertices(ctx.aspan, ctx.bspan, ctx.cdfA, ctx.cdfB, ctx.t, interpolatedCdf);

    const auto byQ = [](const QuantilePoint& lhs, const QuantilePoint& rhs) {
        return lhs.q < rhs.q;
    };
    const auto mid = interpolatedCdf.begin() + sortedPrefix;
    std::stable_sort(mid, interpolatedCdf.end(), byQ);
    std::inplace_merge(interpolatedCdf.begin(), mid, interpolatedCdf.end(), byQ);

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

// Secant interval densities d_k = m_t * Delta q / Delta x between consecutive
// vertices, plus the derived gap threshold maxDensity * gapThresholdFactor. Shared
// by both the secant-based and closed-form reconstructions, which start from the
// same per-interval densities and gap classification.
void computeSecantDensities(const TransportContext& ctx,
                            const std::vector<QuantilePoint>& vertices,
                            AlphaReconstruction& result) {
    const std::size_t n = vertices.size();
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
    result.gapThreshold = maxDensity * gapThresholdFactor;
}

// Whether the interpolated TF must pin alpha to zero at its support start / end.
// This happens when either input TF has zero alpha at the corresponding support
// boundary, so a peak does not bleed across a transport gap.
struct BoundaryZeroFlags {
    bool atStart = false;
    bool atEnd = false;
};

BoundaryZeroFlags computeBoundaryZeroFlags(const TransportContext& ctx) {
    const double alphaAtStartA = static_cast<double>(evaluate(ctx.aspan, supportMin(ctx.cdfA)).a);
    const double alphaAtStartB = static_cast<double>(evaluate(ctx.bspan, supportMin(ctx.cdfB)).a);
    const double alphaAtEndA = static_cast<double>(evaluate(ctx.aspan, supportMax(ctx.cdfA)).a);
    const double alphaAtEndB = static_cast<double>(evaluate(ctx.bspan, supportMax(ctx.cdfB)).a);
    return BoundaryZeroFlags{(alphaAtStartA <= eps) || (alphaAtStartB <= eps),
                             (alphaAtEndA <= eps) || (alphaAtEndB <= eps)};
}

// Uniformly rescale alpha on [compStart, compEnd] so the output piecewise-linear
// trapezoidal mass matches the target mass share m_t * sum(Delta q) that X_t
// assigns to the component. Used by both reconstructions.
void rescaleComponentMass(const TransportContext& ctx,
                          const std::vector<QuantilePoint>& vertices, AlphaReconstruction& result,
                          std::size_t compStart, std::size_t compEnd) {
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

// Reconstruct piecewise-linear alpha from quantile vertices via secant interval
// densities d_k = m_t * Delta q / Delta x and width-weighted vertex averaging.
AlphaReconstruction reconstructAlpha(const TransportContext& ctx,
                                     const std::vector<QuantilePoint>& vertices) {
    AlphaReconstruction result;
    const std::size_t n = vertices.size();
    if (n < 2) return result;

    computeSecantDensities(ctx, vertices, result);

    result.alpha.assign(n, 0.0);

    const BoundaryZeroFlags boundary = computeBoundaryZeroFlags(ctx);
    const bool zeroAtStart = boundary.atStart;
    const bool zeroAtEnd = boundary.atEnd;

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

        rescaleComponentMass(ctx, vertices, result, compStart, compEnd);
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

// Precomputed coefficients for inverting X_t(q) on a single sub-interval.
struct PrecomputedInversion {
    bool valid = false;
    bool sAZero = false;
    bool sBZero = false;

    // Flat terms
    double flatC = 0.0;
    double flatSlopeQ = 0.0;

    // Quadratic terms (both slopes non-zero)
    double A = 0.0;
    double B = 0.0;
    double a0 = 0.0;
    double a1 = 0.0;
    double b0 = 0.0;
    double b1 = 0.0;
    double cConst = 0.0;

    // Linear + Square-root terms (exactly one slope zero)
    double wLin = 0.0;
    double wRoot = 0.0;
    double c0Lin = 0.0;
    double c1Lin = 0.0;
    double c0Root = 0.0;
    double C = 0.0;
    double p0 = 0.0;
    double p1 = 0.0;
    double d1 = 0.0;
    double d0_base = 0.0; // x gets added to this
};

// Sub-interval between consecutive quantile levels: q in [qLo, qHi], x in
// [xLo, xHi] under X_t, with the paired (A, B) knot segments held fixed.
struct SubInterval {
    double qLo = 0.0;
    double qHi = 0.0;
    double xLo = 0.0;
    double xHi = 0.0;
    CdfSegment segA;
    CdfSegment segB;
    PrecomputedInversion inv;
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

PrecomputedInversion precomputeInversion(const CdfSegment& segA, const CdfSegment& segB, double t) {
    PrecomputedInversion inv;
    const double oneMt = 1.0 - t;
    inv.sAZero = std::abs(segA.slope) <= eps;
    inv.sBZero = std::abs(segB.slope) <= eps;

    if (inv.sAZero && inv.sBZero) {
        const double aA = segA.alphaStart;
        const double aB = segB.alphaStart;
        if (aA <= eps || aB <= eps) {
            inv.valid = false;
            return inv;
        }
        inv.flatC = oneMt * (segA.xStart - segA.cdfAtStart / aA) + t * (segB.xStart - segB.cdfAtStart / aB);
        inv.flatSlopeQ = oneMt * segA.mass / aA + t * segB.mass / aB;
        inv.valid = true;
        return inv;
    }

    if (!inv.sAZero && !inv.sBZero) {
        inv.A = oneMt / segA.slope;
        inv.B = t / segB.slope;
        inv.a0 = segA.alphaStart * segA.alphaStart - 2.0 * segA.slope * segA.cdfAtStart;
        inv.a1 = 2.0 * segA.slope * segA.mass;
        inv.b0 = segB.alphaStart * segB.alphaStart - 2.0 * segB.slope * segB.cdfAtStart;
        inv.b1 = 2.0 * segB.slope * segB.mass;
        inv.cConst = oneMt * (segA.xStart - segA.alphaStart / segA.slope) +
                     t * (segB.xStart - segB.alphaStart / segB.slope);
        inv.valid = true;
        return inv;
    }

    const CdfSegment& segLin = inv.sAZero ? segA : segB;
    const CdfSegment& segRoot = inv.sAZero ? segB : segA;
    inv.wLin = inv.sAZero ? oneMt : t;
    inv.wRoot = inv.sAZero ? t : oneMt;

    if (segLin.alphaStart <= eps) {
        inv.valid = false;
        return inv;
    }

    inv.c0Lin = segLin.xStart - segLin.cdfAtStart / segLin.alphaStart;
    inv.c1Lin = segLin.mass / segLin.alphaStart;
    inv.c0Root = segRoot.xStart - segRoot.alphaStart / segRoot.slope;
    inv.C = inv.wRoot / segRoot.slope;
    inv.p0 = segRoot.alphaStart * segRoot.alphaStart - 2.0 * segRoot.slope * segRoot.cdfAtStart;
    inv.p1 = 2.0 * segRoot.slope * segRoot.mass;
    inv.d1 = inv.wLin * inv.c1Lin;
    inv.d0_base = - (inv.wLin * inv.c0Lin + inv.wRoot * inv.c0Root);
    inv.valid = true;
    return inv;
}

// Solve X_t(q) = x on a sub-interval where Q_A lies on segA and Q_B(q) on segB.
// Using precomputed inversion parameters for absolute speed.
double invertXtOnSegmentPrecomputed(const SubInterval& sub, double t, double x) {
    const double qLo = sub.qLo;
    const double qHi = sub.qHi;
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

    const auto& inv = sub.inv;
    if (!inv.valid) return std::numeric_limits<double>::quiet_NaN();

    if (inv.sAZero && inv.sBZero) {
        if (std::abs(inv.flatSlopeQ) <= eps) return std::numeric_limits<double>::quiet_NaN();
        return clampToRange((x - inv.flatC) / inv.flatSlopeQ);
    }

    if (!inv.sAZero && !inv.sBZero) {
        const double d = x - inv.cConst;
        const double E0 = d * d - inv.A * inv.A * inv.a0 - inv.B * inv.B * inv.b0;
        const double E1 = -(inv.A * inv.A * inv.a1 + inv.B * inv.B * inv.b1);
        const double Ac = (inv.A * inv.A * inv.a1 - inv.B * inv.B * inv.b1) * (inv.A * inv.A * inv.a1 - inv.B * inv.B * inv.b1);
        const double Bc = 2.0 * E0 * E1 - 4.0 * inv.A * inv.A * inv.B * inv.B * (inv.a0 * inv.b1 + inv.a1 * inv.b0);
        const double Cc = E0 * E0 - 4.0 * inv.A * inv.A * inv.B * inv.B * inv.a0 * inv.b0;

        const auto roots = solveQuadratic(Ac, Bc, Cc);
        const auto residual = [&](double q) {
            const double pa = std::max(0.0, inv.a0 + inv.a1 * q);
            const double pb = std::max(0.0, inv.b0 + inv.b1 * q);
            return inv.A * std::sqrt(pa) + inv.B * std::sqrt(pb) - d;
        };
        if (roots.linear) {
            return std::isnan(roots.r0) ? std::numeric_limits<double>::quiet_NaN()
                                        : clampToRange(roots.r0);
        }
        return pickBranch(roots.r0, roots.r1, residual);
    }

    const double d0 = x + inv.d0_base;
    const double Ac = inv.d1 * inv.d1;
    const double Bc = -2.0 * d0 * inv.d1 - inv.C * inv.C * inv.p1;
    const double Cc = d0 * d0 - inv.C * inv.C * inv.p0;

    const auto roots = solveQuadratic(Ac, Bc, Cc);
    const auto residual = [&](double q) {
        const double p = std::max(0.0, inv.p0 + inv.p1 * q);
        return inv.C * std::sqrt(p) - (d0 - inv.d1 * q);
    };
    if (roots.linear) {
        return std::isnan(roots.r0) ? std::numeric_limits<double>::quiet_NaN()
                                    : clampToRange(roots.r0);
    }
    return pickBranch(roots.r0, roots.r1, residual);
}

// Solve X_t(q) = x on a sub-interval where Q_A lies on segA and Q_B(q) on segB.
double invertXtOnSegment(const CdfSegment& segA, const CdfSegment& segB, double t, double x) {
    SubInterval temp;
    temp.qLo = std::max(segA.qStart, segB.qStart);
    temp.qHi = std::min(segA.qEnd, segB.qEnd);
    temp.segA = segA;
    temp.segB = segB;
    temp.inv = precomputeInversion(segA, segB, t);
    return invertXtOnSegmentPrecomputed(temp, t, x);
}

std::vector<SubInterval> buildSubIntervalsFromLevels(const TransportContext& ctx,
                                                     const std::vector<double>& levels,
                                                     bool computeInversion = false);

// Build sub-intervals from a sorted quantile level set. Each consecutive pair
// (q_k, q_{k+1}) lies entirely inside one (A_i, B_j) knot-segment pairing; on
// that pairing X_t(q) is a sum of at most two square-root terms in q.
// The per-interval inversion coefficients (SubInterval::inv) are only needed by
// invertXtOnSegmentPrecomputed (the evaluateInterpolatedAlpha* paths), so they
// are computed here; callers that only need segment/position data skip them.
std::vector<SubInterval> buildSubIntervals(const TransportContext& ctx) {
    return buildSubIntervalsFromLevels(ctx, knotInducedQuantileLevels(ctx.cdfA, ctx.cdfB),
                                       /*computeInversion=*/true);
}

// Closed-form vertex opacity: m_t / X_t'(q_k), averaged from the left and right
// sub-intervals that meet at q_k (they agree when alpha is continuous at knots).
double vertexDensityClosedForm(const std::vector<SubInterval>& intervals, double q,
                               double targetMass, double t) {
    // intervals are sorted ascending by qLo and are non-overlapping, so qHi is also
    // ascending. A vertex q matches at most one interval on each side: the left
    // neighbour has qHi == q, the right neighbour has qLo == q. Binary-search both
    // instead of scanning every interval per vertex (was O(n*M) overall; now
    // O(n log M)). The short local window after each lower_bound absorbs the eps
    // tolerance at the search boundary.
    double left = 0.0;
    double right = 0.0;
    double wL = 0.0;
    double wR = 0.0;

    // Right neighbour: first interval with qLo >= q - eps, then eps-match on qLo.
    for (auto it = std::lower_bound(intervals.begin(), intervals.end(), q - eps,
                                    [](const SubInterval& s, double v) { return s.qLo < v; });
         it != intervals.end() && it->qLo <= q + eps; ++it) {
        if (std::abs(q - it->qLo) <= eps) {
            right = densityAtQuantile(it->segA, it->segB, t, q, targetMass);
            wR = it->qHi - it->qLo;
            break;
        }
    }

    // Left neighbour: first interval with qHi >= q - eps, then eps-match on qHi.
    for (auto it = std::lower_bound(intervals.begin(), intervals.end(), q - eps,
                                    [](const SubInterval& s, double v) { return s.qHi < v; });
         it != intervals.end() && it->qHi <= q + eps; ++it) {
        if (std::abs(q - it->qHi) <= eps) {
            left = densityAtQuantile(it->segA, it->segB, t, q, targetMass);
            wL = it->qHi - it->qLo;
            break;
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

    computeSecantDensities(ctx, vertices, result);

    result.alpha.assign(n, 0.0);
    for (std::size_t i = 0; i < n; ++i) {
        result.alpha[i] = vertexDensityClosedForm(intervals, vertices[i].q, ctx.targetMass, ctx.t);
    }

    // Boundary conditions: zero out if either endpoint of either TF has zero alpha.
    const BoundaryZeroFlags boundary = computeBoundaryZeroFlags(ctx);
    const bool zeroAtStart = boundary.atStart;
    const bool zeroAtEnd = boundary.atEnd;

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
        rescaleComponentMass(ctx, vertices, result, compStart, compEnd);
    }

    for (auto& aVal : result.alpha) aVal = std::max(0.0, aVal);
    return result;
}

// Build a per-interval (segA, segB) lookup keyed by qLo. levels is assumed sorted
// strictly ascending and to contain every breakpoint of L plus any refinements;
// each consecutive pair lies entirely inside one (A_i, B_j) pairing.
std::vector<SubInterval> buildSubIntervalsFromLevels(const TransportContext& ctx,
                                                     const std::vector<double>& levels,
                                                     bool computeInversion) {
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
        if (computeInversion) {
            sub.inv = precomputeInversion(sub.segA, sub.segB, ctx.t);
        }
        intervals.push_back(sub);
    }
    return intervals;
}

// Refine the quantile level set by bisecting sub-intervals where the straight line
// between closed-form endpoint opacities deviates from the closed-form opacity at the
// sub-interval midpoint by more than relTol (relative error, measured in x).
// As relTol -> 0 the piecewise-linear knot output converges to the continuous alpha_t.
//
// Sub-intervals where the A and B segment slopes have opposite signs are refined
// first in practice: X_t'(q) can have an interior extremum there, producing an
// interior opacity minimum or maximum that a coarse level set cannot represent.
void refineQuantileLevelsByClosedForm(const TransportContext& ctx, std::vector<double>& levels,
                                      double relTol, std::size_t maxLevels,
                                      std::size_t maxIterations) {
    if (relTol <= 0.0) return;

    // Incremental refinement. Each sub-interval lies entirely within a single
    // (A_i, B_j) knot-segment pairing, so splitting it at its midpoint produces two
    // children that inherit the parent's segA/segB unchanged. We therefore build the
    // sub-interval table once and, on every iteration, recompute the closed-form
    // midpoint error for only the two children of the interval we split, instead of
    // rebuilding and re-evaluating the sqrt-heavy densities for every interval. The
    // selection (strict greater-than, lowest-qLo wins ties) and the inserted levels
    // are identical to the rebuild-every-iteration formulation.
    struct RefineInterval {
        double qLo = 0.0;
        double qHi = 0.0;
        double xLo = 0.0;
        double xHi = 0.0;
        CdfSegment segA;
        CdfSegment segB;
        double relErr = -1.0;  // < 0 means "skip" (matches buildSubIntervals filtering)
    };

    const auto computeErr = [&](RefineInterval& s) {
        if (s.qHi - s.qLo <= eps) {
            s.relErr = -1.0;
            return;
        }
        const double qMid = 0.5 * (s.qLo + s.qHi);
        const double aLo = densityAtQuantile(s.segA, s.segB, ctx.t, s.qLo, ctx.targetMass);
        const double aHi = densityAtQuantile(s.segA, s.segB, ctx.t, s.qHi, ctx.targetMass);
        const double aMid = densityAtQuantile(s.segA, s.segB, ctx.t, qMid, ctx.targetMass);
        if (aMid <= 0.0 && aLo <= 0.0 && aHi <= 0.0) {
            s.relErr = -1.0;
            return;
        }
        const double xMid = xtOnSegment(s.segA, s.segB, ctx.t, qMid);
        const double dx = s.xHi - s.xLo;
        if (dx <= eps) {
            s.relErr = -1.0;
            return;
        }
        const double aMidLinear = aLo + (aHi - aLo) * (xMid - s.xLo) / dx;
        const double absErr = std::abs(aMid - aMidLinear);
        const double scale = std::max({std::abs(aMid), std::abs(aMidLinear), eps});
        s.relErr = absErr / scale;
    };

    const auto subs = buildSubIntervalsFromLevels(ctx, levels);
    std::vector<RefineInterval> ris;
    ris.reserve(subs.size() + maxIterations);
    for (const auto& s : subs) {
        RefineInterval ri;
        ri.qLo = s.qLo;
        ri.qHi = s.qHi;
        ri.xLo = s.xLo;
        ri.xHi = s.xHi;
        ri.segA = s.segA;
        ri.segB = s.segB;
        computeErr(ri);
        ris.push_back(ri);
    }

    for (std::size_t iter = 0; iter < maxIterations; ++iter) {
        if (levels.size() >= maxLevels) break;

        double maxRelError = 0.0;
        std::size_t worst = std::numeric_limits<std::size_t>::max();
        for (std::size_t i = 0; i < ris.size(); ++i) {
            if (ris[i].relErr > maxRelError) {
                maxRelError = ris[i].relErr;
                worst = i;
            }
        }
        if (worst == std::numeric_limits<std::size_t>::max() || maxRelError <= relTol) break;

        // Capture parent data before any mutation/insertion invalidates references.
        const double parentQHi = ris[worst].qHi;
        const double parentXHi = ris[worst].xHi;
        const CdfSegment segA = ris[worst].segA;
        const CdfSegment segB = ris[worst].segB;
        const double qMid = 0.5 * (ris[worst].qLo + parentQHi);

        const std::size_t before = levels.size();
        insertQuantileLevel(levels, qMid);
        if (levels.size() == before) break;  // duplicate level: refinement frozen

        const double xMid = xtOnSegment(segA, segB, ctx.t, qMid);

        // Left child reuses the parent slot.
        ris[worst].qHi = qMid;
        ris[worst].xHi = xMid;
        computeErr(ris[worst]);

        // Right child inherits the same pairing.
        RefineInterval right;
        right.qLo = qMid;
        right.qHi = parentQHi;
        right.xLo = xMid;
        right.xHi = parentXHi;
        right.segA = segA;
        right.segB = segB;
        computeErr(right);

        ris.insert(ris.begin() + static_cast<std::ptrdiff_t>(worst) + 1, right);
    }
}

// ---------------------------------------------------------------------------
// Closed-form maximiser of the opacity-chord error on a single sub-interval.
//
// On a sub-interval of L (one source segment, one destination segment) the
// transported opacity is alpha_t(X_t(q)) = m_t / g(q) with g(q) = X_t'(q), and
// the piecewise-linear output draws the chord that is linear in x = X_t(q):
//
//   chord(x) = alpha(qLo) + w (x - xLo),  w = (alpha(qHi) - alpha(qLo))/(xHi - xLo).
//
// The error e(q) = alpha_t(X_t(q)) - chord(X_t(q)) vanishes at both endpoints, so
// its extremum is interior. Because x'(q) = g(q), stationarity e'(q) = 0 is
//
//   (*)  m_t g'(q) + w g(q)^3 = 0          <=>   d(alpha)/dx = w,
//
// i.e. the mean-value/tangent condition: the worst chord error sits where the
// curve's slope in x equals the chord slope. We solve (*) in closed form.
//
// Single-radical case (exactly one segment curved): writing the active radical as
// rho = sqrt(p0 + p1 q), the stretch is g = d1 + k/rho with k = weight*mCurved, and
// (*) collapses to a CUBIC in rho:
//
//   w (d1 rho + k)^3 = m_t k p1 / 2,
//
// solved exactly (Cardano/trigonometric). Then q* = (rho*^2 - p0)/p1.
//
// Two-non-zero-slope case: the two radicals couple, so (*) is a higher-degree
// polynomial in one radical variable. We seed q from the single-radical cubic of
// each term (freezing the other at the interval midpoint) and polish with
// safeguarded Newton on (*) using the analytic e', e''. All evaluations are
// closed form; no inversion of X_t is required.

// Real roots of a*rho^3 + b*rho^2 + c*rho + d = 0 (up to three).
struct CubicRoots {
    double r[3] = {std::numeric_limits<double>::quiet_NaN(),
                   std::numeric_limits<double>::quiet_NaN(),
                   std::numeric_limits<double>::quiet_NaN()};
    int count = 0;
};

CubicRoots solveCubic(double a, double b, double c, double d) {
    CubicRoots out;
    const double scale = std::max({std::abs(a), std::abs(b), std::abs(c), std::abs(d), 1.0});
    if (std::abs(a) <= 1e-14 * scale) {
        const auto qr = solveQuadratic(b, c, d);
        if (!std::isnan(qr.r0)) out.r[out.count++] = qr.r0;
        if (!std::isnan(qr.r1)) out.r[out.count++] = qr.r1;
        return out;
    }
    const double p2 = b / a;
    const double p1 = c / a;
    const double p0 = d / a;
    const double shift = p2 / 3.0;
    const double P = p1 - p2 * p2 / 3.0;
    const double Q = 2.0 * p2 * p2 * p2 / 27.0 - p2 * p1 / 3.0 + p0;
    const double disc = (Q * Q) / 4.0 + (P * P * P) / 27.0;
    if (disc > 0.0) {
        const double sq = std::sqrt(disc);
        const double u = std::cbrt(-Q / 2.0 + sq);
        const double v = std::cbrt(-Q / 2.0 - sq);
        out.r[out.count++] = u + v - shift;
    } else if (disc < 0.0) {
        const double m = 2.0 * std::sqrt(-P / 3.0);
        const double arg = std::clamp(3.0 * Q / (P * m), -1.0, 1.0);
        const double theta = std::acos(arg) / 3.0;
        constexpr double twoPiOver3 = 2.0943951023931953;
        out.r[out.count++] = m * std::cos(theta) - shift;
        out.r[out.count++] = m * std::cos(theta - twoPiOver3) - shift;
        out.r[out.count++] = m * std::cos(theta - 2.0 * twoPiOver3) - shift;
    } else {
        const double u = std::cbrt(-Q / 2.0);
        out.r[out.count++] = 2.0 * u - shift;
        out.r[out.count++] = -u - shift;
    }
    return out;
}

// Per-interval closed forms of g(q)=X_t'(q) and its derivatives, reconstructed from
// the same segment constants used by densityAtQuantile / xtOnSegment.
struct StretchClosedForm {
    bool valid = false;
    bool bothFlat = false;
    bool aFlat = false;
    bool bFlat = false;
    double oneMt = 0.0, tt = 0.0, mA = 0.0, mB = 0.0;
    double pa0 = 0.0, pa1 = 0.0, pb0 = 0.0, pb1 = 0.0;
    double aFlatVal = 0.0, bFlatVal = 0.0;
};

StretchClosedForm makeStretch(const CdfSegment& segA, const CdfSegment& segB, double t) {
    StretchClosedForm s;
    s.oneMt = 1.0 - t;
    s.tt = t;
    s.mA = segA.mass;
    s.mB = segB.mass;
    s.aFlat = std::abs(segA.slope) <= eps;
    s.bFlat = std::abs(segB.slope) <= eps;
    s.aFlatVal = segA.alphaStart;
    s.bFlatVal = segB.alphaStart;
    if (!s.aFlat) {
        s.pa0 = segA.alphaStart * segA.alphaStart - 2.0 * segA.slope * segA.cdfAtStart;
        s.pa1 = 2.0 * segA.slope * segA.mass;
    }
    if (!s.bFlat) {
        s.pb0 = segB.alphaStart * segB.alphaStart - 2.0 * segB.slope * segB.cdfAtStart;
        s.pb1 = 2.0 * segB.slope * segB.mass;
    }
    s.bothFlat = s.aFlat && s.bFlat;
    s.valid = true;
    return s;
}

double stretchG(const StretchClosedForm& s, double q) {
    const double aA = s.aFlat ? s.aFlatVal : std::sqrt(std::max(0.0, s.pa0 + s.pa1 * q));
    const double aB = s.bFlat ? s.bFlatVal : std::sqrt(std::max(0.0, s.pb0 + s.pb1 * q));
    double g = 0.0;
    if (aA > eps) g += s.oneMt * s.mA / aA;
    if (aB > eps) g += s.tt * s.mB / aB;
    return g;
}

double stretchGPrime(const StretchClosedForm& s, double q) {
    double gp = 0.0;
    if (!s.aFlat) {
        const double pa = std::max(eps, s.pa0 + s.pa1 * q);
        gp += -s.oneMt * s.mA * s.pa1 / (2.0 * pa * std::sqrt(pa));
    }
    if (!s.bFlat) {
        const double pb = std::max(eps, s.pb0 + s.pb1 * q);
        gp += -s.tt * s.mB * s.pb1 / (2.0 * pb * std::sqrt(pb));
    }
    return gp;
}

double stretchGpp(const StretchClosedForm& s, double q) {
    double gpp = 0.0;
    if (!s.aFlat) {
        const double pa = std::max(eps, s.pa0 + s.pa1 * q);
        gpp += 0.75 * s.oneMt * s.mA * s.pa1 * s.pa1 / (pa * pa * std::sqrt(pa));
    }
    if (!s.bFlat) {
        const double pb = std::max(eps, s.pb0 + s.pb1 * q);
        gpp += 0.75 * s.tt * s.mB * s.pb1 * s.pb1 / (pb * pb * std::sqrt(pb));
    }
    return gpp;
}

// Solve the exact cubic (*) for the single-active-radical configuration `sf`
// (exactly one of the two sides is flat). Appends interior roots in (qLo,qHi).
void singleRadicalCandidates(const StretchClosedForm& sf, double w, double mt, double qLo,
                             double qHi, std::vector<double>& cand) {
    double d1 = 0.0, p0 = 0.0, p1 = 0.0, k = 0.0;
    if (sf.aFlat && !sf.bFlat) {
        if (sf.aFlatVal > eps) d1 = sf.oneMt * sf.mA / sf.aFlatVal;
        p0 = sf.pb0;
        p1 = sf.pb1;
        k = sf.tt * sf.mB;  // g = d1 + k/rho, rho = sqrt(p0+p1 q)
    } else if (sf.bFlat && !sf.aFlat) {
        if (sf.bFlatVal > eps) d1 = sf.tt * sf.mB / sf.bFlatVal;
        p0 = sf.pa0;
        p1 = sf.pa1;
        k = sf.oneMt * sf.mA;
    } else {
        return;
    }
    if (std::abs(p1) <= eps || std::abs(k) <= eps) return;
    // w (d1 rho + k)^3 = m_t k p1 / 2  ->  cubic in rho.
    const double A3 = w * d1 * d1 * d1;
    const double A2 = 3.0 * w * d1 * d1 * k;
    const double A1 = 3.0 * w * d1 * k * k;
    const double A0 = w * k * k * k - mt * k * p1 / 2.0;
    const auto roots = solveCubic(A3, A2, A1, A0);
    for (int i = 0; i < roots.count; ++i) {
        const double rho = roots.r[i];
        if (std::isnan(rho) || rho <= 0.0) continue;
        const double q = (rho * rho - p0) / p1;
        if (q > qLo + eps && q < qHi - eps) cand.push_back(q);
    }
}

// Interior q in (qLo,qHi) maximising |e(q)|, in closed form where possible.
// Returns NaN if no interior stationary point exists (e.g. both segments flat).
double optimalErrorQuantile(const CdfSegment& segA, const CdfSegment& segB, double t, double mt,
                            double qLo, double qHi, double xLo, double xHi) {
    const double dx = xHi - xLo;
    if (dx <= eps || qHi - qLo <= eps) return std::numeric_limits<double>::quiet_NaN();

    const StretchClosedForm s = makeStretch(segA, segB, t);
    if (!s.valid || s.bothFlat) return std::numeric_limits<double>::quiet_NaN();

    const double gLo = stretchG(s, qLo);
    const double gHi = stretchG(s, qHi);
    if (gLo <= eps || gHi <= eps) return std::numeric_limits<double>::quiet_NaN();
    const double aLo = mt / gLo;
    const double aHi = mt / gHi;
    const double w = (aHi - aLo) / dx;

    const auto eDeriv = [&](double q) {
        const double g = stretchG(s, q);
        if (g <= eps) return 0.0;
        const double gp = stretchGPrime(s, q);
        return -mt * gp / (g * g) - w * g;  // d(alpha)/dq - w x'(q)
    };
    const auto eDeriv2 = [&](double q) {
        const double g = stretchG(s, q);
        if (g <= eps) return 0.0;
        const double gp = stretchGPrime(s, q);
        const double gpp = stretchGpp(s, q);
        return -mt * (gpp / (g * g) - 2.0 * gp * gp / (g * g * g)) - w * gp;
    };
    const auto absErr = [&](double q) {
        const double g = stretchG(s, q);
        if (g <= eps) return 0.0;
        const double a = mt / g;
        const double x = (1.0 - t) * quantileOnSegment(segA, q) + t * quantileOnSegment(segB, q);
        const double chord = aLo + w * (x - xLo);
        return std::abs(a - chord);
    };

    std::vector<double> cand;
    cand.reserve(6);

    if (s.aFlat != s.bFlat) {
        singleRadicalCandidates(s, w, mt, qLo, qHi, cand);  // exact
    } else {
        const double qMid = 0.5 * (qLo + qHi);
        for (int side = 0; side < 2; ++side) {
            StretchClosedForm sf = s;
            if (side == 0) {
                sf.aFlat = true;
                sf.aFlatVal = std::sqrt(std::max(eps, s.pa0 + s.pa1 * qMid));
            } else {
                sf.bFlat = true;
                sf.bFlatVal = std::sqrt(std::max(eps, s.pb0 + s.pb1 * qMid));
            }
            singleRadicalCandidates(sf, w, mt, qLo, qHi, cand);
        }
        if (cand.empty()) cand.push_back(qMid);

        // Safeguarded Newton on e'(q)=0 using analytic e''.
        for (double& q : cand) {
            const double lo = qLo + eps;
            const double hi = qHi - eps;
            q = std::clamp(q, lo, hi);
            for (int it = 0; it < 24; ++it) {
                const double f = eDeriv(q);
                const double df = eDeriv2(q);
                if (std::abs(df) <= 1e-12) break;
                double qNext = q - f / df;
                if (!(qNext > lo && qNext < hi)) qNext = std::clamp(qNext, lo, hi);
                const bool converged = std::abs(qNext - q) < 1e-10;
                q = qNext;
                if (converged) break;
            }
        }
    }

    double bestQ = std::numeric_limits<double>::quiet_NaN();
    double bestE = 0.0;
    for (double q : cand) {
        if (!(q > qLo + eps && q < qHi - eps)) continue;
        const double e = absErr(q);
        if (e > bestE) {
            bestE = e;
            bestQ = q;
        }
    }
    return bestQ;
}

// Error-optimal sample placement. Like refineQuantileLevelsByClosedForm, but instead
// of probing the geometric midpoint, each sub-interval is scored by the *maximum*
// opacity deviation of the piecewise-linear chord from the exact transported opacity,
//
//   e(q) = | alpha_t(X_t(q)) - chord(X_t(q)) |,  alpha_t(X_t(q)) = m_t / X_t'(q),
//
// where chord is the straight line in x connecting the endpoint opacities (the line
// the output actually draws on the interval). Both X_t(q) and X_t'(q) are closed
// form on the interval (one source and one destination knot segment), so e(q) is
// evaluated directly with no inversion of X_t. e vanishes at both endpoints (the
// chord matches the exact opacity there by construction), so its maximiser q* is
// interior and satisfies the tangent condition d(alpha)/dx = w. optimalErrorQuantile
// solves that condition in closed form (an exact cubic in the radical variable for
// single-curved-segment intervals, a closed-form seed plus safeguarded Newton when
// both segments are curved), and q* is inserted as the new level. Splitting at the
// error maximiser removes the largest remaining deviation per inserted control point.
void refineQuantileLevelsByMaxError(const TransportContext& ctx, std::vector<double>& levels,
                                    double relTol, std::size_t maxLevels,
                                    std::size_t maxIterations) {
    if (relTol <= 0.0) return;

    struct RefineInterval {
        double qLo = 0.0;
        double qHi = 0.0;
        double xLo = 0.0;
        double xHi = 0.0;
        CdfSegment segA;
        CdfSegment segB;
        double relErr = -1.0;  // < 0 means "skip" (matches buildSubIntervals filtering)
        double qStar = 0.0;    // quantile of the maximum deviation within [qLo, qHi]
    };

    // Locate q* in (qLo, qHi) maximising |deviation| in closed form (optimalErrorQuantile)
    // and record the relative error there. If no interior maximiser exists (flat regions),
    // the interval contributes zero error and is skipped.
    const auto computeErr = [&](RefineInterval& s) {
        s.qStar = 0.5 * (s.qLo + s.qHi);
        if (s.qHi - s.qLo <= eps) {
            s.relErr = -1.0;
            return;
        }
        const double dx = s.xHi - s.xLo;
        if (dx <= eps) {
            s.relErr = -1.0;
            return;
        }
        const double aLo = densityAtQuantile(s.segA, s.segB, ctx.t, s.qLo, ctx.targetMass);
        const double aHi = densityAtQuantile(s.segA, s.segB, ctx.t, s.qHi, ctx.targetMass);

        // Closed-form maximiser of |e(q)| via the tangent condition d(alpha)/dx = w.
        const double qStar =
            optimalErrorQuantile(s.segA, s.segB, ctx.t, ctx.targetMass, s.qLo, s.qHi, s.xLo, s.xHi);
        if (std::isnan(qStar)) {
            // No interior stationary point: alpha is affine in x on this interval, so the
            // endpoint chord is exact and the interval needs no refinement.
            s.relErr = -1.0;
            return;
        }
        s.qStar = std::clamp(qStar, s.qLo, s.qHi);

        const double aStar = densityAtQuantile(s.segA, s.segB, ctx.t, s.qStar, ctx.targetMass);
        const double xStar = xtOnSegment(s.segA, s.segB, ctx.t, s.qStar);
        const double chordStar = aLo + (aHi - aLo) * (xStar - s.xLo) / dx;
        if (aStar <= 0.0 && aLo <= 0.0 && aHi <= 0.0) {
            s.relErr = -1.0;
            return;
        }
        const double absErr = std::abs(aStar - chordStar);
        const double scale = std::max({std::abs(aStar), std::abs(chordStar), eps});
        s.relErr = absErr / scale;
    };

    const auto subs = buildSubIntervalsFromLevels(ctx, levels);
    std::vector<RefineInterval> ris;
    ris.reserve(subs.size() + maxIterations);
    for (const auto& s : subs) {
        RefineInterval ri;
        ri.qLo = s.qLo;
        ri.qHi = s.qHi;
        ri.xLo = s.xLo;
        ri.xHi = s.xHi;
        ri.segA = s.segA;
        ri.segB = s.segB;
        computeErr(ri);
        ris.push_back(ri);
    }

    for (std::size_t iter = 0; iter < maxIterations; ++iter) {
        if (levels.size() >= maxLevels) break;

        double maxRelError = 0.0;
        std::size_t worst = std::numeric_limits<std::size_t>::max();
        for (std::size_t i = 0; i < ris.size(); ++i) {
            if (ris[i].relErr > maxRelError) {
                maxRelError = ris[i].relErr;
                worst = i;
            }
        }
        if (worst == std::numeric_limits<std::size_t>::max() || maxRelError <= relTol) break;

        // Capture parent data before any mutation/insertion invalidates references.
        const double parentQHi = ris[worst].qHi;
        const double parentXHi = ris[worst].xHi;
        const CdfSegment segA = ris[worst].segA;
        const CdfSegment segB = ris[worst].segB;
        double qSplit = ris[worst].qStar;
        // Guard against a degenerate maximiser landing on an endpoint: fall back to the
        // midpoint so the split always makes progress.
        if (!(qSplit > ris[worst].qLo + eps) || !(qSplit < parentQHi - eps)) {
            qSplit = 0.5 * (ris[worst].qLo + parentQHi);
        }

        const std::size_t before = levels.size();
        insertQuantileLevel(levels, qSplit);
        if (levels.size() == before) break;  // duplicate level: refinement frozen

        const double xSplit = xtOnSegment(segA, segB, ctx.t, qSplit);

        // Left child reuses the parent slot.
        ris[worst].qHi = qSplit;
        ris[worst].xHi = xSplit;
        computeErr(ris[worst]);

        // Right child inherits the same pairing.
        RefineInterval right;
        right.qLo = qSplit;
        right.qHi = parentQHi;
        right.xLo = xSplit;
        right.xHi = parentXHi;
        right.segA = segA;
        right.segB = segB;
        computeErr(right);

        ris.insert(ris.begin() + static_cast<std::ptrdiff_t>(worst) + 1, right);
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

// Shared driver for every optimal-transport interpolation entry point. The public variants
// differ only in how they pick quantile levels and reconstruct vertex opacities; everything
// around that is identical: clamp t, capture the interior support, sanitise the inputs (which
// makes clamp-to-edge support explicit), handle the degenerate early-outs, guard against
// vanishing mass, build the transport context and interpolated domain bounds, and finally trim
// the synthetic boundary vertices back off the result so the returned TF spans only the
// transported interior support again. The reconstruct callable receives the transport context
// and the interpolated domain bounds and returns the output vertices for its sampling strategy.
template <typename Reconstruct>
std::vector<TFPrimitiveData> optimalTransportInterpolationImpl(std::span<const TFPrimitiveData> tfA,
                                                               std::span<const TFPrimitiveData> tfB,
                                                               double t, Reconstruct&& reconstruct) {
    t = std::clamp(t, 0.0, 1.0);

    const auto [interiorMinA, interiorMaxA] = minMaxPos(tfA);
    const auto [interiorMinB, interiorMaxB] = minMaxPos(tfB);

    auto a = sanitize(tfA);
    auto b = sanitize(tfB);

    const std::span<const TFPrimitiveData> aspan{a.data(), a.size()};
    const std::span<const TFPrimitiveData> bspan{b.data(), b.size()};

    if (a.empty()) return trimSyntheticEdges(std::move(b), interiorMinB, interiorMaxB);
    if (b.empty()) return trimSyntheticEdges(std::move(a), interiorMinA, interiorMaxA);
    if (t <= 0.0) return trimSyntheticEdges(std::move(a), interiorMinA, interiorMaxA);
    if (t >= 1.0) return trimSyntheticEdges(std::move(b), interiorMinB, interiorMaxB);

    const Cdf cdfA = computeCdf(aspan);
    const Cdf cdfB = computeCdf(bspan);

    if (cdfA.totalMass <= eps || cdfB.totalMass <= eps ||
        (1.0 - t) * cdfA.totalMass + t * cdfB.totalMass <= eps) {
        return linearBlend(aspan, bspan, t);
    }

    const TransportContext ctx = makeTransportContext(aspan, bspan, cdfA, cdfB, t);

    const double domainMin = (1.0 - t) * a.front().pos + t * b.front().pos;
    const double domainMax = (1.0 - t) * a.back().pos + t * b.back().pos;

    auto result = reconstruct(ctx, domainMin, domainMax);

    const double trimMin = (1.0 - t) * interiorMinA + t * interiorMinB;
    const double trimMax = (1.0 - t) * interiorMaxA + t * interiorMaxB;
    return trimSyntheticEdges(std::move(result), trimMin, trimMax);
}

}  // namespace

// ===========================================================================
// Public API
// ===========================================================================
//
// Four quantile-sampling strategies are provided for comparison:
//
//   optimalTransportInterpolationUniformQ              — uniform q grid, secant opacities
//   optimalTransportInterpolationUniformQExact         — uniform q grid, exact opacities
//   optimalTransportInterpolationAdaptiveQSecant       — adaptive q (midpoint), secant opacities
//   optimalTransportInterpolationAdaptiveQExact        — adaptive q (midpoint), exact opacities
//   optimalTransportInterpolationOpacityOptimalQExact  — opacity-optimal q, exact opacities

std::vector<TFPrimitiveData> optimalTransportInterpolationUniformQ(
    std::span<const TFPrimitiveData> tfA, std::span<const TFPrimitiveData> tfB, double t,
    std::size_t samplesPerSegment) {
    return optimalTransportInterpolationImpl(
        tfA, tfB, t,
        [&](const TransportContext& ctx, double domainMin, double domainMax) {
            // Legacy uniform quantile sampling: sub-divide each CDF segment into
            // samplesPerSegment equal steps in q. Kept unchanged for comparison with the
            // adaptive paths below.
            const auto levels = mergedQuantileLevels(ctx.cdfA, ctx.cdfB, samplesPerSegment);
            return optimalTransportInterpolationFromLevels(ctx, levels, domainMin, domainMax);
        });
}

std::vector<TFPrimitiveData> optimalTransportInterpolationUniformQExact(
    std::span<const TFPrimitiveData> tfA, std::span<const TFPrimitiveData> tfB, double t,
    std::size_t samplesPerSegment) {
    return optimalTransportInterpolationImpl(
        tfA, tfB, t,
        [&](const TransportContext& ctx, double domainMin, double domainMax) {
            // Same uniform quantile grid as optimalTransportInterpolationUniformQ, but
            // reconstruct vertex opacities with the exact closed-form alpha_k = m_t / X_t'(q_k).
            const auto levels = mergedQuantileLevels(ctx.cdfA, ctx.cdfB, samplesPerSegment);
            return optimalTransportInterpolationClosedFormFromLevels(ctx, levels, domainMin,
                                                                     domainMax);
        });
}

std::vector<TFPrimitiveData> optimalTransportInterpolationAdaptiveQExact(
    std::span<const TFPrimitiveData> tfA, std::span<const TFPrimitiveData> tfB, double t,
    const QuantileRefinementOptions& opts) {
    return optimalTransportInterpolationImpl(
        tfA, tfB, t,
        [&](const TransportContext& ctx, double domainMin, double domainMax) {
            // Knot-induced quantile levels, then bisect sub-intervals where the
            // piecewise-linear chord between endpoint opacities deviates from the closed-form
            // opacity at the midpoint (relative tolerance, measured in x). This targets
            // sub-intervals where X_t'(q) curves most strongly, e.g. when the paired segment
            // slopes have opposite sign.
            auto levels = knotInducedQuantileLevels(ctx.cdfA, ctx.cdfB);
            refineQuantileLevelsByClosedForm(ctx, levels, opts.relativeTolerance,
                                             opts.maxQuantileLevels,
                                             opts.maxRefinementIterations);
            return optimalTransportInterpolationClosedFormFromLevels(ctx, levels, domainMin,
                                                                     domainMax);
        });
}

std::vector<TFPrimitiveData> optimalTransportInterpolationOpacityOptimalQExact(
    std::span<const TFPrimitiveData> tfA, std::span<const TFPrimitiveData> tfB, double t,
    const QuantileRefinementOptions& opts) {
    return optimalTransportInterpolationImpl(
        tfA, tfB, t,
        [&](const TransportContext& ctx, double domainMin, double domainMax) {
            // Knot-induced quantile levels, then insert each new level at the quantile q* that
            // maximises the opacity deviation between the exact transported opacity and the
            // piecewise-linear chord on the worst sub-interval (relative tolerance, measured in
            // x). Unlike optimalTransportInterpolationAdaptiveQExact, which probes the geometric
            // midpoint, this places samples exactly where the chord error peaks, so the same
            // opacity tolerance is met with fewer control points. Vertex opacities remain the
            // exact closed-form alpha_k = m_t / X_t'(q_k).
            auto levels = knotInducedQuantileLevels(ctx.cdfA, ctx.cdfB);
            refineQuantileLevelsByMaxError(ctx, levels, opts.relativeTolerance,
                                           opts.maxQuantileLevels,
                                           opts.maxRefinementIterations);
            return optimalTransportInterpolationClosedFormFromLevels(ctx, levels, domainMin,
                                                                     domainMax);
        });
}

std::vector<TFPrimitiveData> optimalTransportInterpolationAdaptiveQSecant(
    std::span<const TFPrimitiveData> tfA, std::span<const TFPrimitiveData> tfB, double t,
    const QuantileRefinementOptions& opts) {
    return optimalTransportInterpolationImpl(
        tfA, tfB, t,
        [&](const TransportContext& ctx, double domainMin, double domainMax) {
            // Adaptive vertex placement identical to the closed-form path: start from the
            // knot-induced quantile breakpoints and bisect where the chord between exact
            // endpoint opacities deviates most from the exact transported density. Unlike the
            // closed-form path, the per-vertex opacities are then reconstructed with the cheap
            // secant-density scheme (optimalTransportInterpolationFromLevels) rather than the
            // exact alpha_k = m_t / X_t'(q_k). This isolates the benefit of adaptive placement
            // from the cost of exact opacities.
            auto levels = knotInducedQuantileLevels(ctx.cdfA, ctx.cdfB);
            refineQuantileLevelsByClosedForm(ctx, levels, opts.relativeTolerance,
                                             opts.maxQuantileLevels,
                                             opts.maxRefinementIterations);
            return optimalTransportInterpolationFromLevels(ctx, levels, domainMin, domainMax);
        });
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

    // Transfer function uses clamp-to-edge
    x = std::clamp(x, intervals.front().xLo, intervals.back().xHi);

    // Binary search the sub-interval that brackets x in output coordinates. The xLo
    // values are strictly increasing because X_t is monotone (it is a sum of two
    // monotone-in-q quantile functions).
    auto it = std::upper_bound(intervals.begin(), intervals.end(), x,
                               [](double xv, const SubInterval& s) { return xv < s.xHi; });
    if (it == intervals.end()) it = intervals.end() - 1;
    const SubInterval& sub = *it;

    const double q = invertXtOnSegmentPrecomputed(sub, t, x);
    if (std::isnan(q)) return 0.0;

    return densityAtQuantile(sub.segA, sub.segB, t, q, ctx.targetMass);
}

std::vector<double> evaluateInterpolatedAlphaGrid(std::span<const TFPrimitiveData> tfA,
                                                   std::span<const TFPrimitiveData> tfB, double t,
                                                   std::span<const double> xs) {
    std::vector<double> out(xs.size(), 0.0);
    if (xs.empty()) return out;

    t = std::clamp(t, 0.0, 1.0);

    auto a = sanitize(tfA);
    auto b = sanitize(tfB);

    const std::span<const TFPrimitiveData> aspan{a.data(), a.size()};
    const std::span<const TFPrimitiveData> bspan{b.data(), b.size()};

    if (a.empty() || b.empty()) return out;
    if (t <= 0.0) {
        for (std::size_t i = 0; i < xs.size(); ++i)
            out[i] = static_cast<double>(evaluate(aspan, xs[i]).a);
        return out;
    }
    if (t >= 1.0) {
        for (std::size_t i = 0; i < xs.size(); ++i)
            out[i] = static_cast<double>(evaluate(bspan, xs[i]).a);
        return out;
    }

    const Cdf cdfA = computeCdf(aspan);
    const Cdf cdfB = computeCdf(bspan);
    if (cdfA.totalMass <= eps || cdfB.totalMass <= eps) return out;

    const TransportContext ctx = makeTransportContext(aspan, bspan, cdfA, cdfB, t);
    const auto intervals = buildSubIntervals(ctx);
    if (intervals.empty()) return out;

    const double xMin = intervals.front().xLo;
    const double xMax = intervals.back().xHi;
    const double alphaAtMin = densityAtQuantile(intervals.front().segA, intervals.front().segB, t,
                                                intervals.front().qLo, ctx.targetMass);
    const double alphaAtMax = densityAtQuantile(intervals.back().segA, intervals.back().segB, t,
                                                intervals.back().qHi, ctx.targetMass);   

    std::size_t si = 0;  // current sub-interval index
    for (std::size_t i = 0; i < xs.size(); ++i) {
        const double x = xs[i];
        if (x <= xMin) {
            out[i] = alphaAtMin;
            continue;
        }
        if (x >= xMax) {
            out[i] = alphaAtMax;
            continue;
        }

        // Advance si to the first sub-interval whose xHi > x.
        while (si + 1 < intervals.size() && intervals[si].xHi <= x) ++si;

        const SubInterval& sub = intervals[si];
        if (x < sub.xLo) continue;  // sits in a gap → 0

        const double q = invertXtOnSegmentPrecomputed(sub, t, x);
        if (std::isnan(q)) continue;
        out[i] = densityAtQuantile(sub.segA, sub.segB, t, q, ctx.targetMass);
    }
    return out;
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
