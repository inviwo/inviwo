/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2022 Inviwo Foundation
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

#include <inviwo/core/interaction/events/event.h>             // for Event
#include <inviwo/core/interaction/events/gestureevent.h>      // for GestureEvent
#include <inviwo/core/interaction/events/gesturestate.h>      // for GestureState, GestureState:...
#include <inviwo/core/interaction/events/interactionevent.h>  // for InteractionEvent
#include <inviwo/core/interaction/events/mousebuttons.h>      // for MouseButton, MouseButton::None
#include <inviwo/core/interaction/events/mouseevent.h>        // for MouseEvent
#include <inviwo/core/interaction/events/pickingevent.h>      // for PickingEvent
#include <inviwo/core/interaction/events/touchevent.h>        // for TouchPoint, TouchEvent
#include <inviwo/core/interaction/events/touchstate.h>        // for TouchState, TouchState::Fin...
#include <inviwo/core/interaction/events/wheelevent.h>        // for WheelEvent

#include <inviwo/core/ports/inport.h>
#include <inviwo/core/ports/outport.h>

#include <inviwo/core/util/glmfmt.h>

#include <functional>  // for function
#include <algorithm>
#include <ranges>
#include <optional>

#include <fmt/format.h>

namespace inviwo {

class IVW_CORE_API EventTransformer {
public:
    struct MouseSM;
    struct GestureSM;
    struct TouchSM;
    struct View {
        std::function<dvec3(const dvec3&)> globalNdcToLocalNdc;
        std::function<void(Event*, Outport*)> propagateEvent;
        std::function<size2_t()> size;
    };

    explicit EventTransformer(const std::vector<View>& views = {});
    ~EventTransformer();

    bool propagateEvent(Event* event, Outport* source);

    std::optional<size_t> getViewIdx(const dvec3& globalNdc) const;

    std::vector<View> views;

    std::optional<size_t> getActiveMouseView() const;
    std::optional<size_t> getActivePickingView() const;

private:
    using Propagator = std::function<void(Event&, const EventTransformer::View&)>;

    void propagateEvent(Event* event, const Propagator& propagate);
    void propagateEvent(PickingEvent& event, const Propagator& propagate);

    void propagateEvent(MouseEvent& event, MouseSM& state, const Propagator& propagate);
    void propagateEvent(WheelEvent& event, MouseSM& state, const Propagator& propagate);
    void propagateEvent(GestureEvent& event, GestureSM& state, const Propagator& propagate);
    void propagateEvent(TouchEvent& event, TouchSM& state, const Propagator& propagate);

    std::unique_ptr<MouseSM> mouse_;
    std::unique_ptr<MouseSM> mousePicking_;

    std::unique_ptr<GestureSM> gesture_;
    std::unique_ptr<GestureSM> gesturePicking_;

    std::unique_ptr<TouchSM> touch_;
    std::unique_ptr<TouchSM> touchPicking_;
};

}  // namespace inviwo
