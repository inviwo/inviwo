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

#include <modules/python3/interface/pyvolume.h>

#include <inviwo/core/util/formatdispatching.h>

#include <modules/python3/interface/inviwopy.h>
#include <modules/python3/interface/pynetwork.h>
#include <modules/python3/interface/pyglmtypes.h>

#include <pybind11/pybind11.h>
#include <pybind11/common.h>
#include <pybind11/numpy.h>

#include <inviwo/core/datastructures/volume/volume.h>
#include <inviwo/core/datastructures/volume/volumeram.h>
#include <inviwo/core/datastructures/volume/volumeramprecision.h>
#include <inviwo/core/ports/volumeport.h>

namespace py = pybind11;

namespace inviwo {

void exposeVolume(py::module &m) {
    /*
    py::class_<Image>(m, "Image")
        .def_property_readonly("dimensions", &Image::getDimensions)
        .def_property_readonly("depth", [](Image &img) { return img.getDepthLayer(); },
    py::return_value_policy::reference)
        .def_property_readonly("picking", [](Image &img) { return img.getPickingLayer(); },
    py::return_value_policy::reference)
        .def_property_readonly("colorLayers", getLayers, py::return_value_policy::reference);
    */

    py::class_<Volume>(m, "Volume")
        .def(py::init<size3_t, const DataFormatBase *>())
        .def_property_readonly("dimensions", &Volume::getDimensions)
        .def_property_readonly("data", [&](Volume &layer) -> py::array {

            auto func = [&](auto pVolume) -> py::array {
                using ValueType = util::PrecsionValueType<decltype(pVolume)>;
                using ComponentType = typename util::value_type<ValueType>::type;

                ComponentType *data = (ComponentType *)pVolume->getDataTyped();
                std::vector<size_t> shape = {pVolume->getDimensions().x, pVolume->getDimensions().y,
                                             pVolume->getDimensions().z,
                                             pVolume->getDataFormat()->getComponents()};

                std::vector<size_t> strides = {
                    sizeof(ComponentType) * pVolume->getDataFormat()->getComponents(),
                    sizeof(ComponentType) * pVolume->getDataFormat()->getComponents() *
                        pVolume->getDimensions().x,
                    sizeof(ComponentType) * pVolume->getDataFormat()->getComponents() *
                        pVolume->getDimensions().x * pVolume->getDimensions().y,
                    sizeof(ComponentType)};

                bool readOnly = false;
                if (readOnly) {
                    return py::array(pybind11::dtype::of<ComponentType>(), shape, strides, data);
                } else {
                    return py::array(pybind11::dtype::of<ComponentType>(), shape, strides, data,
                                     py::cast<>(1));
                }

            };

            return layer.getRepresentation<VolumeRAM>()->dispatch<py::array>(func);
        });

    exposeOutport<VolumeOutport>(m, "Volume");
}
}  // namespace
