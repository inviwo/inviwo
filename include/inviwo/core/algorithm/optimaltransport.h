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
 * @brief Controls adaptive refinement of the quantile level set used by the
 * adaptive-q and opacity-optimal-q interpolators.
 */
struct IVW_CORE_API QuantileRefinementOptions {
    double relativeTolerance = 1e-3;
    std::size_t maxQuantileLevels = 2048;
    std::size_t maxRefinementIterations = 256;
};

/**
 * @brief Interpolate two transfer functions using 1D Optimal Transport (Earth Mover's Distance).
 * Uses uniform-q sampling with secant opacity reconstruction.
 *
 * @param tfA   First transfer function as sorted TFPrimitiveData points.
 * @param tfB   Second transfer function as sorted TFPrimitiveData points.
 * @param t     Interpolation parameter in [0, 1]. t=0 returns tfA, t=1 returns tfB.
 * @return      The interpolated transfer function as a vector of TFPrimitiveData.
 * @see optimalTransportInterpolationOpacityOptimalQExact
 */
IVW_CORE_API std::vector<TFPrimitiveData> optimalTransportInterpolationUniformQ(
    std::span<const TFPrimitiveData> tfA, std::span<const TFPrimitiveData> tfB, double t,
    std::size_t samplesPerSegment = 16);

/**
 * @brief Uniform-q sampling with exact opacity reconstruction.
 * @see optimalTransportInterpolationOpacityOptimalQExact
 */
IVW_CORE_API std::vector<TFPrimitiveData> optimalTransportInterpolationUniformQExact(
    std::span<const TFPrimitiveData> tfA, std::span<const TFPrimitiveData> tfB, double t,
    std::size_t samplesPerSegment = 16);

/**
 * @brief Adaptive-q sampling (midpoint bisection) with secant opacity reconstruction.
 * @see optimalTransportInterpolationOpacityOptimalQExact
 */
IVW_CORE_API std::vector<TFPrimitiveData> optimalTransportInterpolationAdaptiveQSecant(
    std::span<const TFPrimitiveData> tfA, std::span<const TFPrimitiveData> tfB, double t,
    const QuantileRefinementOptions& opts = {});

/**
 * @brief Adaptive-q sampling (midpoint bisection) with exact opacity reconstruction.
 *
 * @param tfA  First transfer function as sorted TFPrimitiveData points.
 * @param tfB  Second transfer function as sorted TFPrimitiveData points.
 * @param t    Interpolation parameter in [0, 1]. t=0 returns tfA, t=1 returns tfB.
 * @param opts Refinement tolerance and limits for the quantile level set.
 * @return     Interpolated transfer function as a vector of TFPrimitiveData.
 * 
 * @see optimalTransportInterpolationOpacityOptimalQExact
 */
IVW_CORE_API std::vector<TFPrimitiveData> optimalTransportInterpolationAdaptiveQExact(
    std::span<const TFPrimitiveData> tfA, std::span<const TFPrimitiveData> tfB, double t,
    const QuantileRefinementOptions& opts = {});

/**
 * @brief Interpolate two transfer functions using 1D optimal transport with opacity-optimal q
 * sampling and exact opacity reconstruction.
 *
 * Instead of linearly fading out the source transfer function while fading in the destination
 * transfer functions, this method uses earth movers distance to interpolate between the source
 * and destination transfer functions.
 *
 * The algorithm:
 * 1. Treats the alpha channel of each TF as a piecewise linear density (PDF).
 * 2. Computes the cumulative distribution function (CDF) for each TF.
 * 3. Inverts the CDFs to obtain quantile functions.
 * 4. Linearly interpolates quantile positions: Q_t(q) = (1-t)*Q_A(q) + t*Q_B(q).
 * 5. Interpolates the RGB color at each quantile point.
 * 6. Recovers the interpolated density by differentiating the interpolated CDF.
 * 7. Scales the result so total mass interpolates linearly between inputs.
 *
 * The method assigns each output vertex the exact transported opacity:
 * alpha_k = m_t / Q_t'(q_k)
 *             = m_t / ((1-t) m_A / alpha_A(Q_A(q_k)) + t m_B / alpha_B(Q_B(q_k))).
 * An alternative approach is to approximate that uses an average of the sub-interval edge point:
 * alpha_k = m_t (Delta q / Delta x),
 * which we refer to as the secant opacity approximation. The secant approximation is faster to
 * evaluate but less accurate.
 *
 * This method differs in *where* refinement inserts new quantile levels.
 * This method inserts control points at the quantile q* that
 * *maximises* the opacity deviation
 *   e(q) = | m_t / Q_t'(q) - chord(Q_t(q)) |
 * between the exact transported opacity and the straight line connecting the
 * sub-interval's endpoint opacities in x (the same chord the piecewise-linear
 * output draws). Because Q_t(q) and Q_t'(q) are closed form on each sub-interval
 * (one source and one destination knot segment), e(q) is evaluated directly with
 * no inversion of Q_t; e vanishes at both endpoints, so its maximiser is interior
 * and is located by a bracketed 1D search (a short uniform scan that brackets the
 * dominant extremum followed by a golden-section refinement).
 *
 * Inserting at the error maximiser removes the largest remaining opacity
 * deviation per control point, so for a fixed opacity tolerance the worst-case
 * opacity error of the output is driven below @p opts.relativeTolerance with
 * fewer control points than midpoint bisection.
 *
 *
 * @param tfA  First transfer function as sorted TFPrimitiveData points.
 * @param tfB  Second transfer function as sorted TFPrimitiveData points.
 * @param t    Interpolation parameter in [0, 1]. t=0 returns tfA, t=1 returns tfB.
 * @param opts Refinement tolerance and limits for the quantile level set.
 * @return     Interpolated transfer function as a vector of TFPrimitiveData.
 */
IVW_CORE_API std::vector<TFPrimitiveData> optimalTransportInterpolationOpacityOptimalQExact(
    std::span<const TFPrimitiveData> tfA, std::span<const TFPrimitiveData> tfB, double t,
    const QuantileRefinementOptions& opts = {});

IVW_CORE_API double evaluateInterpolatedAlpha(std::span<const TFPrimitiveData> tfA,
                                              std::span<const TFPrimitiveData> tfB, double t,
                                              double x);

IVW_CORE_API std::vector<double> evaluateInterpolatedAlphaGrid(
    std::span<const TFPrimitiveData> tfA, std::span<const TFPrimitiveData> tfB, double t,
    std::span<const double> xs);

IVW_CORE_API double earthMoversDistance(std::span<const TFPrimitiveData> tfA,
                                        std::span<const TFPrimitiveData> tfB,
                                        std::size_t samplesPerSegment = 16);

}  // namespace inviwo::algorithm
