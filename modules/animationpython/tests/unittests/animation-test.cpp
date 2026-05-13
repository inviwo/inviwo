/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2026 Inviwo Foundation
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

#include <warn/push>
#include <warn/ignore/all>
#include <gtest/gtest.h>
#include <warn/pop>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/eval.h>

#include <inviwo/core/common/inviwoapplication.h>
#include <modules/python3/python3module.h>
#include <modules/python3/pybindutils.h>
#include <modules/python3/opaquetypes.h>

#include <modules/animation/datastructures/animationstate.h>
#include <modules/animation/datastructures/animationtime.h>
#include <modules/animation/datastructures/animation.h>
#include <modules/animation/datastructures/controlkeyframe.h>
#include <inviwo/core/algorithm/easing.h>

#include <fmt/format.h>

namespace inviwo {

namespace py = ::pybind11;

// ============================================================================
// Enum Tests
// ============================================================================

TEST(AnimationEnumTests, AnimationState) {
    const py::gil_scoped_acquire guard{};

    const std::string source = R"delim(
import ivwanimation

paused = ivwanimation.AnimationState.Paused
playing = ivwanimation.AnimationState.Playing
rendering = ivwanimation.AnimationState.Rendering
)delim";

    auto dict = py::cast<py::dict>(PyDict_Copy(py::globals().ptr()));
    py::eval<py::eval_statements>(source, dict);

    auto paused = dict["paused"].cast<animation::AnimationState>();
    auto playing = dict["playing"].cast<animation::AnimationState>();
    auto rendering = dict["rendering"].cast<animation::AnimationState>();

    EXPECT_EQ(animation::AnimationState::Paused, paused);
    EXPECT_EQ(animation::AnimationState::Playing, playing);
    EXPECT_EQ(animation::AnimationState::Rendering, rendering);
}

TEST(AnimationEnumTests, PlaybackMode) {
    const py::gil_scoped_acquire guard{};

    const std::string source = R"delim(
import ivwanimation

once = ivwanimation.PlaybackMode.Once
loop = ivwanimation.PlaybackMode.Loop
swing = ivwanimation.PlaybackMode.Swing
)delim";

    auto dict = py::cast<py::dict>(PyDict_Copy(py::globals().ptr()));
    py::eval<py::eval_statements>(source, dict);

    auto once = dict["once"].cast<animation::PlaybackMode>();
    auto loop = dict["loop"].cast<animation::PlaybackMode>();
    auto swing = dict["swing"].cast<animation::PlaybackMode>();

    EXPECT_EQ(animation::PlaybackMode::Once, once);
    EXPECT_EQ(animation::PlaybackMode::Loop, loop);
    EXPECT_EQ(animation::PlaybackMode::Swing, swing);
}

TEST(AnimationEnumTests, PlaybackDirection) {
    const py::gil_scoped_acquire guard{};

    const std::string source = R"delim(
import ivwanimation

fwd = ivwanimation.PlaybackDirection.Forward
bwd = ivwanimation.PlaybackDirection.Backward
)delim";

    auto dict = py::cast<py::dict>(PyDict_Copy(py::globals().ptr()));
    py::eval<py::eval_statements>(source, dict);

    auto fwd = dict["fwd"].cast<animation::PlaybackDirection>();
    auto bwd = dict["bwd"].cast<animation::PlaybackDirection>();

    EXPECT_EQ(animation::PlaybackDirection::Forward, fwd);
    EXPECT_EQ(animation::PlaybackDirection::Backward, bwd);
}

TEST(AnimationEnumTests, ControlAction) {
    const py::gil_scoped_acquire guard{};

    const std::string source = R"delim(
import ivwanimation

pause = ivwanimation.ControlAction.Pause
jump = ivwanimation.ControlAction.Jump
)delim";

    auto dict = py::cast<py::dict>(PyDict_Copy(py::globals().ptr()));
    py::eval<py::eval_statements>(source, dict);

    auto pause = dict["pause"].cast<animation::ControlAction>();
    auto jump = dict["jump"].cast<animation::ControlAction>();

    EXPECT_EQ(animation::ControlAction::Pause, pause);
    EXPECT_EQ(animation::ControlAction::Jump, jump);
}

TEST(AnimationEnumTests, EasingType) {
    const py::gil_scoped_acquire guard{};

    const std::string source = R"delim(
import ivwanimation

linear = ivwanimation.EasingType.Linear
cubic = ivwanimation.EasingType.Cubic
bounce = ivwanimation.EasingType.Bounce
)delim";

    auto dict = py::cast<py::dict>(PyDict_Copy(py::globals().ptr()));
    py::eval<py::eval_statements>(source, dict);

    auto linear = dict["linear"].cast<EasingType>();
    auto cubic = dict["cubic"].cast<EasingType>();
    auto bounce = dict["bounce"].cast<EasingType>();

    EXPECT_EQ(EasingType::linear, linear);
    EXPECT_EQ(EasingType::cubic, cubic);
    EXPECT_EQ(EasingType::bounce, bounce);
}

// ============================================================================
// Keyframe Tests
// ============================================================================

TEST(KeyframeTests, FloatKeyframeCreate) {
    const py::gil_scoped_acquire guard{};

    const std::string source = R"delim(
import ivwanimation

kf = ivwanimation.FloatKeyframe(1.5, 42.0)
time = kf.time
value = kf.value
)delim";

    auto dict = py::cast<py::dict>(PyDict_Copy(py::globals().ptr()));
    py::eval<py::eval_statements>(source, dict);

    auto time = dict["time"].cast<double>();
    auto value = dict["value"].cast<double>();

    EXPECT_DOUBLE_EQ(1.5, time);
    EXPECT_DOUBLE_EQ(42.0, value);
}

TEST(KeyframeTests, FloatKeyframeModify) {
    const py::gil_scoped_acquire guard{};

    const std::string source = R"delim(
import ivwanimation

kf = ivwanimation.FloatKeyframe(0.0, 10.0)
kf.time = 2.5
kf.value = 99.0
time = kf.time
value = kf.value
)delim";

    auto dict = py::cast<py::dict>(PyDict_Copy(py::globals().ptr()));
    py::eval<py::eval_statements>(source, dict);

    EXPECT_DOUBLE_EQ(2.5, dict["time"].cast<double>());
    EXPECT_DOUBLE_EQ(99.0, dict["value"].cast<double>());
}

TEST(KeyframeTests, IntKeyframeCreate) {
    const py::gil_scoped_acquire guard{};

    const std::string source = R"delim(
import ivwanimation

kf = ivwanimation.IntKeyframe(3.0, 7)
time = kf.time
value = kf.value
)delim";

    auto dict = py::cast<py::dict>(PyDict_Copy(py::globals().ptr()));
    py::eval<py::eval_statements>(source, dict);

    EXPECT_DOUBLE_EQ(3.0, dict["time"].cast<double>());
    EXPECT_EQ(7, dict["value"].cast<int>());
}

TEST(KeyframeTests, KeyframeSelection) {
    const py::gil_scoped_acquire guard{};

    const std::string source = R"delim(
import ivwanimation

kf = ivwanimation.FloatKeyframe(0.0, 1.0)
sel_before = kf.selected
kf.selected = True
sel_after = kf.selected
)delim";

    auto dict = py::cast<py::dict>(PyDict_Copy(py::globals().ptr()));
    py::eval<py::eval_statements>(source, dict);

    EXPECT_FALSE(dict["sel_before"].cast<bool>());
    EXPECT_TRUE(dict["sel_after"].cast<bool>());
}

TEST(KeyframeTests, KeyframeEasing) {
    const py::gil_scoped_acquire guard{};

    const std::string source = R"delim(
import ivwanimation

kf = ivwanimation.FloatKeyframe(0.0, 1.0)
ease_before = kf.easeIn
kf.easeIn = ivwanimation.EasingType.Cubic
ease_after = kf.easeIn
kf.easeIn = None
ease_none = kf.easeIn
)delim";

    auto dict = py::cast<py::dict>(PyDict_Copy(py::globals().ptr()));
    py::eval<py::eval_statements>(source, dict);

    EXPECT_TRUE(dict["ease_before"].is_none());
    auto easeAfter = dict["ease_after"].cast<EasingType>();
    EXPECT_EQ(EasingType::cubic, easeAfter);
    EXPECT_TRUE(dict["ease_none"].is_none());
}

TEST(KeyframeTests, CameraKeyframeCreate) {
    const py::gil_scoped_acquire guard{};

    const std::string source = R"delim(
import inviwopy
import ivwanimation

kf = ivwanimation.CameraKeyframe(2.0)
time = kf.time
lookFrom = kf.lookFrom
lookTo = kf.lookTo
lookUp = kf.lookUp
direction = kf.direction
)delim";

    auto dict = py::cast<py::dict>(PyDict_Copy(py::globals().ptr()));
    py::eval<py::eval_statements>(source, dict);

    EXPECT_DOUBLE_EQ(2.0, dict["time"].cast<double>());
    // Just verify they are accessible as vec3
    auto lookFrom = dict["lookFrom"].cast<vec3>();
    auto lookTo = dict["lookTo"].cast<vec3>();
    auto lookUp = dict["lookUp"].cast<vec3>();
    auto direction = dict["direction"].cast<vec3>();
    (void)lookFrom;
    (void)lookTo;
    (void)lookUp;
    (void)direction;
}

TEST(KeyframeTests, ControlKeyframeCreate) {
    const py::gil_scoped_acquire guard{};

    const std::string source = R"delim(
import ivwanimation

kf = ivwanimation.ControlKeyframe(5.0, ivwanimation.ControlAction.Jump, 2.0)
time = kf.time
action = kf.action
jumpTime = kf.jumpTime
)delim";

    auto dict = py::cast<py::dict>(PyDict_Copy(py::globals().ptr()));
    py::eval<py::eval_statements>(source, dict);

    EXPECT_DOUBLE_EQ(5.0, dict["time"].cast<double>());
    EXPECT_EQ(animation::ControlAction::Jump, dict["action"].cast<animation::ControlAction>());
    EXPECT_DOUBLE_EQ(2.0, dict["jumpTime"].cast<double>());
}

TEST(KeyframeTests, ControlKeyframeDefaults) {
    const py::gil_scoped_acquire guard{};

    const std::string source = R"delim(
import ivwanimation

kf = ivwanimation.ControlKeyframe(3.0)
action = kf.action
jumpTime = kf.jumpTime
)delim";

    auto dict = py::cast<py::dict>(PyDict_Copy(py::globals().ptr()));
    py::eval<py::eval_statements>(source, dict);

    EXPECT_EQ(animation::ControlAction::Pause, dict["action"].cast<animation::ControlAction>());
    EXPECT_DOUBLE_EQ(0.0, dict["jumpTime"].cast<double>());
}

TEST(KeyframeTests, ButtonKeyframeCreate) {
    const py::gil_scoped_acquire guard{};

    const std::string source = R"delim(
import ivwanimation

kf = ivwanimation.ButtonKeyframe(1.0)
time = kf.time
)delim";

    auto dict = py::cast<py::dict>(PyDict_Copy(py::globals().ptr()));
    py::eval<py::eval_statements>(source, dict);

    EXPECT_DOUBLE_EQ(1.0, dict["time"].cast<double>());
}

TEST(KeyframeTests, CallbackKeyframeCreate) {
    const py::gil_scoped_acquire guard{};

    const std::string source = R"delim(
import ivwanimation

called = False

def do_func():
    global called
    called = True

kf = ivwanimation.CallbackKeyframe(4.0, do_=do_func)
time = kf.time
)delim";

    auto dict = py::cast<py::dict>(PyDict_Copy(py::globals().ptr()));
    py::eval<py::eval_statements>(source, dict);

    EXPECT_DOUBLE_EQ(4.0, dict["time"].cast<double>());
}

TEST(KeyframeTests, KeyframeRepr) {
    const py::gil_scoped_acquire guard{};

    const std::string source = R"delim(
import ivwanimation

kf = ivwanimation.FloatKeyframe(1.5, 42.0)
r = repr(kf)
)delim";

    auto dict = py::cast<py::dict>(PyDict_Copy(py::globals().ptr()));
    py::eval<py::eval_statements>(source, dict);

    auto r = dict["r"].cast<std::string>();
    EXPECT_TRUE(r.find("FloatKeyframe") != std::string::npos);
    EXPECT_TRUE(r.find("1.5") != std::string::npos);
}

// ============================================================================
// KeyframeSequence Tests
// ============================================================================

TEST(KeyframeSequenceTests, FloatSequenceCreate) {
    const py::gil_scoped_acquire guard{};

    const std::string source = R"delim(
import ivwanimation

seq = ivwanimation.FloatKeyframeSequence()
size = len(seq)
)delim";

    auto dict = py::cast<py::dict>(PyDict_Copy(py::globals().ptr()));
    py::eval<py::eval_statements>(source, dict);

    EXPECT_EQ(0u, dict["size"].cast<size_t>());
}

TEST(KeyframeSequenceTests, ControlSequenceCreate) {
    const py::gil_scoped_acquire guard{};

    const std::string source = R"delim(
import ivwanimation

seq = ivwanimation.ControlKeyframeSequence()
size = len(seq)
)delim";

    auto dict = py::cast<py::dict>(PyDict_Copy(py::globals().ptr()));
    py::eval<py::eval_statements>(source, dict);

    EXPECT_EQ(0u, dict["size"].cast<size_t>());
}

TEST(KeyframeSequenceTests, ButtonSequenceCreate) {
    const py::gil_scoped_acquire guard{};

    const std::string source = R"delim(
import ivwanimation

seq = ivwanimation.ButtonKeyframeSequence()
size = len(seq)
)delim";

    auto dict = py::cast<py::dict>(PyDict_Copy(py::globals().ptr()));
    py::eval<py::eval_statements>(source, dict);

    EXPECT_EQ(0u, dict["size"].cast<size_t>());
}

TEST(KeyframeSequenceTests, SequenceSelection) {
    const py::gil_scoped_acquire guard{};

    const std::string source = R"delim(
import ivwanimation

seq = ivwanimation.FloatKeyframeSequence()
sel_before = seq.selected
seq.selected = True
sel_after = seq.selected
)delim";

    auto dict = py::cast<py::dict>(PyDict_Copy(py::globals().ptr()));
    py::eval<py::eval_statements>(source, dict);

    EXPECT_FALSE(dict["sel_before"].cast<bool>());
    EXPECT_TRUE(dict["sel_after"].cast<bool>());
}

// ============================================================================
// Track Tests
// ============================================================================

TEST(TrackTests, ControlTrackCreate) {
    const py::gil_scoped_acquire guard{};

    const std::string source = R"delim(
import ivwanimation

track = ivwanimation.ControlTrack()
name = track.name
enabled = track.enabled
size = len(track)
empty = track.empty
)delim";

    auto dict = py::cast<py::dict>(PyDict_Copy(py::globals().ptr()));
    py::eval<py::eval_statements>(source, dict);

    EXPECT_TRUE(dict["enabled"].cast<bool>());
    EXPECT_EQ(0u, dict["size"].cast<size_t>());
    EXPECT_TRUE(dict["empty"].cast<bool>());
}

TEST(TrackTests, TrackProperties) {
    const py::gil_scoped_acquire guard{};

    const std::string source = R"delim(
import ivwanimation

track = ivwanimation.ControlTrack()
track.name = "MyTrack"
track.enabled = False
track.priority = 5
name = track.name
enabled = track.enabled
priority = track.priority
)delim";

    auto dict = py::cast<py::dict>(PyDict_Copy(py::globals().ptr()));
    py::eval<py::eval_statements>(source, dict);

    EXPECT_EQ("MyTrack", dict["name"].cast<std::string>());
    EXPECT_FALSE(dict["enabled"].cast<bool>());
    EXPECT_EQ(5u, dict["priority"].cast<size_t>());
}

TEST(TrackTests, TrackAddKeyframe) {
    const py::gil_scoped_acquire guard{};

    const std::string source = R"delim(
import ivwanimation

track = ivwanimation.ControlTrack()
kf = track.addKeyframe(1.0)
size_after = len(track)
)delim";

    auto dict = py::cast<py::dict>(PyDict_Copy(py::globals().ptr()));
    py::eval<py::eval_statements>(source, dict);

    EXPECT_EQ(1u, dict["size_after"].cast<size_t>());
}

TEST(TrackTests, TrackAddMultipleKeyframes) {
    const py::gil_scoped_acquire guard{};

    const std::string source = R"delim(
import ivwanimation

track = ivwanimation.ControlTrack()
track.addKeyframe(0.0)
track.addKeyframe(1.0)
track.addKeyframe(2.0)
size = len(track)
times = track.getAllTimes()
)delim";

    auto dict = py::cast<py::dict>(PyDict_Copy(py::globals().ptr()));
    py::eval<py::eval_statements>(source, dict);

    EXPECT_EQ(1u, dict["size"].cast<size_t>());  // 1 sequence, 3 keyframes
    auto times = dict["times"].cast<std::vector<double>>();
    ASSERT_EQ(3u, times.size());
    EXPECT_DOUBLE_EQ(0.0, times[0]);
    EXPECT_DOUBLE_EQ(1.0, times[1]);
    EXPECT_DOUBLE_EQ(2.0, times[2]);
}

TEST(TrackTests, TrackTimeQueries) {
    const py::gil_scoped_acquire guard{};

    const std::string source = R"delim(
import ivwanimation

track = ivwanimation.ControlTrack()
track.addKeyframe(1.0)
track.addKeyframe(3.0)
track.addKeyframe(5.0)
firstTime = track.firstTime
lastTime = track.lastTime
prevTime = track.getPrevTime(4.0)
nextTime = track.getNextTime(2.0)
noPrev = track.getPrevTime(0.5)
)delim";

    auto dict = py::cast<py::dict>(PyDict_Copy(py::globals().ptr()));
    py::eval<py::eval_statements>(source, dict);

    EXPECT_DOUBLE_EQ(1.0, dict["firstTime"].cast<double>());
    EXPECT_DOUBLE_EQ(5.0, dict["lastTime"].cast<double>());
    EXPECT_DOUBLE_EQ(3.0, dict["prevTime"].cast<double>());
    EXPECT_DOUBLE_EQ(3.0, dict["nextTime"].cast<double>());
    EXPECT_TRUE(dict["noPrev"].is_none());
}

TEST(TrackTests, TrackRepr) {
    const py::gil_scoped_acquire guard{};

    const std::string source = R"delim(
import ivwanimation

track = ivwanimation.ControlTrack()
r = repr(track)
)delim";

    auto dict = py::cast<py::dict>(PyDict_Copy(py::globals().ptr()));
    py::eval<py::eval_statements>(source, dict);

    auto r = dict["r"].cast<std::string>();
    EXPECT_TRUE(r.find("ControlTrack") != std::string::npos);
}

TEST(TrackTests, CallbackTrackCreate) {
    const py::gil_scoped_acquire guard{};

    const std::string source = R"delim(
import ivwanimation

track = ivwanimation.CallbackTrack()
name = track.name
size = len(track)
)delim";

    auto dict = py::cast<py::dict>(PyDict_Copy(py::globals().ptr()));
    py::eval<py::eval_statements>(source, dict);

    EXPECT_EQ(0u, dict["size"].cast<size_t>());
}

// ============================================================================
// Animation Tests
// ============================================================================

TEST(AnimationTests, AnimationCreate) {
    const py::gil_scoped_acquire guard{};

    const std::string source = R"delim(
import ivwanimation

anim = ivwanimation.Animation("TestAnim")
name = anim.name
size = len(anim)
empty = anim.empty
)delim";

    auto dict = py::cast<py::dict>(PyDict_Copy(py::globals().ptr()));
    py::eval<py::eval_statements>(source, dict);

    EXPECT_EQ("TestAnim", dict["name"].cast<std::string>());
    EXPECT_EQ(0u, dict["size"].cast<size_t>());
    EXPECT_TRUE(dict["empty"].cast<bool>());
}

TEST(AnimationTests, AnimationDefaultName) {
    const py::gil_scoped_acquire guard{};

    const std::string source = R"delim(
import ivwanimation

anim = ivwanimation.Animation()
name = anim.name
)delim";

    auto dict = py::cast<py::dict>(PyDict_Copy(py::globals().ptr()));
    py::eval<py::eval_statements>(source, dict);

    EXPECT_EQ("Animation", dict["name"].cast<std::string>());
}

TEST(AnimationTests, AnimationSetName) {
    const py::gil_scoped_acquire guard{};

    const std::string source = R"delim(
import ivwanimation

anim = ivwanimation.Animation()
anim.name = "NewName"
name = anim.name
)delim";

    auto dict = py::cast<py::dict>(PyDict_Copy(py::globals().ptr()));
    py::eval<py::eval_statements>(source, dict);

    EXPECT_EQ("NewName", dict["name"].cast<std::string>());
}

TEST(AnimationTests, AnimationTimeQueries) {
    const py::gil_scoped_acquire guard{};

    const std::string source = R"delim(
import ivwanimation

anim = ivwanimation.Animation()
firstTime_empty = anim.firstTime
lastTime_empty = anim.lastTime
)delim";

    auto dict = py::cast<py::dict>(PyDict_Copy(py::globals().ptr()));
    py::eval<py::eval_statements>(source, dict);

    EXPECT_DOUBLE_EQ(0.0, dict["firstTime_empty"].cast<double>());
    EXPECT_DOUBLE_EQ(0.0, dict["lastTime_empty"].cast<double>());
}

TEST(AnimationTests, AnimationRepr) {
    const py::gil_scoped_acquire guard{};

    const std::string source = R"delim(
import ivwanimation

anim = ivwanimation.Animation("MyAnim")
r = repr(anim)
)delim";

    auto dict = py::cast<py::dict>(PyDict_Copy(py::globals().ptr()));
    py::eval<py::eval_statements>(source, dict);

    auto r = dict["r"].cast<std::string>();
    EXPECT_TRUE(r.find("Animation") != std::string::npos);
    EXPECT_TRUE(r.find("MyAnim") != std::string::npos);
}

TEST(AnimationTests, AnimationClear) {
    const py::gil_scoped_acquire guard{};

    const std::string source = R"delim(
import ivwanimation

anim = ivwanimation.Animation()
anim.clear()
size = len(anim)
)delim";

    auto dict = py::cast<py::dict>(PyDict_Copy(py::globals().ptr()));
    py::eval<py::eval_statements>(source, dict);

    EXPECT_EQ(0u, dict["size"].cast<size_t>());
}

// ============================================================================
// AnimationTimeState Tests
// ============================================================================

TEST(AnimationTimeStateTests, Create) {
    const py::gil_scoped_acquire guard{};

    const std::string source = R"delim(
import ivwanimation

ats = ivwanimation.AnimationTimeState(2.5, ivwanimation.AnimationState.Playing)
time = ats.time
state = ats.state
)delim";

    auto dict = py::cast<py::dict>(PyDict_Copy(py::globals().ptr()));
    py::eval<py::eval_statements>(source, dict);

    EXPECT_DOUBLE_EQ(2.5, dict["time"].cast<double>());
    EXPECT_EQ(animation::AnimationState::Playing, dict["state"].cast<animation::AnimationState>());
}

TEST(AnimationTimeStateTests, Modify) {
    const py::gil_scoped_acquire guard{};

    const std::string source = R"delim(
import ivwanimation

ats = ivwanimation.AnimationTimeState(0.0, ivwanimation.AnimationState.Paused)
ats.time = 5.0
ats.state = ivwanimation.AnimationState.Playing
time = ats.time
state = ats.state
)delim";

    auto dict = py::cast<py::dict>(PyDict_Copy(py::globals().ptr()));
    py::eval<py::eval_statements>(source, dict);

    EXPECT_DOUBLE_EQ(5.0, dict["time"].cast<double>());
    EXPECT_EQ(animation::AnimationState::Playing, dict["state"].cast<animation::AnimationState>());
}

// ============================================================================
// Vec Keyframe Tests
// ============================================================================

TEST(KeyframeTests, Vec3KeyframeCreate) {
    const py::gil_scoped_acquire guard{};

    const std::string source = R"delim(
import inviwopy
import ivwanimation

kf = ivwanimation.Vec3Keyframe(1.0, inviwopy.glm.vec3(1.0, 2.0, 3.0))
time = kf.time
value = kf.value
)delim";

    auto dict = py::cast<py::dict>(PyDict_Copy(py::globals().ptr()));
    py::eval<py::eval_statements>(source, dict);

    EXPECT_DOUBLE_EQ(1.0, dict["time"].cast<double>());
    auto val = dict["value"].cast<vec3>();
    EXPECT_FLOAT_EQ(1.0f, val.x);
    EXPECT_FLOAT_EQ(2.0f, val.y);
    EXPECT_FLOAT_EQ(3.0f, val.z);
}

// ============================================================================
// InvalidationKeyframe Tests
// ============================================================================

TEST(KeyframeTests, InvalidationKeyframeCreate) {
    const py::gil_scoped_acquire guard{};

    const std::string source = R"delim(
import ivwanimation

kf = ivwanimation.InvalidationKeyframe(3.0)
time = kf.time
)delim";

    auto dict = py::cast<py::dict>(PyDict_Copy(py::globals().ptr()));
    py::eval<py::eval_statements>(source, dict);

    EXPECT_DOUBLE_EQ(3.0, dict["time"].cast<double>());
}

// ============================================================================
// InvalidationKeyframeSequence Tests
// ============================================================================

TEST(KeyframeSequenceTests, InvalidationSequenceCreate) {
    const py::gil_scoped_acquire guard{};

    const std::string source = R"delim(
import ivwanimation

seq = ivwanimation.InvalidationKeyframeSequence()
seq.path = "processor/property"
size = len(seq)
path = seq.path
)delim";

    auto dict = py::cast<py::dict>(PyDict_Copy(py::globals().ptr()));
    py::eval<py::eval_statements>(source, dict);

    EXPECT_EQ(0u, dict["size"].cast<size_t>());
    EXPECT_EQ("processor/property", dict["path"].cast<std::string>());
}

// ============================================================================
// Interpolation Tests
// ============================================================================

TEST(InterpolationTests, FloatSequenceHasInterpolation) {
    const py::gil_scoped_acquire guard{};

    const std::string source = R"delim(
import ivwanimation

seq = ivwanimation.FloatKeyframeSequence()
interp = seq.interpolation
displayName = interp.displayName
classId = interp.classIdentifier
r = repr(interp)
)delim";

    auto dict = py::cast<py::dict>(PyDict_Copy(py::globals().ptr()));
    py::eval<py::eval_statements>(source, dict);

    auto displayName = dict["displayName"].cast<std::string>();
    auto classId = dict["classId"].cast<std::string>();

    EXPECT_FALSE(displayName.empty());
    EXPECT_FALSE(classId.empty());
}

// ============================================================================
// Module import test
// ============================================================================

TEST(ModuleTests, Import) {
    const py::gil_scoped_acquire guard{};

    const std::string source = R"delim(
import ivwanimation

# Verify all major types are accessible
types = [
    ivwanimation.AnimationState,
    ivwanimation.PlaybackMode,
    ivwanimation.PlaybackDirection,
    ivwanimation.ControlAction,
    ivwanimation.EasingType,
    ivwanimation.EasingMode,
    ivwanimation.AnimationTimeState,
    ivwanimation.Keyframe,
    ivwanimation.BaseKeyframe,
    ivwanimation.FloatKeyframe,
    ivwanimation.DoubleKeyframe,
    ivwanimation.IntKeyframe,
    ivwanimation.Vec2Keyframe,
    ivwanimation.Vec3Keyframe,
    ivwanimation.Vec4Keyframe,
    ivwanimation.CameraKeyframe,
    ivwanimation.ControlKeyframe,
    ivwanimation.ButtonKeyframe,
    ivwanimation.CallbackKeyframe,
    ivwanimation.InvalidationKeyframe,
    ivwanimation.KeyframeSequence,
    ivwanimation.FloatKeyframeSequence,
    ivwanimation.ControlKeyframeSequence,
    ivwanimation.ButtonKeyframeSequence,
    ivwanimation.CallbackKeyframeSequence,
    ivwanimation.InvalidationKeyframeSequence,
    ivwanimation.Interpolation,
    ivwanimation.Track,
    ivwanimation.BasePropertyTrack,
    ivwanimation.ControlTrack,
    ivwanimation.CallbackTrack,
    ivwanimation.Animation,
    ivwanimation.AnimationController,
    ivwanimation.MainAnimation,
    ivwanimation.WorkspaceAnimations,
]
count = len(types)
)delim";

    auto dict = py::cast<py::dict>(PyDict_Copy(py::globals().ptr()));
    py::eval<py::eval_statements>(source, dict);

    auto count = dict["count"].cast<size_t>();
    EXPECT_EQ(35u, count);
}

// ============================================================================
// End-to-End Animation Test
// ============================================================================

TEST(EndToEndTests, AnimateProcessorProperty) {
    const py::gil_scoped_acquire guard{};

    const std::string source = R"delim(
import inviwopy
import ivwanimation

app = inviwopy.app
network = app.network

# Create a processor with a FloatProperty
proc = app.processorFactory.create("org.inviwo.RandomSphereGenerator")
network.addProcessor(proc)

# Get the workspace animation
wa = ivwanimation.getWorkspaceAnimations(app)
mainAnim = wa.mainAnimation
anim = mainAnim.animation
controller = mainAnim.controller

# Get the FloatProperty "scale" from the processor
scaleProp = proc.scale

# Set scale to 1.0 and add a keyframe at t=0
scaleProp.value = 1.0
track = anim.add(scaleProp)
track.addKeyFrameUsingPropertyValue(0.0)

# Set scale to 10.0 and add a keyframe at t=2
scaleProp.value = 10.0
track.addKeyFrameUsingPropertyValue(2.0)

# Verify we have 1 track with keyframes
numTracks = len(anim)
times = anim.getAllTimes()

# Evaluate at t=0: property should be 1.0
controller.eval(0.0, 0.0)
val_at_0 = scaleProp.value

# Evaluate at t=2: property should be 10.0
controller.eval(0.0, 2.0)
val_at_2 = scaleProp.value

# Evaluate at t=1 (midpoint): with linear interpolation should be ~5.5
controller.eval(0.0, 1.0)
val_at_1 = scaleProp.value

# Clean up
anim.clear()
network.removeProcessor(proc)
)delim";

    auto dict = py::cast<py::dict>(PyDict_Copy(py::globals().ptr()));
    py::eval<py::eval_statements>(source, dict);

    EXPECT_EQ(1u, dict["numTracks"].cast<size_t>());
    auto times = dict["times"].cast<std::vector<double>>();
    ASSERT_EQ(2u, times.size());
    EXPECT_DOUBLE_EQ(0.0, times[0]);
    EXPECT_DOUBLE_EQ(2.0, times[1]);

    auto val0 = dict["val_at_0"].cast<float>();
    auto val2 = dict["val_at_2"].cast<float>();
    auto val1 = dict["val_at_1"].cast<float>();

    EXPECT_FLOAT_EQ(1.0f, val0);
    EXPECT_FLOAT_EQ(10.0f, val2);
    // Linear interpolation midpoint: should be between 1.0 and 10.0
    EXPECT_GT(val1, 1.0f);
    EXPECT_LT(val1, 10.0f);
}

TEST(EndToEndTests, AnimateMultipleKeyframes) {
    const py::gil_scoped_acquire guard{};

    const std::string source = R"delim(
import inviwopy
import ivwanimation

app = inviwopy.app
network = app.network

# Create a processor
proc = app.processorFactory.create("org.inviwo.RandomSphereGenerator")
network.addProcessor(proc)

# Get animation
wa = ivwanimation.getWorkspaceAnimations(app)
mainAnim = wa.mainAnimation
anim = mainAnim.animation
controller = mainAnim.controller

# Clear any leftover state from previous tests
anim.clear()

# Get the "size" property by identifier (proc.size conflicts with a method)
sizeProp = proc.getPropertyByIdentifier("size")

# Add keyframes: 0.0->1.0, 1.0->5.0, 2.0->3.0
sizeProp.value = 1.0
track = anim.add(sizeProp)
track.addKeyFrameUsingPropertyValue(0.0)

sizeProp.value = 5.0
track.addKeyFrameUsingPropertyValue(1.0)

sizeProp.value = 3.0
track.addKeyFrameUsingPropertyValue(2.0)

numTimes = len(anim.getAllTimes())

# Evaluate at each keyframe time
controller.eval(0.0, 0.0)
val_at_0 = sizeProp.value

controller.eval(0.0, 1.0)
val_at_1 = sizeProp.value

controller.eval(0.0, 2.0)
val_at_2 = sizeProp.value

# Evaluate between keyframes
controller.eval(0.0, 0.5)
val_at_05 = sizeProp.value

# Clean up
anim.clear()
network.removeProcessor(proc)
)delim";

    auto dict = py::cast<py::dict>(PyDict_Copy(py::globals().ptr()));
    py::eval<py::eval_statements>(source, dict);

    EXPECT_EQ(3u, dict["numTimes"].cast<size_t>());

    auto val0 = dict["val_at_0"].cast<float>();
    auto val1 = dict["val_at_1"].cast<float>();
    auto val2 = dict["val_at_2"].cast<float>();
    auto val05 = dict["val_at_05"].cast<float>();

    EXPECT_FLOAT_EQ(1.0f, val0);
    EXPECT_FLOAT_EQ(5.0f, val1);
    EXPECT_FLOAT_EQ(3.0f, val2);
    // Between t=0 and t=1, should be between 1.0 and 5.0
    EXPECT_GT(val05, 1.0f);
    EXPECT_LT(val05, 5.0f);
}

}  // namespace inviwo
