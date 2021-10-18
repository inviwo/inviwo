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
#include <mutex>
#include <algorithm>

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
    BrushingTarget() : target_{Row.target_} {}
    explicit BrushingTarget(std::string_view target) : target_{findOrAdd(target)} {}
    BrushingTarget(const BrushingTarget& rhs) : target_{rhs.target_} {}

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
    IVW_MODULE_BRUSHINGANDLINKING_API friend std::ostream& operator<<(std::ostream& os,
                                                                      BrushingTarget bt);
    template <class Elem, class Traits>
    friend std::basic_istream<Elem, Traits>& operator>>(std::basic_istream<Elem, Traits>& ss,
                                                        BrushingTarget& bt);

    std::string_view getString() const { return target_; }

    static const BrushingTarget Row;
    static const BrushingTarget Column;

private:
    std::string_view findOrAdd(std::string_view target) {
        static std::mutex mutex;
        static std::vector<std::unique_ptr<const std::string>> targets{};
        std::scoped_lock lock{mutex};
        const auto it = std::find_if(
            targets.begin(), targets.end(),
            [&](const std::unique_ptr<const std::string>& ptr) { return *ptr == target; });
        if (it == targets.end()) {
            return std::string_view{
                *targets.emplace_back(std::make_unique<const std::string>(target))};
        } else {
            return std::string_view{**it};
        }
    }
    std::string_view target_;
};

template <class Elem, class Traits>
std::basic_istream<Elem, Traits>& operator>>(std::basic_istream<Elem, Traits>& ss,
                                             BrushingTarget& bt) {
    std::string str;
    ss >> str;
    bt = BrushingTarget(str);
    return ss;
}

}  // namespace inviwo

namespace std {
template <>
struct hash<typename inviwo::BrushingTarget> {
    size_t operator()(typename inviwo::BrushingTarget const& val) const {
        return std::hash<std::string_view>{}(val.getString());
    }
};

}  // namespace std
