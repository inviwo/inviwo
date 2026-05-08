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

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/datastructures/tfprimitive.h>

#include <vector>
#include <span>

namespace inviwo {

namespace algorithm {

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

}  // namespace algorithm

}  // namespace inviwo
