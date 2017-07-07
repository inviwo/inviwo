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
#include <modules/python3/pybindutils.h>

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


    py::class_<Volume>(m, "Volume")
        .def(py::init<size3_t, const DataFormatBase *>())
        .def("clone" ,[](Volume &self){ return self.clone(); })
        .def_property("modelMatrix", &Volume::getModelMatrix, &Volume::setModelMatrix)
        .def_property("worldMatrix", &Volume::getWorldMatrix, &Volume::setWorldMatrix)
        .def("copyMetaDataFrom", [](Volume &self,Volume &other){ self.copyMetaDataFrom(other); } )
        .def("copyMetaDataTo", [](Volume &self,Volume &other){ self.copyMetaDataTo(other); } )
        .def_property_readonly("dimensions", &Volume::getDimensions)
        .def_property_readonly("data", [&](Volume *volume) -> py::array {

            auto df = volume->getDataFormat();
            auto dims = volume->getDimensions();

            std::vector<size_t> shape = {dims.x, dims.y, dims.z};
            std::vector<size_t> strides = {df->getSize(),
                                           df->getSize() * dims.x,
                                           df->getSize() * dims.x * dims.y};

            if(df->getComponents()>1){
                shape.push_back(df->getComponents());
                strides.push_back(df->getSize() / df->getComponents());
            }

            auto data = volume->getRepresentation<VolumeRAM>()->getData();

            bool readOnly = false;
            if (readOnly) {
                return py::array(pyutil::toNumPyFormat(df), shape, strides, data);
            } else {
                return py::array(pyutil::toNumPyFormat(df), shape, strides, data, py::cast<>(1));
            }

        });

    exposeOutport<VolumeOutport>(m, "Volume");
}
}  // namespace
