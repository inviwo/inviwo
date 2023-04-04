/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2021-2023 Inviwo Foundation
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

constexpr std::array<
    std::tuple<std::string_view, std::string_view, int (units::detail::unit_data::*)() const>, 10>
    baseUnits{{{"meter", "m", &units::detail::unit_data::meter},
               {"kilogram", "kg", &units::detail::unit_data::kg},
               {"second", "s", &units::detail::unit_data::second},
               {"ampere", "A", &units::detail::unit_data::ampere},
               {"kelvin", "K", &units::detail::unit_data::kelvin},
               {"mole", "mol", &units::detail::unit_data::mole},
               {"candela", "Cd", &units::detail::unit_data::candela},
               {"currency", "$", &units::detail::unit_data::currency},
               {"count", "#", &units::detail::unit_data::count},
               {"radian", "rad", &units::detail::unit_data::radian}}};

template <typename OP>
constexpr bool any(units::detail::unit_data a, units::detail::unit_data b, OP op) {
    for (auto&& [name, abbr, getter] : baseUnits) {
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
    for (auto&& [name, abbr, getter] : baseUnits) {
        const auto aPow = std::invoke(getter, a);
        const auto bPow = std::invoke(getter, b);
        if (!std::invoke(op, aPow, bPow)) {
            return false;
        }
    }
    return true;
}

constexpr bool isSubset(units::detail::unit_data part, units::detail::unit_data whole) {
    for (auto&& [name, abbr, getter] : baseUnits) {
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
    for (auto&& [name, abbr, getter] : baseUnits) {
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
    for (auto&& [enabled, group] : util::zip(enabledGroups, unitgroups::groups)) {
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
            util::Combinations<units::detail::unit_data> comb{bases, searchGroupSize};
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
                if (match.size() > 1 && desc.flags.contains(UnitFlag::OnlyByItSelf)) {
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

    // Order units alphabetically to make it deterministic (sort by abbr)
    std::sort(winner.begin(), winner.end(),
              [](const auto& a, const auto& b) { return std::get<1>(a) < std::get<1>(b); });

    return {unit.multiplier() / closestMultiplier, winner};
}

std::back_insert_iterator<fmt::memory_buffer> util::formatUnitTo(
    std::back_insert_iterator<fmt::memory_buffer> it, Unit unit,
    const unitgroups::EnabledGroups& enabledGroups, UseUnitPrefixes usesPrefixes) {

    // All the unicode superscript digits from 0 to 9 but we let 0,1 ("\u2070", "\u00B9") be
    // empty since they are not needed
    constexpr std::array<std::string_view, 10> powers = {
        "", "", "\u00B2", "\u00B3", "\u2074", "\u2075", "\u2076", "\u2077", "\u2078", "\u2079"};

    const auto [mult, niceUnits] =
        ::inviwo::util::findBestSetOfNamedUnits(unit, enabledGroups, usesPrefixes);

    if (mult != 1.0) {
        it = fmt::format_to(it, "{:4.2g} ", mult);
    }
    int neg = 0;
    int pos = 0;
    for (auto&& [prefix, abbr, pow] : niceUnits) {
        if (pow > 0) {
            it = std::copy(prefix.begin(), prefix.end(), it);
            it = std::copy(abbr.begin(), abbr.end(), it);
            it = std::copy(powers[pow].begin(), powers[pow].end(), it);
            ++pos;
        } else if (pow < 0) {
            ++neg;
        }
    }

    if (pos == 0) *it++ = '1';

    if (neg != 0) {
        *it++ = '/';
        if (neg > 1) *it++ = '(';
        for (auto&& [prefix, abbr, pow] : niceUnits) {
            if (pow < 0) {
                it = std::copy(prefix.begin(), prefix.end(), it);
                it = std::copy(abbr.begin(), abbr.end(), it);
                it = std::copy(powers[-pow].begin(), powers[-pow].end(), it);
            }
        }
        if (neg > 1) *it++ = ')';
    }

    return it;
}

}  // namespace inviwo
