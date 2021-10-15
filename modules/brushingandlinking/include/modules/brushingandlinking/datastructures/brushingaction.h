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

#include <string_view>
#include <array>
#include <functional>
#include <ostream>
#include <istream>

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

template <class Elem, class Traits>
std::basic_ostream<Elem, Traits>& operator<<(std::basic_ostream<Elem, Traits>& ss,
                                             BrushingAction action) {
    switch (action) {
        case BrushingAction::Filter:
            ss << "Filter";
            break;
        case BrushingAction::Select:
            ss << "Select";
            break;
        case BrushingAction::Highlight:
            ss << "Highlight";
            break;
        case BrushingAction::NumberOfActions:
        default:
            ss << "Not specified";
    }
    return ss;
}

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

/**
 * Represents a target for brushing and linking actions.
 *
 * Commonly used brushing targets are BrushingTarget::Row and BrushingTarget::Column
 *
 * \see BrushingAndLinkingManager
 */
struct IVW_MODULE_BRUSHINGANDLINKING_API BrushingTarget {
    BrushingTarget() = default;
    explicit BrushingTarget(std::string_view target) : target(target) {}

    inline friend bool operator==(const BrushingTarget& lhs, const BrushingTarget& rhs) {
        return lhs.target == rhs.target;
    }
    inline friend bool operator<(const BrushingTarget& lhs, const BrushingTarget& rhs) {
        return lhs.target < rhs.target;
    }
    inline friend bool operator!=(const BrushingTarget& lhs, const BrushingTarget& rhs) {
        return !operator==(lhs, rhs);
    }
    inline friend bool operator>(const BrushingTarget& lhs, const BrushingTarget& rhs) {
        return operator<(rhs, lhs);
    }
    inline friend bool operator<=(const BrushingTarget& lhs, const BrushingTarget& rhs) {
        return !operator>(lhs, rhs);
    }
    inline friend bool operator>=(const BrushingTarget& lhs, const BrushingTarget& rhs) {
        return !operator<(lhs, rhs);
    }

    IVW_MODULE_BRUSHINGANDLINKING_API friend std::ostream& operator<<(std::ostream& os,
                                                                      const BrushingTarget& bt);

    template <class Elem, class Traits>
    friend std::basic_istream<Elem, Traits>& operator>>(std::basic_istream<Elem, Traits>& ss,
                                                        BrushingTarget& bt);

    static const BrushingTarget Row;
    static const BrushingTarget Column;

    std::string target;
};

template <class Elem, class Traits>
std::basic_istream<Elem, Traits>& operator>>(std::basic_istream<Elem, Traits>& ss,
                                             BrushingTarget& bt) {
    std::string str;
    ss >> str;

    if (str == toString(BrushingTarget::Row.target)) {
        bt = BrushingTarget::Row;
    } else if (str == toString(BrushingTarget::Column)) {
        bt = BrushingTarget::Column;
    } else {
        bt = BrushingTarget(str);
    }

    return ss;
}

}  // namespace inviwo

namespace std {
template <>
struct hash<typename inviwo::BrushingTarget> {
    size_t operator()(typename inviwo::BrushingTarget const& val) const {
        return std::hash<std::string>{}(val.target);
    }
};

}  // namespace std
