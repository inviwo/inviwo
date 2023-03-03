/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2023 Inviwo Foundation
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

#include <inviwopy/pyvolume.h>
#include <inviwopy/pyglmtypes.h>
#include <inviwopy/pyimage.h>  // for the opaque swizzlemask

#include <inviwo/core/util/formatdispatching.h>
#include <inviwo/core/datastructures/volume/volume.h>
#include <inviwo/core/datastructures/volume/volumeram.h>
#include <inviwo/core/datastructures/volume/volumeramprecision.h>
#include <inviwo/core/ports/volumeport.h>
#include <inviwo/core/datastructures/unitsystem.h>
#include <modules/python3/pybindutils.h>
#include <modules/python3/pyportutils.h>
#include <modules/python3/volumepy.h>

#include <pybind11/numpy.h>
#include <pybind11/stl_bind.h>

#include <fmt/format.h>
#include <fmt/ostream.h>

PYBIND11_MAKE_OPAQUE(inviwo::VolumeSequence)

namespace inviwo {

void exposeVolume(pybind11::module& m) {
    namespace py = pybind11;

    py::class_<Volume>(m, "Volume")
        .def(py::init<std::shared_ptr<VolumeRepresentation>>())
        .def(py::init<size3_t, const DataFormatBase*, const SwizzleMask&, InterpolationType,
                      const Wrapping3D&>(),
             py::arg("size"), py::arg("format"), py::arg("swizzleMask") = swizzlemasks::rgba,
             py::arg("interpolation") = InterpolationType::Linear,
             py::arg("wrapping") = wrapping3d::clampAll)
        .def(py::init([](py::array data) { return pyutil::createVolume(data).release(); }))
        .def("clone", [](Volume& self) { return self.clone(); })
        .def_property("modelMatrix", &Volume::getModelMatrix, &Volume::setModelMatrix)
        .def_property("worldMatrix", &Volume::getWorldMatrix, &Volume::setWorldMatrix)
        .def_property("basis", &Volume::getBasis, &Volume::setBasis)
        .def_property("offset", &Volume::getOffset, &Volume::setOffset)
        .def("copyMetaDataFrom", [](Volume& self, Volume& other) { self.copyMetaDataFrom(other); })
        .def("copyMetaDataTo", [](Volume& self, Volume& other) { self.copyMetaDataTo(other); })
        .def_property_readonly("dimensions", &Volume::getDimensions)
        .def_property("swizzlemask", &Volume::getSwizzleMask, &Volume::setSwizzleMask)
        .def_property("interpolation", &Volume::getInterpolation, &Volume::setInterpolation)
        .def_property("wrapping", &Volume::getWrapping, &Volume::setWrapping)
        .def_readwrite("dataMap", &Volume::dataMap_)
        .def_readwrite("axes", &Volume::axes)
        .def("hasRepresentations", &Volume::hasRepresentations)
        .def("addRepresentation", &Volume::addRepresentation)
        .def("removeRepresentation", &Volume::removeRepresentation)
        .def("removeOtherRepresentations", &Volume::removeOtherRepresentations)
        .def("clearRepresentations", &Volume::clearRepresentations)
        .def("invalidateAllOther", &Volume::invalidateAllOther)
        .def("getVolumePyRepresentation",
             [](Volume& self) { return self.getRepresentation<VolumePy>(); })
        .def("getEditableVolumePyRepresentation",
             [](Volume& self) { return self.getEditableRepresentation<VolumePy>(); })
        .def_property(
            "data",
            [&](Volume* volume) -> py::array {
                auto df = volume->getDataFormat();
                auto dims = volume->getDimensions();

                std::vector<size_t> shape = {dims.x, dims.y, dims.z};
                std::vector<size_t> strides = {df->getSize(), df->getSize() * dims.x,
                                               df->getSize() * dims.x * dims.y};

                if (df->getComponents() > 1) {
                    shape.push_back(df->getComponents());
                    strides.push_back(df->getSize() / df->getComponents());
                }

                auto data = volume->getEditableRepresentation<VolumeRAM>()->getData();
                return py::array(pyutil::toNumPyFormat(df), shape, strides, data, py::cast<>(1));
            },
            [](Volume* volume, py::array data) {
                auto rep = volume->getEditableRepresentation<VolumeRAM>();
                pyutil::checkDataFormat<3>(rep->getDataFormat(), rep->getDimensions(), data);

                memcpy(rep->getData(), data.data(0), data.nbytes());
            })
        .def("__repr__", [](const Volume& volume) {
            return fmt::format(
                "<Volume: {} {} dataRange: {} valueRange: {} value: {}{: [}\n"
                "modelMatrix = {}\n"
                "worldMatrix = {}>",
                volume.getDataFormat()->getString(), volume.getDimensions(),
                volume.dataMap_.dataRange, volume.dataMap_.valueRange,
                volume.dataMap_.valueAxis.name, volume.dataMap_.valueAxis.unit,
                volume.getModelMatrix(), volume.getWorldMatrix());
        });

    py::class_<VolumeRepresentation>(m, "VolumeRepresentation")
        .def_property("dimensions", &VolumeRepresentation::getDimensions,
                      &VolumeRepresentation::setDimensions)
        .def_property("swizzleMask", &VolumeRepresentation::getSwizzleMask,
                      &VolumeRepresentation::setSwizzleMask)
        .def_property("interpolation", &VolumeRepresentation::getInterpolation,
                      &VolumeRepresentation::setInterpolation)
        .def_property("wrapping", &VolumeRepresentation::getWrapping,
                      &VolumeRepresentation::setWrapping)
        .def("isValid", &VolumeRepresentation::isValid)
        .def("setValid", &VolumeRepresentation::setValid)
        .def("getOwner", &VolumeRepresentation::getOwner)
        .def_property_readonly("format", &VolumeRepresentation::getDataFormat);

    py::class_<VolumePy, VolumeRepresentation>(m, "VolumePy")
        .def(py::init<py::array, const SwizzleMask&, InterpolationType, const Wrapping3D&>(),
             py::arg("data"), py::arg("swizzleMask") = swizzlemasks::rgba,
             py::arg("interpolation") = InterpolationType::Linear,
             py::arg("wrapping") = wrapping3d::clampAll)
        .def(py::init<size3_t, const DataFormatBase*, const SwizzleMask&, InterpolationType,
                      const Wrapping3D&>(),
             py::arg("size"), py::arg("format"), py::arg("swizzleMask") = swizzlemasks::rgba,
             py::arg("interpolation") = InterpolationType::Linear,
             py::arg("wrapping") = wrapping3d::clampAll)
        .def_property_readonly("data", static_cast<py::array& (VolumePy::*)()>(&VolumePy::data));

    py::bind_vector<VolumeSequence>(m, "VolumeSequence", py::module_local(false));

    exposeStandardDataPorts<Volume>(m, "Volume");
    exposeStandardDataPorts<VolumeSequence>(m, "VolumeSequence");
}

}  // namespace inviwo
