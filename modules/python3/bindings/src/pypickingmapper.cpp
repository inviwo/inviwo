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

#include <inviwopy/pypickingmapper.h>

#include <inviwo/core/interaction/pickingmapper.h>
#include <inviwo/core/interaction/pickingstate.h>
#include <inviwo/core/interaction/events/pickingevent.h>
#include <inviwo/core/processors/processor.h>

#include <pybind11/stl.h>
#include <pybind11/functional.h>

namespace inviwo {

void exposePickingMapper(pybind11::module &m) {
    namespace py = pybind11;

    py::enum_<PickingState>(m, "PickingState")
        .value("None", PickingState::None)
        .value("Started", PickingState::Started)
        .value("Updated", PickingState::Updated)
        .value("Finished", PickingState::Finished);

    py::enum_<PickingPressState>(m, "PickingPressState")
        .value("DoubleClick", PickingPressState::DoubleClick)
        .value("Move", PickingPressState::Move)
        .value("None", PickingPressState::None)
        .value("Press", PickingPressState::Press)
        .value("Release", PickingPressState::Release);

    py::enum_<PickingPressItem>(m, "PickingPressItem")
        .value("None", PickingPressItem::None)
        .value("Primary", PickingPressItem::Primary)
        .value("Secondary", PickingPressItem::Secondary)
        .value("Tertiary", PickingPressItem::Tertiary);

    py::enum_<PickingHoverState>(m, "PickingHoverState")
        .value("Enter", PickingHoverState::Enter)
        .value("Exit", PickingHoverState::Exit)
        .value("Move", PickingHoverState::Move)
        .value("None", PickingHoverState::None);

    py::class_<PickingEvent>(m, "PickingEvent")
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
        .def_property_readonly("canvasSize", &PickingEvent::getCanvasSize)
        .def_property_readonly("state", &PickingEvent::getState)
        .def_property_readonly("pressState", &PickingEvent::getPressState)
        .def_property_readonly("pressItem", &PickingEvent::getPressItem)
        .def_property_readonly("pressItems", &PickingEvent::getPressItems)
        .def_property_readonly("hoverState", &PickingEvent::getHoverState)
        .def_property("used", &PickingEvent::hasBeenUsed,
                      [](PickingEvent *e, bool used) {
                          if (used) {
                              e->markAsUsed();
                          } else {
                              e->markAsUnused();
                          }
                      })
        .def("markkAsUsed", &PickingEvent::markAsUsed)
        .def("markkAsUnused", &PickingEvent::markAsUnused)
        .def("setToolTip", &PickingEvent::setToolTip);

    py::class_<PickingMapper>(m, "PickingMapper")
        .def(py::init([](Processor *p, size_t size, pybind11::function callback) {
            return new PickingMapper(p, size,
                                     [callback](PickingEvent *e) { callback(py::cast(e)); });
        }))
        .def("resize", &PickingMapper::resize)
        .def_property("enabled", &PickingMapper::isEnabled, &PickingMapper::setEnabled)
        .def("pickingId", &PickingMapper::getPickingId)
        .def_property_readonly("color", &PickingMapper::getColor)
        .def_property("size", &PickingMapper::getSize, &PickingMapper::resize);
}

}  // namespace inviwo
