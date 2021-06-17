/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2021 Inviwo Foundation
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

#include <inviwopy/pypropertyowner.h>
#include <inviwo/core/properties/propertyowner.h>
#include <inviwo/core/processors/processor.h>

#include <inviwopy/inviwopy.h>
#include <inviwopy/vectoridentifierwrapper.h>
#include <inviwopy/pyproperties.h>

#include <pybind11/detail/common.h>


namespace inviwo {

void exposePropertyOwner(pybind11::module& m) {
    namespace py = pybind11;

    using PropertyVecWrapper = VectorIdentifierWrapper<std::vector<Property*>>;
    exposeVectorIdentifierWrapper<std::vector<Property*>>(m, "PropertyVecWrapper");

    py::class_<PropertyOwner>(m, "PropertyOwner")
        .def(
            "__getattr__",
            [](PropertyOwner& po, const std::string& key) {
                if (auto prop = po.getPropertyByIdentifier(key)) {
                    return prop;
                } else {
                    throw py::attribute_error{"PropertyOwner (" + po.getIdentifier() +
                                              ") does not have a property with identifier: '" +
                                              key + "'"};
                }
            },
            py::return_value_policy::reference)
        .def_property_readonly(
            "properties", [](PropertyOwner& po) { return PropertyVecWrapper(po.getProperties()); })
        .def("getPropertiesRecursive", &PropertyOwner::getPropertiesRecursive)
        .def(
            "addProperty",
            [](PropertyOwner& po, Property* prop, bool owner) { po.addProperty(prop, owner); },
            py::arg("prop"), py::arg("owner") = false, py::keep_alive<1, 2>{})
        .def(
            "insertProperty",
            [](PropertyOwner& po, size_t index, Property* prop, bool owner) {
                po.insertProperty(index, prop, owner);
            },
            py::arg("index"), py::arg("prop"), py::arg("owner") = false, py::keep_alive<1, 3>{})
        .def("removeProperty",
             [](PropertyOwner& po, Property* prop) { return po.removeProperty(prop); })
        .def("removeProperty",
             [](PropertyOwner& po, size_t index) { return po.removeProperty(index); })
        .def("clear", &PropertyOwner::clear)
        .def("getPropertyByIdentifier", &PropertyOwner::getPropertyByIdentifier,
             py::return_value_policy::reference, py::arg("identifier"),
             py::arg("recursiveSearch") = false)
        .def("getPropertyByPath", &PropertyOwner::getPropertyByPath,
             py::return_value_policy::reference)
        .def("getIdentifier", &PropertyOwner::getIdentifier)
        .def(
            "getOwner", [](PropertyOwner* po) { return po->getOwner(); },
            py::return_value_policy::reference)
        .def("empty", &PropertyOwner::empty)
        .def("size", &PropertyOwner::size)
        .def("isValid", &PropertyOwner::isValid)
        .def("setValid", &PropertyOwner::setValid)
        .def("getInvalidationLevel", &PropertyOwner::getInvalidationLevel)
        .def("invalidate",
             [](PropertyOwner* po) { po->invalidate(InvalidationLevel::InvalidOutput); })
        .def_property_readonly(
            "processor", [](PropertyOwner& p) { return p.getProcessor(); },
            py::return_value_policy::reference)
        .def("setAllPropertiesCurrentStateAsDefault",
             &PropertyOwner::setAllPropertiesCurrentStateAsDefault)
        .def("resetAllPoperties", &PropertyOwner::resetAllPoperties);
}

}  // namespace inviwo
