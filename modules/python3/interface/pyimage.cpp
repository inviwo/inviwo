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
        .def(py::init<size2_t,const DataFormatBase *>())
        .def_property_readonly("dimensions", &Image::getDimensions)
        .def_property_readonly("depth", [](Image &img) { return img.getDepthLayer(); }, py::return_value_policy::reference)
        .def_property_readonly("picking", [](Image &img) { return img.getPickingLayer(); }, py::return_value_policy::reference)
        .def_property_readonly("colorLayers", getLayers, py::return_value_policy::reference);

    py::class_<Layer>(m, "Layer")
        .def(py::init<size2_t,const DataFormatBase *>())
        .def_property_readonly("dimensions", &Layer::getDimensions)
        .def_property_readonly("data", [&](Layer &layer) -> py::array {

            auto func = [&](auto pLayer) -> py::array {
                using ValueType = util::PrecsionValueType<decltype(pLayer)>;
                using ComponentType = typename util::value_type<ValueType>::type;

                ComponentType *data = (ComponentType *)pLayer->getDataTyped();
                std::vector<size_t> shape = {pLayer->getDimensions().x, pLayer->getDimensions().y,
                                             pLayer->getDataFormat()->getComponents()};

                std::vector<size_t> strides = {
                    sizeof(ComponentType) * pLayer->getDataFormat()->getComponents(),
                    sizeof(ComponentType) * pLayer->getDataFormat()->getComponents() *
                        pLayer->getDimensions().x,
                    sizeof(ComponentType)};

                bool readOnly = false;
                if (readOnly) {
                    return py::array_t<ComponentType>(shape, strides, (ComponentType *)data);
                } else {
                    return py::array_t<ComponentType>(shape, strides, (ComponentType *)data,
                                                      py::cast<>(1));
                }

            };

            return layer.getRepresentation<LayerRAM>()->dispatch<py::array>(func);
        });

    exposeOutport<ImageOutport>(m, "Image")
        .def_property_readonly("dimensions", &ImageOutport::getDimensions);
}
}  // namespace
