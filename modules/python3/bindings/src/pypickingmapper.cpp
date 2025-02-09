/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2025 Inviwo Foundation
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

#include <inviwopy/pypickingmapper.h>
#include <inviwopy/pyflags.h>

#include <pybind11/functional.h>
#include <pybind11/stl.h>
#include <pybind11/stl/filesystem.h>
#include <pybind11/numpy.h>

#include <inviwo/core/interaction/pickingmapper.h>
#include <inviwo/core/interaction/pickingstate.h>
#include <inviwo/core/interaction/events/pickingevent.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/datastructures/camera.h>
#include <inviwo/core/properties/cameraproperty.h>
#include <modules/python3/opaquetypes.h>
#include <modules/python3/polymorphictypehooks.h>

namespace inviwo {

void exposePickingMapper(pybind11::module& m) {
    namespace py = pybind11;

    auto pickingState = py::enum_<PickingState>(m, "PickingState")
                            .value("NoState", PickingState::None)
                            .value("Started", PickingState::Started)
                            .value("Updated", PickingState::Updated)
                            .value("Finished", PickingState::Finished);

    exposeFlags<PickingState>(m, pickingState, "PickingStates");

    auto pickingPressState = py::enum_<PickingPressState>(m, "PickingPressState")
                                 .value("DoubleClick", PickingPressState::DoubleClick)
                                 .value("Move", PickingPressState::Move)
                                 .value("NoPress", PickingPressState::None)
                                 .value("Press", PickingPressState::Press)
                                 .value("Release", PickingPressState::Release);

    exposeFlags<PickingPressState>(m, pickingPressState, "PickingPressStates");

    auto pickingPressItem = py::enum_<PickingPressItem>(m, "PickingPressItem")
                                .value("NoItem", PickingPressItem::None)
                                .value("Primary", PickingPressItem::Primary)
                                .value("Secondary", PickingPressItem::Secondary)
                                .value("Tertiary", PickingPressItem::Tertiary);

    exposeFlags<PickingPressItem>(m, pickingPressItem, "PickingPressItems");

    auto pickingHoverState = py::enum_<PickingHoverState>(m, "PickingHoverState")
                                 .value("Enter", PickingHoverState::Enter)
                                 .value("Exit", PickingHoverState::Exit)
                                 .value("Move", PickingHoverState::Move)
                                 .value("NoHover", PickingHoverState::None);

    exposeFlags<PickingHoverState>(m, pickingHoverState, "PickingHoverStates");

    py::class_<PickingEvent, Event>(m, "PickingEvent")
        .def(py::init<const PickingAction*, InteractionEvent*, PickingState, PickingPressState,
                      PickingPressItem, PickingHoverState, PickingPressItems, size_t, size_t,
                      size_t, size_t, dvec3, dvec3>(),
             py::arg("pickingAction"), py::arg("event"), py::arg("state"), py::arg("pressState"),
             py::arg("pressItem"), py::arg("hoverState"), py::arg("pressedState"),
             py::arg("pickedGlobalId"), py::arg("currentGlobalId"), py::arg("pressedGlobalId"),
             py::arg("previousGlobalId"), py::arg("pressedNDC"), py::arg("previousNDC"))
        .def_property_readonly("pickedId", &PickingEvent::getPickedId)
        .def_property_readonly("globalPickingId", &PickingEvent::getGlobalPickingId)
        .def_property_readonly("currentGlobalPickingId", &PickingEvent::getCurrentGlobalPickingId)
        .def_property_readonly("currentLocalPickingId", &PickingEvent::getCurrentLocalPickingId)
        .def_property_readonly("position", &PickingEvent::getPosition)
        .def_property_readonly("depth", &PickingEvent::getDepth)
        .def_property_readonly("previousGlobalPickingId", &PickingEvent::getPreviousGlobalPickingId)
        .def_property_readonly("previousLocalPickingId", &PickingEvent::getPreviousLocalPickingId)
        .def_property_readonly("previousPosition", &PickingEvent::getPreviousPosition)
        .def_property_readonly("previousDepth", &PickingEvent::getPreviousDepth)
        .def_property_readonly("pressedGlobalPickingId", &PickingEvent::getPressedGlobalPickingId)
        .def_property_readonly("pressedLocalPickingId", &PickingEvent::getPressedLocalPickingId)
        .def_property_readonly("pressedPosition", &PickingEvent::getPressedPosition)
        .def_property_readonly("pressedDepth", &PickingEvent::getPressedDepth)
        .def_property_readonly("deltaPosition", &PickingEvent::getDeltaPosition)
        .def_property_readonly("deltaDepth", &PickingEvent::getDeltaDepth)
        .def_property_readonly("deltaPressedPosition", &PickingEvent::getDeltaPressedPosition)
        .def_property_readonly("deltaPressedDepth", &PickingEvent::getDeltaPressedDepth)
        .def_property_readonly("ndc", &PickingEvent::getNDC)
        .def_property_readonly("previousNDC", &PickingEvent::getPreviousNDC)
        .def_property_readonly("pressedNDC", &PickingEvent::getPressedNDC)
        .def("getWorldSpaceDeltaAtPressDepth", &PickingEvent::getWorldSpaceDeltaAtPressDepth)
        .def_property_readonly("canvasSize", &PickingEvent::getCanvasSize)
        .def_property_readonly("state", &PickingEvent::getState)
        .def_property_readonly("pressState", &PickingEvent::getPressState)
        .def_property_readonly("pressItem", &PickingEvent::getPressItem)
        .def_property_readonly("pressItems", &PickingEvent::getPressItems)
        .def_property_readonly("hoverState", &PickingEvent::getHoverState)
        .def_property_readonly("modifiers", &PickingEvent::modifiers)
        .def("getEvent", &PickingEvent::getEvent)
        .def("setToolTip", &PickingEvent::setToolTip)
        .def_property_readonly_static("chash", &PickingEvent::chash);

    py::class_<PickingMapper>(m, "PickingMapper")
        .def(py::init([](Processor* p, size_t size, pybind11::function callback) {
            return new PickingMapper(p, size, [callback](PickingEvent* e) {
                try {
                    const pybind11::gil_scoped_acquire guard{};
                    callback(py::cast(e));
                } catch (const py::error_already_set& e) {
                    log::exception(e);
                }
            });
        }))
        .def("resize", &PickingMapper::resize)
        .def_property("enabled", &PickingMapper::isEnabled, &PickingMapper::setEnabled)
        .def("pickingId", &PickingMapper::getPickingId)
        .def_property_readonly("color", &PickingMapper::getColor)
        .def_property("size", &PickingMapper::getSize, &PickingMapper::resize);
}

}  // namespace inviwo
