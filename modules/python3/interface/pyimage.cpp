/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017 Inviwo Foundation
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

#include <modules/python3/interface/pyimage.h>

#include <inviwo/core/util/formatdispatching.h>
#include <inviwo/core/datastructures/image/layer.h>
#include <inviwo/core/datastructures/image/layerram.h>
#include <inviwo/core/datastructures/image/layerramprecision.h>

#include <modules/python3/interface/inviwopy.h>
#include <modules/python3/interface/pynetwork.h>
#include <modules/python3/interface/pyglmtypes.h>
#include <modules/python3/pybindutils.h>

#include <pybind11/pybind11.h>
#include <pybind11/common.h>
#include <pybind11/numpy.h>

namespace py = pybind11;

namespace inviwo {

auto getLayers = [](Image *img) {
    std::vector<Layer *> layers;
    for (size_t idx = 0; idx < img->getNumberOfColorLayers(); idx++) {
        layers.push_back(img->getColorLayer(idx));
    }
    return layers;
};

void exposeImage(py::module &m) {

    py::class_<Image>(m, "Image")
        .def(py::init<size2_t, const DataFormatBase *>())
        .def_property_readonly("dimensions", &Image::getDimensions)
        .def_property_readonly("depth", [](Image &img) { return img.getDepthLayer(); },
                               py::return_value_policy::reference)
        .def_property_readonly("picking", [](Image &img) { return img.getPickingLayer(); },
                               py::return_value_policy::reference)
        .def_property_readonly("colorLayers", getLayers, py::return_value_policy::reference);

    py::class_<Layer>(m, "Layer")
        .def(py::init<size2_t, const DataFormatBase *>())
        .def_property_readonly("dimensions", &Layer::getDimensions)
        .def_property_readonly("data", [&](Layer *layer) -> py::array {

            auto df = layer->getDataFormat();
            auto dims = layer->getDimensions();


            std::vector<size_t> shape = {dims.x, dims.y};
            std::vector<size_t> strides = {df->getSize(), 
                                           df->getSize() * dims.x};

            if(df->getComponents()>1){
                shape.push_back(df->getComponents());
                strides.push_back(df->getSize() / df->getComponents());
            }

            auto data = layer->getRepresentation<LayerRAM>()->getData();

            bool readOnly = false;
            if (readOnly) {
                return py::array(pyutil::toNumPyFormat(df), shape, strides, data);
            } else {
                return py::array(pyutil::toNumPyFormat(df), shape, strides, data, py::cast<>(1));
            }
        });

    exposeOutport<ImageOutport>(m, "Image")
        .def_property_readonly("dimensions", &ImageOutport::getDimensions);
}
}  // namespace
