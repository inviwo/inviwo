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

#include <inviwo/dataframe/util/selectionutil.h>
#include <inviwo/core/datastructures/buffer/buffer.h>
#include <inviwo/core/datastructures/buffer/bufferram.h>
#include <inviwo/core/util/formatdispatching.h>
#include <inviwo/core/util/zip.h>

#include <limits>

namespace inviwo::util {

namespace {

template <typename T>
std::pair<T, T> adjustLimits(double min, double max) {
    if constexpr (std::is_integral_v<T>) {
        if constexpr (std::is_unsigned_v<T>) {
            min = std::max(min, 0.0);
            max = std::max(max, 0.0);
        }
        return {static_cast<T>(std::ceil(min)), static_cast<T>(std::floor(max))};
    } else {
        return {static_cast<T>(min), static_cast<T>(max)};
    }
}

BitSet rangeSelection(const BufferBase* buffer, double min, double max) {
    const auto* buf = buffer->getRepresentation<BufferRAM>();
#include <warn/push>
#include <warn/ignore/conversion>  // Ignore double->float warnings
    return buf->dispatch<BitSet, dispatching::filter::Scalars>([min, max](auto brprecision) {
        using ValueType = util::PrecisionValueType<decltype(brprecision)>;
        BitSet selected;
        // Avoid conversions in the loop
        const auto [tmin, tmax] = adjustLimits<ValueType>(min, max);
        for (auto&& [ind, elem] : util::enumerate(brprecision->getDataContainer())) {
            if (elem < tmin || elem > tmax) {
                continue;
            } else {
                selected.add(ind);
            }
        }
        return selected;
    });
#include <warn/pop>
}

}  // namespace

BitSet boxSelect(const dvec2& start, const dvec2& end, const BufferBase* xAxis,
                 const BufferBase* yAxis) {

    if (xAxis == nullptr || yAxis == nullptr) {
        return {};
    }

    // For efficiency:
    // 1. Determine selection along x-axis
    // 2. Determine selection along y-axis using the subset from 1

    auto selectedIndicesX = rangeSelection(xAxis, start[0], end[0]);

#include <warn/push>
#include <warn/ignore/conversion>  // Ignore double->float warnings
    // Use indices filted by x-axis as input
    const auto* ybuf = yAxis->getRepresentation<BufferRAM>();
    auto selectedIndices = ybuf->dispatch<BitSet, dispatching::filter::Scalars>(
        [selectedIndicesX, min = start[1], max = end[1]](auto brprecision) {
            using ValueType = util::PrecisionValueType<decltype(brprecision)>;
            auto data = brprecision->getDataContainer();
            BitSet selected;
            // Avoid conversions in the loop
            const auto [tmin, tmax] = adjustLimits<ValueType>(min, max);
            for (auto ind : selectedIndicesX) {
                if (data[ind] < tmin || data[ind] > tmax) {
                    continue;
                } else {
                    selected.add(ind);
                }
            }
            return selected;
        });
#include <warn/pop>
    return selectedIndices;
}

BitSet boxFilter(const dvec2& start, const dvec2& end, const BufferBase* xAxis,
                 const BufferBase* yAxis) {
    if (xAxis == nullptr || yAxis == nullptr) {
        return {};
    }

    auto filtered = boxSelect(start, end, xAxis, yAxis);
    // invert selection
    filtered.flipRange(0, static_cast<std::uint32_t>(xAxis->getSize()));

    return filtered;
}

}  // namespace inviwo::util
