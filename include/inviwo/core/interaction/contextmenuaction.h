/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2025 Inviwo Foundation
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
#include <inviwo/core/util/fmtutils.h>

#include <flags/flags.h>
#include <string_view>
#include <iosfwd>
#include <cstdint>
#include <span>
#include <string>
#include <vector>
#include <optional>
#include <variant>
#include <any>

namespace inviwo {

struct ContextMenuAction;
struct ContextMenuSeparator;
struct ContextMenuSubmenu;
using ContextMenuEntry = std::variant<ContextMenuAction, ContextMenuSubmenu, ContextMenuSeparator>;

/**
 * Data structure for a custom menu action in a context menu. The id should be unique, for example
 * <tt>"<getIdentifier()>.myAction"</tt>. Otherwise it cannot be guaranteed that the correct
 * InteractionHandler/Processor handles the corresponding ContextMenuEvent. If a menu action is
 * triggered, a ContextMenuEvent with the id will be propagated through the processor network.
 *
 * @see ContextMenuEvent ContextMenuSeparator ContextMenuSubmenu
 */
struct ContextMenuAction {
    std::string id;
    std::string label = {};
    std::optional<std::string> iconPath = std::nullopt;
    std::any data = {};
    bool enabled = true;
};

struct ContextMenuSeparator {};

struct ContextMenuSubmenu {
    std::string label;
    std::optional<std::string> iconPath = std::nullopt;
    std::vector<ContextMenuEntry> childEntries;
};

/**
 * Various categories for default and custom callbacks in a context menu.
 */
enum class ContextMenuCategory : int {  // NOLINT(performance-enum-size)
    Empty = 0,
    Image = 1 << 0,     //!< Save and copy image layers @see utilqt::addImageActions
    View = 1 << 1,      //!< Adjust camera view @see utilqt::addViewActions
    Widget = 1 << 2,    //!< Show/hide canvas, fullscreen @see CanvasProcessorWidgetQt::contextMenu
    Callback = 1 << 3,  //!< Additional actions for custom callbacks triggering ContextMenuEvent
};

ALLOW_FLAGS_FOR_ENUM(ContextMenuCategory)
using ContextMenuCategories = flags::flags<ContextMenuCategory>;

namespace util {

constexpr ContextMenuCategories defaultMenuCategories =
    ContextMenuCategory::Image | ContextMenuCategory::View | ContextMenuCategory::Widget;

}  // namespace util

IVW_CORE_API std::string_view enumToStr(ContextMenuCategory a);

inline std::string_view format_as(ContextMenuCategory a) { return enumToStr(a); }

}  // namespace inviwo

#ifndef DOXYGEN_SHOULD_SKIP_THIS
template <>
struct fmt::formatter<inviwo::ContextMenuCategories>
    : inviwo::FlagsFormatter<inviwo::ContextMenuCategories> {};
#endif
