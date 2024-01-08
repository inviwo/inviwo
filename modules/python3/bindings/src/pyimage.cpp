/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2024 Inviwo Foundation
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

#include <inviwopy/pyimage.h>
#include <inviwopy/pyglmtypes.h>

#include <inviwo/core/util/formatdispatching.h>
#include <inviwo/core/datastructures/image/layer.h>
#include <inviwo/core/datastructures/image/image.h>
#include <inviwo/core/datastructures/image/layerram.h>
#include <inviwo/core/datastructures/image/layerramprecision.h>
#include <inviwo/core/io/datawriterfactory.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/util/stringconversion.h>
#include <inviwo/core/util/exception.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/ports/imageport.h>
#include <modules/python3/pybindutils.h>
#include <modules/python3/pyportutils.h>
#include <modules/python3/layerpy.h>

#include <warn/push>
#include <warn/ignore/shadow>
#include <pybind11/numpy.h>
#include <pybind11/stl.h>
#include <pybind11/stl/filesystem.h>
#include <warn/pop>

#include <fmt/format.h>
#include <fmt/std.h>

namespace py = pybind11;

namespace inviwo {

auto getLayers = [](Image* img) {
    pybind11::list list;
    for (size_t idx = 0; idx < img->getNumberOfColorLayers(); idx++) {
        list.append(py::cast(img->getColorLayer(idx), py::return_value_policy::reference_internal,
                             py::cast(img)));
    }
    return list;
};

void exposeImage(py::module& m) {

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

    py::class_<SwizzleMask>(m, "SwizzleMask")
        .def(py::init<>([]() { return swizzlemasks::rgba; }))
        .def(py::init<SwizzleMask>())
        .def(py::init<>([](ImageChannel r, ImageChannel g, ImageChannel b, ImageChannel a) {
                 return SwizzleMask{r, g, b, a};
             }),
             py::arg("red"), py::arg("green"), py::arg("blue"), py::arg("alpha"))
        // clang-format off
        .def_property_readonly_static("rgb", [](py::object) { return swizzlemasks::rgb; })
        .def_property_readonly_static("rgba", [](py::object) { return swizzlemasks::rgba; })
        .def_property_readonly_static("rgbZeroAlpha", [](py::object) { return swizzlemasks::rgbZeroAlpha; })
        .def_property_readonly_static("luminance", [](py::object) { return swizzlemasks::luminance; })
        .def_property_readonly_static("luminanceAlpha", [](py::object) { return swizzlemasks::luminanceAlpha; })
        .def_property_readonly_static("redGreen", [](py::object) { return swizzlemasks::redGreen; })
        .def_property_readonly_static("depth", [](py::object) { return swizzlemasks::depth; })
        // clang-format on
        .def("__repr__", [](const SwizzleMask& self) { return toString(self); });

    py::enum_<InterpolationType>(m, "InterpolationType")
        .value("Linear", InterpolationType::Linear)
        .value("Nearest", InterpolationType::Nearest);

    py::enum_<Wrapping>(m, "Wrapping")
        .value("Clamp", Wrapping::Clamp)
        .value("Repeat", Wrapping::Repeat)
        .value("Mirror", Wrapping::Mirror);

    py::class_<Image>(m, "Image")
        .def(py::init<size2_t, const DataFormatBase*>())
        .def(py::init<std::shared_ptr<Layer>>())
        .def(py::init<std::vector<std::shared_ptr<Layer>>>())
        .def("setDimensions", &Image::setDimensions)
        .def("addColorLayer", &Image::addColorLayer)
        .def("clone", [](Image& self) { return self.clone(); })
        .def_property_readonly("dimensions", &Image::getDimensions)
        .def_property_readonly(
            "depth", [](Image& img) { return img.getDepthLayer(); },
            py::return_value_policy::reference_internal)
        .def_property_readonly(
            "picking", [](Image& img) { return img.getPickingLayer(); },
            py::return_value_policy::reference_internal)
        .def_property_readonly("colorLayers", getLayers)
        .def("__repr__", [](const Image& self) {
            const auto dims = self.getDimensions();
            return fmt::format(
                "<Image:\n  color channels = {}\n  depth = {}\n  picking = {}\n"
                "  format = {}\n  dimensions = {}\n  aspect ratio = {}>",
                self.getNumberOfColorLayers(), self.getDepthLayer() ? "yes" : "no",
                self.getPickingLayer() ? "yes" : "no", self.getDataFormat()->getString(),
                toString(dims),
                dims.y == 0 ? "Invalid"
                            : toString(static_cast<double>(dims.x) / static_cast<double>(dims.y)));
        });

    py::class_<Layer>(m, "Layer")
        .def(py::init<std::shared_ptr<LayerRepresentation>>())
        .def(py::init<size2_t, const DataFormatBase*>())
        .def(py::init<size2_t, const DataFormatBase*, LayerType, const SwizzleMask&,
                      InterpolationType, const Wrapping2D&>())
        .def("clone", [](Layer& self) { return self.clone(); })
        .def(py::init([](py::array data) { return pyutil::createLayer(data).release(); }))
        .def("setDimensions", &Layer::setDimensions)
        .def_property_readonly("layertype", &Layer::getLayerType)
        .def_property_readonly("dimensions", &Layer::getDimensions)
        .def_property("swizzlemask", &Layer::getSwizzleMask, &Layer::setSwizzleMask)
        .def_property("interpolation", &Layer::getInterpolation, &Layer::setInterpolation)
        .def_property("wrapping", &Layer::getWrapping, &Layer::setWrapping)
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
                     throw Exception(IVW_CONTEXT_CUSTOM("exposeImage"), "No writer for {}",
                                     filepath);
                 }
                 writer->writeData(&self, filepath);
             })
        .def_property(
            "data",
            [&](Layer* layer) -> py::array {
                auto df = layer->getDataFormat();
                auto dims = layer->getDimensions();

                std::vector<size_t> shape = {dims.y, dims.x};
                std::vector<size_t> strides = {df->getSize() * dims.x, df->getSize()};

                if (df->getComponents() > 1) {
                    shape.push_back(df->getComponents());
                    strides.push_back(df->getSize() / df->getComponents());
                }

                auto data = layer->getEditableRepresentation<LayerRAM>()->getData();
                return py::array(pyutil::toNumPyFormat(df), shape, strides, data, py::cast<>(1));
            },
            [](Layer* layer, py::array data) {
                auto rep = layer->getEditableRepresentation<LayerRAM>();
                pyutil::checkDataFormat<2>(rep->getDataFormat(), rep->getDimensions(), data);

                if (pybind11::array::c_style == (data.flags() & pybind11::array::c_style)) {
                    memcpy(rep->getData(), data.data(0), data.nbytes());
                } else {
                    throw Exception("Unable to convert from array to LayerRAM",
                                    IVW_CONTEXT_CUSTOM("PyImage"));
                }
            })
        .def("__repr__", [](const Layer& self) {
            return fmt::format(
                "<Layer:\n  type = {}\n  format = {}\n  dimensions = {}\n  swizzlemask = {}\n  "
                "interpolation = {}\n  wrapping = {}\n>",
                self.getLayerType(), self.getDataFormat()->getString(), self.getDimensions(),
                self.getSwizzleMask(), self.getInterpolation(), self.getWrapping());
        });

    py::class_<LayerRepresentation>(m, "LayerRepresentation")
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

    py::class_<LayerPy, LayerRepresentation>(m, "LayerPy")
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

    exposeInport<ImageInport>(m, "Image");
    exposeInport<ImageMultiInport>(m, "ImageMulti");
    exposeOutport<ImageOutport>(m, "Image")
        .def_property_readonly("dimensions", &ImageOutport::getDimensions)
        .def("setDimensions", &ImageOutport::setDimensions)
        .def("setHandleResizeEvents", &ImageOutport::setHandleResizeEvents);
}
}  // namespace inviwo
