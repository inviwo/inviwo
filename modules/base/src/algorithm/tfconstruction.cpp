/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2025 Inviwo Foundation
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

#include <modules/base/algorithm/tfconstruction.h>

#include <algorithm>
#include <ranges>

namespace inviwo::util {

std::vector<TFPrimitiveData> inviwo::util::tfSawTooth(const SawToothOptions& opts) {
    static constexpr auto clamp = [](double pos) { return std::clamp(pos, 0.0, 1.0); };
    auto shift = [s = opts.shift](double pos) { return pos + s; };
    const auto normalize = [range = opts.range](double pos) {
        return (pos - range.x) / (range.y - range.x);
    };

    auto points = opts.points | std::views::transform(shift) | std::views::transform(normalize) |
                  std::views::transform(clamp) | std::ranges::to<std::vector>();

    std::ranges::sort(points);
    auto dups = std::ranges::unique(points);
    points.erase(dups.begin(), dups.end());

    if (points.empty()) {
        return {};
    } else {
        std::vector<TFPrimitiveData> data;
        const auto delta = opts.delta;
        const auto low = vec4{0.0};
        const auto high = vec4{1.0, 1.0, 1.0, opts.alpha};

        if (points.front() > 0.0) {
            const auto p0 = points.front() - delta;
            data.emplace_back(clamp(p0), glm::mix(low, high, (clamp(p0) - p0) / delta));
        }
        for (auto&& [a, b] : std::views::zip(points, points | std::views::drop(1))) {
            data.emplace_back(a, high);
            if (b - a > 2 * delta) {
                data.emplace_back(a + delta, low);
                data.emplace_back(b - delta, low);
            } else if (b - a > delta) {
                data.emplace_back((b + a) / 2.0, glm::mix(high, low, (b - a - delta) / delta));
            }
        }
        data.emplace_back(points.back(), high);

        if (points.back() < 1.0) {
            const auto pn = points.back() + delta;
            data.emplace_back(clamp(pn), glm::mix(low, high, (pn - clamp(pn)) / delta));
        }

        return data;
    }
}

}  // namespace inviwo::util
