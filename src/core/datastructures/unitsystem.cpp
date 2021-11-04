/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2021 Inviwo Foundation
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

#include <inviwo/core/datastructures/unitsystem.h>

#include <inviwo/core/algorithm/permutations.h>
#include <inviwo/core/util/zip.h>

#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/util/settings/unitsettings.h>

#include <algorithm>

namespace inviwo {

namespace {

template <typename OP>
constexpr bool any(units::detail::unit_data a, units::detail::unit_data b, OP op) {
    for (auto&& [name, abbr, getter] : detail::baseUnits) {
        auto aPow = std::invoke(getter, a);
        auto bPow = std::invoke(getter, b);
        if (std::invoke(op, aPow, bPow)) {
            return true;
        }
    }
    return false;
}
template <typename OP>
constexpr bool all(units::detail::unit_data a, units::detail::unit_data b, OP op) {
    for (auto&& [name, abbr, getter] : detail::baseUnits) {
        const auto aPow = std::invoke(getter, a);
        const auto bPow = std::invoke(getter, b);
        if (!std::invoke(op, aPow, bPow)) {
            return false;
        }
    }
    return true;
}

constexpr bool isSubset(units::detail::unit_data part, units::detail::unit_data whole) {
    for (auto&& [name, abbr, getter] : detail::baseUnits) {
        const auto pPow = std::invoke(getter, part);
        const auto wPow = std::invoke(getter, whole);

        if (wPow == 0 && pPow != 0) return false;
    }
    return true;
}

/**
 * Find the largest integer factor x such that x*part <= whole
 */
constexpr int maxPower(units::detail::unit_data part, units::detail::unit_data whole) {
    int factor = 0;
    for (auto&& [name, abbr, getter] : detail::baseUnits) {
        auto pPow = std::invoke(getter, part);
        auto wPow = std::invoke(getter, whole);

        if (pPow != 0) {
            const int ratio = wPow / pPow;

            if (factor > 0) {
                if (ratio < 0) return 0;
                factor = std::min(factor, ratio);
            } else if (factor < 0) {
                if (ratio > 0) return 0;
                factor = std::max(factor, ratio);
            } else {
                factor = ratio;
            }
        }
    }
    return factor;
}

}  // namespace

unitgroups::EnabledGroups util::getSystemUnitGroups() {
    unitgroups::EnabledGroups enabledGroups = unitgroups::allGroups;

    if (InviwoApplication::isInitialized()) {
        if (auto us = InviwoApplication::getPtr()->getSettingsByType<UnitSettings>()) {
            for (auto&& [enabled, prop] : util::zip(enabledGroups, us->unitGroups)) {
                enabled = prop.get();
            }
        }
    }
    return enabledGroups;
}

std::unordered_map<units::detail::unit_data, std::vector<UnitDesc>> util::getUnitGroupsFor(
    Unit unit, const unitgroups::EnabledGroups& enabledGroups) {

    std::unordered_map<units::detail::unit_data, std::vector<UnitDesc>> groups;
    for (const auto& [enabled, group] : util::zip(enabledGroups, unitgroups::groups)) {
        if (!enabled) continue;
        for (const auto& desc : group.units) {
            if (isSubset(desc.unit.base_units(), unit.base_units()) &&
                inviwo::all(desc.unit.base_units(), unit.base_units(),
                            [](auto a, auto b) { return std::abs(a) <= std::abs(b); }))
                groups[desc.unit.base_units()].push_back(desc);
        }
    }
    return groups;
}

std::pair<double, std::vector<std::tuple<std::string_view, std::string_view, int>>>
util::findBestSetOfNamedUnits(Unit unit, const unitgroups::EnabledGroups& enabledGroups,
                              UseUnitPrefixes usesPrefixes) {

    auto groups = getUnitGroupsFor(unit, enabledGroups);

    std::vector<units::detail::unit_data> bases;
    for (const auto& [base, descs] : groups) {
        bases.push_back(base);
    }

    std::vector<std::vector<std::pair<units::detail::unit_data, int>>> matches;
    {
        std::vector<std::pair<units::detail::unit_data, int>> match;
        match.reserve(groups.size());
        for (size_t searchGroupSize = 1; searchGroupSize <= groups.size(); ++searchGroupSize) {
            util::Combinations comb(util::span{bases}, searchGroupSize);
            do {
                auto test = Unit{}.base_units();
                match.clear();
                for (auto base : comb) {
                    const auto pow = maxPower(base, unit.base_units());
                    test = test * base.pow(pow);
                    match.emplace_back(base, pow);
                }
                if (test == unit.base_units()) {
                    matches.push_back(match);
                }
            } while (comb.next());

            if (!matches.empty()) break;
        }
    }

    auto closestMultiplier = std::numeric_limits<double>::max();

    static constexpr std::string_view empty = "";
    constexpr auto siPrefixHalf = static_cast<int>(unitgroups::prefixes.size() / 2);
    std::vector<std::string_view> prefixes;

    std::vector<std::tuple<std::string_view, std::string_view, int>> winner;

    for (const auto& match : matches) {
        std::vector<size_t> groupSizes;
        for (const auto& [base, pow] : match) {
            groupSizes.push_back(groups[base].size());
        }

        util::IndexProduct<size_t> inds(groupSizes);
        do {
            double testMultiplier = 1.0;
            for (auto&& [ind, m] : util::zip(inds, match)) {
                auto&& [base, pow] = m;
                const auto& desc = groups[base][ind];
                if (pow < 0 && desc.flags.contains(UnitFlag::OnlyPositivePowers)) {
                    testMultiplier = std::numeric_limits<double>::max();
                }
                testMultiplier *= std::pow(desc.unit.multiplier(), pow);
            }

            prefixes.clear();
            prefixes.resize(match.size(), empty);
            if (usesPrefixes == UseUnitPrefixes::Yes) {
                for (auto&& [ind, m, prefix] : util::zip(inds, match, prefixes)) {
                    auto&& [base, pow] = m;
                    const auto& desc = groups[base][ind];
                    if (!desc.flags.contains(UnitFlag::UsesPrefix)) continue;

                    const auto logRatio = std::log10(std::abs(unit.multiplier() / testMultiplier));
                    auto prefixInd = static_cast<int>(logRatio / pow / 3);
                    prefixInd = std::clamp(prefixInd, -siPrefixHalf, siPrefixHalf);
                    const auto& pre = unitgroups::prefixes[siPrefixHalf + prefixInd];
                    prefix = pre.abbr;
                    testMultiplier *= std::pow(pre.unit.multiplier(), pow);
                }
            }

            if (std::abs(unit.multiplier() - testMultiplier) <
                std::abs(unit.multiplier() - closestMultiplier)) {

                closestMultiplier = testMultiplier;

                winner.clear();
                for (auto&& [ind, m, prefix] : util::zip(inds, match, prefixes)) {
                    auto&& [base, pow] = m;
                    winner.emplace_back(prefix, groups[base][ind].abbr, pow);
                }
            }

        } while (inds.next());
    }

    return {unit.multiplier() / closestMultiplier, winner};
}

}  // namespace inviwo
