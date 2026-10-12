/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2026 Inviwo Foundation
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

#include <modules/plotting/utils/statsutils.h>

#include <inviwo/core/datastructures/buffer/buffer.h>
#include <inviwo/core/datastructures/buffer/bufferram.h>
#include <inviwo/core/datastructures/representationconverter.h>
#include <inviwo/core/datastructures/representationconverterfactory.h>
#include <inviwo/core/util/exception.h>
#include <inviwo/core/util/formatdispatching.h>
#include <inviwo/core/util/glmmat.h>
#include <inviwo/core/util/glmvec.h>
#include <inviwo/core/util/sourcecontext.h>
#include <inviwo/core/util/zip.h>

#include <stdlib.h>
#include <memory>
#include <ostream>
#include <unordered_set>
#include <ranges>

#include <glm/mat2x2.hpp>
#include <glm/matrix.hpp>
#include <glm/vec2.hpp>

namespace inviwo {
namespace statsutil {
namespace detail {
template <typename Tx, typename Ty>
RegresionResult linearRegresion(const Tx& X, const Ty& Y) {
    RegresionResult res;
    if (X.getSize() != Y.getSize()) {
        throw Exception("Buffers are not of equal length");
    }
    auto& xvec = X.getDataContainer();
    auto& yvec = Y.getDataContainer();

    // Ax = b;
    // Minimize the sum of squares of individual errors
    // http://users.metu.edu.tr/csert/me310/me310_5_regression.pdf
    dmat2 A = dmat2(xvec.size(), 0, 0, 0);
    dvec2 b(0);
    double sumX = 0;
    double sumY = 0;

    // Iterate over containers in sync
    for (auto&& [x, y] : std::views::zip(xvec, yvec)) {
        if constexpr (std::is_floating_point_v<decltype(x)>) {
            if (std::isnan(x)) {
                --A[0][0];
                continue;
            }
        }
        if constexpr (std::is_floating_point_v<decltype(y)>) {
            if (std::isnan(y)) {
                --A[0][0];
                continue;
            }
        }
        sumX += static_cast<double>(x);
        sumY += static_cast<double>(y);
        A[1][1] += static_cast<double>(x * x);
        b.y += static_cast<double>(y) * static_cast<double>(x);
    }

    b.x = sumY;
    A[1][0] = A[0][1] = sumX;
    dvec2 km = glm::inverse(A) * b;
    res.k = km.y;
    res.m = km.x;
    res.r2 = 0;
    double meanX = sumX / A[0][0];
    double meanY = sumY / A[0][0];
    double stdX = 0;
    double stdY = 0;

    for (auto&& [x, y] : std::views::zip(xvec, yvec)) {
        if constexpr (std::is_floating_point_v<decltype(x)>) {
            if (std::isnan(x)) continue;
        }
        if constexpr (std::is_floating_point_v<decltype(y)>) {
            if (std::isnan(y)) continue;
        }
        const auto xm = static_cast<double>(x) - meanX;
        const auto ym = static_cast<double>(y) - meanY;

        stdX += xm * xm;
        stdY += ym * ym;

        res.r2 += xm * ym;
    }

    stdX /= A[0][0];
    stdY /= A[0][0];

    stdX = std::sqrt(stdX);
    stdY = std::sqrt(stdY);

    res.r2 /= A[0][0];
    res.r2 /= stdX * stdY;

    res.corr = std::abs(res.r2);
    if (res.k < 0) res.corr = -res.corr;
    res.r2 *= res.r2;

    return res;
}

}  // namespace detail

RegresionResult linearRegresion(const BufferBase& X, const BufferBase& Y) {
    return X.getRepresentation<BufferRAM>()
        ->dispatch<RegresionResult, dispatching::filter::Scalars>([&](auto Xbuf) {
            return Y.getRepresentation<BufferRAM>()
                ->dispatch<RegresionResult, dispatching::filter::Scalars>(
                    [&](auto Ybuf) { return detail::linearRegresion(*Xbuf, *Ybuf); });
        });
}

std::ostream& operator<<(std::ostream& os, RegresionResult res) {
    os << "y = " << res.k << "x + " << res.m << "(r2: " << res.r2 << ", corr: " << res.corr;

    return os;
}

}  // namespace statsutil

}  // namespace inviwo
