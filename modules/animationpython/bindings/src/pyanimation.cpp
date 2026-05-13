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

#include <ivwanimation/pyanimation.h>

#include <pybind11/pybind11.h>
#include <pybind11/functional.h>
#include <pybind11/stl.h>
#include <pybind11/stl_bind.h>
#include <pybind11/chrono.h>

#include <inviwo/core/algorithm/easing.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/properties/propertyowner.h>
#include <inviwo/core/properties/property.h>

#include <modules/animation/datastructures/animationstate.h>
#include <modules/animation/datastructures/animationtime.h>
#include <modules/animation/datastructures/keyframe.h>
#include <modules/animation/datastructures/basekeyframe.h>
#include <modules/animation/datastructures/valuekeyframe.h>
#include <modules/animation/datastructures/camerakeyframe.h>
#include <modules/animation/datastructures/controlkeyframe.h>
#include <modules/animation/datastructures/buttonkeyframe.h>
#include <modules/animation/datastructures/callbackkeyframe.h>
#include <modules/animation/datastructures/keyframesequence.h>
#include <modules/animation/datastructures/basekeyframesequence.h>
#include <modules/animation/datastructures/valuekeyframesequence.h>
#include <modules/animation/datastructures/controlkeyframesequence.h>
#include <modules/animation/datastructures/buttonkeyframesequence.h>
#include <modules/animation/datastructures/callbackkeyframesequence.h>
#include <modules/animation/datastructures/invalidationtrack.h>
#include <modules/animation/datastructures/track.h>
#include <modules/animation/datastructures/basetrack.h>
#include <modules/animation/datastructures/propertytrack.h>
#include <modules/animation/datastructures/controltrack.h>
#include <modules/animation/datastructures/callbacktrack.h>
#include <modules/animation/datastructures/cameratrack.h>
#include <modules/animation/datastructures/buttontrack.h>
#include <modules/animation/datastructures/animation.h>
#include <modules/animation/interpolation/interpolation.h>
#include <modules/animation/interpolation/constantinterpolation.h>
#include <modules/animation/interpolation/linearinterpolation.h>
#include <modules/animation/interpolation/cameralinearinterpolation.h>
#include <modules/animation/interpolation/camerasphericalinterpolation.h>
#include <modules/animation/animationcontroller.h>
#include <modules/animation/mainanimation.h>
#include <modules/animation/workspaceanimations.h>
#include <modules/animation/animationmodule.h>

#include <inviwo/core/util/moduleutils.h>

#include <modules/python3/opaquetypes.h>
#include <modules/python3/polymorphictypehooks.h>

#include <fmt/format.h>

namespace py = pybind11;

namespace inviwo {

using namespace animation;

namespace {

// Helper to convert Seconds (std::chrono::duration<double>) to/from Python float
struct SecondsConverter {
    static double toDouble(Seconds s) { return s.count(); }
    static Seconds fromDouble(double d) { return Seconds{d}; }
};

void exposeEnums(py::module& m) {
    py::enum_<AnimationState>(m, "AnimationState")
        .value("Paused", AnimationState::Paused)
        .value("Playing", AnimationState::Playing)
        .value("Rendering", AnimationState::Rendering);

    py::enum_<PlaybackMode>(m, "PlaybackMode")
        .value("Once", PlaybackMode::Once)
        .value("Loop", PlaybackMode::Loop)
        .value("Swing", PlaybackMode::Swing);

    py::enum_<PlaybackDirection>(m, "PlaybackDirection")
        .value("Forward", PlaybackDirection::Forward)
        .value("Backward", PlaybackDirection::Backward);

    py::enum_<ControlAction>(m, "ControlAction")
        .value("Pause", ControlAction::Pause)
        .value("Jump", ControlAction::Jump);

    py::enum_<EasingType>(m, "EasingType")
        .value("Linear", EasingType::linear)
        .value("Quadratic", EasingType::quadratic)
        .value("Cubic", EasingType::cubic)
        .value("Quartic", EasingType::quartic)
        .value("Quintic", EasingType::quintic)
        .value("Sine", EasingType::sine)
        .value("Circular", EasingType::circular)
        .value("Exponential", EasingType::exponential)
        .value("Elastic", EasingType::elastic)
        .value("Back", EasingType::back)
        .value("Bounce", EasingType::bounce);

    py::enum_<EasingMode>(m, "EasingMode")
        .value("In", EasingMode::in)
        .value("Out", EasingMode::out)
        .value("InOut", EasingMode::inOut);
}

void exposeAnimationTimeState(py::module& m) {
    py::classh<AnimationTimeState>(m, "AnimationTimeState")
        .def(py::init([](double time, AnimationState state) {
                 return std::make_unique<AnimationTimeState>(Seconds{time}, state);
             }),
             py::arg("time"), py::arg("state"))
        .def_property(
            "time", [](const AnimationTimeState& s) { return s.time.count(); },
            [](AnimationTimeState& s, double t) { s.time = Seconds{t}; })
        .def_readwrite("state", &AnimationTimeState::state)
        .def("__repr__", [](const AnimationTimeState& s) {
            return fmt::format("<AnimationTimeState: time={}, state={}>", s.time.count(),
                               static_cast<int>(s.state));
        });
}

void exposeKeyframe(py::module& m) {
    py::classh<Keyframe>(m, "Keyframe")
        .def_property(
            "time", [](const Keyframe& k) { return k.getTime().count(); },
            [](Keyframe& k, double t) { k.setTime(Seconds{t}); })
        .def_property("selected", &Keyframe::isSelected, &Keyframe::setSelected)
        .def("__repr__", [](const Keyframe& k) {
            return fmt::format("<Keyframe: time={}>", k.getTime().count());
        });
}

void exposeBaseKeyframe(py::module& m) {
    py::classh<BaseKeyframe, Keyframe>(m, "BaseKeyframe")
        .def_property(
            "easeIn",
            [](const BaseKeyframe& k) -> py::object {
                auto e = k.getEaseIn();
                if (e) return py::cast(*e);
                return py::none();
            },
            [](BaseKeyframe& k, std::optional<EasingType> e) { k.setEaseIn(e); })
        .def_property(
            "easeOut",
            [](const BaseKeyframe& k) -> py::object {
                auto e = k.getEaseOut();
                if (e) return py::cast(*e);
                return py::none();
            },
            [](BaseKeyframe& k, std::optional<EasingType> e) { k.setEaseOut(e); });
}

template <typename T>
void exposeValueKeyframe(py::module& m, const std::string& name) {
    using VK = ValueKeyframe<T>;
    py::classh<VK, BaseKeyframe>(m, name.c_str())
        .def(py::init([](double time, const T& value) {
                 return std::make_unique<VK>(Seconds{time}, value);
             }),
             py::arg("time"), py::arg("value"))
        .def(py::init([](double time) { return std::make_unique<VK>(Seconds{time}); }),
             py::arg("time"))
        .def_property("value", py::overload_cast<>(&VK::getValue, py::const_), &VK::setValue)
        .def("__repr__", [name](const VK& k) {
            return fmt::format("<{}: time={}>", name, k.getTime().count());
        });
}

void exposeCameraKeyframe(py::module& m) {
    py::classh<CameraKeyframe, BaseKeyframe>(m, "CameraKeyframe")
        .def(py::init([](double time) { return std::make_unique<CameraKeyframe>(Seconds{time}); }),
             py::arg("time"))
        .def_property("lookFrom", &CameraKeyframe::getLookFrom, &CameraKeyframe::setLookFrom)
        .def_property("lookTo", &CameraKeyframe::getLookTo, &CameraKeyframe::setLookTo)
        .def_property("lookUp", &CameraKeyframe::getLookUp, &CameraKeyframe::setLookUp)
        .def_property_readonly("direction", &CameraKeyframe::getDirection)
        .def("__repr__", [](const CameraKeyframe& k) {
            return fmt::format("<CameraKeyframe: time={}>", k.getTime().count());
        });
}

void exposeControlKeyframe(py::module& m) {
    py::classh<ControlKeyframe, BaseKeyframe>(m, "ControlKeyframe")
        .def(py::init([](double time, ControlAction action, double jumpTime) {
                 return std::make_unique<ControlKeyframe>(Seconds{time}, action, Seconds{jumpTime});
             }),
             py::arg("time"), py::arg("action") = ControlAction::Pause, py::arg("jumpTime") = 0.0)
        .def_property("action", &ControlKeyframe::getAction, &ControlKeyframe::setAction)
        .def_property(
            "jumpTime", [](const ControlKeyframe& k) { return k.getJumpTime().count(); },
            [](ControlKeyframe& k, double t) { k.setJumpTime(Seconds{t}); })
        .def("__repr__", [](const ControlKeyframe& k) {
            return fmt::format("<ControlKeyframe: time={}, action={}>", k.getTime().count(),
                               k.getAction() == ControlAction::Pause ? "Pause" : "Jump");
        });
}

void exposeButtonKeyframe(py::module& m) {
    py::classh<ButtonKeyframe, BaseKeyframe>(m, "ButtonKeyframe")
        .def(py::init([](double time) { return std::make_unique<ButtonKeyframe>(Seconds{time}); }),
             py::arg("time"))
        .def("__repr__", [](const ButtonKeyframe& k) {
            return fmt::format("<ButtonKeyframe: time={}>", k.getTime().count());
        });
}

void exposeCallbackKeyframe(py::module& m) {
    py::classh<CallbackKeyframe, BaseKeyframe>(m, "CallbackKeyframe")
        .def(
            py::init([](double time, std::function<void()> doFunc, std::function<void()> undoFunc) {
                return std::make_unique<CallbackKeyframe>(Seconds{time}, std::move(doFunc),
                                                          std::move(undoFunc));
            }),
            py::arg("time"), py::arg("do_") = py::none(), py::arg("undo") = py::none())
        .def("__repr__", [](const CallbackKeyframe& k) {
            return fmt::format("<CallbackKeyframe: time={}>", k.getTime().count());
        });
}

void exposeInvalidationKeyframe(py::module& m) {
    py::classh<InvalidationKeyframe, BaseKeyframe>(m, "InvalidationKeyframe")
        .def(py::init(
                 [](double time) { return std::make_unique<InvalidationKeyframe>(Seconds{time}); }),
             py::arg("time"))
        .def("__repr__", [](const InvalidationKeyframe& k) {
            return fmt::format("<InvalidationKeyframe: time={}>", k.getTime().count());
        });
}

void exposeKeyframeSequence(py::module& m) {
    py::classh<KeyframeSequence>(m, "KeyframeSequence")
        .def_property_readonly("size", &KeyframeSequence::size)
        .def("__len__", &KeyframeSequence::size)
        .def(
            "__getitem__",
            [](KeyframeSequence& seq, size_t i) -> Keyframe& {
                if (i >= seq.size()) throw py::index_error();
                return seq[i];
            },
            py::return_value_policy::reference_internal)
        .def_property_readonly("firstTime",
                               [](const KeyframeSequence& s) { return s.getFirstTime().count(); })
        .def_property_readonly("lastTime",
                               [](const KeyframeSequence& s) { return s.getLastTime().count(); })
        .def(
            "getPrevTime",
            [](const KeyframeSequence& s, double at) -> py::object {
                auto t = s.getPrevTime(Seconds{at});
                if (t) return py::cast(t->count());
                return py::none();
            },
            py::arg("at"))
        .def(
            "getNextTime",
            [](const KeyframeSequence& s, double at) -> py::object {
                auto t = s.getNextTime(Seconds{at});
                if (t) return py::cast(t->count());
                return py::none();
            },
            py::arg("at"))
        .def_property("selected", &KeyframeSequence::isSelected, &KeyframeSequence::setSelected)
        .def(
            "remove", [](KeyframeSequence& seq, size_t i) { return seq.remove(i); },
            py::arg("index"))
        .def(
            "remove", [](KeyframeSequence& seq, Keyframe* key) { return seq.remove(key); },
            py::arg("key"))
        .def("__repr__", [](const KeyframeSequence& seq) {
            return fmt::format("<KeyframeSequence: {} keyframes>", seq.size());
        });
}

void exposeControlKeyframeSequence(py::module& m) {
    py::classh<ControlKeyframeSequence, KeyframeSequence>(m, "ControlKeyframeSequence")
        .def(py::init<>())
        .def("__repr__", [](const ControlKeyframeSequence& seq) {
            return fmt::format("<ControlKeyframeSequence: {} keyframes>", seq.size());
        });
}

void exposeButtonKeyframeSequence(py::module& m) {
    py::classh<ButtonKeyframeSequence, KeyframeSequence>(m, "ButtonKeyframeSequence")
        .def(py::init<>())
        .def("__repr__", [](const ButtonKeyframeSequence& seq) {
            return fmt::format("<ButtonKeyframeSequence: {} keyframes>", seq.size());
        });
}

void exposeCallbackKeyframeSequence(py::module& m) {
    py::classh<CallbackKeyframeSequence, KeyframeSequence>(m, "CallbackKeyframeSequence")
        .def(py::init<>())
        .def("__repr__", [](const CallbackKeyframeSequence& seq) {
            return fmt::format("<CallbackKeyframeSequence: {} keyframes>", seq.size());
        });
}

void exposeInvalidationKeyframeSequence(py::module& m) {
    py::classh<InvalidationKeyframeSequence, KeyframeSequence>(m, "InvalidationKeyframeSequence")
        .def(py::init<>())
        .def_readwrite("path", &InvalidationKeyframeSequence::path)
        .def("__repr__", [](const InvalidationKeyframeSequence& seq) {
            return fmt::format("<InvalidationKeyframeSequence: {} keyframes>", seq.size());
        });
}

void exposeValueKeyframeSequenceBase(py::module& m) {
    py::classh<ValueKeyframeSequence>(m, "ValueKeyframeSequence")
        .def_property(
            "interpolation",
            [](ValueKeyframeSequence& seq) -> Interpolation& { return seq.getInterpolation(); },
            [](ValueKeyframeSequence& seq, std::unique_ptr<Interpolation> interp) {
                seq.setInterpolation(std::move(interp));
            },
            py::return_value_policy::reference_internal);
}

template <typename Key>
void exposeKeyframeSequenceTyped(py::module& m, const std::string& name) {
    using KST = KeyframeSequenceTyped<Key>;
    py::classh<KST, KeyframeSequence, ValueKeyframeSequence>(m, name.c_str())
        .def(py::init<>())
        .def("__repr__", [name](const KST& seq) {
            return fmt::format("<{}: {} keyframes>", name, seq.size());
        });
}

void exposeInterpolation(py::module& m) {
    py::classh<Interpolation, PropertyOwner>(m, "Interpolation")
        .def_property_readonly(
            "displayName", [](const Interpolation& i) { return std::string{i.getDisplayName()}; })
        .def_property_readonly(
            "classIdentifier",
            [](const Interpolation& i) { return std::string{i.getClassIdentifier()}; })
        .def("__repr__", [](const Interpolation& i) {
            return fmt::format("<Interpolation: '{}'>", i.getDisplayName());
        });
}

void exposeTrack(py::module& m) {
    py::classh<Track>(m, "Track")
        .def_property("enabled", &Track::isEnabled, &Track::setEnabled)
        .def_property(
            "name", [](const Track& t) { return std::string{t.getName()}; },
            [](Track& t, std::string_view name) { t.setName(name); })
        .def_property("priority", &Track::getPriority, &Track::setPriority)
        .def_property_readonly("classIdentifier",
                               [](const Track& t) { return std::string{t.getClassIdentifier()}; })
        .def_property_readonly("firstTime", [](const Track& t) { return t.getFirstTime().count(); })
        .def_property_readonly("lastTime", [](const Track& t) { return t.getLastTime().count(); })
        .def(
            "getPrevTime",
            [](const Track& t, double at) -> py::object {
                auto time = t.getPrevTime(Seconds{at});
                if (time) return py::cast(time->count());
                return py::none();
            },
            py::arg("at"))
        .def(
            "getNextTime",
            [](const Track& t, double at) -> py::object {
                auto time = t.getNextTime(Seconds{at});
                if (time) return py::cast(time->count());
                return py::none();
            },
            py::arg("at"))
        .def("getAllTimes",
             [](const Track& t) {
                 auto times = t.getAllTimes();
                 std::vector<double> result;
                 result.reserve(times.size());
                 for (auto& s : times) result.push_back(s.count());
                 return result;
             })
        .def("__len__", &Track::size)
        .def_property_readonly("empty", &Track::empty)
        .def(
            "__getitem__",
            [](Track& t, size_t i) -> KeyframeSequence& {
                if (i >= t.size()) throw py::index_error();
                return t[i];
            },
            py::return_value_policy::reference_internal)
        .def(
            "addKeyframe",
            [](Track& t, double time, bool asNewSequence) {
                return t.add(Seconds{time}, asNewSequence);
            },
            py::return_value_policy::reference_internal, py::arg("time"),
            py::arg("asNewSequence") = false)
        .def(
            "removeSequence", [](Track& t, size_t i) { return t.remove(i); }, py::arg("index"))
        .def(
            "removeSequence", [](Track& t, KeyframeSequence* seq) { return t.remove(seq); },
            py::arg("sequence"))
        .def(
            "removeKeyframe", [](Track& t, Keyframe* key) { return t.remove(key); },
            py::arg("keyframe"))
        .def("__repr__", [](const Track& t) {
            return fmt::format("<Track: '{}', {} sequences, enabled={}>", t.getName(), t.size(),
                               t.isEnabled());
        });
}

void exposeBasePropertyTrack(py::module& m) {
    py::classh<BasePropertyTrack>(m, "BasePropertyTrack")
        .def_property(
            "property", [](BasePropertyTrack& t) -> Property* { return t.getProperty(); },
            [](BasePropertyTrack& t, Property* p) { t.setProperty(p); },
            py::return_value_policy::reference_internal)
        .def(
            "addKeyFrameUsingPropertyValue",
            [](BasePropertyTrack& t, double time) {
                return t.addKeyFrameUsingPropertyValue(Seconds{time});
            },
            py::return_value_policy::reference_internal, py::arg("time"))
        .def(
            "addSequenceUsingPropertyValue",
            [](BasePropertyTrack& t, double time) {
                return t.addSequenceUsingPropertyValue(Seconds{time});
            },
            py::return_value_policy::reference_internal, py::arg("time"));
}

void exposeControlTrack(py::module& m) {
    py::classh<ControlTrack, Track>(m, "ControlTrack")
        .def(py::init<>())
        .def("__repr__", [](const ControlTrack& t) {
            return fmt::format("<ControlTrack: '{}', {} sequences>", t.getName(), t.size());
        });
}

void exposeCallbackTrack(py::module& m) {
    py::classh<CallbackTrack, Track>(m, "CallbackTrack")
        .def(py::init<>())
        .def("__repr__", [](const CallbackTrack& t) {
            return fmt::format("<CallbackTrack: '{}', {} sequences>", t.getName(), t.size());
        });
}

void exposeAnimationClass(py::module& m) {
    py::classh<Animation>(m, "Animation")
        .def(py::init([](std::string_view name) { return Animation{nullptr, name}; }),
             py::arg("name") = "Animation")
        .def_property(
            "name", [](const Animation& a) -> const std::string& { return a.getName(); },
            [](Animation& a, std::string_view name) { a.setName(name); })
        .def("__len__", &Animation::size)
        .def_property_readonly("empty", &Animation::empty)
        .def(
            "__getitem__",
            [](Animation& a, size_t i) -> Track& {
                if (i >= a.size()) throw py::index_error();
                return a[i];
            },
            py::return_value_policy::reference_internal)
        .def(
            "__iter__", [](Animation& a) { return py::make_iterator(a.begin(), a.end()); },
            py::keep_alive<0, 1>())
        .def(
            "add", [](Animation& a, Property* property) { return a.add(property); },
            py::return_value_policy::reference_internal, py::arg("property"))
        .def(
            "addKeyframe",
            [](Animation& a, Property* property, double time) {
                return a.addKeyframe(property, Seconds{time});
            },
            py::return_value_policy::reference_internal, py::arg("property"), py::arg("time"))
        .def(
            "addKeyframeSequence",
            [](Animation& a, Property* property, double time) {
                return a.addKeyframeSequence(property, Seconds{time});
            },
            py::return_value_policy::reference_internal, py::arg("property"), py::arg("time"))
        .def(
            "removeTrack", [](Animation& a, size_t i) { return a.remove(i); }, py::arg("index"))
        .def(
            "removeTrack", [](Animation& a, Track* track) { return a.remove(track); },
            py::arg("track"))
        .def(
            "removeKeyframe", [](Animation& a, Keyframe* key) { return a.remove(key); },
            py::arg("keyframe"))
        .def(
            "removeKeyframeSequence",
            [](Animation& a, KeyframeSequence* seq) { return a.remove(seq); }, py::arg("sequence"))
        .def("clear", &Animation::clear)
        .def("getAllTimes",
             [](const Animation& a) {
                 auto times = a.getAllTimes();
                 std::vector<double> result;
                 result.reserve(times.size());
                 for (auto& s : times) result.push_back(s.count());
                 return result;
             })
        .def_property_readonly("firstTime",
                               [](const Animation& a) { return a.getFirstTime().count(); })
        .def_property_readonly("lastTime",
                               [](const Animation& a) { return a.getLastTime().count(); })
        .def(
            "getPrevTime",
            [](const Animation& a, double at) -> py::object {
                auto t = a.getPrevTime(Seconds{at});
                if (t) return py::cast(t->count());
                return py::none();
            },
            py::arg("at"))
        .def(
            "getNextTime",
            [](const Animation& a, double at) -> py::object {
                auto t = a.getNextTime(Seconds{at});
                if (t) return py::cast(t->count());
                return py::none();
            },
            py::arg("at"))
        .def("__repr__", [](const Animation& a) {
            return fmt::format("<Animation: '{}', {} tracks>", a.getName(), a.size());
        });
}

void exposeAnimationController(py::module& m) {
    py::classh<AnimationController, PropertyOwner>(m, "AnimationController")
        .def("play", &AnimationController::play)
        .def("pause", &AnimationController::pause)
        .def("stop", &AnimationController::stop)
        .def("render", &AnimationController::render)
        .def("tick", &AnimationController::tick)
        .def("jumpToPrevKeyframe", &AnimationController::jumpToPrevKeyframe)
        .def("jumpToNextKeyframe", &AnimationController::jumpToNextKeyframe)
        .def("jumpToPrevControlKeyframe", &AnimationController::jumpToPrevControlKeyframe)
        .def("jumpToNextControlKeyframe", &AnimationController::jumpToNextControlKeyframe)
        .def_property_readonly("state", &AnimationController::getState)
        .def_property("playbackDirection", &AnimationController::getPlaybackDirection,
                      &AnimationController::setPlaybackDirection)
        .def_property_readonly(
            "currentTime", [](const AnimationController& c) { return c.getCurrentTime().count(); })
        .def_property_readonly("deltaTime",
                               [](const AnimationController& c) { return c.deltaTime().count(); })
        .def(
            "eval",
            [](AnimationController& c, double oldTime, double newTime) {
                c.eval(Seconds{oldTime}, Seconds{newTime});
            },
            py::arg("oldTime"), py::arg("newTime"))
        .def_property_readonly(
            "animation", [](AnimationController& c) -> Animation& { return c.getAnimation(); },
            py::return_value_policy::reference_internal)
        .def("__repr__", [](const AnimationController& c) {
            return fmt::format("<AnimationController: state={}, time={}>",
                               static_cast<int>(c.getState()), c.getCurrentTime().count());
        });
}

void exposeMainAnimation(py::module& m) {
    py::classh<MainAnimation>(m, "MainAnimation")
        .def_property_readonly(
            "animation", [](MainAnimation& ma) -> Animation& { return ma.get(); },
            py::return_value_policy::reference_internal)
        .def_property_readonly(
            "controller",
            [](MainAnimation& ma) -> AnimationController& { return ma.getController(); },
            py::return_value_policy::reference_internal)
        .def("__repr__", [](const MainAnimation& ma) {
            return fmt::format("<MainAnimation: '{}'>", ma.get().getName());
        });
}

void exposeWorkspaceAnimations(py::module& m) {
    py::classh<WorkspaceAnimations>(m, "WorkspaceAnimations")
        .def("__len__", &WorkspaceAnimations::size)
        .def(
            "__getitem__",
            [](WorkspaceAnimations& wa, size_t i) -> Animation& {
                if (i >= wa.size()) throw py::index_error();
                return wa[i];
            },
            py::return_value_policy::reference_internal)
        .def(
            "__iter__",
            [](WorkspaceAnimations& wa) { return py::make_iterator(wa.begin(), wa.end()); },
            py::keep_alive<0, 1>())
        .def(
            "add",
            [](WorkspaceAnimations& wa, std::string_view name) -> Animation& {
                return wa.add(name);
            },
            py::return_value_policy::reference_internal, py::arg("name") = "Animation")
        .def(
            "erase", [](WorkspaceAnimations& wa, size_t index) { wa.erase(index); },
            py::arg("index"))
        .def("clear", &WorkspaceAnimations::clear)
        .def_property_readonly(
            "mainAnimation",
            [](WorkspaceAnimations& wa) -> MainAnimation& { return wa.getMainAnimation(); },
            py::return_value_policy::reference_internal)
        .def(
            "get",
            [](WorkspaceAnimations& wa, size_t index) -> Animation& { return wa.get(index); },
            py::return_value_policy::reference_internal, py::arg("index"))
        .def(
            "get", [](WorkspaceAnimations& wa, std::string_view name) { return wa.get(name); },
            py::arg("name"))
        .def("__repr__", [](const WorkspaceAnimations& wa) {
            return fmt::format("<WorkspaceAnimations: {} animations>", wa.size());
        });
}

}  // namespace

void exposeAnimation(pybind11::module& m) {
    exposeEnums(m);
    exposeAnimationTimeState(m);

    // Keyframes
    exposeKeyframe(m);
    exposeBaseKeyframe(m);
    exposeValueKeyframe<float>(m, "FloatKeyframe");
    exposeValueKeyframe<double>(m, "DoubleKeyframe");
    exposeValueKeyframe<int>(m, "IntKeyframe");
    exposeValueKeyframe<vec2>(m, "Vec2Keyframe");
    exposeValueKeyframe<vec3>(m, "Vec3Keyframe");
    exposeValueKeyframe<vec4>(m, "Vec4Keyframe");
    exposeValueKeyframe<dvec2>(m, "DVec2Keyframe");
    exposeValueKeyframe<dvec3>(m, "DVec3Keyframe");
    exposeValueKeyframe<dvec4>(m, "DVec4Keyframe");
    exposeValueKeyframe<ivec2>(m, "IVec2Keyframe");
    exposeValueKeyframe<ivec3>(m, "IVec3Keyframe");
    exposeValueKeyframe<ivec4>(m, "IVec4Keyframe");
    exposeValueKeyframe<mat2>(m, "Mat2Keyframe");
    exposeValueKeyframe<mat3>(m, "Mat3Keyframe");
    exposeValueKeyframe<mat4>(m, "Mat4Keyframe");
    exposeCameraKeyframe(m);
    exposeControlKeyframe(m);
    exposeButtonKeyframe(m);
    exposeCallbackKeyframe(m);
    exposeInvalidationKeyframe(m);

    // KeyframeSequences
    exposeKeyframeSequence(m);
    exposeValueKeyframeSequenceBase(m);
    exposeKeyframeSequenceTyped<ValueKeyframe<float>>(m, "FloatKeyframeSequence");
    exposeKeyframeSequenceTyped<ValueKeyframe<double>>(m, "DoubleKeyframeSequence");
    exposeKeyframeSequenceTyped<ValueKeyframe<int>>(m, "IntKeyframeSequence");
    exposeKeyframeSequenceTyped<ValueKeyframe<vec2>>(m, "Vec2KeyframeSequence");
    exposeKeyframeSequenceTyped<ValueKeyframe<vec3>>(m, "Vec3KeyframeSequence");
    exposeKeyframeSequenceTyped<ValueKeyframe<vec4>>(m, "Vec4KeyframeSequence");
    exposeKeyframeSequenceTyped<ValueKeyframe<dvec2>>(m, "DVec2KeyframeSequence");
    exposeKeyframeSequenceTyped<ValueKeyframe<dvec3>>(m, "DVec3KeyframeSequence");
    exposeKeyframeSequenceTyped<ValueKeyframe<dvec4>>(m, "DVec4KeyframeSequence");
    exposeKeyframeSequenceTyped<ValueKeyframe<ivec2>>(m, "IVec2KeyframeSequence");
    exposeKeyframeSequenceTyped<ValueKeyframe<ivec3>>(m, "IVec3KeyframeSequence");
    exposeKeyframeSequenceTyped<ValueKeyframe<ivec4>>(m, "IVec4KeyframeSequence");
    exposeKeyframeSequenceTyped<ValueKeyframe<mat2>>(m, "Mat2KeyframeSequence");
    exposeKeyframeSequenceTyped<ValueKeyframe<mat3>>(m, "Mat3KeyframeSequence");
    exposeKeyframeSequenceTyped<ValueKeyframe<mat4>>(m, "Mat4KeyframeSequence");
    exposeKeyframeSequenceTyped<CameraKeyframe>(m, "CameraKeyframeSequence");
    exposeControlKeyframeSequence(m);
    exposeButtonKeyframeSequence(m);
    exposeCallbackKeyframeSequence(m);
    exposeInvalidationKeyframeSequence(m);

    // Interpolation
    exposeInterpolation(m);

    // Tracks
    exposeTrack(m);
    exposeBasePropertyTrack(m);
    exposeControlTrack(m);
    exposeCallbackTrack(m);

    // Animation & Controller
    exposeAnimationClass(m);
    exposeAnimationController(m);
    exposeMainAnimation(m);
    exposeWorkspaceAnimations(m);

    // Module-level convenience function
    m.def(
        "getWorkspaceAnimations",
        [](InviwoApplication* app) -> WorkspaceAnimations& {
            return util::getModuleByTypeOrThrow<AnimationModule>(app).getWorkspaceAnimations();
        },
        py::return_value_policy::reference, py::arg("app"),
        "Get the WorkspaceAnimations from the AnimationModule.");
}

}  // namespace inviwo
