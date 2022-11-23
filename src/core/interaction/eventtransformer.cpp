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

#include <inviwo/core/interaction/eventtransformer.h>
#include <inviwo/core/util/logcentral.h>

#include <boost/sml.hpp>

namespace sml = boost::sml;

namespace inviwo {

namespace {

namespace mouse {

inline void transformAndSendEvent(
    MouseInteractionEvent& originalEvent, const EventTransformer::View& view,
    const std::function<void(Event&, const EventTransformer::View&)>& propagate) {

    auto newEvent = std::unique_ptr<MouseInteractionEvent>(originalEvent.clone());
    newEvent->setCanvasSize(view.size());
    newEvent->setNdc(view.globalNdcToLocalNdc(originalEvent.ndc()));

    newEvent->markAsVisited(originalEvent);

    propagate(*newEvent, view);

    originalEvent.markAsVisited(*newEvent);
    if (newEvent->hasBeenUsed()) originalEvent.markAsUsed();
}

struct Base {
    MouseInteractionEvent& event;
    std::optional<size_t> viewIdx;
    std::function<void(Event&, const EventTransformer::View&)> propagate;
};
struct Move : Base {};
struct Press : Base {};
struct Release : Base {};
struct DblClk : Base {};
struct Wheel : Base {};

// States
struct Idle {};
struct Pressing {};

struct SM {
    auto operator()() const noexcept {
        using namespace sml;

        static constexpr auto updateView = [](const auto& base,
                                              std::optional<size_t>& activeView) -> void {
            activeView = base.viewIdx;
        };
        static constexpr auto sendEvent = [](const auto& base,
                                             const std::vector<EventTransformer::View>& views,
                                             std::optional<size_t>& activeView) -> void {
            if (activeView && *activeView < views.size()) {
                transformAndSendEvent(base.event, views[*activeView], base.propagate);
            }
        };

        static constexpr auto sameView = [](const auto& base,
                                            std::optional<size_t>& activeView) -> bool {
            return base.viewIdx == activeView;
        };
        static constexpr auto diffView = [](const auto& base,
                                            std::optional<size_t>& activeView) -> bool {
            return base.viewIdx != activeView;
        };

        static constexpr auto zeroMB = [](const auto& base) -> bool {
            return base.event.buttonState().empty();
        };
        static constexpr auto hasMB = [](const auto& base) -> bool {
            return !base.event.buttonState().empty();
        };

        // clang-format off
        return sml::make_transition_table(
            *state<Idle>    + event<Move>                         / (updateView, sendEvent) = state<Idle>,
            state<Idle>     + event<Release>                      / (updateView, sendEvent) = state<Idle>,
            state<Idle>     + event<Press>                        / (updateView, sendEvent) = state<Pressing>,
            state<Idle>     + event<DblClk>                       / (updateView, sendEvent) = state<Idle>,
            state<Idle>     + event<Wheel>                        / (updateView, sendEvent) = state<Idle>,
            state<Pressing> + event<Move>                         / (sendEvent)             = state<Pressing>,
            state<Pressing> + event<Press>                        / (sendEvent)             = state<Pressing>,
            state<Pressing> + event<Release> [hasMB]              / (sendEvent)             = state<Pressing>,
            state<Pressing> + event<Release> [sameView && zeroMB] / (sendEvent)             = state<Idle>,
            state<Pressing> + event<Release> [diffView && zeroMB] / (sendEvent, updateView) = state<Idle>,
            state<Pressing> + event<Wheel>                        / (sendEvent)             = state<Pressing>
        );
        // clang-format on
    }
};

}  // namespace mouse

namespace gesture {

inline void transformAndSendEvent(
    GestureEvent& originalEvent, const EventTransformer::View& view,
    const std::function<void(Event&, const EventTransformer::View&)>& propagate) {

    GestureEvent newEvent{originalEvent};
    newEvent.setCanvasSize(view.size());
    newEvent.setNdc(view.globalNdcToLocalNdc(originalEvent.ndc()));

    newEvent.markAsVisited(originalEvent);

    propagate(newEvent, view);

    originalEvent.markAsVisited(newEvent);
    if (newEvent.hasBeenUsed()) originalEvent.markAsUsed();
}

struct Base {
    GestureEvent& event;
    std::optional<size_t> viewIdx;
    std::function<void(Event&, const EventTransformer::View&)> propagate;
};
struct NoGesture : Base {};
struct Started : Base {};
struct Updated : Base {};
struct Finished : Base {};
struct Canceled : Base {};

// States
struct Idle {};
struct Active {};

struct SM {
    auto operator()() const noexcept {
        using namespace sml;

        static constexpr auto updateView = [](const auto& base,
                                              std::optional<size_t>& activeView) -> void {
            activeView = base.viewIdx;
        };
        static constexpr auto sendEvent = [](const auto& base,
                                             const std::vector<EventTransformer::View>& views,
                                             std::optional<size_t>& activeView) -> void {
            if (activeView && *activeView < views.size()) {
                transformAndSendEvent(base.event, views[*activeView], base.propagate);
            }
        };

        static constexpr auto sameView = [](const auto& base,
                                            std::optional<size_t>& activeView) -> bool {
            return base.viewIdx == activeView;
        };
        static constexpr auto diffView = [](const auto& base,
                                            std::optional<size_t>& activeView) -> bool {
            return base.viewIdx != activeView;
        };

        static constexpr auto zeroMB = [](const auto& base) -> bool {
            return base.event.numFingers() == 0;
        };
        static constexpr auto hasMB = [](const auto& base) -> bool {
            return base.event.numFingers() > 0;
        };

        // clang-format off
        return sml::make_transition_table(
            *state<Idle>    + event<NoGesture>                   / (updateView, sendEvent) = state<Idle>,
            state<Idle>     + event<Started>                     / (updateView, sendEvent) = state<Active>,
            state<Idle>     + event<Updated>                     / (updateView, sendEvent) = state<Active>,
            state<Idle>     + event<Finished>                    / (updateView, sendEvent) = state<Idle>,
            state<Idle>     + event<Canceled>                    / (updateView, sendEvent) = state<Idle>,

            state<Active> + event<NoGesture>                     / (sendEvent, updateView) = state<Idle>,
            state<Active> + event<Started>                       / (sendEvent)             = state<Active>,
            state<Active> + event<Updated>                       / (sendEvent)             = state<Active>,
            state<Active> + event<Finished> [hasMB]              / (sendEvent)             = state<Active>,
            state<Active> + event<Finished> [sameView && zeroMB] / (sendEvent)             = state<Idle>,
            state<Active> + event<Finished> [diffView && zeroMB] / (sendEvent, updateView) = state<Idle>,
            state<Active> + event<Canceled>                      / (sendEvent)             = state<Idle>
        );
        // clang-format on
    }
};

}  // namespace gesture

namespace touch {

inline void transformAndSendEvent(
    TouchEvent& originalEvent, const EventTransformer::View& view,
    const std::function<void(Event&, const EventTransformer::View&)>& propagate) {

    TouchEvent newEvent{originalEvent};
    /*
    newEvent.setCanvasSize(view.size());
    newEvent.setNdc(view.globalNdcToLocalNdc(originalEvent.ndc()));

    newEvent.markAsVisited(originalEvent);

    propagate(newEvent, view);

    originalEvent.markAsVisited(newEvent);
    if (newEvent.hasBeenUsed()) originalEvent.markAsUsed();
    */
}

struct Base {
    TouchPoint& event;
    std::optional<size_t> viewIdx;
};
struct None : Base {};
struct Started : Base {};
struct Updated : Base {};
struct Stationary : Base {};
struct Finished : Base {};

// States
struct Idle {};
struct Active {};

struct SM {
    auto operator()() const noexcept {
        using namespace sml;

        static constexpr auto updateView = [](const auto& base,
                                              std::optional<size_t>& activeView) -> void {
            activeView = base.viewIdx;
        };
        static constexpr auto sendEvent = [](const auto& base,
                                             const std::vector<EventTransformer::View>& views,
                                             std::optional<size_t>& activeView) -> void {
            if (activeView && *activeView < views.size()) {
                transformAndSendEvent(base.event, views[*activeView], base.propagate);
            }
        };

        // clang-format off
        return sml::make_transition_table(
            *state<Idle>    + event<None>                        / updateView = state<Idle>,
            state<Idle>     + event<Started>                     / updateView = state<Active>,
            state<Idle>     + event<Updated>                     / updateView = state<Active>,
            state<Idle>     + event<Stationary>                  / updateView = state<Idle>,
            state<Idle>     + event<Finished>                    / updateView = state<Idle>,

            state<Active> + event<None>                          / updateView = state<Idle>,
            state<Active> + event<Started>                                    = state<Active>,
            state<Active> + event<Updated>                                    = state<Active>,
            state<Active> + event<Stationary>                                 = state<Active>,
            state<Active> + event<Finished>                                   = state<Idle>
        );
        // clang-format on
    }
};

}  // namespace touch

}  // namespace

struct EventTransformer::MouseSM {
    MouseSM(const std::vector<EventTransformer::View>& views)
        : activeView{}, sm{views, activeView} {}

    std::optional<size_t> activeView;
    sml::sm<mouse::SM> sm;
};

struct EventTransformer::GestureSM {
    GestureSM(const std::vector<EventTransformer::View>& views)
        : activeView{}, sm{views, activeView} {}

    std::optional<size_t> activeView;
    sml::sm<gesture::SM> sm;
};

struct EventTransformer::TouchSM {
    TouchSM(const std::vector<EventTransformer::View>& views)
        : activeView{}, sm{views, activeView} {}

    std::optional<size_t> activeView;
    sml::sm<gesture::SM> sm;
};

EventTransformer::EventTransformer(const std::vector<View>& someViews)
    : views{someViews}
    , mouse_{std::make_unique<MouseSM>(views)}
    , mousePicking_{std::make_unique<MouseSM>(views)}
    , gesture_{std::make_unique<GestureSM>(views)}
    , gesturePicking_{std::make_unique<GestureSM>(views)}
    , touch_{std::make_unique<TouchSM>(views)}
    , touchPicking_{std::make_unique<TouchSM>(views)} {}

EventTransformer::~EventTransformer() = default;

std::optional<size_t> EventTransformer::getViewIdx(const dvec3& globalNdc) const {
    const auto it = std::ranges::find_if(views, [&](const auto& view) {
        const auto pos = glm::dvec2{view.globalNdcToLocalNdc(globalNdc)};
        return glm::all(glm::greaterThanEqual(pos, glm::dvec2(-1.0))) &&
               glm::all(glm::lessThanEqual(pos, glm::dvec2(1.0)));
    });
    if (it != views.end()) {
        return static_cast<size_t>(std::distance(views.begin(), it));
    } else {
        return std::nullopt;
    }
}

std::optional<size_t> EventTransformer::getActiveMouseView() const { return mouse_->activeView; }
std::optional<size_t> EventTransformer::getActivePickingView() const {
    return mousePicking_->activeView;
}

bool EventTransformer::propagateEvent(Event* event, Outport* source) {
    bool propagated = false;
    propagateEvent(event, [source, &propagated](Event& e, const EventTransformer::View& v) {
        propagated = true;
        v.propagateEvent(&e, source);
    });
    return propagated;
}
void EventTransformer::propagateEvent(Event* event, const Propagator& propagate) {
    if (auto* me = event->getAs<MouseEvent>()) {
        propagateEvent(*me, *mouse_, propagate);
    } else if (auto* we = event->getAs<WheelEvent>()) {
        propagateEvent(*we, *mouse_, propagate);
    } else if (auto* ge = event->getAs<GestureEvent>()) {
        propagateEvent(*ge, *gesture_, propagate);
    } else if (auto* te = event->getAs<TouchEvent>()) {
        propagateEvent(*te, *touch_, propagate);
    } else if (auto* pe = event->getAs<PickingEvent>()) {
        propagateEvent(*pe, propagate);
    }
}

void EventTransformer::propagateEvent(MouseEvent& event, MouseSM& state,
                                      const Propagator& propagate) {
    switch (event.state()) {
        case MouseState::Press: {
            state.sm.process_event(mouse::Press{event, getViewIdx(event.ndc()), propagate});
            break;
        }
        case MouseState::Move: {
            state.sm.process_event(mouse::Move{event, getViewIdx(event.ndc()), propagate});
            break;
        }
        case MouseState::Release: {
            state.sm.process_event(mouse::Release{event, getViewIdx(event.ndc()), propagate});
            break;
        }
        case MouseState::DoubleClick: {
            state.sm.process_event(mouse::DblClk{event, getViewIdx(event.ndc()), propagate});
            break;
        }
    }
}

void EventTransformer::propagateEvent(WheelEvent& event, MouseSM& state,
                                      const Propagator& propagate) {
    state.sm.process_event(mouse::Wheel{event, getViewIdx(event.ndc()), propagate});
}

void EventTransformer::propagateEvent(GestureEvent& event, GestureSM& state,
                                      const Propagator& propagate) {
    switch (event.state()) {
        case GestureState::NoGesture: {
            state.sm.process_event(gesture::NoGesture{event, getViewIdx(event.ndc()), propagate});
            break;
        }
        case GestureState::Started: {
            state.sm.process_event(gesture::Started{event, getViewIdx(event.ndc()), propagate});
            break;
        }
        case GestureState::Updated: {
            state.sm.process_event(gesture::Updated{event, getViewIdx(event.ndc()), propagate});
            break;
        }
        case GestureState::Finished: {
            state.sm.process_event(gesture::Finished{event, getViewIdx(event.ndc()), propagate});
            break;
        }
        case GestureState::Canceled: {
            state.sm.process_event(gesture::Canceled{event, getViewIdx(event.ndc()), propagate});
            break;
        }
    }
}

void EventTransformer::propagateEvent(TouchEvent& event, TouchSM& state,
                                      const Propagator& propagate) {}

void EventTransformer::propagateEvent(PickingEvent& pickEvent, const Propagator& propagate) {

    const auto prop = [&](Event& newEvent, const EventTransformer::View& v) {
        const auto localPressNDC = v.globalNdcToLocalNdc(pickEvent.getPressedNDC());
        const auto localPreviousNDC = v.globalNdcToLocalNdc(pickEvent.getPreviousNDC());

        PickingEvent newPickEvent(
            pickEvent.getPickingAction(), static_cast<InteractionEvent*>(&newEvent),
            pickEvent.getState(), pickEvent.getPressState(), pickEvent.getPressItem(),
            pickEvent.getHoverState(), pickEvent.getPressItems(), pickEvent.getGlobalPickingId(),
            pickEvent.getCurrentGlobalPickingId(), pickEvent.getPressedGlobalPickingId(),
            pickEvent.getPreviousGlobalPickingId(), localPressNDC, localPreviousNDC);

        newPickEvent.markAsVisited(pickEvent);

        propagate(newPickEvent, v);

        pickEvent.markAsVisited(newPickEvent);
        if (newPickEvent.hasBeenUsed()) pickEvent.markAsUsed();
    };

    if (auto* me = pickEvent.getEvent()->getAs<MouseEvent>()) {
        propagateEvent(*me, *mousePicking_, prop);
    } else if (auto* we = pickEvent.getEvent()->getAs<WheelEvent>()) {
        propagateEvent(*we, *mousePicking_, prop);
    } else if (auto* ge = pickEvent.getEvent()->getAs<GestureEvent>()) {
        propagateEvent(*ge, *gesturePicking_, prop);
    }
}

}  // namespace inviwo
