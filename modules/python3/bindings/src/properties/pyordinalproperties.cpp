/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2025 Inviwo Foundation
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

#include <inviwopy/properties/pyordinalproperties.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/util/defaultvalues.h>
#include <inviwo/core/util/stdextensions.h>
#include <inviwo/core/util/foreacharg.h>
#include <modules/python3/opaquetypes.h>
#include <modules/python3/polymorphictypehooks.h>

#include <pybind11/stl.h>
#include <pybind11/functional.h>
#include <fmt/format.h>

namespace py = pybind11;

namespace inviwo {

struct OrdinalPropertyHelper {
    template <typename T>
    auto operator()(pybind11::module& m) {
        namespace py = pybind11;
        using P = OrdinalProperty<T>;

        auto classname = Defaultvalues<T>::getName() + "Property";

        py::class_<P, Property> prop(m, classname.c_str());
        prop.def(py::init([](std::string_view identifier, std::string_view name, const T& value,
                             const T& min, const T& max, const T& increment,
                             InvalidationLevel invalidationLevel, PropertySemantics semantics) {
                     return new P(identifier, name, value, min, max, increment, invalidationLevel,
                                  semantics);
                 }),
                 py::arg("identifier"), py::arg("name"),
                 py::arg("value") = Defaultvalues<T>::getVal(),
                 py::arg("min") = Defaultvalues<T>::getMin(),
                 py::arg("max") = Defaultvalues<T>::getMax(),
                 py::arg("increment") = Defaultvalues<T>::getInc(),
                 py::arg("invalidationLevel") = InvalidationLevel::InvalidOutput,
                 py::arg("semantics") = PropertySemantics::Default)
            .def(py::init([](std::string_view identifier, std::string_view name, Document help,
                             const T& value, const std::pair<T, ConstraintBehavior>& min,
                             const std::pair<T, ConstraintBehavior>& max, const T& increment,
                             InvalidationLevel invalidationLevel, PropertySemantics semantics) {
                     return new P(identifier, name, std::move(help), value, min, max, increment,
                                  invalidationLevel, semantics);
                 }),
                 py::arg("identifier"), py::arg("name"), py::arg("help") = Document{},
                 py::arg("value") = Defaultvalues<T>::getVal(),
                 py::arg("min") =
                     std::pair{Defaultvalues<T>::getMin(), ConstraintBehavior::Editable},
                 py::arg("max") =
                     std::pair{Defaultvalues<T>::getMax(), ConstraintBehavior::Editable},
                 py::arg("increment") = Defaultvalues<T>::getInc(),
                 py::arg("invalidationLevel") = InvalidationLevel::InvalidOutput,
                 py::arg("semantics") = PropertySemantics::Default)
            .def(py::init([](std::string_view identifier, std::string_view name, const T& value,
                             const std::pair<T, ConstraintBehavior>& min,
                             const std::pair<T, ConstraintBehavior>& max, const T& increment,
                             InvalidationLevel invalidationLevel, PropertySemantics semantics) {
                     return new P(identifier, name, value, min, max, increment, invalidationLevel,
                                  semantics);
                 }),
                 py::arg("identifier"), py::arg("name"),
                 py::arg("value") = Defaultvalues<T>::getVal(),
                 py::arg("min") =
                     std::pair{Defaultvalues<T>::getMin(), ConstraintBehavior::Editable},
                 py::arg("max") =
                     std::pair{Defaultvalues<T>::getMax(), ConstraintBehavior::Editable},
                 py::arg("increment") = Defaultvalues<T>::getInc(),
                 py::arg("invalidationLevel") = InvalidationLevel::InvalidOutput,
                 py::arg("semantics") = PropertySemantics::Default)
            .def_property(
                "value", [](P& p) { return p.get(); }, [](P& p, T t) { p.set(t); })
            .def_property("minValue", &P::getMinValue, &P::setMinValue)
            .def_property("maxValue", &P::getMaxValue, &P::setMaxValue)
            .def_property("increment", &P::getIncrement, &P::setIncrement)
            .def("__repr__", [](P& v) { return fmt::to_string(v.get()); });

        return prop;
    }
};

void exposeOrdinalProperties(py::module& m) {
    using OrdinalPropetyTypes = std::tuple<float, int, size_t, glm::i64, double, vec2, vec3, vec4,
                                           dvec2, dvec3, dvec4, ivec2, ivec3, ivec4, size2_t,
                                           size3_t, size4_t, mat2, mat3, mat4, dmat2, dmat3, dmat4>;
    util::for_each_type<OrdinalPropetyTypes>{}(OrdinalPropertyHelper{}, m);
}

}  // namespace inviwo
