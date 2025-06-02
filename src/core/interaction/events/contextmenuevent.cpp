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

ContextMenuEvent::ContextMenuEvent(std::string_view id, KeyModifiers modifiers, std::any data)
    : id_{id}, modifiers_{modifiers}, data_{std::move(data)} {}


ContextMenuEvent::~ContextMenuEvent() = default;

ContextMenuEvent* ContextMenuEvent::clone() const { return new ContextMenuEvent(*this); }

std::string_view ContextMenuEvent::getId() const { return id_; }

uint64_t ContextMenuEvent::hash() const { return chash(); }

const std::any& ContextMenuEvent::data() const { return data_; }
void ContextMenuEvent::setData(std::any data) { data_ = std::move(data); }

KeyModifiers ContextMenuEvent::modifiers() const { return modifiers_; }
void ContextMenuEvent::setModifiers(KeyModifiers modifiers) { modifiers_ = modifiers; }

}  // namespace inviwo
