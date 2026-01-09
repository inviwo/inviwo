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

#include <modules/base/algorithm/tfconstruction.h>

#include <glm/geometric.hpp>  // glm::cross, glm::mix

#include <algorithm>
#include <ranges>
#include <set>
#include <optional>

namespace inviwo::util {

std::vector<TFPrimitiveData> tfSawTooth(const SawToothOptions& opts) {
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

std::optional<double> intersection(const TFPrimitiveData& f1, const TFPrimitiveData& f2,
                                   const TFPrimitiveData& g1, const TFPrimitiveData& g2,
                                   double epsilon) {
    const auto fp1 = dvec2{f1.pos, f1.color.a};
    const auto fp2 = dvec2{f2.pos, f2.color.a};

    const auto gp1 = dvec2{g1.pos, g1.color.a};
    const auto gp2 = dvec2{g2.pos, g2.color.a};

    const double df = (fp2.y - fp1.y) / (fp2.x - fp1.x);
    const double dg = (gp2.y - gp1.y) / (gp2.x - gp1.x);

    if (std::abs(df - dg) < epsilon) {
        return std::nullopt;  // parallel or identical
    }

    const double b_f = fp1.y - df * fp1.x;
    const double b_g = gp1.y - dg * gp1.x;
    const double xInter = (b_g - b_f) / (df - dg);

    const double start = std::max(fp1.x, gp1.x);
    const double end = std::min(fp2.x, gp2.x);

    if (xInter >= start - epsilon && xInter <= end + epsilon) {
        return xInter;
    }

    return std::nullopt;
}

std::optional<double> intersection(const TFPrimitiveData& p1, const TFPrimitiveData& p2,
                                   const TFPrimitiveData& point, IntersectDir dir) {
    const double y1 = p1.color.a;
    const double y2 = p2.color.a;

    const double yp = point.color.a;

    // Check if the segment crosses y = yp
    if ((y1 - yp) * (y2 - yp) >= 0) {
        return std::nullopt;  // No crossing
    }

    // Linear interpolation
    const double t = (yp - y1) / (y2 - y1);
    const double x = p1.pos + t * (p2.pos - p1.pos);

    if ((dir == IntersectDir::Left && x > point.pos) ||
        (dir == IntersectDir::Right && x < point.pos)) {
        return std::nullopt;
    } else {
        return x;
    }
}

namespace {

struct BreakPoint {
    enum class Type : std::uint8_t { Node, Intersection };
    double pos;
    Type type;

    auto operator<=>(const BreakPoint& other) const { return pos <=> other.pos; }
};

double errorByRemoving(const std::vector<TFPrimitiveData>& points, int i) {
    const auto& prev = points[i - 1];
    const auto& curr = points[i];
    const auto& next = points[i + 1];

    const double x = (curr.pos - prev.pos) / (next.pos - prev.pos);
    return glm::compMax(glm::abs(glm::mix(prev.color, next.color, x) - curr.color));
};

/// removes redundant points from the tf
void cleanTf(std::vector<TFPrimitiveData>& points, double epsilon) {
    // remove from the front
    for (int i = 1; i < static_cast<int>(points.size()); ++i) {
        if (points[i].color == points[i - 1].color) {
            points.erase(points.begin() + i - 1);
        } else {
            break;
        }
    }

    // remove from the end
    for (int i = static_cast<int>(points.size()) - 1; i >= 1; --i) {
        if (points[i].color == points[i - 1].color) {
            points.erase(points.begin() + i);
        } else {
            break;
        }
    }

    // remove in the middle
    for (int i = 1; i < static_cast<int>(points.size()) - 1;) {
        if (errorByRemoving(points, i) < epsilon) {
            points.erase(points.begin() + i);
        } else {
            ++i;
        }
    }
}

std::set<BreakPoint> findBreakPoints(const std::span<const TFPrimitiveData>& f,
                                     const std::span<const TFPrimitiveData>& g, double epsilon) {
    std::set<BreakPoint> breakpoints;

    for (const auto& pt : f) breakpoints.emplace(pt.pos, BreakPoint::Type::Node);
    for (const auto& pt : g) breakpoints.emplace(pt.pos, BreakPoint::Type::Node);

    for (size_t i = 1; i < f.size(); ++i) {
        for (size_t j = 1; j < g.size(); ++j) {
            if (auto maybeCross = intersection(f[i - 1], f[i], g[j - 1], g[j], epsilon)) {
                breakpoints.emplace(*maybeCross, BreakPoint::Type::Intersection);
            }
        }
    }
    for (size_t i = 1; i < f.size(); ++i) {
        if (auto maybeCross = intersection(f[i - 1], f[i], g.front(), IntersectDir::Left)) {
            breakpoints.emplace(*maybeCross, BreakPoint::Type::Intersection);
        }
        if (auto maybeCross = intersection(f[i - 1], f[i], g.back(), IntersectDir::Right)) {
            breakpoints.emplace(*maybeCross, BreakPoint::Type::Intersection);
        }
    }
    for (size_t i = 1; i < g.size(); ++i) {
        if (auto maybeCross = intersection(g[i - 1], g[i], f.front(), IntersectDir::Left)) {
            breakpoints.emplace(*maybeCross, BreakPoint::Type::Intersection);
        }
        if (auto maybeCross = intersection(g[i - 1], g[i], f.back(), IntersectDir::Right)) {
            breakpoints.emplace(*maybeCross, BreakPoint::Type::Intersection);
        }
    }
    return breakpoints;
}

void addPoint(std::vector<TFPrimitiveData>& result, double pos, const vec4& cf,
              const std::span<const TFPrimitiveData>& g, dvec2& range, double epsilon) {
    result.emplace_back(pos, cf);
    if (pos + epsilon < range[1]) {
        result.emplace_back(pos + epsilon, util::interpolateColor(g, pos + epsilon));
    }
}

}  // namespace

std::vector<TFPrimitiveData> tfMax(std::span<const TFPrimitiveData> f,
                                   std::span<const TFPrimitiveData> g, dvec2 range,
                                   double epsilon) {
    if (f.empty()) {
        return {g.begin(), g.end()};
    }

    if (g.empty()) {
        return {f.begin(), f.end()};
    }

    const auto breakpoints = findBreakPoints(f, g, epsilon);

    enum class Prev : std::uint8_t { F, G };
    std::vector<TFPrimitiveData> result;
    Prev prev = Prev::F;
    for (const auto& bp : breakpoints) {
        const auto cf = util::interpolateColor(f, bp.pos);
        const auto cg = util::interpolateColor(g, bp.pos);

        if (bp.type == BreakPoint::Type::Intersection) {
            if (prev == Prev::F) {
                addPoint(result, bp.pos, cf, g, range, epsilon);
                prev = Prev::G;
            } else {
                addPoint(result, bp.pos, cg, f, range, epsilon);
                prev = Prev::F;
            }
        } else {
            if (cf.a >= cg.a) {
                result.emplace_back(bp.pos, cf);
                prev = Prev::F;
            } else {
                result.emplace_back(bp.pos, cg);
                prev = Prev::G;
            }
        }
    }

    cleanTf(result, epsilon);

    return result;
}

}  // namespace inviwo::util
