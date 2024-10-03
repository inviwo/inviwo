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

#include <inviwopy/properties/pyoptionproperties.h>
#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/util/defaultvalues.h>
#include <inviwo/core/util/stdextensions.h>
#include <inviwo/core/util/foreacharg.h>
#include <inviwo/core/util/stringconversion.h>

#include <modules/python3/opaquetypes.h>
#include <modules/python3/polymorphictypehooks.h>

#include <pybind11/stl.h>
#include <pybind11/functional.h>
#include <fmt/format.h>

#include <string>

namespace py = pybind11;

namespace inviwo {

struct OptionPropertyHelper {
    template <typename T>
    auto operator()(pybind11::module& m) {
        namespace py = pybind11;
        using P = OptionProperty<T>;
        using O = OptionPropertyOption<T>;

        auto classname = "OptionProperty" + Defaultvalues<T>::getName();
        auto optionclassname = Defaultvalues<T>::getName() + "Option";

        py::class_<O>(m, optionclassname.c_str())
            .def(py::init<>())
            .def(py::init<std::string_view, std::string_view, const T&>())
            .def_readwrite("id", &O::id_)
            .def_readwrite("name", &O::name_)
            .def_readwrite("value", &O::value_);

        py::class_<P, BaseOptionProperty> prop(m, classname.c_str());
        prop.def(py::init([](std::string_view identifier, std::string_view name, Document help,
                             std::vector<OptionPropertyOption<T>> options, size_t selectedIndex,
                             InvalidationLevel invalidationLevel, PropertySemantics semantics) {
                     return new P(identifier, name, std::move(help), options, selectedIndex,
                                  invalidationLevel, semantics);
                 }),
                 py::arg("identifier"), py::arg("name"), py::arg("help") = Document{},
                 py::arg("options") = std::vector<OptionPropertyOption<T>>{},
                 py::arg("selectedIndex") = 0,
                 py::arg("invalidationLevel") = InvalidationLevel::InvalidOutput,
                 py::arg("semantics") = PropertySemantics::Default)

            .def(py::init([](std::string_view identifier, std::string_view name,
                             std::vector<OptionPropertyOption<T>> options, size_t selectedIndex,
                             InvalidationLevel invalidationLevel, PropertySemantics semantics) {
                     return new P(identifier, name, options, selectedIndex, invalidationLevel,
                                  semantics);
                 }),
                 py::arg("identifier"), py::arg("name"),
                 py::arg("options") = std::vector<OptionPropertyOption<T>>{},
                 py::arg("selectedIndex") = 0,
                 py::arg("invalidationLevel") = InvalidationLevel::InvalidOutput,
                 py::arg("semantics") = PropertySemantics::Default)

            .def("addOption", [](P* p, std::string_view id, std::string_view displayName,
                                 const T& t) { p->addOption(id, displayName, t); })

            .def_property_readonly("values", &P::getValues)
            .def("removeOption", py::overload_cast<size_t>(&P::removeOption))
            .def("removeOption", py::overload_cast<std::string_view>(&P::removeOption))

            .def_property(
                "value", [](P* p) { return p->get(); }, [](P* p, T& t) { p->set(t); })
            .def_property("selectedValue", &P::getSelectedValue,
                          [](P* p, const T& val) { p->setSelectedValue(val); })
            .def_property("selectedIdentifier", &P::getSelectedIdentifier,
                          &P::setSelectedIdentifier)
            .def_property("selectedDisplayName", &P::getSelectedDisplayName,
                          &P::setSelectedDisplayName)

            .def("replaceOptions",
                 [](P* p, const std::vector<std::string>& ids,
                    const std::vector<std::string>& displayNames,
                    const std::vector<T>& values) { p->replaceOptions(ids, displayNames, values); })

            .def("replaceOptions",
                 [](P* p, std::vector<OptionPropertyOption<T>> options) {
                     p->replaceOptions(options);
                 })
            .def("__repr__", [](P& v) { return inviwo::toString(v.get()); });

        return prop;
    }
};

void exposeOptionProperties(py::module& m) {

    py::class_<BaseOptionProperty, Property>(m, "BaseOptionProperty")
        .def_property_readonly("clearOptions", &BaseOptionProperty::clearOptions)
        .def_property_readonly("size", &BaseOptionProperty::size)

        .def_property("selectedIndex", &BaseOptionProperty::getSelectedIndex,
                      &BaseOptionProperty::setSelectedIndex)
        .def_property("selectedIdentifier", &BaseOptionProperty::getSelectedIdentifier,
                      &BaseOptionProperty::setSelectedIdentifier)
        .def_property("selectedDisplayName", &BaseOptionProperty::getSelectedDisplayName,
                      &BaseOptionProperty::setSelectedDisplayName)

        .def("isSelectedIndex", &BaseOptionProperty::isSelectedIndex)
        .def("isSelectedIdentifier", &BaseOptionProperty::isSelectedIdentifier)
        .def("isSelectedDisplayName", &BaseOptionProperty::isSelectedDisplayName)

        .def_property_readonly("identifiers", &BaseOptionProperty::getIdentifiers)
        .def_property_readonly("displayNames", &BaseOptionProperty::getDisplayNames);

    using OptionPropertyTypes = std::tuple<double, float, int, std::string>;

    util::for_each_type<OptionPropertyTypes>{}(OptionPropertyHelper{}, m);
}

}  // namespace inviwo
