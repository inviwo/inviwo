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
    size_t active_globalId = PickingManager::VoidId;
};

struct PreviousFsmState {
    size_t globalId = PickingManager::VoidId;
    dvec3 ndc{0};
};

struct PressFsmState {
    size_t globalId = PickingManager::VoidId;
    dvec3 ndc{0};
    PickingPressItems pressedState = PickingPressItem::None;  ///< Items initiated the press
};

// States
struct Idle {};
struct HasId {};
struct Pressing {};
struct PressingOutside {};

// events;
struct BaseEvent {
    BaseEvent(const size_t globalId, TouchEvent* event, EventPropagator* propagator)
        : globalId{globalId}, event{event}, propagator{propagator} {}
    const size_t globalId;
    TouchEvent* event;
    EventPropagator* propagator;
};
struct Started : BaseEvent {
    using BaseEvent::BaseEvent;
};
struct Updated : BaseEvent {
    using BaseEvent::BaseEvent;
};
struct Finished : BaseEvent {
    using BaseEvent::BaseEvent;
};

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
        if (res.index == PickingManager::VoidId || !res.action) return;

        auto te = fsmEvent.event;
        PickingPressItem pressItem = PickingPressItem::None;
        PickingPressItems pressedState = press.pressedState;
        if (pressState == PickingPressState::Press) {
            pressItem = touchToPressItems(te, te->touchPoints());
        } else if (pressState == PickingPressState::Release) {
            pressItem = touchToPressItems(te, te->touchPoints());
            // Remove current press state
            // (erasing a non-existing bit will flip the bit...)
            if (pressedState & pressItem) {
                pressedState.erase(pressItem);
            }

            if (pressedState & PickingPressItem::Primary) {
                pressItem = PickingPressItem::Primary;
            } else if (pressedState & PickingPressItem::Secondary) {
                pressItem = PickingPressItem::Secondary;
            } else if (pressedState & PickingPressItem::Tertiary) {
                pressItem = PickingPressItem::Tertiary;
            } else {
                // Do nothing:
                // For touch screen pressItem will always be PickingPressItem::Primary, which has
                // been erased above
            }

            if (pressedState & pressItem) {
                // Remove the item causing the press
                pressedState.erase(pressItem);
            }
        }

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
        const auto pressingOutside = sml::state<PressingOutside>;

        const auto uids = uid<Started>();
        const auto uidu = uid<Updated>();
        const auto uidf = uid<Finished>();

        using S = PickingState;
        using P = PickingPressState;
        using H = PickingHoverState;

        const auto ups = [](const auto& e, PressFsmState& pressState) {
            // Update pressed state
            pressState.globalId = e.globalId;
            for (const auto& p : e.event->touchPoints()) {
                if (isPressed(p, e.event->getDevice()->getType())) {
                    pressState.ndc = p.ndc();
                    break;
                }
            }
            pressState.pressedState = touchToPressItems(e.event, e.event->touchPoints());
        };
        const auto updatePressedOutside = [](const auto& e, PressFsmState& pressState) {
            // update pressed outside
            pressState.pressedState = touchToPressItems(e.event, e.event->touchPoints());
        };
        const auto rps = [](PressFsmState& pressState) {
            // Reset press state
            pressState.globalId = 0;
            pressState.ndc = dvec3{0};
            pressState.pressedState = PickingPressItem::None;
        };

        // guards
        const auto sameId = [](const FsmState& state, const auto& e) -> bool {
            return e.globalId == state.active_globalId && e.globalId != PickingManager::VoidId;
        };
        const auto diffId = [](const FsmState& state, const auto& e) -> bool {
            return e.globalId != state.active_globalId && e.globalId != PickingManager::VoidId;
        };
        const auto zeroId = [](const auto& e) -> bool { return e.globalId == 0; };
        const auto zeroMB = [](const FsmState&, const auto& e) -> bool {
            return touchToPressItems(e.event, e.event->touchPoints()) == PickingPressItem::None;
        };

        // Keep track of states related to entering/moving/exiting picking objects
        // idle: Not over any picking object
        // hasId: Over a picking object
        // pressing: Any touch point is pressed (TODO: separate different types of pressitems)

        // Possible TODO: Detect double click

        // clang-format off
        return sml::make_transition_table(
           *idle + event<Started>  [!zeroId && zeroMB]  / (uids, send<Started>(S::Started, P::None, H::Enter)) = hasId,
            idle + event<Started>  [!zeroId && !zeroMB] / (uids, ups, send<Started>(S::Started, P::Press, H::Enter)) = pressing,
            idle + event<Started>  [zeroId && !zeroMB]  / (updatePressedOutside) = pressingOutside,
            idle + event<Updated>  [!zeroId && zeroMB]  / (uidu, send<Updated>(S::Started, P::None, H::Enter)) = hasId,
            idle + event<Updated>  [zeroId && !zeroMB]  / (updatePressedOutside) = pressingOutside,
            idle + event<Finished> = idle,
            idle + sml::on_entry<_> / rps,

            hasId + event<Updated> [sameId && zeroMB] / (send<Updated>(S::Updated, P::None, H::Move)),
            hasId + event<Updated> [zeroId && zeroMB] / (send<Updated>(S::Finished, P::None, H::Exit), uidu) = idle,
            hasId + event<Updated> [diffId && zeroMB] / (send<Updated>(S::Finished, P::None, H::Exit, false, false), uidu, send<Updated>(S::Started, P::None, H::Enter)),
            hasId + event<Updated>  [sameId && !zeroMB] / (ups, send<Updated>(S::Updated, P::Press, H::None)) = pressing,
            hasId + event<Updated>  [diffId && !zeroMB] / (send<Updated>(S::Finished, P::None, H::Exit, false, false), ups, uidu, send<Updated>(S::Updated, P::Press, H::Enter)) = pressing,
            hasId + event<Finished> / (send<Finished>(S::Finished, P::None, H::Exit)) = idle,
            hasId + sml::on_entry<_> / rps,

            pressing + event<Updated> [sameId && zeroMB] / (send<Updated>(S::Updated, P::Release, H::None)) = hasId,
            pressing + event<Updated> [zeroId && zeroMB] / (send<Updated>(S::Finished, P::Release, H::Exit), uidu) = idle,
            pressing + event<Updated> [diffId && zeroMB] / (send<Updated>(S::Finished, P::Release, H::Exit, true, false), uidu, send<Updated>(S::Started, P::None, H::Enter)) = hasId,
            pressing + event<Updated> [sameId && !zeroMB] / (send<Updated>(S::Updated, P::Move, H::Move)),
            pressing + event<Updated> [zeroId && !zeroMB] / (send<Updated>(S::Updated, P::Move, H::Move)),
            pressing + event<Updated>  [diffId && !zeroMB] / (send<Updated>(S::Updated, P::Move, H::Move)),
            pressing + event<Finished> / (send<Finished>(S::Finished, P::Release, H::Exit)) = idle,
            
            // Store the press state (Mouse events keep this state internally, but touch do not)
            pressingOutside + event<Updated> [!zeroId && zeroMB] / (uidu, send<Updated>(S::Started, P::None, H::Enter)) = hasId,
            pressingOutside + event<Updated> [zeroMB] = idle,
            pressingOutside + event<Finished> = idle
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

PickingControllerTouchState::PickingControllerTouchState(PickingControllerTouchState&& other)
    : tsm{std::move(other.tsm)} {}

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
    switch (ps) {
        case PickingState::None:
            break;
        case PickingState::Started:
            tsm->sm.process_event(fsmt::Started{globalId, e, propagator});
            break;
        case PickingState::Updated:
            tsm->sm.process_event(fsmt::Updated{globalId, e, propagator});
            break;
        case PickingState::Finished:
            tsm->sm.process_event(fsmt::Finished{globalId, e, propagator});
            break;
    }
}

}  // namespace inviwo
