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
#pragma once

#include <modules/brushingandlinking/brushingandlinkingmoduledefine.h>

#include <inviwo/core/util/stringconversion.h>
#include <inviwo/core/util/fmtutils.h>

#include <string_view>
#include <array>
#include <iosfwd>

#include <flags/flags.h>

namespace inviwo {

/**
 * type of action for Brushing and Linking
 *
 * \see BrushingAndLinkingManager
 */
enum class BrushingAction {
    Filter,     //!< filter the given indices and mark them as removed
    Select,     //!< replace the current selection of indices
    Highlight,  //!< replace the currently highlighted indices
    NumberOfActions
};

constexpr std::array<BrushingAction, 3> BrushingActions{
    BrushingAction::Filter, BrushingAction::Select, BrushingAction::Highlight};

enum class BrushingModification {
    Filtered = 0x01,
    Selected = 0x02,
    Highlighted = 0x04,
};

constexpr BrushingModification fromAction(BrushingAction action) {
    switch (action) {
        case BrushingAction::Filter:
            return BrushingModification::Filtered;
        case BrushingAction::Select:
            return BrushingModification::Selected;
        case BrushingAction::Highlight:
            return BrushingModification::Highlighted;
        default:
            return BrushingModification::Selected;
    }
}

ALLOW_FLAGS_FOR_ENUM(BrushingModification)
using BrushingModifications = flags::flags<BrushingModification>;

IVW_MODULE_BRUSHINGANDLINKING_API std::string_view enumToStr(BrushingAction dt);
IVW_MODULE_BRUSHINGANDLINKING_API std::string_view enumToStr(BrushingModification dt);
IVW_MODULE_BRUSHINGANDLINKING_API std::ostream& operator<<(std::ostream& ss, BrushingAction action);
IVW_MODULE_BRUSHINGANDLINKING_API std::ostream& operator<<(std::ostream& ss,
                                                           BrushingModification action);
IVW_MODULE_BRUSHINGANDLINKING_API std::ostream& operator<<(std::ostream& ss,
                                                           BrushingModifications action);

/**
 * Represents a target for brushing and linking actions.
 *
 * Commonly used brushing targets are BrushingTarget::Row and BrushingTarget::Column
 *
 * \see BrushingAndLinkingManager
 */
struct IVW_MODULE_BRUSHINGANDLINKING_API BrushingTarget {
    BrushingTarget() : target_{Row.target_} {}
    explicit BrushingTarget(std::string_view target) : target_{findOrAdd(target)} {}
    BrushingTarget(const BrushingTarget& rhs) : target_{rhs.target_} {}
    BrushingTarget& operator=(const BrushingTarget& rhs) {
        target_ = rhs.target_;
        return *this;
    }

    inline friend bool operator==(BrushingTarget lhs, BrushingTarget rhs) {
        // Can optimize equal since we know each targets points to a unique str.
        // And there will be no sub strings
        return lhs.target_.data() == rhs.target_.data();
    }
    inline friend bool operator<(BrushingTarget lhs, BrushingTarget rhs) {
        return lhs.target_ < rhs.target_;
    }
    inline friend bool operator!=(BrushingTarget lhs, BrushingTarget rhs) {
        return !operator==(lhs, rhs);
    }
    inline friend bool operator>(BrushingTarget lhs, BrushingTarget rhs) {
        return operator<(rhs, lhs);
    }
    inline friend bool operator<=(BrushingTarget lhs, BrushingTarget rhs) {
        return !operator>(lhs, rhs);
    }
    inline friend bool operator>=(BrushingTarget lhs, BrushingTarget rhs) {
        return !operator<(lhs, rhs);
    }
    inline friend std::ostream& operator<<(std::ostream& os, BrushingTarget bt) {
        os << bt.getString();
        return os;
    }

    IVW_CORE_API friend std::istream& operator>>(std::istream& ss, BrushingTarget& bt);

    std::string_view getString() const { return target_; }

    static const BrushingTarget Row;
    static const BrushingTarget Column;

private:
    std::string_view findOrAdd(std::string_view target);

    std::string_view target_;
};

}  // namespace inviwo

template <>
struct std::hash<typename inviwo::BrushingTarget> {
    size_t operator()(typename inviwo::BrushingTarget const& val) const {
        return std::hash<std::string_view>{}(val.getString());
    }
};

template <>
struct fmt::formatter<inviwo::BrushingAction> : inviwo::FlagFormatter<inviwo::BrushingAction> {};

template <>
struct fmt::formatter<inviwo::BrushingModification>
    : inviwo::FlagFormatter<inviwo::BrushingModification> {};

template <>
struct fmt::formatter<inviwo::BrushingModifications>
    : inviwo::FlagsFormatter<inviwo::BrushingModifications> {};
