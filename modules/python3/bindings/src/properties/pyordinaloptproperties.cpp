/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2026 Inviwo Foundation
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

#include <inviwopy/properties/pyordinaloptproperties.h>

#include <pybind11/stl.h>
#include <pybind11/functional.h>

#include <inviwo/core/properties/ordinaloptproperty.h>
#include <inviwo/core/util/defaultvalues.h>
#include <inviwo/core/util/stdextensions.h>
#include <inviwo/core/util/foreacharg.h>

#include <modules/python3/opaquetypes.h>
#include <modules/python3/polymorphictypehooks.h>

#include <fmt/format.h>

namespace py = pybind11;

namespace inviwo {

namespace {

struct OrdinalOptPropertyHelper {
    template <typename T>
    auto operator()(pybind11::module& m) {
        namespace py = pybind11;
        using P = OrdinalOptProperty<T>;

        auto classname = Defaultvalues<T>::getName() + "OptProperty";

        py::classh<P, Property> prop(m, classname.c_str());
        prop.def(py::init([](std::string_view identifier, std::string_view name, Document help,
                             const std::optional<T>& value,
                             const std::pair<T, ConstraintBehavior>& min,
                             const std::pair<T, ConstraintBehavior>& max, const T& increment,
                             InvalidationLevel invalidationLevel, PropertySemantics semantics) {
                     return new P(identifier, name, std::move(help), value, min, max, increment,
                                  invalidationLevel, semantics);
                 }),
                 py::arg("identifier"), py::arg("name"), py::arg("help") = Document{},
                 py::arg("value") = std::nullopt,
                 py::arg("min") =
                     std::pair{Defaultvalues<T>::getMin(), ConstraintBehavior::Editable},
                 py::arg("max") =
                     std::pair{Defaultvalues<T>::getMax(), ConstraintBehavior::Editable},
                 py::arg("increment") = Defaultvalues<T>::getInc(),
                 py::arg("invalidationLevel") = InvalidationLevel::InvalidOutput,
                 py::arg("semantics") = PropertySemantics::Default)
            .def(py::init([](std::string_view identifier, std::string_view name,
                             const std::optional<T>& value,
                             const std::pair<T, ConstraintBehavior>& min,
                             const std::pair<T, ConstraintBehavior>& max, const T& increment,
                             InvalidationLevel invalidationLevel, PropertySemantics semantics) {
                     return new P(identifier, name, value, min, max, increment, invalidationLevel,
                                  semantics);
                 }),
                 py::arg("identifier"), py::arg("name"), py::arg("value") = std::nullopt,
                 py::arg("min") =
                     std::pair{Defaultvalues<T>::getMin(), ConstraintBehavior::Editable},
                 py::arg("max") =
                     std::pair{Defaultvalues<T>::getMax(), ConstraintBehavior::Editable},
                 py::arg("increment") = Defaultvalues<T>::getInc(),
                 py::arg("invalidationLevel") = InvalidationLevel::InvalidOutput,
                 py::arg("semantics") = PropertySemantics::Default)
            .def_property(
                "value", [](P& p) { return p.get(); }, [](P& p, std::optional<T> t) { p.set(t); })
            .def_property("minValue", &P::getMinValue, &P::setMinValue)
            .def_property("maxValue", &P::getMaxValue, &P::setMaxValue)
            .def_property("increment", &P::getIncrement, &P::setIncrement)
            .def("__getitem__",
                 [](P& p, size_t idx) {
                     if (idx >= util::extent_v<T>) throw py::index_error();
                     return p.get(idx);
                 })
            .def("__getitem__",
                 [](P& p, py::tuple indices) {
                     if (indices.size() != 2) {
                         throw py::index_error();
                     }
                     const auto i = indices[0].cast<int>();
                     const auto j = indices[1].cast<int>();
                     if (i >= static_cast<int>(util::extent_v<T, 0>) ||
                         j >= static_cast<int>(util::extent_v<T, 1>) || i < 0 || j < 0) {
                         throw py::index_error();
                     }
                     return p.get(i, j);
                 })
            .def("__setitem__",
                 [](P& p, int idx, const P::component_type& t) {
                     if (idx >= util::extent_v<T> || idx < 0) throw py::index_error();
                     p.set(t, idx);
                 })
            .def("__setitem__",
                 [](P& p, py::tuple indices, const P::component_type& t) {
                     if (indices.size() != 2) {
                         throw py::index_error();
                     }
                     const auto i = indices[0].cast<int>();
                     const auto j = indices[1].cast<int>();
                     if (i >= static_cast<int>(util::extent_v<T, 0>) ||
                         j >= static_cast<int>(util::extent_v<T, 1>) || i < 0 || j < 0) {
                         throw py::index_error();
                     }
                     p.set(t, i, j);
                 })
            .def("clear", &P::clear)
            .def("__repr__", [](P& v) {
                if (v.get().has_value()) {
                    return fmt::to_string(*v.get());
                } else {
                    return std::string{"<empty>"};
                }
            });

        return prop;
    }
};

}  // namespace

void exposeOrdinalOptProperties(py::module& m) {
    using OrdinalPropetyTypes = std::tuple<float, int, size_t, glm::i64, double, vec2, vec3, vec4,
                                           dvec2, dvec3, dvec4, ivec2, ivec3, ivec4, size2_t,
                                           size3_t, size4_t, mat2, mat3, mat4, dmat2, dmat3, dmat4>;
    util::for_each_type<OrdinalPropetyTypes>{}(OrdinalOptPropertyHelper{}, m);
}

}  // namespace inviwo
