/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2023 Inviwo Foundation
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

#include <inviwo/core/datastructures/buffer/buffer.h>                   // for BufferBase
#include <inviwo/core/datastructures/buffer/bufferram.h>                // for BufferRAM
#include <inviwo/core/datastructures/representationconverter.h>         // for RepresentationCon...
#include <inviwo/core/datastructures/representationconverterfactory.h>  // for RepresentationCon...
#include <inviwo/core/util/exception.h>                                 // for Exception
#include <inviwo/core/util/formatdispatching.h>                         // for Scalars
#include <inviwo/core/util/glmmat.h>                                    // for dmat2
#include <inviwo/core/util/glmvec.h>                                    // for dvec2
#include <inviwo/core/util/sourcecontext.h>                             // for IVW_CONTEXT_CUSTOM
#include <inviwo/core/util/zip.h>                                       // for zipper, get, zip

#include <stdlib.h>       // for abs
#include <memory>         // for unique_ptr
#include <ostream>        // for operator<<, basic...
#include <unordered_set>  // for unordered_set

#include <glm/mat2x2.hpp>  // for mat<>::col_type
#include <glm/matrix.hpp>  // for inverse
#include <glm/vec2.hpp>    // for vec<>::(anonymous)
#include <half/half.hpp>   // for operator*

namespace inviwo {
namespace statsutil {
namespace detail {
template <typename Tx, typename Ty>
RegresionResult linearRegresion(const Tx& X, const Ty& Y) {
    RegresionResult res;
    if (X.getSize() != Y.getSize()) {
        throw Exception("Buffers are not of equal length",
                        IVW_CONTEXT_CUSTOM("statsutil::detail::linearRegresion"));
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
    for (auto&& i : util::zip(xvec, yvec)) {
        auto x = get<0>(i);
        auto y = get<1>(i);
        if (std::isnan(static_cast<double>(x)) || std::isnan(static_cast<double>(y))) {
            --A[0][0];
            continue;
        }
        sumX += x;
        sumY += y;
        A[1][1] += x * x;
        b.y += y * x;
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

    for (auto&& i : util::zip(xvec, yvec)) {
        if (std::isnan(static_cast<double>(get<0>(i))) ||
            std::isnan(static_cast<double>(get<1>(i)))) {
            continue;
        }
        auto x = get<0>(i) - meanX;
        auto y = get<1>(i) - meanY;

        stdX += x * x;
        stdY += y * y;

        res.r2 += x * y;
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
