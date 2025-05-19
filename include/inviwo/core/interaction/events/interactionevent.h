/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2025 Inviwo Foundation
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
#include <inviwo/core/io/serialization/serializable.h>
#include <inviwo/core/interaction/events/event.h>
#include <inviwo/core/interaction/events/keyboardkeys.h>
#include <inviwo/core/interaction/contextmenuaction.h>
#include <inviwo/core/util/glmvec.h>

#include <functional>
#include <string_view>
#include <span>

namespace inviwo {

class IVW_CORE_API InteractionEvent : public Event {
public:
    explicit InteractionEvent(KeyModifiers modifiers = KeyModifiers(flags::empty));
    InteractionEvent(const InteractionEvent& rhs) = default;
    InteractionEvent& operator=(const InteractionEvent& that) = default;
    virtual InteractionEvent* clone() const override = 0;
    virtual ~InteractionEvent() = default;

    KeyModifiers modifiers() const;
    void setModifiers(KeyModifiers modifiers);
    std::string modifierNames() const;

    /**
     * Display a tool tip using the optionally set tool tip callback.
     * If no tool tip callback is set, the function does nothing.
     * The supported formation depends on the used back end, but simple html is usually supported.
     * Calling the function with an empty sting will hide any existing tool tip.
     */
    void setToolTip(std::string_view tooltip) const;

    using ToolTipCallback = std::function<void(std::string_view)>;
    /**
     * Set a tool tip call back function. The function should display a tool tip with the given
     * string at the position of the event. This function is usually called by the originating
     * event canvas, and not any regular user code.
     */
    void setToolTipCallback(ToolTipCallback callback);
    const ToolTipCallback& getToolTipCallback() const;

    /**
     * Show a context menu at the given @p normalizedPosition. The custom menu @p entries are added
     * before any other default actions based on @p categories. When any of the custom menu actions
     * in the @p entries is triggered, a ContexMenuEvent with the corresponding entry ID will be
     * propagated through the processor network.
     * @param normalizedPosition  position in normalized coordinates
     * @param entries     list of custom menu items
     * @param categories  determines which menu actions should be included in the context menu
     *
     * @see ContextMenuCategories
     */
    void showContextMenu(dvec2 normalizedPosition, std::span<ContextMenuEntry> entries,
                         ContextMenuCategories categories = ContextMenuCategory::Callback);

    using ContextMenuCallback = std::function<void(dvec2, std::span<ContextMenuEntry>,
                                                   ContextMenuCategories, InteractionEvent*)>;
    void setContextMenuCallback(ContextMenuCallback callback);
    const ContextMenuCallback& getContexMenuCallback() const;

protected:
    KeyModifiers modifiers_;
    ToolTipCallback tooltip_;
    ContextMenuCallback contextMenuCallback_;
};

}  // namespace inviwo
