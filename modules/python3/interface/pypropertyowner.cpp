/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018 Inviwo Foundation
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

#include <modules/python3/interface/pypropertyowner.h>

#include <modules/python3/interface/inviwopy.h>

#include <inviwo/core/properties/propertyowner.h>

#include <pybind11/detail/common.h>

namespace pybind11 {
PYBIND11_RUNTIME_EXCEPTION(attribute_error, PyExc_AttributeError)
}

namespace inviwo {

void exposePropertyOwner(pybind11::module &m) {
    namespace py = pybind11;

    py::class_<PropertyOwner, std::unique_ptr<PropertyOwner, py::nodelete>>(m, "PropertyOwner")
        .def("getPath", &PropertyOwner::getPath)
        .def_property_readonly("properties", &PropertyOwner::getProperties,
                               py::return_value_policy::reference)
        .def("__getattr__",
             [](PropertyOwner &po, const std::string &key) {
                 auto prop = po.getPropertyByIdentifier(key);
                 if (prop) {
                      return pybind11::cast(prop);
                 } else {
                     throw py::attribute_error{};
                 }
             },
             py::return_value_policy::reference)
        .def("getPropertiesRecursive", &PropertyOwner::getPropertiesRecursive)
        .def("addProperty",
             [](PropertyOwner &po, Property *prop) { po.addProperty(prop); })
        .def("getPropertyByIdentifier", &PropertyOwner::getPropertyByIdentifier,
             py::return_value_policy::reference, py::arg("identifier"),
             py::arg("recursiveSearch") = false)
        .def("getPropertyByPath", &PropertyOwner::getPropertyByPath,
             py::return_value_policy::reference)
        .def("size", &PropertyOwner::size)
        .def("setValid", &PropertyOwner::setValid)
        .def("getInvalidationLevel", &PropertyOwner::getInvalidationLevel)
        .def("invalidate",
             [](PropertyOwner *po) { po->invalidate(InvalidationLevel::InvalidOutput); })
        .def_property_readonly("processor", [](PropertyOwner &p) { return p.getProcessor(); },
                               py::return_value_policy::reference)
        .def("setAllPropertiesCurrentStateAsDefault",
             &PropertyOwner::setAllPropertiesCurrentStateAsDefault)
        .def("resetAllPoperties", &PropertyOwner::resetAllPoperties);
}

}  // namespace inviwo
