/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2025 Inviwo Foundation
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

#include <inviwopy/pyimagetypes.h>
#include <inviwopy/pyglmtypes.h>

#include <inviwo/core/datastructures/image/imagetypes.h>

#include <modules/python3/opaquetypes.h>
#include <modules/python3/polymorphictypehooks.h>

namespace inviwo {

void exposeImageTypes(pybind11::module& m) {
    namespace py = pybind11;

    py::enum_<LayerType>(m, "LayerType")
        .value("Color", LayerType::Color)
        .value("Depth", LayerType::Depth)
        .value("Picking", LayerType::Picking);

    py::enum_<ImageChannel>(m, "ImageChannel")
        .value("Red", ImageChannel::Red)
        .value("Green", ImageChannel::Green)
        .value("Blue", ImageChannel::Blue)
        .value("Alpha", ImageChannel::Alpha)
        .value("Zero", ImageChannel::Zero)
        .value("One", ImageChannel::One);

    py::classh<SwizzleMask>(m, "SwizzleMask")
        .def(py::init<>([]() { return swizzlemasks::rgba; }))
        .def(py::init<SwizzleMask>())
        .def(py::init<>([](ImageChannel r, ImageChannel g, ImageChannel b, ImageChannel a) {
                 return SwizzleMask{r, g, b, a};
             }),
             py::arg("red"), py::arg("green"), py::arg("blue"), py::arg("alpha"))
        // clang-format off
        .def_property_readonly_static("rgb", [](const py::object&) { return swizzlemasks::rgb; })
        .def_property_readonly_static("rgba", [](const py::object&) { return swizzlemasks::rgba; })
        .def_property_readonly_static("rgbZeroAlpha", [](const py::object&) { return swizzlemasks::rgbZeroAlpha; })
        .def_property_readonly_static("luminance", [](const py::object&) { return swizzlemasks::luminance; })
        .def_property_readonly_static("luminanceAlpha", [](const py::object&) { return swizzlemasks::luminanceAlpha; })
        .def_property_readonly_static("redGreen", [](const py::object&) { return swizzlemasks::redGreen; })
        .def_property_readonly_static("depth", [](const py::object&) { return swizzlemasks::depth; })
        // clang-format on
        .def_static("defaultColor",
                    [](int components) { return swizzlemasks::defaultColor(components); })
        .def_static("defaultData",
                    [](int components) { return swizzlemasks::defaultData(components); })
        .def("__repr__", [](const SwizzleMask& self) { return toString(self); });

    py::enum_<InterpolationType>(m, "InterpolationType")
        .value("Linear", InterpolationType::Linear)
        .value("Nearest", InterpolationType::Nearest);

    py::enum_<Wrapping>(m, "Wrapping")
        .value("Clamp", Wrapping::Clamp)
        .value("Repeat", Wrapping::Repeat)
        .value("Mirror", Wrapping::Mirror);
}

}  // namespace inviwo
