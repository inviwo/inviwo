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
#include <inviwo/core/interaction/events/event.h>
#include <inviwo/core/interaction/events/interactionevent.h>
#include <inviwo/core/util/constexprhash.h>
#include <inviwo/core/util/glmvec.h>

#include <string_view>
#include <memory>
#include <any>

namespace inviwo {

/**
 * A ContextMenuEvent is triggered when a custom context menu entry is selected.
 *
 * @see ContextMenuAction
 */
class IVW_CORE_API ContextMenuEvent : public Event {
public:
    ContextMenuEvent(std::string_view id, KeyModifiers modifiers, std::any data);
    ContextMenuEvent(const ContextMenuEvent& rhs) = default;
    ContextMenuEvent(ContextMenuEvent&&) = default;
    ContextMenuEvent& operator=(const ContextMenuEvent& that) = default;
    ContextMenuEvent& operator=(ContextMenuEvent&&) = default;

    virtual ~ContextMenuEvent();

    virtual ContextMenuEvent* clone() const override;

    /**
     * ID of the triggered context menu action
     */
    std::string_view getId() const;

    virtual uint64_t hash() const override;
    static constexpr uint64_t chash() {
        return util::constexpr_hash("org.inviwo.ContextMenuEvent");
    }

    KeyModifiers modifiers() const;
    void setModifiers(KeyModifiers modifiers);

    const std::any& data() const;
    void setData(std::any data);

private:
    std::string id_;
    KeyModifiers modifiers_;
    std::any data_;
};

}  // namespace inviwo
