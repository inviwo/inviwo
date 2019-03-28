/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2019 Inviwo Foundation
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

#include <modules/animationqt/widgets/editorconstants.h>

#include <array>
#include <limits>
#include <cmath>

#include <warn/push>
#include <warn/ignore/all>
#include <QApplication>
#include <warn/pop>

namespace inviwo {

namespace animation {

qreal getSnapTime(const qreal& actualTime, const qreal& scale) {
    if (QApplication::keyboardModifiers() & Qt::AltModifier) {
        return actualTime;
    }
    qreal snapScale = (scale >= 1) ? std::round(scale) + 1 : 3 - std::round(1 / scale);
    snapScale = std::max(snapScale, 0.0);  // not over 1 second
    snapScale = std::min(snapScale, 5.0);  // not under 1/32 second
    const qreal snapToGridResolution = widthPerSecond / pow(2, snapScale);
    const qreal snapTime = std::round(actualTime / snapToGridResolution) * snapToGridResolution;

    return snapTime;
}

FindDivisionsResult findDivisions(double start, double stop, int divisions) {

    auto niceStart = start;
    auto dist = stop - start;
    double factor = 1.0;

    // normalize the distance to 1 to 10
    while (dist > 10) {
        dist /= 10.0;
        niceStart /= 10.0;
        factor *= 10.0;
    }
    while (dist < 1) {
        dist *= 10.0;
        niceStart *= 10.0;
        factor /= 10.0;
    }

    // create a nice number of intervals
    auto count = std::round(dist);
    const auto scaler = [](int i) {
        const std::array<double, 4> f{{1.0, 2.0, 4.0, 5.0}};
        return std::pow(10, i / f.size()) * f[i % f.size()];
    };

    if (count < divisions) {
        int i = 0;
        while (std::abs(count * scaler(i) - divisions) >
               std::abs(count * scaler(i + 1) - divisions)) {
            ++i;
        }

        count *= scaler(i);
        niceStart *= scaler(i);
        factor /= scaler(i);
    } else {
        int i = 0;
        while (std::abs(count / scaler(i) - divisions) >
               std::abs(count / scaler(i + 1) - divisions)) {
            ++i;
        }
        count /= scaler(i);
        niceStart /= scaler(i);
        factor *= scaler(i);
    }

    niceStart = std::round(niceStart) * factor;

    // make sure we cover all of start to stop
    while (niceStart > start) {
        niceStart -= factor;
        ++count;
    }
    while (niceStart + count * factor < stop) {
        ++count;
    }

    // figure out how many digits are needed
    int int_digits = 0;
    int frac_digits = 0;
    auto remainder = std::remainder(factor, 1.0);

    if (niceStart + count * factor > 1.0) {
        int_digits = static_cast<int>(std::floor(std::log10(niceStart + count * factor))) + 1;
    }
    while (std::abs(remainder) > 100.0 * std::numeric_limits<double>::epsilon()) {
        frac_digits++;
        remainder = std::remainder(remainder * 10.0, 1.0);
    }

    return {niceStart, factor, static_cast<size_t>(count), int_digits, frac_digits};
}
}  // namespace animation

}  // namespace inviwo
