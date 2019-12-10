/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2019 Inviwo Foundation
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

#include <inviwo/core/util/formatdispatching.h>
#include <inviwo/core/datastructures/image/layer.h>
#include <inviwo/core/datastructures/image/layerram.h>
#include <inviwo/core/datastructures/image/layerramprecision.h>

#include <inviwo/core/io/datawriterfactory.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/util/stringconversion.h>
#include <inviwo/core/common/inviwoapplication.h>

#include <inviwopy/inviwopy.h>
#include <inviwopy/pynetwork.h>
#include <inviwopy/pyglmtypes.h>
#include <modules/python3/pybindutils.h>
#include <modules/python3/pyportutils.h>

#include <warn/push>
#include <warn/ignore/shadow>
#include <pybind11/numpy.h>
#include <pybind11/stl.h>
#include <warn/pop>

#include <fmt/format.h>

PYBIND11_MAKE_OPAQUE(inviwo::SwizzleMask);

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
        .def(py::init<ImageChannel, ImageChannel, ImageChannel, ImageChannel>(), py::arg("red"),
             py::arg("green"), py::arg("blue"), py::arg("alpha"))
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

    py::class_<Image, std::shared_ptr<Image>>(m, "Image")
        .def(py::init<size2_t, const DataFormatBase*>())
        .def(py::init<std::shared_ptr<Layer>>())
        .def("clone", [](Image& self) { return self.clone(); })
        .def_property_readonly("dimensions", &Image::getDimensions)
        .def_property_readonly("depth", [](Image& img) { return img.getDepthLayer(); },
                               py::return_value_policy::reference_internal)
        .def_property_readonly("picking", [](Image& img) { return img.getPickingLayer(); },
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

    py::class_<Layer, std::shared_ptr<Layer>>(m, "Layer")
        .def(py::init<size2_t, const DataFormatBase*>())
        .def("clone", [](Layer& self) { return self.clone(); })
        .def(py::init([](py::array data) { return pyutil::createLayer(data).release(); }))
        .def_property_readonly("dimensions", &Layer::getDimensions)
        .def_property("swizzlemask", &Layer::getSwizzleMask, &Layer::setSwizzleMask)
        .def("save",
             [](Layer& self, std::string filepath) {
                 auto ext = filesystem::getFileExtension(filepath);

                 auto writer = InviwoApplication::getPtr()
                                   ->getDataWriterFactory()
                                   ->getWriterForTypeAndExtension<Layer>(ext);
                 if (!writer) {
                     throw Exception("No write for extension " + ext);
                 }
                 writer->writeData(&self, filepath);
             })
        .def_property(
            "data",
            [&](Layer* layer) -> py::array {
                auto df = layer->getDataFormat();
                auto dims = layer->getDimensions();

                std::vector<size_t> shape = {dims.x, dims.y};
                std::vector<size_t> strides = {df->getSize(), df->getSize() * dims.x};

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

                memcpy(rep->getData(), data.data(0), data.nbytes());
            })
        .def("__repr__", [](const Layer& self) {
            return fmt::format(
                "<Layer:\n  type = {}\n  format = {}\n  dimensions = {}\n  swizzlemask = {}>",
                toString(self.getLayerType()), self.getDataFormat()->getString(),
                toString(self.getDimensions()), toString(self.getSwizzleMask()));
        });

    exposeInport<ImageInport>(m, "Image");
    exposeInport<ImageMultiInport>(m, "ImageMulti");
    exposeOutport<ImageOutport>(m, "Image")
        .def_property_readonly("dimensions", &ImageOutport::getDimensions);
}
}  // namespace inviwo
