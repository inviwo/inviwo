/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019 Inviwo Foundation
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

#include <modules/basegl/baseglmoduledefine.h>
#include <inviwo/core/common/inviwo.h>

#include <modules/basegl/viewmanager.h>
#include <inviwo/core/interaction/events/mouseevent.h>

#include <sml/sml.hpp>
namespace sml = boost::sml;

namespace inviwo {

namespace vmsm {

using View = typename ViewManager::View;
using ViewId = typename ViewManager::ViewId;
using Propagator = typename ViewManager::Propagator;

struct State {
    ViewId view = ViewManager::noView;
};

// States
struct Idle {};
struct Pressing {};

// events;
struct BaseEvent {
    BaseEvent(ViewId view, MouseEvent* event, const Propagator& propagator)
        : view{view}, me{event}, propagator{propagator} {}
    const ViewId view;
    MouseEvent* me;
    const Propagator& propagator;
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
struct Enter : BaseEvent {
    using BaseEvent::BaseEvent;
};
struct Leave : BaseEvent {
    using BaseEvent::BaseEvent;
};

IVW_MODULE_BASEGL_API void scaleEventToView(MouseEvent& me, const View& view);

// Actions
template <typename E>
auto ffw() {
    return [](const E& e, State& s, const ViewManager& viewManager) {
        if (s.view >= viewManager.size()) return;
        MouseEvent newEvent(*e.me);
        scaleEventToView(newEvent, viewManager[s.view]);

        e.propagator(&newEvent, s.view);
        if (newEvent.hasBeenUsed()) e.me->markAsUsed();
        e.me->markAsVisited(newEvent);
    };
}
template <typename E>
auto leave() {
    return [](const E& e, State& s, const ViewManager& viewManager) {
        if (s.view >= viewManager.size()) return;
        MouseEvent me(MouseButton::None, MouseState::Leave, MouseButton::None, KeyModifier::None,
                      dvec2{-1.0}, uvec2{viewManager[s.view].size}, 1.0);
        e.propagator(&me, s.view);
    };
}
template <typename E>
auto enter() {
    return [](const E& e, State& s, const ViewManager& viewManager) {
        if (s.view >= viewManager.size()) return;
        MouseEvent newEvent(*e.me);
        scaleEventToView(newEvent, viewManager[s.view]);
        newEvent.setState(MouseState::Enter);
        e.propagator(&newEvent, s.view);
    };
}

template <typename E>
auto id() {
    return [](const E& e, State& s) { s.view = e.view; };
}

struct Fsm {
    auto operator()() const noexcept {
        // states
        const auto idle = sml::state<Idle>;
        const auto pressing = sml::state<Pressing>;

        const auto log = [](const State& state, const auto& e) -> void {
            LogInfoCustom("FSM", "E: " << e.view << " S: " << state.view);
        };

        // guards
        const auto sameId = [](const State& state, const auto& e) -> bool {
            return e.view == state.view;
        };
        const auto zeroMB = [](const auto& e) -> bool { return e.me->buttonState().empty(); };

        using namespace sml;
        // clang-format off
        return sml::make_transition_table(
            *idle + event<Enter> / (id<Enter>(), ffw<Enter>()) = idle,
             idle + event<Leave> / (id<Leave>(), ffw<Leave>()) = idle,

             idle + event<Move> [sameId] / (ffw<Move>()) = idle,
             idle + event<Move> [!sameId] / 
                (leave<Move>(), id<Move>(), enter<Move>(), ffw<Move>()) = idle,
             idle + event<Press> / (id<Press>(), ffw<Press>()) = pressing,
             idle + event<DblClk> / (id<DblClk>(), ffw<DblClk>()) = idle, 

             pressing + event<Move> / ffw<Move>() = pressing,
             pressing + event<Press> / ffw<Press>() = pressing,
             pressing + event<Release> [!zeroMB] / ffw<Release>() = pressing,
             pressing + event<Release> [zeroMB]  / 
                (ffw<Release>(), leave<Release>(), id<Release>(), enter<Release>()) = idle
        );
        // clang-format on
    }
};

}  // namespace vmsm

class IVW_MODULE_BASEGL_API ViewManagerState {
public:
    using ViewId = typename ViewManager::ViewId;
    using Propagator = typename ViewManager::Propagator;

    ViewManagerState(const ViewManager& viewManager) : state{}, sm{state, viewManager} {}
    ViewManagerState(const ViewManagerState&) = delete;
    ViewManagerState& operator=(const ViewManagerState&) = delete;

    vmsm::State state;
    sml::sm<vmsm::Fsm> sm;

    bool propagateEvent(MouseEvent* e, ViewId view, const Propagator& propagator) {

        switch (e->state()) {
            case MouseState::Press: {
                sm.process_event(vmsm::Press{view, e, propagator});
                break;
            }
            case MouseState::Move: {
                sm.process_event(vmsm::Move{view, e, propagator});
                break;
            }
            case MouseState::Release: {
                sm.process_event(vmsm::Release{view, e, propagator});
                break;
            }
            case MouseState::DoubleClick: {
                sm.process_event(vmsm::DblClk{view, e, propagator});
                break;
            }
            case MouseState::Enter: {
                sm.process_event(vmsm::Enter{view, e, propagator});
                break;
            }
            case MouseState::Leave: {
                sm.process_event(vmsm::Leave{ViewManager::noView, e, propagator});
                break;
            }
        }
        return true;
    }
};

}  // namespace inviwo
