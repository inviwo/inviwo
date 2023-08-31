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
#pragma once

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/util/stdextensions.h>
#include <inviwo/core/util/logcentral.h>

#include <flags/flags.h>
#include <fmt/format.h>
#include <llnl-units/units.hpp>

#include <string_view>
#include <tuple>
#include <unordered_map>
#include <optional>
#include <iterator>
#include <vector>
#include <utility>
#include <span>

namespace inviwo {

using Unit = units::precise_unit;

struct IVW_CORE_API Axis {
    std::string name;
    Unit unit;
};

namespace util {

constexpr std::array<std::string_view, 4> defaultAxesNames = {"x", "y", "z", "t"};
constexpr std::array<Unit, 4> defaultAxesUnits = {Unit{}, Unit{}, Unit{}, Unit{}};

template <size_t N>
std::array<Axis, N> defaultAxes() {
    static_assert(N <= defaultAxesNames.size());
    return util::make_array<N>([](auto i) {
        return Axis{std::string{defaultAxesNames[i]}, defaultAxesUnits[i]};
    });
}

}  // namespace util

enum class UnitFlag {
    None = 0,
    UsesPrefix = 1 << 0,
    OnlyPositivePowers = 1 << 1,
    OnlyByItSelf = 1 << 2
};
ALLOW_FLAGS_FOR_ENUM(UnitFlag)
using UnitFlags = flags::flags<UnitFlag>;

struct UnitDesc {
    Unit unit;
    std::string_view name;
    std::string_view abbr;
    UnitFlags flags;
};

namespace unitgroups {

// clang-format off
constexpr std::array<UnitDesc, 10> si = {{
    {units::precise::meter,    "meter",    "m",   UnitFlag::UsesPrefix},
    {units::precise::kg,       "kilogram", "kg",  UnitFlag::None},
    {units::precise::second,   "second",   "s",   UnitFlag::UsesPrefix},
    {units::precise::Ampere,   "ampere",   "A",   UnitFlag::UsesPrefix},
    {units::precise::Kelvin,   "kelvin",   "K",   UnitFlag::UsesPrefix},
    {units::precise::mol,      "mole",     "mol", UnitFlag::UsesPrefix},
    {units::precise::candela,  "candela",  "Cd",  UnitFlag::UsesPrefix},
    {units::precise::currency, "currency", "$",   UnitFlag::UsesPrefix},
    {units::precise::count,    "count",    "#",   UnitFlag::None},
    {units::precise::radian,   "radian",   "rad", UnitFlag::None}
}};

constexpr std::array<UnitDesc, 17> derived = {{
    {units::precise::hertz,     "hertz",     "Hz",  UnitFlag::UsesPrefix |
                                                        UnitFlag::OnlyPositivePowers |
                                                        UnitFlag::OnlyByItSelf},
    {units::precise::volt,      "volt",      "V",   UnitFlag::UsesPrefix},
    {units::precise::newton,    "newton",    "N",   UnitFlag::UsesPrefix},
    {units::precise::Pa,        "pascal",    "Pa",  UnitFlag::UsesPrefix},
    {units::precise::joule,     "joule",     "J",   UnitFlag::UsesPrefix},
    {units::precise::watt,      "watt",      "W",   UnitFlag::UsesPrefix},
    {units::precise::farad,     "farad",     "F",   UnitFlag::UsesPrefix},
    {units::precise::siemens,   "siemens",   "S",   UnitFlag::UsesPrefix},
    {units::precise::weber,     "weber",     "Wb",  UnitFlag::UsesPrefix},
    {units::precise::tesla,     "tesla",     "T",   UnitFlag::UsesPrefix},
    {units::precise::henry,     "henry",     "H",   UnitFlag::UsesPrefix},
    {units::precise::lumen,     "lumen",     "lm",  UnitFlag::UsesPrefix},
    {units::precise::lux,       "lux",       "lx",  UnitFlag::UsesPrefix},
    {units::precise::becquerel, "becquerel", "Bq",  UnitFlag::UsesPrefix},
    {units::precise::gray,      "gray",      "Gy",  UnitFlag::UsesPrefix},
    {units::precise::sievert,   "sievert",   "Sv",  UnitFlag::UsesPrefix},
    {units::precise::katal,     "katal",     "kat", UnitFlag::UsesPrefix}
}};

constexpr std::array<UnitDesc, 3> extra = {{
    {units::precise::bar, "bar",   "bar", UnitFlag::UsesPrefix},
    {units::precise::L,   "liter", "L",   UnitFlag::UsesPrefix},
    {units::precise::g,   "gram",  "g",   UnitFlag::UsesPrefix}
}};

constexpr std::array<UnitDesc, 5> time = {{
    {units::precise::time::min,  "minute", "min",  UnitFlag::None},
    {units::precise::time::h,    "hour",   "h",    UnitFlag::None},
    {units::precise::time::day,  "day",    "day",  UnitFlag::None},
    {units::precise::time::week, "week",   "week", UnitFlag::None},
    {units::precise::time::yr,   "year",   "yr",   UnitFlag::None}
}};

constexpr std::array<UnitDesc, 1> temperature = {{
    {units::precise::temperature::celsius,  "celsius", "°C", UnitFlag::None},
}};


constexpr Unit elementary_charge{1.602176634e-19, units::precise::C};

constexpr std::array<UnitDesc, 4> atomic = {{
    {units::precise::distance::angstrom,  "\u00C5ngstr\u00F6m", "\u00C5", UnitFlag::None},
    {units::precise::energy::eV,          "electron volt",      "eV",     UnitFlag::UsesPrefix},
    {units::precise::energy::hartree,     "hartree",            "Ha",     UnitFlag::None},
    {elementary_charge,                   "elementary charge",  "e",      UnitFlag::None}
}};

constexpr std::array<UnitDesc, 3> astronomical = {{
    {units::precise::distance::parsec,  "parsec",            "pc", UnitFlag::None},
    {units::precise::distance::au,      "astronomical unit", "au", UnitFlag::None},
    {units::precise::distance::ly,      "light-year",        "ly", UnitFlag::None},
}};


constexpr std::array<UnitDesc, 17> prefixes = {{
    {units::precise::yocto, "yocto", "y", UnitFlag::None},
    {units::precise::zepto, "zepto", "z", UnitFlag::None},
    {units::precise::atto,  "atto",  "a", UnitFlag::None},
    {units::precise::femto, "femto", "f", UnitFlag::None},
    {units::precise::pico,  "pico",  "p", UnitFlag::None},
    {units::precise::nano,  "nano",  "n", UnitFlag::None},
    {units::precise::micro, "micro", "\u00B5", UnitFlag::None},
    {units::precise::milli, "milli", "m", UnitFlag::None},
    {units::precise::one,   "",      "",  UnitFlag::None},
    {units::precise::kilo,  "kilo",  "k", UnitFlag::None},
    {units::precise::mega,  "mega",  "M", UnitFlag::None},
    {units::precise::giga,  "giga",  "G", UnitFlag::None},
    {units::precise::tera,  "tera",  "T", UnitFlag::None},
    {units::precise::peta,  "peta",  "P", UnitFlag::None},
    {units::precise::exa,   "exa",   "E", UnitFlag::None},
    {units::precise::zetta, "zetta", "Z", UnitFlag::None},
    {units::precise::yotta, "yotta", "Y", UnitFlag::None},
}};

// clang-format on

struct Group {
    std::string_view name;
    std::span<const UnitDesc> units;
};

constexpr std::array<Group, 7> groups = {{{"SI", si},
                                          {"Derived", derived},
                                          {"Extra", extra},
                                          {"Time", time},
                                          {"Temperature", temperature},
                                          {"Atomic", atomic},
                                          {"Astronomical", astronomical}}};

using EnabledGroups = std::array<bool, groups.size()>;
constexpr EnabledGroups allGroups = {true, true, true, true, true, true, true};
constexpr EnabledGroups siGroups = {true, false, false, false, false, false, false};
constexpr EnabledGroups extGroups = {true, true, true, false, false, false, false};

}  // namespace unitgroups

namespace util {

enum class UseUnitPrefixes { Yes, No };

IVW_CORE_API unitgroups::EnabledGroups getSystemUnitGroups();

IVW_CORE_API std::unordered_map<units::detail::unit_data, std::vector<UnitDesc>> getUnitGroupsFor(
    Unit unit, const unitgroups::EnabledGroups& enabledGroups);

IVW_CORE_API std::pair<double, std::vector<std::tuple<std::string_view, std::string_view, int>>>
findBestSetOfNamedUnits(Unit unit, const unitgroups::EnabledGroups& enabledGroups,
                        UseUnitPrefixes usesPrefixes);

IVW_CORE_API std::back_insert_iterator<fmt::memory_buffer> formatUnitTo(
    std::back_insert_iterator<fmt::memory_buffer> it, Unit unit,
    const unitgroups::EnabledGroups& enabledGroups, UseUnitPrefixes usesPrefixes);

}  // namespace util

}  // namespace inviwo

/**
 * Formatting specialization for Units
 * The Units have a Format Specification Mini-Language
 *
 * spec         ::= [unit_spec][":" format_spec]
 *
 * format_spec ::= fmt standard format specifier
 *
 * unit_spec   ::= [space][braces][prefix][units]
 *
 * space       ::= " "
 * braces      ::= "(" | "["
 * prefix      ::= "p" | "P"
 * units       ::= "si" | "ext" | "sys" | "all"
 *
 * Space:
 *    " "     Add a leading space to the unit
 *
 * Braces:    By default no braces are added.
 *    "("     Surround the unit in (unit)
 *    "["     Surround the unit in [unit]
 *
 * Prefix:
 *    "p"     Use SI prefixes (yocto to Yotta) to reduce any multiplier (default)
 *    "P"     Don't use any prefixes
 *
 * Units:
 *    "si"    Only use the basic SI unit and combination of those.
 *    "ext"   Use unit from the SI, derived and extra groups.
 *    "sys"   Use the systems currently selected set of unit groups (default)
 *    "all"   Use all known unit groups.
 *
 */
template <>
struct fmt::formatter<::inviwo::Unit> {
    formatter<std::string_view> formatter_;

    enum class UnitSystem { SI, Ext, Sys, All };
    enum class Braces { None = 0, Paren, Square };

    ::inviwo::util::UseUnitPrefixes usePrefix = ::inviwo::util::UseUnitPrefixes::Yes;
    UnitSystem unitsystem = UnitSystem::Sys;
    Braces braces = Braces::None;
    bool leadingSpace = false;

    constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin()) {
        auto it = ctx.begin();
        auto end = ctx.end();

        const auto range = std::string_view(it, end - it);
        const auto endPos = range.find_first_of(":}");
        const auto endIt =
            endPos != std::string_view::npos ? it + endPos + (range[endPos] == ':' ? 1 : 0) : end;
        auto unitFormat = range.substr(0, endPos);

        if (unitFormat.size() > 0 && unitFormat[0] == ' ') {
            leadingSpace = true;
            unitFormat.remove_prefix(1);
        }

        if (unitFormat.size() > 0 && unitFormat[0] == '(') {
            braces = Braces::Paren;
            unitFormat.remove_prefix(1);
        } else if (unitFormat.size() > 0 && unitFormat[0] == '[') {
            braces = Braces::Square;
            unitFormat.remove_prefix(1);
        }

        if (unitFormat.size() > 0 && unitFormat[0] == 'p') {
            usePrefix = ::inviwo::util::UseUnitPrefixes::Yes;
            unitFormat.remove_prefix(1);
        } else if (unitFormat.size() > 0 && unitFormat[0] == 'P') {
            usePrefix = ::inviwo::util::UseUnitPrefixes::No;
            unitFormat.remove_prefix(1);
        }

        if (unitFormat.empty()) {
        } else if (unitFormat == "si") {
            unitsystem = UnitSystem::SI;
        } else if (unitFormat == "ext") {
            unitsystem = UnitSystem::Ext;
        } else if (unitFormat == "sys") {
            unitsystem = UnitSystem::Sys;
        } else if (unitFormat == "all") {
            unitsystem = UnitSystem::All;
        } else {
            throw format_error("Invalid unit format found");
        }

        ctx.advance_to(endIt);
        return formatter_.parse(ctx);
    }

    // Formats the point p using the parsed format specification (presentation)
    // stored in this formatter.
    template <typename FormatContext>
    auto format(const ::inviwo::Unit& unit, FormatContext& ctx) -> decltype(ctx.out()) {
        // ctx.out() is an output iterator to write to.

        if (unit == ::inviwo::Unit{}) return ctx.out();

        const ::inviwo::unitgroups::EnabledGroups enabledGroups = [&]() {
            if (unitsystem == UnitSystem::SI) {
                return ::inviwo::unitgroups::siGroups;
            } else if (unitsystem == UnitSystem::Ext) {
                return ::inviwo::unitgroups::extGroups;
            } else if (unitsystem == UnitSystem::All) {
                return ::inviwo::unitgroups::allGroups;
            } else {
                return ::inviwo::util::getSystemUnitGroups();
            }
        }();

        fmt::memory_buffer buff;
        auto it = std::back_inserter(buff);

        if (leadingSpace) *it++ = ' ';
        if (braces == Braces::Paren) *it++ = '(';
        if (braces == Braces::Square) *it++ = '[';
        if (!units::is_valid(unit)) {
            it = fmt::format_to(it, "Invalid");
        } else if (units::is_error(unit)) {
            it = fmt::format_to(it, "Error");
        } else {
            it = ::inviwo::util::formatUnitTo(it, unit, enabledGroups, usePrefix);
        }
        if (braces == Braces::Paren) *it++ = ')';
        if (braces == Braces::Square) *it++ = ']';

        return formatter_.format(std::string_view{buff.data(), buff.size()}, ctx);
    }
};
