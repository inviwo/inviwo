/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016 Inviwo Foundation
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

namespace inviwo {
namespace statsutil {
namespace detail {
template <typename Tx, typename Ty>
RegresionResult linearRegresion(const Tx &X, const Ty &Y) {
    RegresionResult res;

    ivwAssert(X.getSize() == Y.getSize(), "Buffers are not of equal length");

    auto &xvec = X.getDataContainer();
    auto &yvec = Y.getDataContainer();

    // Ax = b;
    dmat2 A = dmat2(xvec.size(), 0, 0, 0);
    dvec2 b(0);
    double meanX = 0;
    double meanY = 0;

    {
        auto xit = xvec.begin();
        auto yit = yvec.begin();

        for (; xit != xvec.end() && yit != yvec.end(); ++xit, ++yit) {
            auto x = *xit;
            auto y = *yit;
            meanX += x;
            meanY += y;
            A[0][1] += x;
            A[1][1] += x * x;
            b.x += y;
            b.y += y * x;
        }
    }
    A[1][0] = A[0][1];
    dvec2 km = glm::inverse(A) * b;
    res.k = km.y;
    res.m = km.x;
    res.r2 = 0;
    meanX /= A[0][0];
    meanY /= A[0][0];
    double stdX = 0;
    double stdY = 0;

    {
        auto xit = xvec.begin();
        auto yit = yvec.begin();

        for (; xit != xvec.end() && yit != yvec.end(); ++xit, ++yit) {
            auto x = *xit - meanX;
            auto y = *yit - meanY;

            stdX += x * x;
            stdY += y * y;

            res.r2 += x * y;
        }
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

RegresionResult linearRegresion(const BufferBase &X, const BufferBase &Y) {
    return X.getRepresentation<BufferRAM>()
        ->dispatch<RegresionResult, dispatching::filter::Scalars>([&](auto Xbuf) {
            return Y.getRepresentation<BufferRAM>()
                ->dispatch<RegresionResult, dispatching::filter::Scalars>(
                    [&](auto Ybuf) { return detail::linearRegresion(*Xbuf, *Ybuf); });
        });
}

}  // namespace statsutil

}  // namespace inviwo
