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

#include <inviwopy/pyvolume.h>
#include <inviwopy/pyglmtypes.h>
#include <inviwopy/pyimage.h>  // for the opaque swizzlemask

#include <inviwo/core/util/formatdispatching.h>
#include <inviwo/core/datastructures/volume/volume.h>
#include <inviwo/core/datastructures/volume/volumeram.h>
#include <inviwo/core/datastructures/volume/volumeramprecision.h>
#include <inviwo/core/ports/volumeport.h>
#include <inviwo/core/datastructures/unitsystem.h>
#include <inviwo/core/util/exception.h>
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
        .def(
            "getVolumePyRepresentation",
            [](Volume& self) { return self.getRepresentation<VolumePy>(); },
            pybind11::return_value_policy::reference_internal)
        .def(
            "getEditableVolumePyRepresentation",
            [](Volume& self) { return self.getEditableRepresentation<VolumePy>(); },
            pybind11::return_value_policy::reference_internal)
        .def_property(
            "data",
            [&](Volume* volume) -> py::array {
                auto rep = volume->getRepresentation<VolumePy>();
                return rep->data();
            },
            [](Volume* volume, py::array data) {
                if (volume->hasRepresentation<VolumePy>()) {
                    volume->removeRepresentation(volume->getRepresentation<VolumePy>());
                }
                auto rep =
                    std::make_shared<VolumePy>(data, volume->getSwizzleMask(),
                                               volume->getInterpolation(), volume->getWrapping());
                volume->addRepresentation(rep);
                volume->invalidateAllOther(rep.get());
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
        .def_property_readonly(
            "data", static_cast<const py::array& (VolumePy::*)() const>(&VolumePy::data));


    py::class_<VolumeSequence>(m, "VolumeSequence", py::module_local(false))
        .def(py::init<>())
        .def(py::init<std::span<std::shared_ptr<const Volume>>>(), py::arg("volumes"))
        .def(py::init<const VolumeSequence&>(), "Copy constructor")

        .def("__getitem__",
             [](const VolumeSequence& v, std::ptrdiff_t i) -> std::shared_ptr<const Volume> {
                 if (i < 0 && (i += v.size()) < 0) {
                     throw py::index_error();
                 }
                 if (static_cast<size_t>(i) >= v.size()) {
                     throw py::index_error();
                 }
                 return v[static_cast<size_t>(i)];
             })

        .def(
            "__iter__",
            [](const VolumeSequence& v) {
                using ItType = VolumeSequence::const_iterator;
                return py::make_iterator<py::return_value_policy::copy, ItType, ItType,
                                         std::shared_ptr<const Volume>>(v.begin(), v.end());
            },
            py::keep_alive<0, 1>())
        .def(
            "__bool__", [](const VolumeSequence& v) -> bool { return !v.empty(); },
            "Check whether the list is nonempty")
        .def("__len__", &VolumeSequence::size)
        .def(
            "erase",
            [](VolumeSequence& v, size_t i) {
                if (i >= v.size()) {
                    throw py::index_error();
                }
                v.erase(v.begin() + i);
            },
            "erases element at index ``i``")

        .def("empty", &VolumeSequence::empty, "checks whether the container is empty")
        .def("size", &VolumeSequence::size, "returns the number of elements")
        .def("push_back",
             static_cast<void (VolumeSequence::*)(std::shared_ptr<const Volume>)>(
                 &VolumeSequence::push_back),
             "adds an element to the end")
        .def("pop_back", &VolumeSequence::pop_back, "removes the last element")

        .def("reserve", &VolumeSequence::reserve, "reserves storage")
        .def("shrink_to_fit", &VolumeSequence::shrink_to_fit,
             "reduces memory usage by freeing unused memory")

        .def("clear", &VolumeSequence::clear, "clears the contents")
        .def(
            "front",
            [](VolumeSequence& v) {
                if (v.size()) {
                    return v.front();
                } else {
                    throw py::index_error();
                }
            },
            "access the first element")

        .def(
            "back",
            [](VolumeSequence& v) {
                if (v.size()) {
                    return v.back();
                } else {
                    throw py::index_error();
                }
            },
            "access the last element ")
        .def(
            "append",
            [](VolumeSequence& v, std::shared_ptr<const Volume> value) { v.push_back(value); },
            py::arg("x"), "Add an item to the end of the list")
        .def(
            "extend",
            [](VolumeSequence& v, const VolumeSequence& src) {
                v.insert(v.end(), src.begin(), src.end());
            },
            py::arg("L"), "Extend the list by appending all the items in the given list")
        .def(
            "insert",
            [](VolumeSequence& v, std::ptrdiff_t i, std::shared_ptr<const Volume> x) {
                // Can't use wrap_i; i == v.size() is OK
                if (i < 0) {
                    i += v.size();
                }
                if (i < 0 || static_cast<size_t>(i) > v.size()) {
                    throw py::index_error();
                }
                v.insert(v.begin() + i, x);
            },
            py::arg("i"), py::arg("x"), "Insert an item at a given position.");

    exposeStandardDataPorts<Volume>(m, "Volume");
    exposeStandardDataPorts<VolumeSequence>(m, "VolumeSequence");
}

}  // namespace inviwo
