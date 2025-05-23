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

#include <inviwo/core/interaction/events/contextmenuevent.h>
#include <inviwo/core/interaction/events/interactionevent.h>

namespace inviwo {

ContextMenuEvent::ContextMenuEvent(std::string_view id, InteractionEvent* event)
    : id_{id}, owner_{nullptr}, event_{event} {}

ContextMenuEvent::ContextMenuEvent(std::string_view id, std::unique_ptr<InteractionEvent> event)
    : ContextMenuEvent(id, event.get()) {
    owner_ = std::move(event);
}

ContextMenuEvent::ContextMenuEvent(const ContextMenuEvent& rhs)
    : Event{rhs}, id_{rhs.id_}, owner_{rhs.event_->clone()}, event_{owner_.get()} {}

ContextMenuEvent& ContextMenuEvent::operator=(const ContextMenuEvent& that) {
    if (this != &that) {
        Event::operator=(that);
        owner_.reset(that.event_->clone());
        event_ = owner_.get();
    }
    return *this;
}

ContextMenuEvent::~ContextMenuEvent() = default;

ContextMenuEvent* ContextMenuEvent::clone() const { return new ContextMenuEvent(*this); }

std::string_view ContextMenuEvent::getId() const { return id_; }

uint64_t ContextMenuEvent::hash() const { return chash(); }

InteractionEvent* ContextMenuEvent::getEvent() const { return event_; }

}  // namespace inviwo
