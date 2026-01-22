/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2026 Inviwo Foundation
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
#include <inviwopy/util/pydatasequence.h>

#include <pybind11/functional.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>

#include <inviwo/core/datastructures/image/image.h>
#include <inviwo/core/io/datawriterfactory.h>
#include <inviwo/core/ports/imageport.h>
#include <inviwo/core/util/stringconversion.h>
#include <inviwo/core/util/glmfmt.h>

#include <modules/python3/pybindutils.h>
#include <modules/python3/pyportutils.h>
#include <modules/python3/opaquetypes.h>
#include <modules/python3/polymorphictypehooks.h>

#include <fmt/format.h>
#include <fmt/std.h>

namespace inviwo {

auto getLayers = [](Image* img) {
    pybind11::list list;
    for (size_t idx = 0; idx < img->getNumberOfColorLayers(); idx++) {
        list.append(pybind11::cast(img->getColorLayer(idx),
                                   pybind11::return_value_policy::reference_internal,
                                   pybind11::cast(img)));
    }
    return list;
};

void exposeImage(pybind11::module& m) {
    namespace py = pybind11;

    py::classh<Image>(m, "Image")
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
                self.getPickingLayer() ? "yes" : "no", self.getDataFormat()->getString(), dims,
                dims.y == 0 ? "Invalid"
                            : toString(static_cast<double>(dims.x) / static_cast<double>(dims.y)));
        });

    exposeInport<ImageInport>(m, "Image");
    exposeInport<ImageMultiInport>(m, "ImageMulti");
    exposeOutport<ImageOutport>(m, "Image")
        .def_property_readonly("dimensions", &ImageOutport::getDimensions)
        .def("setDimensions", &ImageOutport::setDimensions)
        .def("setHandleResizeEvents", &ImageOutport::setHandleResizeEvents);

    util::exportDataSequenceFor<Image>(m, "Image");
}
}  // namespace inviwo
