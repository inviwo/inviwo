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

#include <inviwopy/pylayer.h>
#include <inviwopy/pyglmtypes.h>

#include <pybind11/functional.h>
#include <pybind11/stl.h>
#include <pybind11/stl/filesystem.h>
#include <pybind11/numpy.h>

#include <inviwo/core/datastructures/image/layer.h>
#include <inviwo/core/datastructures/image/layerram.h>
#include <inviwo/core/datastructures/image/layerramprecision.h>
#include <inviwo/core/ports/layerport.h>
#include <inviwo/core/io/datawriterfactory.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/util/stringconversion.h>
#include <inviwo/core/util/exception.h>
#include <inviwo/core/common/inviwoapplication.h>

#include <modules/python3/pybindutils.h>
#include <modules/python3/pyportutils.h>
#include <modules/python3/layerpy.h>
#include <modules/python3/opaquetypes.h>
#include <modules/python3/polymorphictypehooks.h>

#include <fmt/format.h>
#include <fmt/std.h>
#include <iostream>

namespace inviwo {

void exposeLayer(pybind11::module& m) {
    namespace py = pybind11;

    py::classh<Layer, StructuredGridEntity<2>>(m, "Layer", py::multiple_inheritance{})
        .def(py::init<std::shared_ptr<LayerRepresentation>>())
        .def(py::init<size2_t, const DataFormatBase*>())
        .def(py::init<size2_t, const DataFormatBase*, LayerType, const SwizzleMask&,
                      InterpolationType, const Wrapping2D&>())
        .def(py::init([](py::array data) { return pyutil::createLayer(data).release(); }))
        .def("clone", [](Layer& self) { return self.clone(); })
        .def("setDimensions", &Layer::setDimensions)
        .def_property_readonly("layertype", &Layer::getLayerType)
        .def_property("swizzlemask", &Layer::getSwizzleMask, &Layer::setSwizzleMask)
        .def_property("interpolation", &Layer::getInterpolation, &Layer::setInterpolation)
        .def_property("wrapping", &Layer::getWrapping, &Layer::setWrapping)
        .def_readwrite("dataMap", &Layer::dataMap)
        .def_readwrite("axes", &Layer::axes)
        .def_static("modelMatrixFromDimensions",
                    &LayerConfig::aspectPreservingModelMatrixFromDimensions)
        .def(
            "getLayerPyRepresentation",
            [](Layer& self) { return self.getRepresentation<LayerPy>(); },
            pybind11::return_value_policy::reference_internal)
        .def(
            "getEditableLayerPyRepresentation",
            [](Layer& self) { return self.getEditableRepresentation<LayerPy>(); },
            pybind11::return_value_policy::reference_internal)
        .def("save",
             [](Layer& self, const std::filesystem::path& filepath) {
                 auto writer = InviwoApplication::getPtr()
                                   ->getDataWriterFactory()
                                   ->getWriterForTypeAndExtension<Layer>(filepath);
                 if (!writer) {
                     throw Exception(SourceContext{}, "No writer for {}", filepath);
                 }
                 writer->writeData(&self, filepath);
             })
        .def_property(
            "data",
            [&](Layer* layer) -> py::array {
                const auto* df = layer->getDataFormat();
                auto dims = layer->getDimensions();

                std::vector<size_t> shape = {dims.y, dims.x};
                std::vector<size_t> strides = {df->getSizeInBytes() * dims.x, df->getSizeInBytes()};

                if (df->getComponents() > 1) {
                    shape.push_back(df->getComponents());
                    strides.push_back(df->getSizeInBytes() / df->getComponents());
                }

                auto* data = layer->getEditableRepresentation<LayerRAM>()->getData();
                return {pyutil::toNumPyFormat(df), shape, strides, data, py::cast<>(1)};
            },
            [](Layer* layer, const py::array& data) {
                auto* rep = layer->getEditableRepresentation<LayerRAM>();
                pyutil::checkDataFormat<2>(rep->getDataFormat(), rep->getDimensions(), data);

                if (pybind11::array::c_style == (data.flags() & pybind11::array::c_style)) {
                    memcpy(rep->getData(), data.data(0), data.nbytes());
                } else {
                    throw Exception("Unable to convert from array to LayerRAM");
                }
            })
        .def("__repr__", [](const Layer& self) {
            return fmt::format(
                "<Layer:\n  type = {}\n  format = {}\n  dimensions = {}\n  swizzlemask = {}\n  "
                "interpolation = {}\n  wrapping = {}\n>",
                self.getLayerType(), self.getDataFormat()->getString(), self.getDimensions(),
                self.getSwizzleMask(), self.getInterpolation(), self.getWrapping());
        });

    py::classh<LayerRepresentation>(m, "LayerRepresentation")
        .def_property_readonly("layertype", &LayerRepresentation::getLayerType)
        .def_property("dimensions", &LayerRepresentation::getDimensions,
                      &LayerRepresentation::setDimensions)
        .def_property("swizzleMask", &LayerRepresentation::getSwizzleMask,
                      &LayerRepresentation::setSwizzleMask)
        .def_property("interpolation", &LayerRepresentation::getInterpolation,
                      &LayerRepresentation::setInterpolation)
        .def_property("wrapping", &LayerRepresentation::getWrapping,
                      &LayerRepresentation::setWrapping)
        .def("isValid", &LayerRepresentation::isValid)
        .def("setValid", &LayerRepresentation::setValid)
        .def("getOwner", &LayerRepresentation::getOwner)
        .def_property_readonly("format", &LayerRepresentation::getDataFormat)
        .def("__repr__", [](const LayerRepresentation& self) {
            return fmt::format(
                "<LayerRepresentation:\n  type = {}\n  format = {}\n  dimensions = {}\n  "
                "swizzlemask = {}\n  interpolation = {}\n  wrapping = {}\n>",
                self.getLayerType(), self.getDataFormat()->getString(), self.getDimensions(),
                self.getSwizzleMask(), self.getInterpolation(), self.getWrapping());
        });

    py::classh<LayerPy, LayerRepresentation>(m, "LayerPy")
        .def(py::init<py::array, LayerType, const SwizzleMask&, InterpolationType,
                      const Wrapping2D&>(),
             py::arg("data"), py::arg("layerType") = LayerType::Color,
             py::arg("swizzleMask") = swizzlemasks::rgba,
             py::arg("interpolation") = InterpolationType::Linear,
             py::arg("wrapping") = wrapping2d::clampAll)
        .def(py::init<size2_t, LayerType, const DataFormatBase*, const SwizzleMask&,
                      InterpolationType, const Wrapping2D&>(),
             py::arg("size"), py::arg("layerType"), py::arg("format"),
             py::arg("swizzleMask") = swizzlemasks::rgba,
             py::arg("interpolation") = InterpolationType::Linear,
             py::arg("wrapping") = wrapping2d::clampAll)
        .def_property_readonly("data", static_cast<py::array& (LayerPy::*)()>(&LayerPy::data))
        .def("__repr__", [](const LayerPy& self) {
            return fmt::format(
                "<LayerPy:\n  type = {}\n  format = {}\n  dimensions = {}\n  swizzlemask = {}\n  "
                "interpolation = {}\n  wrapping = {}\n>",
                self.getLayerType(), self.getDataFormat()->getString(), self.getDimensions(),
                self.getSwizzleMask(), self.getInterpolation(), self.getWrapping());
        });

    exposeStandardDataPorts<Layer>(m, "Layer");
}

}  // namespace inviwo
