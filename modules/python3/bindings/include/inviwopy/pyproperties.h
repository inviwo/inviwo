/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2021 Inviwo Foundation
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

#pragma once

#include <warn/push>
#include <warn/ignore/shadow>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <warn/pop>

#include <inviwopy/inviwopy.h>

#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/ordinalrefproperty.h>
#include <inviwo/core/properties/minmaxproperty.h>
#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/properties/compositeproperty.h>
#include <inviwo/core/properties/optionproperty.h>

#include <fmt/format.h>

/*
 * The python type caster for polymorphic types can only lookup exact
 * matches. I.e. if we have a property Basis that derives from CompositeProperty
 * which derives from Property, and we try to cast a Property pointer pointing to a Basis
 * and only Property and CompositeProperty are exposed to python and not Basis. Python will
 * not find a exact match, and the returned wrapper will be of the Pointer class used.
 * In this cases Property. To get at least some support for unexposed properties
 * derived from CompositeProperty and BaseOptionProperty we add a specialization here
 * that will mean that in the example above we will get a CompositeProperty wrapper instead of
 * of a Property wrapper.
 * see polymorphic_type_hook at the end of include/pybind11/detail/type_caster_base.h
 */
namespace pybind11 {
template <>
struct polymorphic_type_hook<inviwo::Property> {
    static const void* get(const inviwo::Property* prop, const std::type_info*& type) {
        if (!prop) {  // default implementation if prop is null
            type = nullptr;
            return dynamic_cast<const void*>(prop);
        }

        // check if the exact type is registed in python, then return that.
        const auto& id = typeid(*prop);
        if (detail::get_type_info(id)) {
            type = &id;
            return dynamic_cast<const void*>(prop);
        }

        // else check if we know a more derived base then Property and return that.
        if (auto cp = dynamic_cast<const inviwo::CompositeProperty*>(prop)) {
            type = &typeid(inviwo::CompositeProperty);
            return dynamic_cast<const void*>(cp);
        } else if (auto op = dynamic_cast<const inviwo::BaseOptionProperty*>(prop)) {
            type = &typeid(inviwo::BaseOptionProperty);
            return dynamic_cast<const void*>(op);
        }

        // default implementation for prop != null
        type = &id;
        return dynamic_cast<const void*>(prop);
    }
};
}  // namespace pybind11

namespace inviwo {

template <typename T, typename P, typename C>
void pyTemplateProperty(C& prop) {
    prop.def_property(
            "value", [](P& p) { return p.get(); }, [](P& p, T t) { p.set(t); })
        .def("__repr__", [](P& v) { return inviwo::toString(v.get()); });
}

template <typename PropertyType, typename T>
struct OrdinalPropertyIterator {
    OrdinalPropertyIterator(PropertyType* prop)
        : property_(prop)
        , cur(prop->getMinValue())
        , inc(prop->getIncrement())
        , begin(prop->getMinValue())
        , end(prop->getMaxValue()) {
        property_->set(property_->getMinValue());
    }

    OrdinalPropertyIterator(PropertyType* prop, T begin, T end)
        : property_(prop), cur(begin), inc(prop->getIncrement()), begin(begin), end(end) {
        property_->set(property_->getMinValue());
    }

    OrdinalPropertyIterator(PropertyType* prop, T begin, T end, T inc)
        : property_(prop), cur(begin), inc(inc), begin(begin), end(end) {
        property_->set(property_->getMinValue());
    }

    OrdinalPropertyIterator* iter() { return this; }

    T next() {
        if (cur > end) {
            throw pybind11::stop_iteration();
        } else {
            property_->set(cur);
            cur += inc;
            return cur - inc;
        }
    }

    PropertyType* property_;
    T cur;
    T inc;
    T begin;
    T end;
};

template <typename T, typename P, typename M, typename PC>
void addOrdinalPropertyIterator(M& m, PC& pc, std::string_view suffix) {
    const auto itclassname = fmt::format("{}{}", Defaultvalues<T>::getName().view(), suffix);
    using IT = OrdinalPropertyIterator<P, T>;
    pybind11::class_<IT>(m, itclassname.c_str())
        .def(pybind11::init<P*>())
        .def("__next__", &IT::next)
        .def("__iter__", &IT::iter);

    pc.def("__iter__", [&](P* p) { return IT(p); });
    pc.def("foreach", [&](P* p, T begin, T end) { return IT(p, begin, end); });
    pc.def("foreach", [&](P* p, T begin, T end, T inc) { return IT(p, begin, end, inc); });
}

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
            .def("__repr__", [](P& v) { return inviwo::toString(v.get()); });

        if constexpr (util::rank<T>::value == 0) {
            addOrdinalPropertyIterator<T, P>(m, prop, "PropertyIterator");
        }

        return prop;
    }
};

struct OrdinalRefPropertyHelper {
    template <typename T>
    auto operator()(pybind11::module& m) {
        namespace py = pybind11;
        using P = OrdinalRefProperty<T>;

        auto classname = Defaultvalues<T>::getName() + "RefProperty";

        py::class_<P, Property> prop(m, classname.c_str());
        prop.def(py::init([](std::string_view identifier, std::string_view name,
                             std::function<T()> get, std::function<void(const T&)> set,
                             const std::pair<T, ConstraintBehavior>& min,
                             const std::pair<T, ConstraintBehavior>& max, const T& increment,
                             InvalidationLevel invalidationLevel, PropertySemantics semantics) {
                     return new P(identifier, name, std::move(get), std::move(set), min, max,
                                  increment, invalidationLevel, semantics);
                 }),
                 py::arg("identifier"), py::arg("name"), py::arg("get"), py::arg("set"),
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
            .def("setGetAndSet", &P::setGetAndSet)
            .def("__repr__", [](P& v) { return inviwo::toString(v.get()); });

        if constexpr (util::rank<T>::value == 0) {
            addOrdinalPropertyIterator<T, P>(m, prop, "RefPropertyIterator");
        }

        return prop;
    }
};

struct MinMaxHelper {
    template <typename T>
    auto operator()(pybind11::module& m) {
        namespace py = pybind11;
        using P = MinMaxProperty<T>;
        using range_type = glm::tvec2<T, glm::defaultp>;

        auto classname = Defaultvalues<T>::getName() + "MinMaxProperty";

        py::class_<P, Property> prop(m, classname.c_str());
        prop.def(py::init([](std::string_view identifier, std::string_view name, const T& valueMin,
                             const T& valueMax, const T& rangeMin, const T& rangeMax,
                             const T& increment, const T& minSeperation,
                             InvalidationLevel invalidationLevel, PropertySemantics semantics) {
                     return new P(identifier, name, valueMin, valueMax, rangeMin, rangeMax,
                                  increment, minSeperation, invalidationLevel, semantics);
                 }),
                 py::arg("identifier"), py::arg("name"),
                 py::arg("valueMin") = Defaultvalues<T>::getMin(),
                 py::arg("valueMax") = Defaultvalues<T>::getMax(),
                 py::arg("rangeMin") = Defaultvalues<T>::getMin(),
                 py::arg("rangeMax") = Defaultvalues<T>::getMax(),
                 py::arg("increment") = Defaultvalues<T>::getInc(), py::arg("minSeperation") = 0,
                 py::arg("invalidationLevel") = InvalidationLevel::InvalidOutput,
                 py::arg("semantics") = PropertySemantics::Default)
            .def_property("rangeMin", &P::getRangeMin, &P::setRangeMin)
            .def_property("rangeMax", &P::getRangeMax, &P::setRangeMax)
            .def_property("increment", &P::getIncrement, &P::setIncrement)
            .def_property("minSeparation", &P::getMinSeparation, &P::setMinSeparation)
            .def_property("range", &P::getRange, &P::setRange);

        pyTemplateProperty<range_type, P>(prop);

        return prop;
    }
};

struct OptionPropertyHelper {

    template <typename T>
    auto operator()(pybind11::module& m) {
        namespace py = pybind11;
        using P = TemplateOptionProperty<T>;
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
        prop.def(py::init([](std::string_view identifier, std::string_view name,
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

            .def("replaceOptions", [](P* p, std::vector<OptionPropertyOption<T>> options) {
                p->replaceOptions(options);
            });

        return prop;
    }
};

void exposeProperties(pybind11::module& m);

}  // namespace inviwo
