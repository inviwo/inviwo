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

#include <inviwo/core/util/stdextensions.h>
#include <inviwo/core/util/zip.h>

#include <inviwo/core/interaction/pickingmanager.h>
#include <inviwo/core/interaction/pickingmapper.h>
#include <inviwo/core/interaction/pickingaction.h>
#include <inviwo/core/interaction/pickingstate.h>
#include <inviwo/core/interaction/events/event.h>
#include <inviwo/core/interaction/events/pickingevent.h>
#include <inviwo/core/interaction/events/mouseevent.h>
#include <inviwo/core/interaction/events/eventpropagator.h>
#include <inviwo/core/interaction/pickingcontrollermousestate.h>

#include <sml/sml.hpp>
namespace sml = boost::sml;

#include <warn/push>
#include <warn/ignore/all>
#include <gtest/gtest.h>
#include <warn/pop>

namespace inviwo {

namespace test {

// States
struct none {};
struct started {};
struct updated {};
struct finished {};

struct idle {};
struct active {};

// Events
struct press {};
struct release {};
struct enterId {};
struct exitId {};
struct move {
    int dir = 0;
};

/* // for debugging
auto enter = [](std::string state) {
    return [state]() { std::cout << "enter: " << state << std::endl; };
};
auto exit = [](std::string state) {
    return [state]() { std::cout << "exit:  " << state << std::endl; };
};
*/

auto moveGuardUp = [](move e) -> bool { return e.dir >= 0; };
auto moveGuardDown = [](move e) -> bool { return e.dir < 0; };

struct fsmState {
    int count = 0;
};

auto moveAction = [](move e, fsmState& s) {
    s.count += e.dir;
    // std::cout << "Move   " << e.dir << std::endl;
};

// clang-format off
struct TestSM {
    auto operator()() const noexcept {
        using namespace sml;
        return sml::make_transition_table(
           *state<none> + event<enterId> = state<started>,
            state<started> + event<move> / moveAction = state<updated>,
            state<updated> + event<move>[moveGuardUp] / moveAction = state<updated>,
            state<updated> + event<move>[moveGuardDown] / moveAction = state<updated>,
            state<updated> + event<exitId> = state<finished>,
            state<finished> = state<none>,

           *state<idle> + event<move>  = state<active>,
            state<active> + event<exitId> = state<idle>

            // debug 
            /*
            state<none> + sml::on_entry<_> / enter("none"),
            state<started> + sml::on_entry<_> / enter("started"),
            state<updated> + sml::on_entry<_> / enter("updated"),
            state<finished> + sml::on_entry<_> / enter("finished"),

            state<idle> + sml::on_entry<_> / enter("idle"),
            state<active> + sml::on_entry<_> / enter("active"),

            state<none> + sml::on_exit<_> / exit("none"),
            state<started> + sml::on_exit<_> / exit("started"),
            state<updated> + sml::on_exit<_> / exit("updated"),
            state<finished> + sml::on_exit<_> / exit("finished"),

            state<idle> + sml::on_exit<_> / exit("idle"),
            state<active> + sml::on_exit<_> / exit("active")
            */
        );
    }
};
// clang-format on

}  // namespace test

TEST(PickingControllerTest, TestSystem) {
    using namespace test;
    fsmState s;

    sml::sm<TestSM> sm{s};

    ASSERT_TRUE(sm.is(sml::state<none>));

    sm.process_event(enterId{});

    ASSERT_TRUE(sm.is(sml::state<started>));

    sm.process_event(move{5});

    ASSERT_TRUE(sm.is(sml::state<updated>));

    sm.process_event(move{5});

    ASSERT_TRUE(sm.is(sml::state<updated>));

    sm.process_event(move{-1});

    ASSERT_TRUE(sm.is(sml::state<updated>));

    sm.process_event(exitId{});

    ASSERT_TRUE(sm.is(sml::state<none>));
}

struct TestPropagator : EventPropagator {
    virtual void propagateEvent(Event* event, Outport*) { events.emplace_back(event->clone()); };
    std::vector<std::unique_ptr<Event>> events;
};

/*  Global picking indices
 *  9 | 0 0 0 0 0 0 0 0 0 0
 *  8 | 0 0 0 0 0 0 0 0 0 0
 *  7 | 0 0 0 0 5 5 5 5 0 0
 *  6 | 0 0 0 0 5 5 5 5 0 0
 *  5 | 0 0 2 2 2 2 5 5 0 0
 *  4 | 0 0 2 2 2 2 5 5 0 0
 *  3 | 0 0 2 2 2 2 0 0 0 0
 *  2 | 0 0 2 2 2 2 0 0 0 0
 *  1 | 0 0 0 0 0 0 0 0 0 0
 *  0 | 0 0 0 0 0 0 0 0 0 0
 *  -----------------------
 *    | 0 1 2 3 4 5 6 7 8 9
 *
 * Two PickAction
 *  1: pos 1 size 2;
 *  2: pos 3 size 3;
 *
 * Global index 2 will map to Pick Action 1, id 1;
 * Global index 5 will map to Pick Action 2, id 2;
 */

constexpr auto makeCanvas() {
    // clang-format off
    std::array<std::array<size_t, 10>, 10> canvas =
    {{{0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
      {0, 0, 0, 0, 5, 5, 5, 5, 0, 0},
      {0, 0, 0, 0, 5, 5, 5, 5, 0, 0},
      {0, 0, 2, 2, 2, 2, 5, 5, 0, 0},
      {0, 0, 2, 2, 2, 2, 5, 5, 0, 0},
      {0, 0, 2, 2, 2, 2, 0, 0, 0, 0},
      {0, 0, 2, 2, 2, 2, 0, 0, 0, 0},
      {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}};
    // clang-format on
    return canvas;
}

MouseEvent mouseEvent(MouseButton mb, MouseState ms, MouseButtons mbs, uvec2 pos, uvec2 cdim) {
    return MouseEvent{mb, ms, mbs, KeyModifiers(flags::empty), dvec2{pos} / dvec2{cdim}, cdim, 0.5};
}

void testPickingEvent(Event* event, PickingState state, PickingHoverState hoverState,
                      PickingPressState pressState, PickingPressItem pressItem,
                      PickingPressItems pressedState, size_t pickedGlobalId, size_t currentGlobalId,
                      size_t previousGlobalId, size_t pressedGlobalId, size_t pickedLocalId,
                      size_t currentLocalId, size_t previousLocalId, size_t pressedLocalId,
                      dvec3 currentNDC, dvec3 previousNDC, dvec3 pressedNDC) {
    ASSERT_TRUE(event);
    auto pe = event->getAs<PickingEvent>();
    ASSERT_TRUE(pe);
    EXPECT_EQ(pe->getState(), state);
    EXPECT_EQ(pe->getPressState(), pressState);
    EXPECT_EQ(pe->getPressItem(), pressItem);
    EXPECT_EQ(pe->getHoverState(), hoverState);
    EXPECT_EQ(pe->getPressItems(), pressedState);

    EXPECT_EQ(pe->getGlobalPickingId(), pickedGlobalId);
    EXPECT_EQ(pe->getCurrentGlobalPickingId(), currentGlobalId);
    EXPECT_EQ(pe->getPressedGlobalPickingId(), pressedGlobalId);
    EXPECT_EQ(pe->getPreviousGlobalPickingId(), previousGlobalId);

    EXPECT_EQ(pe->getPickedId(), pickedLocalId);
    if (pe->getCurrentLocalPickingId().first) {
        EXPECT_EQ(pe->getCurrentLocalPickingId().second, currentLocalId);
    }
    if (pe->getPressedLocalPickingId().first) {
        EXPECT_EQ(pe->getPressedLocalPickingId().second, pressedLocalId);
    }
    if (pe->getPreviousLocalPickingId().first) {
        EXPECT_EQ(pe->getPreviousLocalPickingId().second, previousLocalId);
    }

    EXPECT_EQ(pe->getNDC(), currentNDC);
    EXPECT_EQ(pe->getPreviousNDC(), previousNDC);
    EXPECT_EQ(pe->getPressedNDC(), pressedNDC);

    EXPECT_TRUE(pe->getPickingAction());
}

using B = MouseButton;
using Bs = MouseButtons;
using S = MouseState;

using PS = PickingState;
using PPS = PickingPressState;
using PPI = PickingPressItem;
using PPIs = PickingPressItems;
using PHS = PickingHoverState;

const auto canvas = makeCanvas();
const uvec2 dim{canvas.size(), canvas.size()};
const auto ndc = [](uvec2 pos) { return dvec3{2.0 * dvec2{pos} / dvec2{10.0} - dvec2{1.0}, 0.5}; };

TEST(PickingControllerTest, IdleMove) {
    PickingManager pm;
    pm.registerPickingAction(nullptr, [](PickingEvent*) {}, 2);
    pm.registerPickingAction(nullptr, [](PickingEvent*) {}, 3);
    PickingControllerMouseState ms(&pm);
    {
        uvec2 pos{1, 2};  // id = 0
        auto event = mouseEvent(B::None, S::Move, Bs(flags::empty), pos, dim);
        TestPropagator tp;
        ms.propagateEvent(&event, &tp, canvas[dim.y - pos.y][pos.x]);
        ASSERT_EQ(tp.events.size(), 0);
    }
    {
        SCOPED_TRACE("Move 1,1 -> 1,2");
        uvec2 pos{1, 2};  // id = 0
        auto event = mouseEvent(B::None, S::Move, Bs(flags::empty), pos, dim);
        TestPropagator tp;
        ms.propagateEvent(&event, &tp, canvas[dim.y - pos.y][pos.x]);
        ASSERT_EQ(tp.events.size(), 0);
    }
}

TEST(PickingControllerTest, MoveMouseAround) {
    PickingManager pm;
    pm.registerPickingAction(nullptr, [](PickingEvent*) {}, 2);
    pm.registerPickingAction(nullptr, [](PickingEvent*) {}, 3);
    PickingControllerMouseState ms(&pm);
    {
        uvec2 pos{1, 1};  // id = 0
        auto event = mouseEvent(B::None, S::Move, Bs(flags::empty), pos, dim);

        TestPropagator tp;
        ms.propagateEvent(&event, &tp, canvas[dim.y - pos.y][pos.x]);
        ASSERT_EQ(tp.events.size(), 0);
    }
    {
        SCOPED_TRACE("Move 1,1 -> 3,3");
        // uvec2 pre{1, 1};  // id = 0
        uvec2 pos{3, 3};  // id = 1
        auto event = mouseEvent(B::None, S::Move, Bs(flags::empty), pos, dim);
        TestPropagator tp;
        ms.propagateEvent(&event, &tp, canvas[dim.y - pos.y][pos.x]);
        ASSERT_EQ(tp.events.size(), 1);
        testPickingEvent(tp.events[0].get(), PS::Started, PHS::Enter, PPS::None, PPI::None,
                         PPI::None, 2, 2, 0, 0, 1, 1, 0, 0, ndc(pos), dvec3{0.0}, dvec3{0.0});
    }
    {
        SCOPED_TRACE("Move 3,3 -> 3,4");
        uvec2 pre{3, 3};  // id = 1
        uvec2 pos{3, 4};  // id = 1
        auto event = mouseEvent(B::None, S::Move, Bs(flags::empty), pos, dim);
        TestPropagator tp;
        ms.propagateEvent(&event, &tp, canvas[dim.y - pos.y][pos.x]);
        ASSERT_EQ(tp.events.size(), 1);
        testPickingEvent(tp.events[0].get(), PS::Updated, PHS::Move, PPS::None, PPI::None,
                         PPI::None, 2, 2, 2, 0, 1, 1, 1, 0, ndc(pos), ndc(pre), dvec3{0.0});
    }
    {
        SCOPED_TRACE("Move 3,4 -> 1,4");
        uvec2 pre{3, 4};  // id = 1
        uvec2 pos{1, 4};  // id = 0
        auto event = mouseEvent(B::None, S::Move, Bs(flags::empty), pos, dim);

        TestPropagator tp;
        ms.propagateEvent(&event, &tp, canvas[dim.y - pos.y][pos.x]);
        ASSERT_EQ(tp.events.size(), 1);
        testPickingEvent(tp.events[0].get(), PS::Finished, PHS::Exit, PPS::None, PPI::None,
                         PPI::None, 2, 0, 2, 0, 1, 0, 1, 0, ndc(pos), ndc(pre), dvec3{0.0});
    }
    {
        SCOPED_TRACE("Move 1,4 -> 3,3 again");
        uvec2 pre{1, 4};  // id = 0
        uvec2 pos{3, 3};  // id = 1
        auto event = mouseEvent(B::None, S::Move, Bs(flags::empty), pos, dim);
        TestPropagator tp;
        ms.propagateEvent(&event, &tp, canvas[dim.y - pos.y][pos.x]);
        ASSERT_EQ(tp.events.size(), 1);
        testPickingEvent(tp.events[0].get(), PS::Started, PHS::Enter, PPS::None, PPI::None,
                         PPI::None, 2, 2, 2, 0, 1, 1, 1, 0, ndc(pos), ndc(pre), dvec3{0.0});
    }
    {
        SCOPED_TRACE("Move 3,3 -> 6,6");
        uvec2 pre{3, 3};  // id = 1
        uvec2 pos{6, 6};  // id = 2
        auto event = mouseEvent(B::None, S::Move, Bs(flags::empty), pos, dim);
        TestPropagator tp;
        ms.propagateEvent(&event, &tp, canvas[dim.y - pos.y][pos.x]);
        ASSERT_EQ(tp.events.size(), 2);
        {
            SCOPED_TRACE("Exit event");
            testPickingEvent(tp.events[0].get(), PS::Finished, PHS::Exit, PPS::None, PPI::None,
                             PPI::None, 2, 5, 2, 0, 1, 2, 1, 0, ndc(pos), ndc(pre), dvec3{0.0});
        }
        {
            SCOPED_TRACE("Enter event");
            testPickingEvent(tp.events[1].get(), PS::Started, PHS::Enter, PPS::None, PPI::None,
                             PPI::None, 5, 5, 2, 0, 2, 2, 1, 0, ndc(pos), ndc(pre), dvec3{0.0});
        }
    }
}

void doMousePress(PickingControllerMouseState& ms) {
    {
        uvec2 pos{1, 1};  // id = 0
        auto event = mouseEvent(B::None, S::Move, Bs(flags::empty), pos, dim);

        TestPropagator tp;
        ms.propagateEvent(&event, &tp, canvas[dim.y - pos.y][pos.x]);
        ASSERT_EQ(tp.events.size(), 0);
    }
    {  // Move
        // uvec2 pre{1, 1};  // id = 0
        uvec2 pos{3, 3};  // id = 1
        auto event = mouseEvent(B::None, S::Move, Bs(flags::empty), pos, dim);
        TestPropagator tp;
        ms.propagateEvent(&event, &tp, canvas[dim.y - pos.y][pos.x]);
        ASSERT_EQ(tp.events.size(), 1);
        SCOPED_TRACE("Enter 3,3");
        testPickingEvent(tp.events[0].get(), PS::Started, PHS::Enter, PPS::None, PPI::None,
                         PPI::None, 2, 2, 0, 0, 1, 1, 0, 0, ndc(pos), dvec3{0.0}, dvec3{0.0});
    }
    {                     // Press
        uvec2 pre{3, 3};  // id = 1
        uvec2 pos{3, 3};  // id = 1
        auto event = mouseEvent(B::Left, S::Press, Bs{B::Left}, pos, dim);
        TestPropagator tp;
        ms.propagateEvent(&event, &tp, canvas[dim.y - pos.y][pos.x]);
        ASSERT_EQ(tp.events.size(), 1);
        SCOPED_TRACE("Press 3,3");
        testPickingEvent(tp.events[0].get(), PS::Updated, PHS::None, PPS::Press, PPI::Primary,
                         PPIs{PPI::Primary}, 2, 2, 2, 2, 1, 1, 1, 1, ndc(pos), ndc(pre), ndc(pos));
    }
}

TEST(PickingControllerTest, MousePressRelease) {
    PickingManager pm;
    pm.registerPickingAction(nullptr, [](PickingEvent*) {}, 2);
    pm.registerPickingAction(nullptr, [](PickingEvent*) {}, 3);
    PickingControllerMouseState ms(&pm);
    {
        SCOPED_TRACE("Press-Release");
        doMousePress(ms);
    }
    {
        SCOPED_TRACE("Drag 3,3 -> 3,4");
        uvec2 pre{3, 3};  // id = 1
        uvec2 pos{3, 4};  // id = 1

        auto event = mouseEvent(B::None, S::Move, Bs{B::Left}, pos, dim);
        TestPropagator tp;
        ms.propagateEvent(&event, &tp, canvas[dim.y - pos.y][pos.x]);
        ASSERT_EQ(tp.events.size(), 1);
        testPickingEvent(tp.events[0].get(), PS::Updated, PHS::Move, PPS::Move, PPI::None,
                         PPIs{PPI::Primary}, 2, 2, 2, 2, 1, 1, 1, 1, ndc(pos), ndc(pre),
                         ndc(uvec2{3, 3}));
    }
    {
        SCOPED_TRACE("Release 3,4");
        uvec2 pre{3, 4};  // id = 1
        uvec2 pos{3, 4};  // id = 1
        auto event = mouseEvent(B::Left, S::Release, Bs{flags::empty}, pos, dim);
        TestPropagator tp;
        ms.propagateEvent(&event, &tp, canvas[dim.y - pos.y][pos.x]);
        ASSERT_EQ(tp.events.size(), 1);
        testPickingEvent(tp.events[0].get(), PS::Updated, PHS::None, PPS::Release, PPI::Primary,
                         PPIs{flags::empty}, 2, 2, 2, 2, 1, 1, 1, 1, ndc(pos), ndc(pre),
                         ndc(uvec2{3, 3}));
    }
    {
        SCOPED_TRACE("Move 3,4 -> 3,3");
        uvec2 pre{3, 4};  // id = 1
        uvec2 pos{3, 3};  // id = 1
        auto event = mouseEvent(B::None, S::Move, Bs(flags::empty), pos, dim);
        TestPropagator tp;
        ms.propagateEvent(&event, &tp, canvas[dim.y - pos.y][pos.x]);
        ASSERT_EQ(tp.events.size(), 1);
        testPickingEvent(tp.events[0].get(), PS::Updated, PHS::Move, PPS::None, PPI::None,
                         PPI::None, 2, 2, 2, 0, 1, 1, 1, 0, ndc(pos), ndc(pre), dvec3{0.0});
    }
}

TEST(PickingControllerTest, MouseDrag1to2) {
    PickingManager pm;
    pm.registerPickingAction(nullptr, [](PickingEvent*) {}, 2);
    pm.registerPickingAction(nullptr, [](PickingEvent*) {}, 3);
    PickingControllerMouseState ms(&pm);
    {
        SCOPED_TRACE("Drag");
        doMousePress(ms);
    }
    {
        SCOPED_TRACE("Drag 3,3 -> 6,6");
        uvec2 pre{3, 3};  // id = 1
        uvec2 pos{6, 6};  // id = 2

        auto event = mouseEvent(B::None, S::Move, Bs{B::Left}, pos, dim);
        TestPropagator tp;
        ms.propagateEvent(&event, &tp, canvas[dim.y - pos.y][pos.x]);
        ASSERT_EQ(tp.events.size(), 1);
        testPickingEvent(tp.events[0].get(), PS::Updated, PHS::Move, PPS::Move, PPI::None,
                         PPI::Primary, 2, 5, 2, 2, 1, 0, 1, 1, ndc(pos), ndc(pre), ndc(pre));
    }
}

TEST(PickingControllerTest, MouseDrag1to0) {
    PickingManager pm;
    pm.registerPickingAction(nullptr, [](PickingEvent*) {}, 2);
    pm.registerPickingAction(nullptr, [](PickingEvent*) {}, 3);
    PickingControllerMouseState ms(&pm);
    {
        SCOPED_TRACE("Drag");
        doMousePress(ms);
    }
    {
        SCOPED_TRACE("Drag 3,3 -> 1,1");
        uvec2 pre{3, 3};  // id = 1
        uvec2 pos{1, 1};  // id = 0

        auto event = mouseEvent(B::None, S::Move, Bs{B::Left}, pos, dim);
        TestPropagator tp;
        ms.propagateEvent(&event, &tp, canvas[dim.y - pos.y][pos.x]);
        ASSERT_EQ(tp.events.size(), 1);
        testPickingEvent(tp.events[0].get(), PS::Updated, PHS::Move, PPS::Move, PPI::None,
                         PPIs{PPI::Primary}, 2, 0, 2, 2, 1, 0, 1, 1, ndc(pos), ndc(pre),
                         ndc(uvec2{3, 3}));
    }
    {
        SCOPED_TRACE("Release 1,1");
        uvec2 pre{1, 1};  // id = 0
        uvec2 pos{1, 1};  // id = 0

        auto event = mouseEvent(B::Left, S::Release, Bs{flags::empty}, pos, dim);
        TestPropagator tp;
        ms.propagateEvent(&event, &tp, canvas[dim.y - pos.y][pos.x]);
        ASSERT_EQ(tp.events.size(), 1);
        testPickingEvent(tp.events[0].get(), PS::Finished, PHS::Exit, PPS::Release, PPI::Primary,
                         PPIs{flags::empty}, 2, 0, 2, 2, 1, 0, 1, 1, ndc(pos), ndc(pre),
                         ndc(uvec2{3, 3}));
    }
}

TEST(PickingControllerTest, MouseDrag0to1) {
    PickingManager pm;
    pm.registerPickingAction(nullptr, [](PickingEvent*) {}, 2);
    pm.registerPickingAction(nullptr, [](PickingEvent*) {}, 3);
    PickingControllerMouseState ms(&pm);
    {
        uvec2 pos{1, 1};  // id = 0
        auto event = mouseEvent(B::None, S::Move, Bs(flags::empty), pos, dim);

        TestPropagator tp;
        ms.propagateEvent(&event, &tp, canvas[dim.y - pos.y][pos.x]);
        ASSERT_EQ(tp.events.size(), 0);
    }
    {
        SCOPED_TRACE("Press 1,1");
        // uvec2 pre{1, 1};  // id = 0
        uvec2 pos{1, 1};  // id = 0

        auto event = mouseEvent(B::Left, S::Press, Bs{B::Left}, pos, dim);
        TestPropagator tp;
        ms.propagateEvent(&event, &tp, canvas[dim.y - pos.y][pos.x]);
        ASSERT_EQ(tp.events.size(), 0);
    }

    {
        SCOPED_TRACE("Drag 1,1 -> 3,3");
        // uvec2 pre{1, 1};  // id = 0
        uvec2 pos{3, 3};  // id = 1

        auto event = mouseEvent(B::None, S::Move, Bs{B::Left}, pos, dim);
        TestPropagator tp;
        ms.propagateEvent(&event, &tp, canvas[dim.y - pos.y][pos.x]);
        ASSERT_EQ(tp.events.size(), 0);
    }
    {
        SCOPED_TRACE("Release 3,3");
        // uvec2 pre{3, 3};  // id = 1
        uvec2 pos{3, 3};  // id = 1

        auto event = mouseEvent(B::Left, S::Release, Bs{flags::empty}, pos, dim);
        TestPropagator tp;
        ms.propagateEvent(&event, &tp, canvas[dim.y - pos.y][pos.x]);
        ASSERT_EQ(tp.events.size(), 1);
        testPickingEvent(tp.events[0].get(), PS::Started, PHS::Enter, PPS::None, PPI::Primary,
                         PPIs{flags::empty}, 2, 2, 0, 0, 1, 1, 1, 1, ndc(pos), dvec3{0.0},
                         dvec3{0.0});
    }
}

}  // namespace inviwo
