/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2024 Inviwo Foundation
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
#include <inviwopy/vectoridentifierwrapper.h>

#include <pybind11/functional.h>
#include <pybind11/stl.h>
#include <pybind11/stl/filesystem.h>
#include <pybind11/numpy.h>

#include <inviwo/core/properties/propertyowner.h>
#include <inviwo/core/processors/processor.h>

#include <modules/python3/opaquetypes.h>
#include <modules/python3/polymorphictypehooks.h>

#include <pybind11/detail/common.h>

namespace inviwo {

void exposePropertyOwner(pybind11::module& m) {
    namespace py = pybind11;

    using PropertyVecWrapper =
        VectorIdentifierWrapper<typename std::vector<Property*>::const_iterator>;
    exposeVectorIdentifierWrapper<typename std::vector<Property*>::const_iterator>(
        m, "PropertyVecWrapper");

    py::class_<PropertyOwner>(m, "PropertyOwner", py::multiple_inheritance{}, py::dynamic_attr{})
        .def(
            "__getattr__",
            [](PropertyOwner& po, const std::string& key) {
                if (auto prop = po.getPropertyByIdentifier(key)) {
                    return prop;
                } else {
                    throw py::attribute_error{fmt::format(
                        "PropertyOwner ({}) does not have a property with identifier: '{}'",
                        po.getIdentifier(), key)};
                }
            },
            py::return_value_policy::reference)
        .def(
            "__setattr__",
            [](py::object po, py::object key, py::object value) {
                const auto isProperty = [&]() {
                    auto pyProperty =
                        py::module_::import("inviwopy").attr("properties").attr("Property");
                    return py::isinstance(po.attr(key), pyProperty);
                };

                const auto path = [](const PropertyOwner* owner) {
                    std::string path;
                    const auto traverse = [&](auto& self, const PropertyOwner* owner) -> void {
                        if (owner) {
                            self(self, owner->getOwner());
                            path.append(owner->getIdentifier());
                            path.push_back('.');
                        }
                    };
                    traverse(traverse, owner->getOwner());
                    path.append(owner->getIdentifier());
                    return path;
                };

                auto builtins = py::module_::import("builtins");
                auto type = builtins.attr("type");

                const auto skey = py::str(key).cast<std::string>();
                const auto owner = po.cast<PropertyOwner*>();
                if (auto prop = owner->getPropertyByIdentifier(skey)) {
                    if (isProperty()) {
                        throw py::attribute_error{fmt::format(
                            "The member '{0}' is a registered Property of type '{3}',\n"
                            "in PropertyOwner '{1}' of type '{2}'.\n"
                            "To rebind the member unregister (remove) the property from "
                            "'{1}' first.\nIf you were trying to set the 'value' of the '{0}' "
                            "property, you should assign to '{0}.value' instead.\nSee help({0}) "
                            "for more details",
                            skey, path(owner), type(po).attr("__name__").cast<std::string>(),
                            prop->getClassIdentifier())};
                    }
                }

                auto object = builtins.attr("object");
                object.attr("__setattr__")(po, key, value);
            },
            py::return_value_policy::reference)
        .def_property_readonly("properties",
                               [](PropertyOwner& po) {
                                   return PropertyVecWrapper(po.getProperties().begin(),
                                                             po.getProperties().end());
                               })
        .def("getPropertiesRecursive",
             [](PropertyOwner& po) { return po.getPropertiesRecursive(); })
        .def(
            "addProperty",
            [](PropertyOwner& po, Property* prop, bool owner) { po.addProperty(prop, owner); },
            py::arg("prop"), py::arg("owner") = false, py::keep_alive<1, 2>{})
        .def(
            "addProperties",
            [](PropertyOwner& po, std::vector<Property*> props, bool owner) {
                for (auto p : props) {
                    po.addProperty(p, owner);
                }
            },
            py::arg("props"), py::arg("owner") = false, py::keep_alive<1, 2>{})
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
             [](PropertyOwner* po) { po->invalidate(InvalidationLevel::InvalidOutput, nullptr); })
        .def_property_readonly(
            "processor", [](PropertyOwner& p) { return p.getProcessor(); },
            py::return_value_policy::reference)
        .def("setAllPropertiesCurrentStateAsDefault",
             &PropertyOwner::setAllPropertiesCurrentStateAsDefault)
        .def("resetAllPoperties", &PropertyOwner::resetAllProperties);
}

}  // namespace inviwo
