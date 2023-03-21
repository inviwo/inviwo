/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2020-2023 Inviwo Foundation
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

#include <inviwopy/pyevent.h>
#include <inviwopy/pyflags.h>

#include <inviwo/core/interaction/events/event.h>
#include <inviwo/core/interaction/events/keyboardkeys.h>
#include <inviwo/core/interaction/events/interactionevent.h>
#include <inviwo/core/interaction/events/keyboardevent.h>
#include <inviwo/core/interaction/events/mousebuttons.h>
#include <inviwo/core/interaction/events/mouseinteractionevent.h>
#include <inviwo/core/interaction/events/mouseevent.h>
#include <inviwo/core/interaction/events/wheelevent.h>
#include <inviwo/core/interaction/events/touchstate.h>
#include <inviwo/core/interaction/events/touchevent.h>
#include <inviwo/core/interaction/events/resizeevent.h>
#include <inviwo/core/interaction/events/viewevent.h>
#include <inviwo/core/ports/inport.h>
#include <inviwo/core/ports/outport.h>
#include <inviwo/core/processors/processor.h>

#include <pybind11/stl.h>
#include <pybind11/functional.h>

namespace inviwo {

void exposeEvents(pybind11::module& m) {
    namespace py = pybind11;

    auto keyModifier = py::enum_<KeyModifier>(m, "KeyModifier")
                           .value("NoModifier", KeyModifier::None)
                           .value("Control", KeyModifier::Control)
                           .value("Shift", KeyModifier::Shift)
                           .value("Alt", KeyModifier::Alt)
                           .value("Super", KeyModifier::Super)
                           .value("Menu", KeyModifier::Menu)
                           .value("Meta", KeyModifier::Meta);

    exposeFlags<KeyModifier>(m, keyModifier, "KeyModifiers");

    auto keyState = py::enum_<KeyState>(m, "KeyState")
                        .value("Press", KeyState::Press)
                        .value("Release", KeyState::Release);

    exposeFlags<KeyState>(m, keyState, "KeyStates");

    py::enum_<IvwKey>(m, "IvwKey")
        .value("Undefined", IvwKey::Undefined)
        .value("Unknown", IvwKey::Unknown)
        .value("Space", IvwKey::Space)
        .value("Exclam", IvwKey::Exclam)
        .value("QuoteDbl", IvwKey::QuoteDbl)
        .value("NumberSign", IvwKey::NumberSign)
        .value("Dollar", IvwKey::Dollar)
        .value("Percent", IvwKey::Percent)
        .value("Ampersand", IvwKey::Ampersand)
        .value("Apostrophe", IvwKey::Apostrophe)
        .value("ParenLeft", IvwKey::ParenLeft)
        .value("ParenRight", IvwKey::ParenRight)
        .value("Asterisk", IvwKey::Asterisk)
        .value("Plus", IvwKey::Plus)
        .value("Comma", IvwKey::Comma)
        .value("Minus", IvwKey::Minus)
        .value("Period", IvwKey::Period)
        .value("Slash", IvwKey::Slash)
        .value("Num0", IvwKey::Num0)
        .value("Num1", IvwKey::Num1)
        .value("Num2", IvwKey::Num2)
        .value("Num3", IvwKey::Num3)
        .value("Num4", IvwKey::Num4)
        .value("Num5", IvwKey::Num5)
        .value("Num6", IvwKey::Num6)
        .value("Num7", IvwKey::Num7)
        .value("Num8", IvwKey::Num8)
        .value("Num9", IvwKey::Num9)
        .value("Colon", IvwKey::Colon)
        .value("Semicolon", IvwKey::Semicolon)
        .value("Less", IvwKey::Less)
        .value("Equal", IvwKey::Equal)
        .value("Greater", IvwKey::Greater)
        .value("Question", IvwKey::Question)
        .value("A", IvwKey::A)
        .value("B", IvwKey::B)
        .value("C", IvwKey::C)
        .value("D", IvwKey::D)
        .value("E", IvwKey::E)
        .value("F", IvwKey::F)
        .value("G", IvwKey::G)
        .value("H", IvwKey::H)
        .value("I", IvwKey::I)
        .value("J", IvwKey::J)
        .value("K", IvwKey::K)
        .value("L", IvwKey::L)
        .value("M", IvwKey::M)
        .value("N", IvwKey::N)
        .value("O", IvwKey::O)
        .value("P", IvwKey::P)
        .value("Q", IvwKey::Q)
        .value("R", IvwKey::R)
        .value("S", IvwKey::S)
        .value("T", IvwKey::T)
        .value("U", IvwKey::U)
        .value("V", IvwKey::V)
        .value("W", IvwKey::W)
        .value("X", IvwKey::X)
        .value("Y", IvwKey::Y)
        .value("Z", IvwKey::Z)
        .value("BracketLeft", IvwKey::BracketLeft)
        .value("Backslash", IvwKey::Backslash)
        .value("BracketRight", IvwKey::BracketRight)
        .value("GraveAccent", IvwKey::GraveAccent)
        .value("AsciiCircum", IvwKey::AsciiCircum)
        .value("Underscore", IvwKey::Underscore)
        .value("BraceLeft", IvwKey::BraceLeft)
        .value("Bar", IvwKey::Bar)
        .value("BraceRight", IvwKey::BraceRight)
        .value("AsciiTilde", IvwKey::AsciiTilde)
        .value("World1", IvwKey::World1)
        .value("World2", IvwKey::World2)
        .value("Escape", IvwKey::Escape)
        .value("Enter", IvwKey::Enter)
        .value("Tab", IvwKey::Tab)
        .value("Backspace", IvwKey::Backspace)
        .value("Insert", IvwKey::Insert)
        .value("Delete", IvwKey::Delete)
        .value("Right", IvwKey::Right)
        .value("Left", IvwKey::Left)
        .value("Down", IvwKey::Down)
        .value("Up", IvwKey::Up)
        .value("PageUp", IvwKey::PageUp)
        .value("PageDown", IvwKey::PageDown)
        .value("Home", IvwKey::Home)
        .value("End", IvwKey::End)
        .value("CapsLock", IvwKey::CapsLock)
        .value("ScrollLock", IvwKey::ScrollLock)
        .value("NumLock", IvwKey::NumLock)
        .value("PrintScreen", IvwKey::PrintScreen)
        .value("Pause", IvwKey::Pause)
        .value("F1", IvwKey::F1)
        .value("F2", IvwKey::F2)
        .value("F3", IvwKey::F3)
        .value("F4", IvwKey::F4)
        .value("F5", IvwKey::F5)
        .value("F6", IvwKey::F6)
        .value("F7", IvwKey::F7)
        .value("F8", IvwKey::F8)
        .value("F9", IvwKey::F9)
        .value("F10", IvwKey::F10)
        .value("F11", IvwKey::F11)
        .value("F12", IvwKey::F12)
        .value("F13", IvwKey::F13)
        .value("F14", IvwKey::F14)
        .value("F15", IvwKey::F15)
        .value("F16", IvwKey::F16)
        .value("F17", IvwKey::F17)
        .value("F18", IvwKey::F18)
        .value("F19", IvwKey::F19)
        .value("F20", IvwKey::F20)
        .value("F21", IvwKey::F21)
        .value("F22", IvwKey::F22)
        .value("F23", IvwKey::F23)
        .value("F24", IvwKey::F24)
        .value("F25", IvwKey::F25)
        .value("KP0", IvwKey::KP0)
        .value("KP1", IvwKey::KP1)
        .value("KP2", IvwKey::KP2)
        .value("KP3", IvwKey::KP3)
        .value("KP4", IvwKey::KP4)
        .value("KP5", IvwKey::KP5)
        .value("KP6", IvwKey::KP6)
        .value("KP7", IvwKey::KP7)
        .value("KP8", IvwKey::KP8)
        .value("KP9", IvwKey::KP9)
        .value("KPDecimal", IvwKey::KPDecimal)
        .value("KPDivide", IvwKey::KPDivide)
        .value("KPMultiply", IvwKey::KPMultiply)
        .value("KPSubtract", IvwKey::KPSubtract)
        .value("KPAdd", IvwKey::KPAdd)
        .value("KPEnter", IvwKey::KPEnter)
        .value("KPEqual", IvwKey::KPEqual)
        .value("LeftShift", IvwKey::LeftShift)
        .value("LeftControl", IvwKey::LeftControl)
        .value("LeftAlt", IvwKey::LeftAlt)
        .value("LeftSuper", IvwKey::LeftSuper)
        .value("RightShift", IvwKey::RightShift)
        .value("RightControl", IvwKey::RightControl)
        .value("RightAlt", IvwKey::RightAlt)
        .value("RightSuper", IvwKey::RightSuper)
        .value("Menu", IvwKey::Menu)
        .value("LeftMeta", IvwKey::LeftMeta)
        .value("RightMeta", IvwKey::RightMeta);

    auto mouseButton = py::enum_<MouseButton>(m, "MouseButton")
                           .value("NoButton", MouseButton::None)
                           .value("Left", MouseButton::Left)
                           .value("Middle", MouseButton::Middle)
                           .value("Right", MouseButton::Right);

    exposeFlags<MouseButton>(m, mouseButton, "MouseButtons");

    auto mouseState = py::enum_<MouseState>(m, "MouseState")
                          .value("Press", MouseState::Press)
                          .value("Move", MouseState::Move)
                          .value("Release", MouseState::Release)
                          .value("DoubleClick", MouseState::DoubleClick);

    exposeFlags<MouseState>(m, mouseState, "MouseStates");

    auto touchState = py::enum_<TouchState>(m, "TouchState")
                          .value("NoTouch", TouchState::None)
                          .value("Started", TouchState::Started)
                          .value("Updated", TouchState::Updated)
                          .value("Stationary", TouchState::Stationary)
                          .value("Finished", TouchState::Finished);

    exposeFlags<TouchState>(m, touchState, "TouchStates");

    py::class_<Event>(m, "Event")
        .def("clone", &Event::clone)
        .def("hash", &Event::hash)
        .def("shouldPropagateTo", &Event::shouldPropagateTo)
        .def("markAsUsed", &Event::markAsUsed)
        .def("hasBeenUsed", &Event::hasBeenUsed)
        .def("markAsUnused", &Event::markAsUnused)
        .def("setUsed", &Event::setUsed)
        .def_property("used", &Event::hasBeenUsed,
                      [](Event* e, bool used) {
                          if (used) {
                              e->markAsUsed();
                          } else {
                              e->markAsUnused();
                          }
                      })
        .def("markAsVisited", static_cast<bool (Event::*)(Processor*)>(&Event::markAsVisited))
        .def("markAsVisited", static_cast<void (Event::*)(Event&)>(&Event::markAsVisited))
        .def("hasVisitedProcessor", &Event::hasVisitedProcessor)
        .def("getVisitedProcessors", &Event::getVisitedProcessors)
        .def("__repr__", [](Event* event) {
            std::ostringstream oss;
            event->print(oss);
            return oss.str();
        });

    py::class_<InteractionEvent, Event>(m, "InteractionEvent")
        .def("modifiers", &InteractionEvent::modifiers)
        .def("setModifiers", &InteractionEvent::setModifiers)
        .def("modifierNames", &InteractionEvent::modifierNames)
        .def("setToolTip", &InteractionEvent::setToolTip);

    py::class_<KeyboardEvent, InteractionEvent>(m, "KeyboardEvent")
        .def(py::init<IvwKey, KeyState, KeyModifiers, uint32_t, const std::string&>(),
             py::arg("key") = IvwKey::Unknown, py::arg("state") = KeyState::Press,
             py::arg("modifiers") = KeyModifier::None, py::arg("nativeVirtualKey") = 0,
             py::arg("utfText") = "")
        .def(py::init<KeyboardEvent>())
        .def_property("state", &KeyboardEvent::state, &KeyboardEvent::setState)
        .def_property("key", &KeyboardEvent::key, &KeyboardEvent::setKey)
        .def_property("nativeVirtualKey", &KeyboardEvent::getNativeVirtualKey,
                      &KeyboardEvent::setNativeVirtualKey)
        .def_property("text", &KeyboardEvent::text, &KeyboardEvent::setText)
        .def_property_readonly_static("chash", &KeyboardEvent::chash);

    py::class_<MouseInteractionEvent, InteractionEvent>(m, "MouseInteractionEvent")
        .def_property("buttonState", &MouseInteractionEvent::buttonState,
                      &MouseInteractionEvent::setButtonState)
        .def_property("pos", &MouseInteractionEvent::pos, &MouseInteractionEvent::setPos)
        .def_property("canvasSize", &MouseInteractionEvent::canvasSize,
                      &MouseInteractionEvent::setCanvasSize)
        .def_property("depth", &MouseInteractionEvent::depth, &MouseInteractionEvent::setDepth)
        .def_property("posNormalized", &MouseInteractionEvent::posNormalized,
                      &MouseInteractionEvent::setPosNormalized)
        .def_property_readonly("ndc", &MouseInteractionEvent::ndc)
        .def_property_readonly("x", &MouseInteractionEvent::x)
        .def_property_readonly("y", &MouseInteractionEvent::y)
        .def_property_readonly("buttonName", &MouseInteractionEvent::buttonName);

    py::class_<MouseEvent, MouseInteractionEvent>(m, "MouseEvent")
        .def(py::init<MouseButton, MouseState, MouseButtons, KeyModifiers, dvec2, uvec2, double>(),
             py::arg("button") = MouseButton::Left, py::arg("state") = MouseState::Press,
             py::arg("buttonState") = MouseButton::None, py::arg("modifiers") = KeyModifier::None,
             py::arg("normalizedPosition") = dvec2(0), py::arg("canvasSize") = uvec2(0),
             py::arg("depth") = 1.0)
        .def_property("button", &MouseEvent::button, &MouseEvent::setButton)
        .def_property("state", &MouseEvent::state, &MouseEvent::setState)
        .def_property_readonly_static("chash", &MouseEvent::chash);

    py::class_<WheelEvent, MouseInteractionEvent>(m, "WheelEvent")
        .def(py::init<MouseButtons, KeyModifiers, dvec2, dvec2, uvec2, double>(),
             py::arg("buttonState") = MouseButton::None, py::arg("modifiers") = KeyModifier::None,
             py::arg("delta") = dvec2(0), py::arg("normalizedPosition") = dvec2(0),
             py::arg("canvasSize") = uvec2(0), py::arg("depth") = 1.0)
        .def_property("delta", &WheelEvent::delta, &WheelEvent::setDelta)
        .def_property_readonly_static("chash", &WheelEvent::chash);

    py::class_<TouchPoint>(m, "TouchPoint")
        .def(py::init<>())
        .def(py::init<int, TouchState, dvec2, dvec2, dvec2, uvec2, double, double>(), py::arg("id"),
             py::arg("touchState"), py::arg("posNormalized"), py::arg("prevPosNormalized"),
             py::arg("pressedPosNormalized"), py::arg("canvasSize"), py::arg("pressure"),
             py::arg("depth ") = 1.0)
        .def_property_readonly("state", &TouchPoint::state)
        .def_property("id", &TouchPoint::id, &TouchPoint::setId)
        .def_property("pos", &TouchPoint::pos, &TouchPoint::setPos)
        .def_property("posNormalized", &TouchPoint::posNormalized, &TouchPoint::setPosNormalized)
        .def_property("prevPos", &TouchPoint::prevPos, &TouchPoint::setPrevPos)
        .def_property("prevPosNormalized", &TouchPoint::prevPosNormalized,
                      &TouchPoint::setPrevPosNormalized)
        .def_property("pressedPos", &TouchPoint::pressedPos, &TouchPoint::setPressedPos)
        .def_property("pressedPosNormalized", &TouchPoint::pressedPosNormalized,
                      &TouchPoint::setPressedPosNormalized)
        .def_property("depth", &TouchPoint::depth, &TouchPoint::setDepth)
        .def_property("pressure", &TouchPoint::pressure, &TouchPoint::setPressure)
        .def_property("canvasSize", &TouchPoint::canvasSize, &TouchPoint::setCanvasSize)
        .def_property_readonly("ndc", &TouchPoint::ndc);

    py::enum_<TouchDevice::DeviceType>(m, "TouchDeviceType")
        .value("Screen", TouchDevice::DeviceType::TouchScreen)
        .value("Pad", TouchDevice::DeviceType::TouchPad);

    py::class_<TouchDevice>(m, "TouchDevice")
        .def(py::init<TouchDevice::DeviceType, std::string>(),
             py::arg("type") = TouchDevice::DeviceType::TouchScreen, py::arg("name") = "")
        .def_property("type", &TouchDevice::getType, &TouchDevice::setType)
        .def_property("name", &TouchDevice::getName, &TouchDevice::setName);

    py::class_<TouchEvent, InteractionEvent>(m, "TouchEvent")
        .def(py::init<const std::vector<TouchPoint>&, const TouchDevice*, KeyModifiers>(),
             py::arg("touchPoints"), py::arg("source"), py::arg("modifiers"))
        .def("hasTouchPoints", &TouchEvent::hasTouchPoints)
        .def_property("touchPoints",
                      static_cast<const std::vector<TouchPoint>& (TouchEvent::*)() const>(
                          &TouchEvent::touchPoints),
                      &TouchEvent::setTouchPoints)
        .def_property_readonly("canvasSize", &TouchEvent::canvasSize)
        .def_property_readonly("centerPoint", &TouchEvent::centerPoint)
        .def_property_readonly("centerPointNormalized", &TouchEvent::centerPointNormalized)
        .def_property_readonly("prevCenterPointNormalized", &TouchEvent::prevCenterPointNormalized)
        .def_property_readonly("centerNDC", &TouchEvent::centerNDC)
        .def_property_readonly("averageDepth", &TouchEvent::averageDepth)
        .def_property_readonly("getDevice", &TouchEvent::getDevice)

        .def("findClosestTwoTouchPoints", &TouchEvent::findClosestTwoTouchPoints)
        .def_property_readonly_static("chash", &TouchEvent::chash)
        .def_static("getPickingState", &TouchEvent::getPickingState);

    py::class_<ResizeEvent, Event>(m, "ResizeEvent")
        .def(py::init<size2_t>(), py::arg("newSize"))
        .def(py::init<size2_t, size2_t>(), py::arg("newSize"), py::arg("previousSize"))
        .def(py::init<ResizeEvent>())
        .def_property("size", &ResizeEvent::size, &ResizeEvent::setSize)
        .def_property("previousSize", &ResizeEvent::previousSize, &ResizeEvent::setPreviousSize)
        .def_property_readonly_static("chash", &ResizeEvent::chash);

    py::class_<typename ViewEvent::FlipUp>(m, "ViewEventFlipUp").def(py::init<>());
    py::class_<typename ViewEvent::FitData>(m, "ViewEventFitData").def(py::init<>());

    py::class_<ViewEvent, Event>(m, "ViewEvent")
        .def(py::init<typename ViewEvent::Action>())
        .def_property_readonly("action", &ViewEvent::getAction)
        .def_property_readonly_static("chash", &ViewEvent::chash);
}

}  // namespace inviwo
