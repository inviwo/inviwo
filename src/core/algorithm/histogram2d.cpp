/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2025-2026 Inviwo Foundation
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

#include <inviwo/core/algorithm/histogram2d.h>

namespace inviwo::util::detail {

DataMapper histogramDataMap(const DataMapper& datamap, double effectiveRange) {
    const dvec2 effectiveDataRange{datamap.dataRange.x, datamap.dataRange.x + effectiveRange};
    const dvec2 effectiveValueRange{datamap.valueRange.x,
                                    datamap.mapFromDataToValue(effectiveDataRange.y)};
    return DataMapper{effectiveDataRange, effectiveValueRange, datamap.valueAxis};
}

std::array<std::vector<size_t>, 2> histogram2DTo1D(const std::vector<size_t>& hist2D,
                                                   size2_t numBins) {

    const IndexMapper2D indexMapper{numBins};

    const auto hist1D_1 =
        std::views::iota(0uz, numBins[0]) | std::views::transform([&](auto i) {
            return std::ranges::fold_left(
                std::views::iota(0uz, numBins[1]) |
                    std::views::transform([&](auto j) { return hist2D[indexMapper(i, j)]; }),
                0, std::plus<>{});
        }) |
        std::ranges::to<std::vector>();
    const auto hist1D_2 =
        std::views::iota(0uz, numBins[1]) | std::views::transform([&](auto j) {
            return std::ranges::fold_left(
                std::views::iota(0uz, numBins[0]) |
                    std::views::transform([&](auto i) { return hist2D[indexMapper(i, j)]; }),
                0, std::plus<>{});
        }) |
        std::ranges::to<std::vector>();

    return {hist1D_1, hist1D_2};
}

}  // namespace inviwo::util::detail
