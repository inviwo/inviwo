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

#include <inviwopy/properties/pyminmaxproperties.h>

#include <inviwo/core/properties/minmaxproperty.h>
#include <inviwo/core/util/defaultvalues.h>
#include <inviwo/core/util/stdextensions.h>
#include <inviwo/core/util/foreacharg.h>
#include <inviwo/core/util/stringconversion.h>

#include <modules/python3/opaquetypes.h>
#include <modules/python3/polymorphictypehooks.h>

#include <inviwopy/util/pypropertyhelper.h>

#include <pybind11/stl.h>
#include <pybind11/functional.h>
#include <fmt/format.h>
#include <glm/gtx/io.hpp>

namespace py = pybind11;

namespace inviwo {

struct MinMaxHelper {
    template <typename T>
    auto operator()(pybind11::module& m) {
        using P = MinMaxProperty<T>;
        using range_type = glm::tvec2<T, glm::defaultp>;

        auto classname = Defaultvalues<T>::getName() + "MinMaxProperty";

        py::class_<P, Property> prop(m, classname.c_str());
        prop.def(py::init([](std::string_view identifier, std::string_view name, Document help,
                             const T& valueMin, const T& valueMax, const T& rangeMin,
                             const T& rangeMax, const T& increment, const T& minSeparation,
                             InvalidationLevel invalidationLevel, PropertySemantics semantics) {
                     return new P(identifier, name, std::move(help), valueMin, valueMax, rangeMin,
                                  rangeMax, increment, minSeparation, invalidationLevel, semantics);
                 }),
                 py::arg("identifier"), py::arg("name"), py::arg("help"),
                 py::arg("valueMin") = Defaultvalues<T>::getMin(),
                 py::arg("valueMax") = Defaultvalues<T>::getMax(),
                 py::arg("rangeMin") = Defaultvalues<T>::getMin(),
                 py::arg("rangeMax") = Defaultvalues<T>::getMax(),
                 py::arg("increment") = Defaultvalues<T>::getInc(), py::arg("minSeparation") = 0,
                 py::arg("invalidationLevel") = InvalidationLevel::InvalidOutput,
                 py::arg("semantics") = PropertySemantics::Default)
            .def(py::init([](std::string_view identifier, std::string_view name, const T& valueMin,
                             const T& valueMax, const T& rangeMin, const T& rangeMax,
                             const T& increment, const T& minSeparation,
                             InvalidationLevel invalidationLevel, PropertySemantics semantics) {
                     return new P(identifier, name, valueMin, valueMax, rangeMin, rangeMax,
                                  increment, minSeparation, invalidationLevel, semantics);
                 }),
                 py::arg("identifier"), py::arg("name"),
                 py::arg("valueMin") = Defaultvalues<T>::getMin(),
                 py::arg("valueMax") = Defaultvalues<T>::getMax(),
                 py::arg("rangeMin") = Defaultvalues<T>::getMin(),
                 py::arg("rangeMax") = Defaultvalues<T>::getMax(),
                 py::arg("increment") = Defaultvalues<T>::getInc(), py::arg("minSeparation") = 0,
                 py::arg("invalidationLevel") = InvalidationLevel::InvalidOutput,
                 py::arg("semantics") = PropertySemantics::Default)
            .def_property("rangeMin", &P::getRangeMin, &P::setRangeMin)
            .def_property("rangeMax", &P::getRangeMax, &P::setRangeMax)
            .def_property("increment", &P::getIncrement, &P::setIncrement)
            .def_property("minSeparation", &P::getMinSeparation, &P::setMinSeparation)
            .def_property("range", &P::getRange, &P::setRange)
            .def("__repr__", [](P& v) { return inviwo::toString(v.get()); });

        pyTemplateProperty<range_type, P>(prop);

        return prop;
    }
};

void exposeMinMaxProperties(py::module& m) {
    using MinMaxPropertyTypes = std::tuple<float, double, size_t, glm::i64, int>;
    util::for_each_type<MinMaxPropertyTypes>{}(MinMaxHelper{}, m);
}

}  // namespace inviwo
