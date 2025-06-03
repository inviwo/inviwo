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
#include <set>

namespace inviwo::util {

std::vector<TFPrimitiveData> inviwo::util::tfSawTooth(const SawToothOptions& opts) {
    static constexpr auto clamp = [](double pos) { return std::clamp(pos, 0.0, 1.0); };
    static constexpr auto clamptf = [](TFPrimitiveData data) {
        return TFPrimitiveData{clamp(data.pos), data.color};
    };

    auto shift = [s = opts.shift](TFPrimitiveData data) {
        return TFPrimitiveData{data.pos + s, data.color};
    };
    const auto normalize = [range = opts.range](TFPrimitiveData data) {
        return TFPrimitiveData{(data.pos - range.x) / (range.y - range.x), data.color};
    };

    auto points = opts.points | std::views::transform(shift) | std::views::transform(normalize) |
                  std::views::transform(clamptf) | std::ranges::to<std::vector>();

    std::ranges::sort(points, std::ranges::less{}, &TFPrimitiveData::pos);
    auto dups = std::ranges::unique(points);
    points.erase(dups.begin(), dups.end());

    if (points.empty()) {
        return {};
    } else {
        std::vector<TFPrimitiveData> data;
        const auto delta = opts.delta;
        const auto low = opts.low;

        if (points.front().pos > 0.0) {
            const auto p0 = points.front().pos - delta;
            const auto high = points.front().color;
            data.emplace_back(clamp(p0), glm::mix(low, high, (clamp(p0) - p0) / delta));
        }
        for (auto&& [a, b] : std::views::zip(points, points | std::views::drop(1))) {
            data.emplace_back(a);
            if (b.pos - a.pos > 2 * delta) {
                data.emplace_back(a.pos + delta, low);
                data.emplace_back(b.pos - delta, low);
            } else if (b.pos - a.pos > delta) {
                const auto high = glm::mix(a.color, b.color, 0.5);
                data.emplace_back((b.pos + a.pos) / 2.0,
                                  glm::mix(high, low, (b.pos - a.pos - delta) / delta));
            }
        }
        data.emplace_back(points.back());

        if (points.back().pos < 1.0) {
            const auto pn = points.back().pos + delta;
            const auto high = points.back().color;
            data.emplace_back(clamp(pn), glm::mix(low, high, (pn - clamp(pn)) / delta));
        }

        return data;
    }
}

namespace {
// Helper view to iterate over adjacent pairs (sliding window size 2)
template <std::ranges::forward_range R>
auto adjacent_pairs(R&& r) {
    return std::views::zip(r, std::views::drop(r, 1));
}

vec4 interpolate(const TFPrimitiveData& p1, const TFPrimitiveData& p2, double x) {
    double t = (x - p1.pos) / (p2.pos - p1.pos);
    return p1.color + t * (p2.color - p1.color);
}

bool get_intersection(const TFPrimitiveData& a1, const TFPrimitiveData& a2,
                      const TFPrimitiveData& b1, const TFPrimitiveData& b2, TFPrimitiveData& out,
                      double EPS) {
    double dx1 = a2.pos - a1.pos;
    double dy1 = a2.color.a - a1.color.a;
    double dx2 = b2.pos - b1.pos;
    double dy2 = b2.color.a - b1.color.a;

    double denom = dx1 * dy2 - dy1 * dx2;
    if (std::abs(denom) < EPS) return false;

    double t = ((b1.pos - a1.pos) * dy2 - (b1.color.a - a1.color.a) * dx2) / denom;
    double x = a1.pos + t * dx1;

    if (x < std::max(std::min(a1.pos, a2.pos), std::min(b1.pos, b2.pos)) - EPS ||
        x > std::min(std::max(a1.pos, a2.pos), std::max(b1.pos, b2.pos)) + EPS)
        return false;

    out = {x, interpolate(a1, a2, x)};
    return true;
}

vec4 amax(const vec4& v1, const vec4& v2) { return v1.a >= v2.a ? v1 : v2; }

}  // namespace

std::vector<TFPrimitiveData> tfMax(std::span<const TFPrimitiveData> f,
                                   std::span<const TFPrimitiveData> g) {

    constexpr double EPS = 1e-9;
    constexpr double INF = std::numeric_limits<double>::infinity();

    auto f_pairs = adjacent_pairs(f);
    auto g_pairs = adjacent_pairs(g);

    auto f_it = f_pairs.cbegin();
    auto f_end = f_pairs.cend();

    auto g_it = g_pairs.cbegin();
    auto g_end = g_pairs.cend();

    std::vector<TFPrimitiveData> result;
    constexpr TFPrimitiveData max = TFPrimitiveData{INF, vec4{0}};
    auto maxPair = std::tie(max, max);

    while (f_it != f_end || g_it != g_end) {
        // Get current segments if available
        auto [f1, f2] = f_it != f_end ? *f_it : maxPair;
        auto [g1, g2] = g_it != g_end ? *g_it : maxPair;

        // Determine current interval
        double start = std::max(f1.pos, g1.pos);
        double end = std::min(f2.pos, g2.pos);

        if (f1.pos < g1.pos && f2.pos <= g1.pos) {
            // f only
            if (result.empty() || std::abs(result.back().pos - f1.pos) > EPS) {
                result.push_back(f1);
            }
            result.push_back(f2);
            ++f_it;
            continue;
        }
        if (g1.pos < f1.pos && g2.pos <= f1.pos) {
            // g only
            if (result.empty() || std::abs(result.back().pos - g1.pos) > EPS) {
                result.push_back(g1);
            }
            result.push_back(g2);
            ++g_it;
            continue;
        }

        if (end - start < EPS) {
            if (f2.pos <= g2.pos) {
                ++f_it;
            } else {
                ++g_it;
            }
            continue;
        }

        // Compute both at start and end
        const vec4 yf_start = interpolate(f1, f2, start);
        const vec4 yg_start = interpolate(g1, g2, start);
        const vec4 yf_end = interpolate(f1, f2, end);
        const vec4 yg_end = interpolate(g1, g2, end);

        const TFPrimitiveData p1 = {start, amax(yf_start, yg_start)};
        const TFPrimitiveData p2 = {end, amax(yf_end, yg_end)};

        TFPrimitiveData cross;
        if (get_intersection(f1, f2, g1, g2, cross, EPS) && cross.pos > start + EPS &&
            cross.pos < end - EPS) {
            const TFPrimitiveData left = {start, amax(yf_start, yg_start)};
            const TFPrimitiveData mid = {cross.pos, cross.color};
            const TFPrimitiveData right = {end, amax(yf_end, yg_end)};
            if (result.empty() || std::abs(result.back().pos - left.pos) > EPS) {
                result.push_back(left);
            }
            result.push_back(mid);
            result.push_back(right);
        } else {
            if (result.empty() || std::abs(result.back().pos - p1.pos) > EPS) {
                result.push_back(p1);
            }
            result.push_back(p2);
        }

        if (f2.pos <= g2.pos) {
            ++f_it;
        } else {
            ++g_it;
        }
    }

    return result;
}
}  // namespace inviwo::util
