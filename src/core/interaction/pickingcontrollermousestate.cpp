/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2019 Inviwo Foundation
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

#include <inviwo/core/interaction/pickingcontrollermousestate.h>

#include <inviwo/core/interaction/pickingmanager.h>
#include <inviwo/core/interaction/events/eventpropagator.h>
#include <inviwo/core/interaction/events/mouseevent.h>
#include <inviwo/core/interaction/events/wheelevent.h>
#include <inviwo/core/interaction/events/pickingevent.h>
#include <inviwo/core/interaction/pickingaction.h>
#include <inviwo/core/interaction/pickingcontroller.h>

#include <sml/sml.hpp>
namespace sml = boost::sml;

namespace inviwo {

namespace fsm {

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
    BaseEvent(const size_t globalId, MouseInteractionEvent* event, EventPropagator* propagator)
        : globalId{globalId}, event{event}, propagator{propagator} {}
    const size_t globalId;
    MouseInteractionEvent* event;
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
struct DblClk : BaseEvent {
    using BaseEvent::BaseEvent;
};
struct Wheel : BaseEvent {
    using BaseEvent::BaseEvent;
};

PickingPressItem mouseButtonToPressItem(MouseButton mb) {
    switch (mb) {
        case MouseButton::None:
            return PickingPressItem::None;
        case MouseButton::Left:
            return PickingPressItem::Primary;
        case MouseButton::Middle:
            return PickingPressItem::Tertiary;
        case MouseButton::Right:
            return PickingPressItem::Secondary;
        default:
            return PickingPressItem::None;
    }
}
PickingPressItems mouseButtonsToPressItems(MouseButtons mb) {
    PickingPressItems res = PickingPressItem::None;
    if (mb.count(MouseButton::Left)) res |= PickingPressItem::Primary;
    if (mb.count(MouseButton::Middle)) res |= PickingPressItem::Tertiary;
    if (mb.count(MouseButton::Right)) res |= PickingPressItem::Secondary;
    return res;
}

template <typename E>
auto send(PickingState state, PickingPressState pressState, PickingHoverState hoverState,
          bool updatePrev = true, bool markAsUsed = true) {
    return [state, pressState, hoverState, updatePrev, markAsUsed](
               const E& fsmEvent, FsmState& fsmState, PreviousFsmState& prev, PressFsmState& press,
               PickingManager* pickingManager) {
        auto res = pickingManager->getPickingActionFromIndex(fsmState.active_globalId);
        if (res.index == 0 || !res.action) return;

        auto me = fsmEvent.event;
        const PickingPressItem pressItem =
            (me->hash() == MouseEvent::chash()
                 ? mouseButtonToPressItem(static_cast<MouseEvent*>(me)->button())
                 : PickingPressItem::None);
        const PickingPressItems pressedState = mouseButtonsToPressItems(me->buttonState());

        PickingEvent pe(res.action, me, state, pressState, pressItem, hoverState, pressedState,
                        fsmState.active_globalId, fsmEvent.globalId, press.globalId, prev.globalId,
                        press.ndc, prev.ndc);
        fsmEvent.propagator->propagateEvent(&pe, nullptr);

        if (markAsUsed && pe.hasBeenUsed()) me->markAsUsed();
        if (updatePrev) {
            prev.globalId = fsmState.active_globalId;
            prev.ndc = me->ndc();
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
            pressState.ndc = e.event->ndc();
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
            return e.event->buttonState().empty();
        };

        // clang-format off
        return sml::make_transition_table(
           *idle + event<Move>  [!zeroId && zeroMB] / (uidm, send<Move>(S::Started, P::None, H::Enter)) = hasId,
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
            pressing + event<Press>                      / (send<Press>  (S::Updated,  P::Press,   H::None)),

            hasId + event<DblClk> [sameId] / (send<DblClk>(S::Updated, P::DoubleClick, H::None)),
            pressing + event<DblClk>       / (send<DblClk>(S::Updated, P::DoubleClick, H::None)),

            hasId + event<Wheel> [sameId] / (send<Wheel>(S::Updated, P::None, H::None)),
            pressing + event<Wheel>       / (send<Wheel>(S::Updated, P::None, H::None))
        );
        // clang-format on
    }
};

}  // namespace fsm

struct PickingControllerMouseStateSM {
    PickingControllerMouseStateSM(PickingManager* pickingManager)
        : pickingManager{pickingManager} {}
    PickingManager* pickingManager;
    fsm::FsmState state;
    fsm::PreviousFsmState previousState;
    fsm::PressFsmState pressState;
    sml::sm<fsm::Fsm> sm{state, previousState, pressState, pickingManager};
};

PickingControllerMouseState::PickingControllerMouseState(PickingManager* pickingManager)
    : msm{std::make_unique<PickingControllerMouseStateSM>(pickingManager)} {}

PickingControllerMouseState::~PickingControllerMouseState() = default;

void PickingControllerMouseState::propagateEvent(MouseEvent* e, EventPropagator* propagator,
                                                 size_t globalId) {
    switch (e->state()) {
        case MouseState::Press: {
            msm->sm.process_event(fsm::Press{globalId, e, propagator});
            break;
        }
        case MouseState::Move: {
            msm->sm.process_event(fsm::Move{globalId, e, propagator});
            break;
        }
        case MouseState::Release: {
            msm->sm.process_event(fsm::Release{globalId, e, propagator});
            break;
        }
        case MouseState::DoubleClick: {
            msm->sm.process_event(fsm::DblClk{globalId, e, propagator});
            break;
        }
    }
}

void PickingControllerMouseState::propagateEvent(WheelEvent* e, EventPropagator* propagator,
                                                 size_t globalId) {
    msm->sm.process_event(fsm::Wheel{globalId, e, propagator});
}

}  // namespace inviwo
