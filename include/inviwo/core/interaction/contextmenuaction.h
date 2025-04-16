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

namespace inviwo {

/**
 * Datastructure for a custom menu entry in a context menu. An empty label will result in a menu
 * separator instead of a menu entry. The id should be unique, for example
 * <tt><getIdentifier()>.myAction</tt>. Otherwise it cannnot be guaranteed that the correct
 * InteractionHandler/Processor handles the corresponding ContextMenuEvent. If a menu entry is
 * triggered, a ContextMenuEvent with the id will be propagated through the processor network.
 *
 * \see ContextMenuEvent
 */
struct ContextMenuEntry {
    std::string label;
    std::string id;
};

/**
 * Various categories for default and custom actions in a context menu.
 */
enum class ContextMenuAction : int {
    Empty = 0,
    Image = 1 << 0,   //!< Save and copy image layers \see utilqt::addImageActions
    View = 1 << 1,    //!< Adjust camera view \see utilqt::addViewActions
    Widget = 1 << 2,  //!< Show/hide canvas, fullscreen \see CanvasProcessorWidgetQt::contextMenu
    Custom = 1 << 3,  //!< Additional custom actions
};

ALLOW_FLAGS_FOR_ENUM(ContextMenuAction)
using ContextMenuActions = flags::flags<ContextMenuAction>;

namespace util {

constexpr ContextMenuActions defaultMenuActions =
    ContextMenuAction::Image | ContextMenuAction::View | ContextMenuAction::Widget;

}  // namespace util

IVW_CORE_API std::string_view enumToStr(ContextMenuAction a);

inline std::string_view format_as(ContextMenuAction a) { return enumToStr(a); }

}  // namespace inviwo

#ifndef DOXYGEN_SHOULD_SKIP_THIS
template <>
struct fmt::formatter<inviwo::ContextMenuActions>
    : inviwo::FlagsFormatter<inviwo::ContextMenuActions> {};
#endif
