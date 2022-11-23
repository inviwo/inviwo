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

#include <sml/sml.hpp>

namespace sml = boost::sml;

namespace inviwo {

struct EventTransformerView {
    std::function<bool(const dvec3&)> isNdcInView;
    std::function<dvec3(const dvec3&)> globalNdcToLocalNdc;
    std::function<void(Event*)> propagateEvent;
    std::function<size2_t()> viewSize;
};

namespace sm {

struct Base {
    Base(MouseEvent* event) : event{event} {}
    MouseEvent* event;
};
struct Move : Base {
    using Base::Base;
};
struct Press : Base {
    using Base::Base;
};
struct Release : Base {
    using Base::Base;
};
struct DblClk : Base {
    using Base::Base;
};
struct Wheel : Base {
    using Base::Base;
};

// States
struct Idle {};
struct Pressing {};

struct EventTransformerSM {
    auto operator()() const noexcept {
        const auto idle = sml::state<Idle>;
        const auto pressing = sml::state<Pressing>;

        const auto move = sml::event<Move>;
        const auto press = sml::event<Press>;
        const auto release = sml::event<Release>;

        constexpr auto updateView = [](const auto& event,
                                       const std::vector<EventTransformerView>& views,
                                       std::optional<size_t>& activeView) -> void {
            auto it = std::ranges::find_if(
                views, [&](const auto& view) { return view.isNdcInView(event.event->ndc()); });
            if (it != views.end()) {
                activeView = static_cast<size_t>(std::distance(views.begin(), it));
            } else {
                activeView.reset();
            }
        };

        constexpr auto sendEvent = [](const auto& e, const std::vector<EventTransformerView>& views,
                                      std::optional<size_t>& activeView) -> void {
            if (activeView) {
                MouseEvent newEvent(*e.event);
                newEvent.setCanvasSize(views[*activeView].viewSize());
                auto newNdc = views[*activeView].globalNdcToLocalNdc(e.event->ndc());
                newEvent.setDepth(newNdc.z);
                newEvent.setPosNormalized(dvec2{(newNdc.x + 1.0) * 0.5, (newNdc.y + 1.0) * 0.5});
                views[*activeView].propagateEvent(&newEvent);
            }
        };

        return sml::make_transition_table(
            *idle + move / (static_cast<void>(updateView), sendEvent) = idle,
            idle + press / (static_cast<void>(updateView), sendEvent) = pressing,
            pressing + move / sendEvent = pressing,
            pressing + release / (static_cast<void>(updateView), sendEvent) = idle);
    }
};

}  // namespace sm

class IVW_CORE_API EventTransformer {
public:
    EventTransformer(const std::vector<EventTransformerView>& views)
        : views_{views}, sm{views_, activeView_} {}
    ~EventTransformer() = default;

    void propagateEvent(Event* event, Outport* source) {
        switch (event->hash()) {
            case MouseEvent::chash(): {
                auto me = static_cast<MouseEvent*>(event);
                propagateEvent(me, source);
                break;
            }
        }
    }

    void propagateEvent(MouseEvent* event, Outport* /*source*/) {
        switch (event->state()) {
            case MouseState::Press: {
                sm.process_event(sm::Press{event});
                break;
            }
            case MouseState::Move: {
                sm.process_event(sm::Move{event});
                break;
            }
            case MouseState::Release: {
                sm.process_event(sm::Release{event});
                break;
            }
            case MouseState::DoubleClick: {
                sm.process_event(sm::DblClk{event});
                break;
            }
        }
    }

    std::vector<EventTransformerView> views_;
    std::optional<size_t> activeView_;

    sml::sm<sm::EventTransformerSM> sm;
};

}  // namespace inviwo
