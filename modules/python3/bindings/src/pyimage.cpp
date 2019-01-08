/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2018 Inviwo Foundation
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
#include <inviwo/core/common/inviwoapplication.h>

#include <inviwopy/inviwopy.h>
#include <inviwopy/pynetwork.h>
#include <inviwopy/pyglmtypes.h>
#include <modules/python3/pybindutils.h>
#include <inviwopy/pyport.h>

#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>

namespace py = pybind11;

namespace inviwo {

auto getLayers = [](Image *img) {
    pybind11::list list;
    for (size_t idx = 0; idx < img->getNumberOfColorLayers(); idx++) {
        list.append(py::cast(img->getColorLayer(idx), py::return_value_policy::reference_internal,
                             py::cast(img)));
    }
    return list;
};

void exposeImage(py::module &m) {

    py::class_<Image, std::shared_ptr<Image>>(m, "Image")
        .def(py::init<size2_t, const DataFormatBase *>())
        .def("clone", [](Image &self) { return self.clone(); })
        .def_property_readonly("dimensions", &Image::getDimensions)
        .def_property_readonly("depth", [](Image &img) { return img.getDepthLayer(); },
                               py::return_value_policy::reference_internal)
        .def_property_readonly("picking", [](Image &img) { return img.getPickingLayer(); },
                               py::return_value_policy::reference_internal)
        .def_property_readonly("colorLayers", getLayers);

    py::class_<Layer, std::shared_ptr<Layer>>(m, "Layer")
        .def(py::init<size2_t, const DataFormatBase *>())
        .def("clone", [](Layer &self) { return self.clone(); })
        .def(py::init([](py::array data) { return pyutil::createLayer(data).release(); }))
        .def_property_readonly("dimensions", &Layer::getDimensions)
        .def("save",
             [](Layer &self, std::string filepath) {
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
            [&](Layer *layer) -> py::array {
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
            [](Layer *layer, py::array data) {
                auto rep = layer->getEditableRepresentation<LayerRAM>();
                pyutil::checkDataFormat<2>(rep->getDataFormat(), rep->getDimensions(), data);

                memcpy(rep->getData(), data.data(0), data.nbytes());
            });

    exposeInport<ImageInport>(m, "Image");
    exposeInport<ImageMultiInport>(m, "ImageMulti");
    exposeOutport<ImageOutport>(m, "Image")
        .def_property_readonly("dimensions", &ImageOutport::getDimensions);
}
}  // namespace inviwo
