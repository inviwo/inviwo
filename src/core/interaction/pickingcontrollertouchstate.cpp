/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2021 Inviwo Foundation
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

#include <inviwo/core/interaction/pickingcontrollertouchstate.h>

#include <inviwo/core/interaction/pickingmanager.h>
#include <inviwo/core/interaction/events/eventpropagator.h>
#include <inviwo/core/interaction/events/touchevent.h>
#include <inviwo/core/interaction/events/pickingevent.h>
#include <inviwo/core/interaction/pickingaction.h>
#include <inviwo/core/interaction/pickingcontroller.h>

#include <sml/sml.hpp>
namespace sml = boost::sml;

namespace inviwo {

namespace fsmt {

// State
struct FsmState {
    size_t active_globalId = 0;
};

struct PreviousFsmState {
    size_t globalId = 0;
    dvec3 ndc{0};
};

struct PressFsmState {
    size_t globalId = 0;
    dvec3 ndc{0};
};

// States
struct Idle {};
struct HasId {};
struct Pressing {};

// events;
struct BaseEvent {
    BaseEvent(const size_t globalId, TouchEvent* event, EventPropagator* propagator)
        : globalId{globalId}, event{event}, propagator{propagator} {}
    const size_t globalId;
    TouchEvent* event;
    EventPropagator* propagator;
};
struct Move : BaseEvent {
    using BaseEvent::BaseEvent;
};
struct Press : BaseEvent {
    using BaseEvent::BaseEvent;
};
struct Release : BaseEvent {
    using BaseEvent::BaseEvent;
};

PickingPressState touchToPressState(PickingState ps) {
    switch (ps) {
        case PickingState::Started:
            return PickingPressState::Press;
        case PickingState::Updated:
            return PickingPressState::Move;
        case PickingState::Finished:
            return PickingPressState::Release;
        default:
            return PickingPressState::None;
    }
}
bool isPressed(const TouchPoint& point, const TouchDevice::DeviceType& deviceType) {
    if (deviceType == TouchDevice::DeviceType::TouchPad) {
        return point.state() == TouchState::Stationary;
    } else {
        // Treat touch point on TouchScreen as "button down"
        return true;
    }
}

PickingPressItem touchToPressItems(TouchEvent* e, const std::vector<TouchPoint>& points) {
    if (e->getDevice()->getType() == TouchDevice::DeviceType::TouchPad) {
        // Possible to press touchpads
        auto nPressed = std::accumulate(
            points.begin(), points.end(), 0u, [](size_t v, const auto& p) -> size_t {
                return v + (p.state() == TouchState::Stationary ? 1 : 0);
            });
        if (nPressed == 0) {
            return PickingPressItem::None;
        } else if (nPressed == 1) {
            return PickingPressItem::Primary;
        } else if (nPressed == 2) {
            return PickingPressItem::Secondary;
        } else {  // nPressed > 2
            return PickingPressItem::Tertiary;
        }
    } else {
        // Treat touch point on TouchScreen as "button down"
        return PickingPressItem::Primary;
    }
}

template <typename E>
auto send(PickingState state, PickingPressState pressState, PickingHoverState hoverState,
          bool updatePrev = true, bool markAsUsed = true) {
    return [state, pressState, hoverState, updatePrev, markAsUsed](
               const E& fsmEvent, FsmState& fsmState, PreviousFsmState& prev, PressFsmState& press,
               PickingManager* pickingManager) {
        auto res = pickingManager->getPickingActionFromIndex(fsmState.active_globalId);
        if (res.index == 0 || !res.action) return;

        auto te = fsmEvent.event;
        const PickingPressItem pressItem =
            (te->hash() == TouchEvent::chash() ? touchToPressItems(te, te->touchPoints())
                                               : PickingPressItem::None);
        const PickingPressItems pressedState = pressItem;

        PickingEvent pe(res.action, te, state, pressState, pressItem, hoverState, pressedState,
                        fsmState.active_globalId, fsmEvent.globalId, press.globalId, prev.globalId,
                        press.ndc, prev.ndc);
        fsmEvent.propagator->propagateEvent(&pe, nullptr);

        if (markAsUsed && pe.hasBeenUsed()) te->markAsUsed();
        if (updatePrev) {
            prev.globalId = fsmState.active_globalId;
            prev.ndc = te->centerNDC();
        }
    };
}

template <typename E>
auto uid() {
    return [](const E& e, FsmState& fsmstate) { fsmstate.active_globalId = e.globalId; };
}

struct Fsm {
    auto operator()() const noexcept {
        using namespace sml;
        const auto idle = sml::state<Idle>;
        const auto hasId = sml::state<HasId>;
        const auto pressing = sml::state<Pressing>;

        const auto uidm = uid<Move>();
        const auto uidp = uid<Press>();
        const auto uidr = uid<Release>();

        using S = PickingState;
        using P = PickingPressState;
        using H = PickingHoverState;

        const auto ups = [](const Press& e, PressFsmState& pressState) {
            pressState.globalId = e.globalId;
            for (const auto& p : e.event->touchPoints()) {
                if (isPressed(p, e.event->getDevice()->getType())) {
                    pressState.ndc = p.ndc();
                    break;
                }
            }
        };
        const auto rps = [](PressFsmState& pressState) {
            pressState.globalId = 0;
            pressState.ndc = dvec3{0};
        };

        // guards
        const auto sameId = [](const FsmState& state, const auto& e) -> bool {
            return e.globalId == state.active_globalId;
        };
        const auto diffId = [](const FsmState& state, const auto& e) -> bool {
            return e.globalId != state.active_globalId && e.globalId != 0;
        };
        const auto zeroId = [](const auto& e) -> bool { return e.globalId == 0; };
        const auto zeroMB = [](const FsmState&, const auto& e) -> bool {
            return touchToPressItems(e.event, e.event->touchPoints()) == PickingPressItem::None;
        };

        // clang-format off
        return sml::make_transition_table(
           *idle + event<Move>  [!zeroId && !zeroMB] / (uidm, send<Move>(S::Started, P::Press, H::Enter)) = hasId,
            idle + event<Move>  [!zeroId && zeroMB] / (uidm, send<Move>(S::Started, P::None, H::Enter)) = hasId,
            idle + event<Press> [!zeroId] / (uidp, send<Press>(S::Updated, P::Press, H::Enter)) = pressing,
            idle + event<Release> [!zeroId] / (uidr,  send<Release>(S::Started, P::None, H::Enter)) = hasId,
            idle + sml::on_entry<_> / rps,
             
            hasId + event<Move>  [sameId] / (send<Move>(S::Updated,  P::None, H::Move)),
            hasId + event<Move>  [zeroId] / (send<Move>(S::Finished, P::None, H::Exit), uidm) = idle,
            hasId + event<Move>  [diffId] / (send<Move>(S::Finished, P::None, H::Exit, false, false), uidm, send<Move>(S::Started, P::None, H::Enter)),
            hasId + event<Press> [sameId] / (ups, send<Press>(S::Updated, P::Press, H::None)) = pressing,
            hasId + event<Press> [diffId] / (send<Press>(S::Finished, P::None, H::Exit, false, false), ups, uidp, send<Press>(S::Updated, P::Press, H::Enter)) = pressing,
            hasId + sml::on_entry<_> / rps,

            pressing + event<Move>    [sameId]           / (send<Move>(S::Updated, P::Move, H::Move)),
            pressing + event<Move>    [diffId]           / (send<Move>(S::Updated, P::Move, H::Move)),
            pressing + event<Move>    [zeroId]           / (send<Move>(S::Updated, P::Move, H::Move)),

            pressing + event<Release> [sameId && zeroMB] / (send<Release>(S::Updated,  P::Release, H::None)) = hasId,
            pressing + event<Release> [zeroId && zeroMB] / (send<Release>(S::Finished, P::Release, H::Exit), uidr) = idle,
            pressing + event<Release> [diffId && zeroMB] / (send<Release>(S::Finished, P::Release, H::Exit, true, false), uidr, send<Release>(S::Started, P::None, H::Enter)) = hasId,
            pressing + event<Release> [!zeroMB]          / (send<Release>(S::Updated,  P::Release, H::None)),
            pressing + event<Press>                      / (send<Press>  (S::Updated,  P::Press,   H::None))

        );
        // clang-format on
    }
};

}  // namespace fsmt

struct PickingControllerTouchStateSM {
    PickingControllerTouchStateSM(PickingManager* pickingManager)
        : pickingManager{pickingManager} {}

    PickingManager* pickingManager;
    fsmt::FsmState state;
    fsmt::PreviousFsmState previousState;
    fsmt::PressFsmState pressState;
    sml::sm<fsmt::Fsm> sm{state, previousState, pressState, pickingManager};
};

PickingControllerTouchState::PickingControllerTouchState(PickingManager* pickingManager)
    : tsm{std::make_unique<PickingControllerTouchStateSM>(pickingManager)} {}

PickingControllerTouchState& PickingControllerTouchState::operator=(
    PickingControllerTouchState&& other) {
    tsm = std::move(other.tsm);
    return *this;
}

PickingControllerTouchState::~PickingControllerTouchState() = default;

void PickingControllerTouchState::propagateEvent(TouchEvent* e, EventPropagator* propagator,
                                                 size_t globalId) {
    const auto points = e->touchPoints();
    auto ps = TouchEvent::getPickingState(points);
    auto pressItems = fsmt::touchToPressItems(e, points);
    switch (ps) {
        case PickingState::None:
            break;
        case PickingState::Started:
        case PickingState::Updated:
            if (pressItems == PickingPressItem::None) {
                tsm->sm.process_event(fsmt::Move{globalId, e, propagator});
            } else {
                tsm->sm.process_event(fsmt::Press{globalId, e, propagator});
            }
            break;
        case PickingState::Finished:
            if (pressItems == PickingPressItem::None) {
                tsm->sm.process_event(fsmt::Move{globalId, e, propagator});
            } else {
                tsm->sm.process_event(fsmt::Release{globalId, e, propagator});
            }
            break;
    }
}

}  // namespace inviwo
