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

#include <modules/brushingandlinking/datastructures/brushingaction.h>

#include <inviwo/core/util/exception.h>      // for Exception
#include <inviwo/core/util/ostreamjoiner.h>  // for ostream_joiner, make_ostream_joiner
#include <inviwo/core/util/sourcecontext.h>  // for IVW_CONTEXT_CUSTOM

#include <algorithm>    // for copy, find_if
#include <iostream>     // for char_traits, operator<<, operator>>
#include <mutex>        // for mutex, scoped_lock
#include <string>       // for string, basic_string, basic_string<>::__self...
#include <type_traits>  // for decay_t
#include <vector>       // for vector

#include <flags/iterator.h>  // for operator!=

namespace inviwo {

const BrushingTarget BrushingTarget::Row("row");
const BrushingTarget BrushingTarget::Column("column");

std::string_view BrushingTarget::findOrAdd(std::string_view target) {
    static std::mutex mutex;
    static std::vector<std::unique_ptr<const std::string>> targets{};
    std::scoped_lock lock{mutex};
    const auto it =
        std::find_if(targets.begin(), targets.end(),
                     [&](const std::unique_ptr<const std::string>& ptr) { return *ptr == target; });
    if (it == targets.end()) {
        return std::string_view{*targets.emplace_back(std::make_unique<const std::string>(target))};
    } else {
        return std::string_view{**it};
    }
}

std::string_view enumToStr(BrushingAction action) {
    switch (action) {
        case BrushingAction::Filter:
            return "Filter";
        case BrushingAction::Select:
            return "Select";
        case BrushingAction::Highlight:
            return "Highlight";
    }
    throw Exception(IVW_CONTEXT_CUSTOM("enumToStr"), "Found invalid BrushingAction enum value '{}'",
                    static_cast<int>(action));
}

std::string_view enumToStr(BrushingModification bm) {
    switch (bm) {
        case BrushingModification::Filtered:
            return "Filtered";
        case BrushingModification::Selected:
            return "Selected";
        case BrushingModification::Highlighted:
            return "Highlighted";
    }
    throw Exception(IVW_CONTEXT_CUSTOM("enumToStr"),
                    "Found invalid BrushingModification enum value '{}'", static_cast<int>(bm));
}

std::ostream& operator<<(std::ostream& ss, BrushingAction action) {
    return ss << enumToStr(action);
}
std::ostream& operator<<(std::ostream& ss, BrushingModification action) {
    return ss << enumToStr(action);
}
std::ostream& operator<<(std::ostream& ss, BrushingModifications action) {
    std::copy(action.begin(), action.end(), util::make_ostream_joiner(ss, "+"));
    return ss;
}

std::istream& operator>>(std::istream& ss, BrushingTarget& bt) {
    std::string str;
    ss >> str;
    bt = BrushingTarget(str);
    return ss;
}

}  // namespace inviwo
