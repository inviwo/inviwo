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

#pragma once

#include <cstddef>
#include <span>
#include <vector>

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/datastructures/tfprimitive.h>

namespace inviwo::algorithm {

/**
 * @brief Interpolate two transfer functions using 1D Optimal Transport (Earth Mover's Distance).
 *
 * Instead of linearly crossfading the two transfer functions (which causes spectral splitting
 * and ghosting artifacts), this uses displacement interpolation in the Wasserstein sense.
 * The alpha channel is treated as the mass distribution, and colors are transported along
 * with the mass.
 *
 * The algorithm:
 * 1. Treats the alpha channel of each TF as a piecewise linear density (PDF).
 * 2. Computes the cumulative distribution function (CDF) for each TF.
 * 3. Inverts the CDFs to obtain quantile functions.
 * 4. Linearly interpolates quantile positions: x_t(q) = (1-t)*Q_A(q) + t*Q_B(q).
 * 5. Interpolates the RGB color at each quantile point.
 * 6. Recovers the interpolated density by differentiating the interpolated CDF.
 * 7. Scales the result so total mass interpolates linearly between inputs.
 *
 * @param tfA   First transfer function as sorted TFPrimitiveData points.
 * @param tfB   Second transfer function as sorted TFPrimitiveData points.
 * @param t     Interpolation parameter in [0, 1]. t=0 returns tfA, t=1 returns tfB.
 * @return      The interpolated transfer function as a vector of TFPrimitiveData.
 */
IVW_CORE_API std::vector<TFPrimitiveData> optimalTransportInterpolation(
    std::span<const TFPrimitiveData> tfA, std::span<const TFPrimitiveData> tfB, double t,
    std::size_t samplesPerSegment = 16);

/**
 * @brief Controls adaptive refinement of the quantile level set used by
 * optimalTransportInterpolationClosedForm.
 *
 * Starting from knot-induced quantile breakpoints, refinement repeatedly
 * bisects the sub-interval whose piecewise-linear opacity (connecting exact
 * endpoint densities) deviates most from the exact transported density at the
 * sub-interval midpoint. Bisection stops when the worst relative error falls
 * below @p relativeTolerance or when @p maxQuantileLevels or
 * @p maxRefinementIterations is reached.
 */
struct IVW_CORE_API ClosedFormRefinementOptions {
    double relativeTolerance = 1e-3;
    std::size_t maxQuantileLevels = 2048;
    std::size_t maxRefinementIterations = 256;
};

/**
 * @brief Interpolate two transfer functions using 1D optimal transport with
 * closed-form vertex opacities.
 *
 * Like optimalTransportInterpolation, this treats each TF alpha channel as a
 * piecewise-linear density, builds the cumulative distributions, and forms the
 * Wasserstein displacement interpolation
 *   X_t(q) = (1-t) Q_A(q) + t Q_B(q)
 * at a finite set of quantile levels. Colors are transported with the same
 * quantile pairing.
 *
 * On each quantile sub-interval between consecutive levels, Q_A(q) and Q_B(q)
 * each lie on a single input knot segment. Within such a sub-interval, X_t(q)
 * is a sum of square-root terms in q (or a degenerate linear combination when
 * a segment has constant alpha). The opacity at a sampled quantile q_k is
 * therefore available in closed form as the exact transported density
 *   alpha_k = m_t / X_t'(q_k)
 *             = m_t / ((1-t) m_A / alpha_A(Q_A(q_k)) + t m_B / alpha_B(Q_B(q_k))),
 * where m_t = (1-t) m_A + t m_B. This differs from the secant-based path in
 * optimalTransportInterpolation, which approximates interval density by
 * m_t (Delta q / Delta x) and assigns vertex opacity via a width-weighted
 * average of adjacent interval densities.
 *
 * Quantile levels begin at the union of knot-induced quantiles from both
 * inputs and may be refined adaptively (see ClosedFormRefinementOptions). The
 * output is a piecewise-linear TF connecting the sampled (x_k, alpha_k) pairs;
 * each connected component is rescaled so that its trapezoidal mass matches
 * the share m_t sum(Delta q) assigned by X_t to that component.
 *
 * @param tfA  First transfer function as sorted TFPrimitiveData points.
 * @param tfB  Second transfer function as sorted TFPrimitiveData points.
 * @param t    Interpolation parameter in [0, 1]. t=0 returns tfA, t=1 returns tfB.
 * @param opts Refinement tolerance and limits for the quantile level set.
 * @return     Interpolated transfer function as a vector of TFPrimitiveData.
 */
IVW_CORE_API std::vector<TFPrimitiveData> optimalTransportInterpolationClosedForm(
    std::span<const TFPrimitiveData> tfA, std::span<const TFPrimitiveData> tfB, double t,
    const ClosedFormRefinementOptions& opts = {});

/**
 * @brief Interpolate two transfer functions using 1D optimal transport with
 * closed-form vertex opacities and error-optimal sample placement.
 *
 * Like optimalTransportInterpolationClosedForm this treats each TF alpha channel
 * as a piecewise-linear density, forms the Wasserstein displacement
 * interpolation X_t(q) = (1-t) Q_A(q) + t Q_B(q), and assigns each output vertex
 * the exact transported opacity alpha_k = m_t / X_t'(q_k). The two methods
 * differ only in *where* refinement inserts new quantile levels.
 *
 * optimalTransportInterpolationClosedForm probes the geometric midpoint of the
 * worst sub-interval. This method instead inserts at the quantile q* that
 * *maximises* the opacity deviation
 *   e(q) = | m_t / X_t'(q) - chord(X_t(q)) |
 * between the exact transported opacity and the straight line connecting the
 * sub-interval's endpoint opacities in x (the same chord the piecewise-linear
 * output draws). Because X_t(q) and X_t'(q) are closed form on each sub-interval
 * (one source and one destination knot segment), e(q) is evaluated directly with
 * no inversion of X_t; e vanishes at both endpoints, so its maximiser is interior
 * and is located by a bracketed 1D search (a short uniform scan that brackets the
 * dominant extremum followed by a golden-section refinement).
 *
 * Inserting at the error maximiser removes the largest remaining opacity
 * deviation per control point, so for a fixed opacity tolerance the worst-case
 * opacity error of the output is driven below @p opts.relativeTolerance with
 * fewer control points than midpoint bisection.
 *
 * @param tfA  First transfer function as sorted TFPrimitiveData points.
 * @param tfB  Second transfer function as sorted TFPrimitiveData points.
 * @param t    Interpolation parameter in [0, 1]. t=0 returns tfA, t=1 returns tfB.
 * @param opts Refinement limits and the opacity error threshold (relativeTolerance)
 *             that drives sample placement.
 * @return     Interpolated transfer function as a vector of TFPrimitiveData.
 */
IVW_CORE_API std::vector<TFPrimitiveData> optimalTransportInterpolationOptimalSampling(
    std::span<const TFPrimitiveData> tfA, std::span<const TFPrimitiveData> tfB, double t,
    const ClosedFormRefinementOptions& opts = {});

/**
 * @brief Interpolate two transfer functions using 1D optimal transport with
 * adaptive quantile-level refinement and secant-based opacity reconstruction.
 *
 * This is a hybrid of the two methods above. Like
 * optimalTransportInterpolationClosedForm it starts from the knot-induced
 * quantile breakpoints and adaptively bisects the sub-interval whose chord
 * between exact endpoint opacities deviates most from the exact transported
 * density at the midpoint (see ClosedFormRefinementOptions), so vertices are
 * placed where X_t'(q) curves most strongly instead of on a uniform grid.
 * However, the per-vertex opacity is then reconstructed with the cheap
 * secant-density scheme of optimalTransportInterpolation (interval density
 * m_t (Delta q / Delta x) with width-weighted vertex averaging) rather than the
 * closed-form alpha_k = m_t / X_t'(q_k).
 *
 * The intent is to isolate the benefit of adaptive vertex placement from the
 * cost of exact closed-form opacities: it reaches comparable accuracy to the
 * uniform PWL path at a far lower vertex count without the per-vertex
 * quadratic-root solves of the closed-form path.
 *
 * @param tfA  First transfer function as sorted TFPrimitiveData points.
 * @param tfB  Second transfer function as sorted TFPrimitiveData points.
 * @param t    Interpolation parameter in [0, 1]. t=0 returns tfA, t=1 returns tfB.
 * @param opts Refinement tolerance and limits for the quantile level set.
 * @return     Interpolated transfer function as a vector of TFPrimitiveData.
 */
IVW_CORE_API std::vector<TFPrimitiveData> optimalTransportInterpolationRefined(
    std::span<const TFPrimitiveData> tfA, std::span<const TFPrimitiveData> tfB, double t,
    const ClosedFormRefinementOptions& opts = {});

/**
 * @brief Evaluate the closed-form interpolated opacity alpha_t(x) at one scalar.
 *
 * Assumes piecewise-linear input alphas and the same Wasserstein quantile
 * interpolation as optimalTransportInterpolationClosedForm, but evaluates
 * alpha_t(x) = m_t / X_t'(q) directly at x rather than returning control-point samples.
 *
 * The implementation locates the quantile sub-interval whose transported image
 * [xLo, xHi] contains x, inverts X_t(q) = x on that sub-interval (reducing to a
 * quadratic when both paired input segments have non-zero slope), and returns
 * the harmonic-mean density m_t / X_t'(q) at the recovered q. Returns 0 outside
 * the union of the transported widget supports (including gap regions between
 * disjoint peaks).
 *
 * @param tfA  First transfer function.
 * @param tfB  Second transfer function.
 * @param t    Interpolation parameter in [0, 1].
 * @param x    Scalar position at which to evaluate opacity.
 * @return     Interpolated opacity alpha_t(x), or 0 if x lies outside support.
 */
IVW_CORE_API double evaluateInterpolatedAlpha(std::span<const TFPrimitiveData> tfA,
                                              std::span<const TFPrimitiveData> tfB, double t,
                                              double x);

/**
 * @brief Evaluate alpha_t(x) on a dense sorted grid, reusing setup across all points.
 *
 * Equivalent to calling evaluateInterpolatedAlpha for every element of @p xs, but
 * significantly faster for large grids: sanitisation, CDF construction, and the
 * sub-interval table are computed once (O(N log N)) and the grid is swept with a
 * single co-advancing scan over sub-intervals (O(G + N) after setup), where G is
 * the number of grid points.
 *
 * @param tfA  First transfer function.
 * @param tfB  Second transfer function.
 * @param t    Interpolation parameter in [0, 1].
 * @param xs   Query positions in ascending order.
 * @return     Vector of length xs.size() with alpha_t(xs[i]) at each position.
 */
IVW_CORE_API std::vector<double> evaluateInterpolatedAlphaGrid(
    std::span<const TFPrimitiveData> tfA, std::span<const TFPrimitiveData> tfB, double t,
    std::span<const double> xs);

/**
 * @brief Compute the Earth Mover's Distance (1-Wasserstein) between two transfer functions.
 *
 * Measures the cost of transforming the alpha distribution of tfA into tfB.
 * Uses the closed-form 1D solution: EMD = integral |CDF_A(x) - CDF_B(x)| dx.
 *
 * @param tfA   First transfer function as sorted TFPrimitiveData points.
 * @param tfB   Second transfer function as sorted TFPrimitiveData points.
 * @return      The Earth Mover's Distance (non-negative).
 */
IVW_CORE_API double earthMoversDistance(std::span<const TFPrimitiveData> tfA,
                                        std::span<const TFPrimitiveData> tfB,
                                        std::size_t samplesPerSegment = 16);

}  // namespace inviwo::algorithm
