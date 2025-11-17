/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2021-2025 Inviwo Foundation
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

#include <warn/push>
#include <warn/ignore/all>
#if __has_include(<llnl-units/units.hpp>)
#include <llnl-units/units.hpp>
#else
#include <units/units.hpp>
#endif
#include <warn/pop>

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
    friend bool operator==(const Axis&, const Axis&) = default;
};

namespace util {

constexpr std::array<std::string_view, 4> defaultAxesNames = {"x", "y", "z", "t"};
constexpr std::array<Unit, 4> defaultAxesUnits = {Unit{}, Unit{}, Unit{}, Unit{}};

template <size_t N>
std::array<Axis, N> defaultAxes() {
    static_assert(N <= defaultAxesNames.size());
    return util::make_array<N>(
        [](auto i) { return Axis{std::string{defaultAxesNames[i]}, defaultAxesUnits[i]}; });
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
    {.unit=units::precise::meter,    .name="meter",    .abbr="m",   .flags=UnitFlag::UsesPrefix},
    {.unit=units::precise::kg,       .name="kilogram", .abbr="kg",  .flags=UnitFlag::None},
    {.unit=units::precise::second,   .name="second",   .abbr="s",   .flags=UnitFlag::UsesPrefix},
    {.unit=units::precise::Ampere,   .name="ampere",   .abbr="A",   .flags=UnitFlag::UsesPrefix},
    {.unit=units::precise::Kelvin,   .name="kelvin",   .abbr="K",   .flags=UnitFlag::UsesPrefix},
    {.unit=units::precise::mol,      .name="mole",     .abbr="mol", .flags=UnitFlag::UsesPrefix},
    {.unit=units::precise::candela,  .name="candela",  .abbr="Cd",  .flags=UnitFlag::UsesPrefix},
    {.unit=units::precise::currency, .name="currency", .abbr="$",   .flags=UnitFlag::UsesPrefix},
    {.unit=units::precise::count,    .name="count",    .abbr="#",   .flags=UnitFlag::None},
    {.unit=units::precise::radian,   .name="radian",   .abbr="rad", .flags=UnitFlag::None}
}};

constexpr std::array<UnitDesc, 17> derived = {{
    {.unit=units::precise::hertz,     .name="hertz",     .abbr="Hz",  .flags=UnitFlag::UsesPrefix |
                                                                             UnitFlag::OnlyPositivePowers |
                                                                             UnitFlag::OnlyByItSelf},
    {.unit=units::precise::volt,      .name="volt",      .abbr="V",   .flags=UnitFlag::UsesPrefix},
    {.unit=units::precise::newton,    .name="newton",    .abbr="N",   .flags=UnitFlag::UsesPrefix},
    {.unit=units::precise::Pa,        .name="pascal",    .abbr="Pa",  .flags=UnitFlag::UsesPrefix},
    {.unit=units::precise::joule,     .name="joule",     .abbr="J",   .flags=UnitFlag::UsesPrefix},
    {.unit=units::precise::watt,      .name="watt",      .abbr="W",   .flags=UnitFlag::UsesPrefix},
    {.unit=units::precise::farad,     .name="farad",     .abbr="F",   .flags=UnitFlag::UsesPrefix},
    {.unit=units::precise::siemens,   .name="siemens",   .abbr="S",   .flags=UnitFlag::UsesPrefix},
    {.unit=units::precise::weber,     .name="weber",     .abbr="Wb",  .flags=UnitFlag::UsesPrefix},
    {.unit=units::precise::tesla,     .name="tesla",     .abbr="T",   .flags=UnitFlag::UsesPrefix},
    {.unit=units::precise::henry,     .name="henry",     .abbr="H",   .flags=UnitFlag::UsesPrefix},
    {.unit=units::precise::lumen,     .name="lumen",     .abbr="lm",  .flags=UnitFlag::UsesPrefix},
    {.unit=units::precise::lux,       .name="lux",       .abbr="lx",  .flags=UnitFlag::UsesPrefix},
    {.unit=units::precise::becquerel, .name="becquerel", .abbr="Bq",  .flags=UnitFlag::UsesPrefix},
    {.unit=units::precise::gray,      .name="gray",      .abbr="Gy",  .flags=UnitFlag::UsesPrefix},
    {.unit=units::precise::sievert,   .name="sievert",   .abbr="Sv",  .flags=UnitFlag::UsesPrefix},
    {.unit=units::precise::katal,     .name="katal",     .abbr="kat", .flags=UnitFlag::UsesPrefix}
}};

constexpr std::array<UnitDesc, 3> extra = {{
    {.unit=units::precise::bar, .name="bar",   .abbr="bar", .flags=UnitFlag::UsesPrefix},
    {.unit=units::precise::L,   .name="liter", .abbr="L",   .flags=UnitFlag::UsesPrefix},
    {.unit=units::precise::g,   .name="gram",  .abbr="g",   .flags=UnitFlag::UsesPrefix}
}};

constexpr std::array<UnitDesc, 5> time = {{
    {.unit=units::precise::time::minute,  .name="minute", .abbr="min",  .flags=UnitFlag::None},
    {.unit=units::precise::time::h,       .name="hour",   .abbr="h",    .flags=UnitFlag::None},
    {.unit=units::precise::time::day,     .name="day",    .abbr="day",  .flags=UnitFlag::None},
    {.unit=units::precise::time::week,    .name="week",   .abbr="week", .flags=UnitFlag::None},
    {.unit=units::precise::time::yr,      .name="year",   .abbr="yr",   .flags=UnitFlag::None}
}};

constexpr std::array<UnitDesc, 1> temperature = {{
    {.unit=units::precise::temperature::celsius,  .name="celsius", .abbr="°C", .flags=UnitFlag::None},
}};


constexpr Unit elementary_charge{1.602176634e-19, units::precise::C};

constexpr std::array<UnitDesc, 4> atomic = {{
    {.unit=units::precise::distance::angstrom,  .name="\u00C5ngstr\u00F6m", .abbr="\u00C5", .flags=UnitFlag::None},
    {.unit=units::precise::energy::eV,          .name="electron volt",      .abbr="eV",     .flags=UnitFlag::UsesPrefix},
    {.unit=units::precise::energy::hartree,     .name="hartree",            .abbr="Ha",     .flags=UnitFlag::None},
    {.unit=elementary_charge,                   .name="elementary charge",  .abbr="e",      .flags=UnitFlag::None}
}};

constexpr std::array<UnitDesc, 4> astronomical = {{
    {.unit=units::precise::distance::parsec,  .name="parsec",            .abbr="pc", .flags=UnitFlag::None},
    {.unit=units::precise::distance::au,      .name="astronomical unit", .abbr="au", .flags=UnitFlag::None},
    {.unit=units::precise::distance::ly,      .name="light-year",        .abbr="ly", .flags=UnitFlag::None},
    {.unit=units::precise_unit{1e-26, units::precise::W / units::precise::m / units::precise::m / units::precise::Hz},
                                              .name="Jansky",            .abbr="Jy", .flags=UnitFlag::UsesPrefix},
}};


constexpr std::array<UnitDesc, 17> prefixes = {{
    {.unit=units::precise::yocto, .name="yocto", .abbr="y", .flags=UnitFlag::None},
    {.unit=units::precise::zepto, .name="zepto", .abbr="z", .flags=UnitFlag::None},
    {.unit=units::precise::atto,  .name="atto",  .abbr="a", .flags=UnitFlag::None},
    {.unit=units::precise::femto, .name="femto", .abbr="f", .flags=UnitFlag::None},
    {.unit=units::precise::pico,  .name="pico",  .abbr="p", .flags=UnitFlag::None},
    {.unit=units::precise::nano,  .name="nano",  .abbr="n", .flags=UnitFlag::None},
    {.unit=units::precise::micro, .name="micro", .abbr="\u00B5", .flags=UnitFlag::None},
    {.unit=units::precise::milli, .name="milli", .abbr="m", .flags=UnitFlag::None},
    {.unit=units::precise::one,   .name="",      .abbr="",  .flags=UnitFlag::None},
    {.unit=units::precise::kilo,  .name="kilo",  .abbr="k", .flags=UnitFlag::None},
    {.unit=units::precise::mega,  .name="mega",  .abbr="M", .flags=UnitFlag::None},
    {.unit=units::precise::giga,  .name="giga",  .abbr="G", .flags=UnitFlag::None},
    {.unit=units::precise::tera,  .name="tera",  .abbr="T", .flags=UnitFlag::None},
    {.unit=units::precise::peta,  .name="peta",  .abbr="P", .flags=UnitFlag::None},
    {.unit=units::precise::exa,   .name="exa",   .abbr="E", .flags=UnitFlag::None},
    {.unit=units::precise::zetta, .name="zetta", .abbr="Z", .flags=UnitFlag::None},
    {.unit=units::precise::yotta, .name="yotta", .abbr="Y", .flags=UnitFlag::None},
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

#ifndef DOXYGEN_SHOULD_SKIP_THIS

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

    constexpr auto parse(format_parse_context& ctx) -> format_parse_context::iterator {
        const auto it = ctx.begin();
        const auto end = ctx.end();

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
    auto format(const ::inviwo::Unit& unit, format_context& ctx) const -> format_context::iterator {
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
#endif
