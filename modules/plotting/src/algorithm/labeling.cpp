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

#include <modules/plotting/algorithm/labeling.h>
#include <inviwo/core/util/exception.h>
#include <inviwo/core/util/assertion.h>
#include <inviwo/core/util/glm.h>
#include <inviwo/core/util/zip.h>
#include <inviwo/core/util/logcentral.h>

#include <cmath>
#include <ranges>
#include <algorithm>
#include <numeric>

namespace inviwo::plot {

namespace {

bool almostEqual(double a, double b, int factor = 50) {
    const auto min_a = a - (a - std::nextafter(a, std::numeric_limits<double>::lowest())) * factor;
    const auto max_a = a + (std::nextafter(a, std::numeric_limits<double>::max()) - a) * factor;
    return min_a <= b && max_a >= b;
}

auto rangeView(const LinearRange& range) {
    const auto start = util::almostEqual(range.start, 0.0) ? 0.0 : range.start;
    const auto stop = util::almostEqual(range.stop, 0.0) ? 0.0 : range.stop;
    const auto step = range.step;

    const auto count =
        std::max(1, static_cast<int>(std::floor((stop + 0.5 * step - start) / step)));

    const auto steps = ((start > stop && step > 0) || (start < stop && step < 0)) ? 0 : count + 1;

    return std::views::iota(0, steps) | std::views::transform([start, stop, step, count](int i) {
               if (count == i) return stop;
               const double pos = start + step * i;
               return std::abs(pos) < step * 1.0e-3 ? 0.0 : pos;
           });
}

// utility function for Heckbert's labeling
double niceNumber(double value, bool round) {
    const double exponent = std::floor(std::log10(value));
    const double fraction = value / std::pow(10.0, exponent);
    double niceFraction = 0.0;
    if (round) {
        if (fraction < 1.5) {
            niceFraction = 1.0;
        } else if (fraction < 3.0) {
            niceFraction = 2.0;
        } else if (fraction < 7.0) {
            niceFraction = 5.0;
        } else {
            niceFraction = 10.0;
        }
    } else {
        if (fraction <= 1.0) {
            niceFraction = 1.0;
        } else if (fraction <= 2.0) {
            niceFraction = 2.0;
        } else if (fraction <= 5.0) {
            niceFraction = 5.0;
        } else {
            niceFraction = 10.0;
        }
    }
    return niceFraction * std::pow(10.0, exponent);
}

}  // namespace

void linearRange(const LinearRange& range, std::vector<double>& positions) {
    const auto view = rangeView(range);
    positions.assign(view.begin(), view.end());
}

void linearRange(const dvec2& range, const LinearRange& optRange, int minorTickFrequency,
                 std::vector<double>& major, std::vector<double>& minor) {

    const auto view = rangeView(optRange);
    major.assign(view.begin(), view.end());

    if (minorTickFrequency <= 1) return;

    const auto minorStep = optRange.step / static_cast<double>(minorTickFrequency);
    const auto pre = static_cast<size_t>(
        std::floor(std::max(0.0, std::abs(optRange.start - range.x)) / std::abs(minorStep)));
    const auto post = static_cast<size_t>(
        std::floor(std::max(0.0, std::abs(range.y - optRange.stop)) / std::abs(minorStep)));

    // GCC 14 does not have support for assign_range, so we have to do it manually here
    minor.clear();
    for (auto&& p : std::views::iota(0uz, pre) | std::views::transform([=](size_t i) {
                        return optRange.start - static_cast<double>(pre - i) * minorStep;
                    })) {
        minor.emplace_back(p);
    }

    for (auto&& p : view | std::views::take(std::ranges::size(view) - 1uz) |
                        std::views::transform([&](double x) {
                            return std::views::iota(0, minorTickFrequency - 1) |
                                   std::views::transform([=](int i) {
                                       return x + static_cast<double>(i + 1) * minorStep;
                                   });
                        }) |
                        std::views::join) {
        minor.emplace_back(p);
    }
    for (auto&& p : std::views::iota(0uz, post) | std::views::transform([=](size_t i) {
                        return optRange.stop + static_cast<double>(i + 1) * minorStep;
                    })) {
        minor.emplace_back(p);
    }
}

LinearRange labelingHeckbert(double valueMin, double valueMax, int numTicks) {
    if (numTicks <= 1) {
        throw Exception("expected numTicks > 1");
    }

    const double adjustedExtent = niceNumber(valueMax - valueMin, false);
    const auto step = niceNumber(adjustedExtent / (static_cast<double>(numTicks) - 1.0), true);
    const auto start = std::ceil(valueMin / step) * step;
    const auto stop = std::floor(valueMax / step) * step;

    return {.start = start, .stop = stop, .step = step};
}

// utility functions for the Extended Wilkinson labeling
namespace {

double simplicity(int qIndex, size_t qSize, int j, double labelMin, double labelMax, double step) {
    constexpr double epsilon = std::numeric_limits<double>::epsilon() * 100.0;
    const double delta = std::fmod(labelMin, step);
    const bool zeroIncluded =
        (labelMin < 0.0 && labelMax > 0.0 && (delta < epsilon || step - delta < epsilon));
    return 1.0 - static_cast<double>(qIndex - 1) / static_cast<double>(qSize - 1) -
           static_cast<double>(j) + (zeroIncluded ? 1.0 : 0.0);
}

double simplicityMax(int qIndex, size_t qSize, int j) {
    return 1.0 - static_cast<double>(qIndex - 1) / static_cast<double>(qSize - 1) - j + 1;
}

double coverage(double valueMin, double valueMax, double labelMin, double labelMax) {
    return 1.0 - 0.5 * (std::pow(valueMax - labelMax, 2.0) + std::pow(valueMin - labelMin, 2)) /
                     std::pow(0.1 * (valueMax - valueMin), 2.0);
}

double coverageMax(double valueMin, double valueMax, double span) {
    const double valueRange = valueMax - valueMin;
    if (span > valueRange) {
        const double halfed = (span - valueRange) * 0.5;
        return 1.0 - halfed * halfed / std::pow(0.1 * valueRange, 2.0);
    } else {
        return 1.0;
    }
}

double density(int k, int m, double valueMin, double valueMax, double labelMin, double labelMax) {
    const double r = static_cast<double>(k - 1) / (labelMax - labelMin);
    const double rt =
        static_cast<double>(m - 1) / (std::max(labelMax, valueMax) - std::min(labelMin, valueMin));
    return 2.0 - std::max(r / rt, rt / r);
}

double densityMax(int k, int m) {
    if (k >= m) {
        return 2.0 - static_cast<double>(k - 1) / static_cast<double>(m - 1);
    } else {
        return 1.0;
    }
}

}  // namespace

// NOLINTBEGIN(readability-function-cognitive-complexity)
LinearRange labelingExtendedWilkinson(double valueMin, double valueMax, int numTicks,
                                      std::span<const double> Q) {
    // weights for simplicity, coverage, density, and legibility
    constexpr std::array<double, 4> weights{0.25, 0.2, 0.5, 0.05};
    const bool labelOutsideRange = false;

    // check for too small and too large ranges
    if (std::abs(valueMax - valueMin) < std::numeric_limits<double>::epsilon() * 100.0 ||
        std::abs(valueMax - valueMin) > std::sqrt(std::numeric_limits<double>::max())) {
        const double step = (valueMax - valueMin) / static_cast<double>(numTicks - 1);

        return {.start = valueMin, .stop = valueMax, .step = step};
    }

    auto weightedScore = [&weights](auto scores) {
        return std::inner_product(scores.begin(), scores.end(), weights.begin(), 0.0);
    };

    const int maxAttempts = 100000;  // +inf in the original implementation
    double bestScore = -2.0;

    LinearRange result{.start = 0.0, .stop = 0.0, .step = 1.0};

    int j = 1;

    while (j < maxAttempts) {
        for (auto&& [qIndex, q] : util::enumerate(Q)) {
            std::array<double, 4> scores{simplicityMax(static_cast<int>(qIndex), Q.size(), j), 1.0,
                                         1.0, 1.0};
            if (weightedScore(scores) < bestScore) {
                j = maxAttempts;
                break;
            }

            // loop over increasing number of ticks/labels
            for (auto k : std::ranges::iota_view(2, maxAttempts)) {
                scores[1] = 1.0;
                scores[2] = densityMax(k, numTicks);
                if (weightedScore(scores) < bestScore) {
                    break;
                }

                const double delta = (valueMax - valueMin) / (static_cast<double>((k + 1) * j) * q);
                for (auto zStart = static_cast<int>(std::ceil(std::log10(delta)));
                     auto z : std::ranges::iota_view(zStart)) {
                    const double step = q * static_cast<double>(j) * pow(10.0, z);
                    scores[1] = coverageMax(valueMin, valueMax, step * static_cast<double>(k - 1));
                    if (weightedScore(scores) < bestScore) {
                        break;
                    }
                    const int minStart =
                        static_cast<int>(std::floor(valueMax / step)) * j - (k - 1) * j;
                    const int maxStart = static_cast<int>(std::ceil(valueMin / step)) * j;
                    if (minStart > maxStart) continue;

                    for (auto start : std::ranges::iota_view(minStart, maxStart)) {
                        const double labelMin =
                            static_cast<double>(start) * (step / static_cast<double>(j));
                        const double labelMax = labelMin + step * static_cast<double>(k - 1);
                        const double labelStep = step;

                        const std::array<double, 4> score = {
                            simplicity(static_cast<int>(qIndex), Q.size(), j, labelMin, labelMax,
                                       labelStep),
                            coverage(valueMin, valueMax, labelMin, labelMax),
                            density(k, numTicks, valueMin, valueMax, labelMin, labelMax),
                            1.0  // legibility not considered
                        };
                        if (auto s = weightedScore(score);
                            s > bestScore && (labelMin <= valueMin && labelMax >= valueMax)) {
                            bestScore = s;
                            result.start = labelMin;
                            result.stop = labelMax;
                            result.step = labelStep;
                        }
                    }
                }
            }
        }
        ++j;
    }
    if (!labelOutsideRange) {
        // adjust start and stop limits to lie within the value range
        if (!util::almostEqual(result.start, valueMin) && result.start < valueMin) {
            result.start += result.step;
        }
        if (!util::almostEqual(result.stop, valueMax) && result.stop > valueMax) {
            result.stop -= result.step;
        }
    }

    return result;
}
// NOLINTEND(readability-function-cognitive-complexity)

// utility functions and structs for matplotlibs labeling
//
// based on https://github.com/matplotlib/matplotlib/blob/main/lib/matplotlib/ticker.py
namespace {

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
std::pair<double, double> scaleRange(double valueMin, double valueMax, int n = 1,
                                     double threshold = 100.0) {
    const double dv = std::abs(valueMax - valueMin);  // > 0, since zero check is done before
    const double meanv = (valueMax + valueMin) * 0.5;
    double offset = 0.0;
    if (std::abs(meanv) / dv >= threshold) {
        offset = std::copysign(std::pow(10.0, std::floor(std::log10(std::abs(meanv)))), meanv);
    }
    const double scale = std::pow(10.0, std::floor(std::log10(dv / n)));
    return {scale, offset};
}

//     Make an extended staircase within which the needed step will be
// found. This is probably much larger than necessary.
std::vector<double> staircase(std::span<const double> steps, double scaling = 1.0) {
    // np.concatenate([0.1 * steps[:-1], steps, [10 * steps[1]]])
    auto v = steps | std::views::take(steps.size() - 1) |
             std::views::transform([s = scaling * 0.1](double val) { return val * s; }) |
             std::ranges::to<std::vector>();
    std::ranges::transform(steps, std::back_inserter(v),
                           [scaling](double v) { return v * scaling; });
    v.emplace_back(10.0 * steps[1] * scaling);
    return v;
}

// Helper for `.MaxNLocator`, `.MultipleLocator`, etc.
//
// Take floating-point precision limitations into account when calculating
// tick locations as integer multiples of a step.
class EdgeInteger {
public:
    EdgeInteger(double step, double offset) : step_{step}, offset_{std::abs(offset)} {
        ivwAssert(step >= 0.0, "step must be positive");
    }

    bool closeTo(double ms, double edge) const {
        // Allow more slop when the offset is large compared to the step.
        double tol = 1.0e-10;
        if (offset_ > 0.0) {
            const double digits = std::log10(offset_ / step_);
            tol = std::min(0.4999, std::max(1.0e-10, std::pow(10.0, digits - 12)));
        }
        return std::abs(ms - edge) < tol;
    }

    // Return the largest n: n*step <= x.
    int lessEqual(double x) const {
        const auto d = static_cast<int>(x / step_);
        if (const double m = x - d * step_; closeTo(m / step_, 1.0)) {
            return d + 1;
        }
        return d;
    }

    // Return the smallest n: n*step >= x.
    int greaterEqual(double x) const {
        const auto d = static_cast<int>(x / step_);
        if (const double m = x - d * step_; closeTo(m / step_, 0.0)) {
            return d;
        }
        return d + 1;
    }

private:
    double step_{};
    double offset_{};
};

}  // namespace

LinearRange labelingMatplotlib(double valueMin, double valueMax, int maxTicks,
                               std::span<const double> niceSteps, bool integerTicks,
                               int minNTicks) {
    ivwAssert(minNTicks > 0, "expected minNTicks > 0");
    const int nBins = maxTicks - 1;
    const bool roundNumbers = true;

    const auto [scale, offset] = scaleRange(valueMin, valueMax, nBins);
    const double vMin = valueMin - offset;
    const double vMax = valueMax - offset;
    auto steps = staircase(niceSteps, scale);
    if (integerTicks) {
        // for steps > 1 keep only integers
        steps = steps | std::views::filter([](double v) {
                    return (v < 1.0 || std::abs(v - std::round(v)) < 0.001);
                }) |
                std::ranges::to<std::vector>();
    }
    const double rawStep = (vMax - vMin) / static_cast<double>(nBins);
    // boolean mask of large steps
    auto largeSteps = steps | std::views::transform([rawStep](double v) { return v >= rawStep; });
    size_t iStep = steps.size() - 1;
    if (roundNumbers) {
        // Classic round_numbers mode may require a larger step.
        // Get first multiple of steps that are <= vMin
        auto vMinsFloored =
            steps | std::views::transform([vMin](double s) { return std::floor(vMin / s) * s; });
        auto vMaxsFloored =
            std::views::zip(vMinsFloored, steps) | std::views::transform([nBins](auto elem) {
                return std::get<0>(elem) + std::get<1>(elem) * nBins;
            });
        auto largeStepsRounded =
            std::views::zip(largeSteps, vMaxsFloored) | std::views::transform([vMax](auto elem) {
                return std::get<0>(elem) && (std::get<1>(elem) >= vMax);
            });
        if (auto it = std::ranges::find(largeStepsRounded, true); it != largeStepsRounded.end()) {
            iStep = static_cast<size_t>(std::distance(largeStepsRounded.begin(), it));
        }
    } else {
        if (auto it = std::ranges::find(largeSteps, true); it != largeSteps.end()) {
            iStep = static_cast<size_t>(std::distance(largeSteps.begin(), it));
        }
    }

    // Start at smallest of the steps greater than the raw step, and check
    // if it provides enough ticks. If not, work backwards through
    // smaller steps until one is found that provides enough ticks.
    // for step in steps[:istep+1][::-1]:

    LinearRange result{.start = 0.0, .stop = 0.0, .step = 1.0};
    for (auto step : steps | std::views::take(iStep + 1) | std::views::reverse) {
        if (integerTicks && (std::floor(vMax) - std::ceil(vMin) >= minNTicks - 1)) {
            step = std::max(1.0, step);
        }
        const double bestVMin = std::floor(vMin / step) * step;

        // Find tick locations spanning the vmin-vmax range, taking into
        // account degradation of precision when there is a large offset.
        // The edge ticks beyond vmin and/or vmax are needed for the
        // "round_numbers" autolimit mode.
        const EdgeInteger edge{step, offset};
        const int low = edge.lessEqual(vMin - bestVMin);
        const int high = edge.greaterEqual(vMax - bestVMin);

        // Create ticks and limit them to [vMin, vMax]
        auto ticks =
            std::ranges::iota_view(low, high + 1) |
            std::views::transform([step, bestVMin](auto i) { return i * step + bestVMin; }) |
            std::views::filter([vMin, vMax](double t) { return t >= vMin && t <= vMax; });
        if (std::ranges::distance(ticks) >= minNTicks) {
            result.start = ticks.front() + offset;
            result.stop = ticks.back() + offset;
            result.step = step;
            break;
        }
    }

    return result;
}

void labelingLimits(double valueMin, double valueMax, std::vector<double>& positions,
                    bool includeZero) {

    positions.clear();
    if (almostEqual(valueMin, valueMax)) {
        positions.emplace_back(valueMin);
    } else {
        positions.emplace_back(valueMin);
        if (includeZero && (std::signbit(valueMin) != std::signbit(valueMax)) &&
            !(almostEqual(valueMin, 0.0) || almostEqual(valueMax, 0.0))) {
            // include 0.0 if neither limit is zero and they have different signs
            positions.emplace_back(0.0);
        }
        positions.emplace_back(valueMax);
    }
}

void updateLabelPositions(std::vector<double>& major, std::vector<double>& minor,
                          LabelingAlgorithm algorithm, const dvec2& range, int maxTicks,
                          int minorTickFrequency, bool fillAxis) {
    const auto ensurePositiveRange = [](const dvec2& r, auto&& fun) -> LinearRange {
        if (r.x <= r.y) {
            return fun(r);
        } else {
            const LinearRange o = fun(dvec2(r.y, r.x));
            return {o.stop, o.start, -o.step};
        }
    };

    const auto fill = [&](const dvec2& r, const LinearRange& opt) {
        return fillAxis ? r : dvec2{opt.start, opt.stop};
    };

    switch (algorithm) {
        using enum LabelingAlgorithm;
        case Heckbert: {
            const auto optRange = ensurePositiveRange(
                range, [&](const auto& r) { return labelingHeckbert(r.x, r.y, maxTicks); });
            linearRange(fill(range, optRange), optRange, minorTickFrequency, major, minor);
            break;
        }
        case Matplotlib: {
            const auto optRange = ensurePositiveRange(
                range, [&](const auto& r) { return labelingMatplotlib(r.x, r.y, maxTicks); });
            linearRange(fill(range, optRange), optRange, minorTickFrequency, major, minor);
            break;
        }
        case ExtendedWilkinson: {
            const auto optRange = ensurePositiveRange(range, [&](const auto& r) {
                return labelingExtendedWilkinson(r.x, r.y, maxTicks);
            });
            linearRange(fill(range, optRange), optRange, minorTickFrequency, major, minor);
            break;
        }
        case Limits: {
            minor.clear();
            labelingLimits(range.x, range.y, major);
            break;
        }
    }
}

void updateLabels(std::vector<std::string>& labels, const std::vector<double>& positions,
                  std::string_view format) {
    if (positions.size() != labels.size()) {
        labels.resize(positions.size());
    }
    for (auto&& [pos, label] : std::views::zip(positions, labels)) {
        label.clear();
        fmt::format_to(std::back_inserter(label), fmt::runtime(format), pos);
    }
}

void updateLabels(std::vector<std::string>& labels, std::span<const std::string> srcLabels) {
    labels.assign(srcLabels.begin(), srcLabels.end());
}

}  // namespace inviwo::plot
