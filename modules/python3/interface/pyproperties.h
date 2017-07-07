/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017 Inviwo Foundation
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

#ifndef IVW_PYPROPERTIES_H
#define IVW_PYPROPERTIES_H
#include <pybind11/pybind11.h>

#include <modules/python3/interface/inviwopy.h>

#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/minmaxproperty.h>
#include <inviwo/core/properties/optionproperty.h>

namespace inviwo {

template <typename T>
struct HasOwnerDeleter {
    void operator()(T *p) {
        if (p && p->getOwner() == nullptr) delete p;
    }
};

template <typename T>
using PropertyPtr = std::unique_ptr<T, HasOwnerDeleter<T>>;



template <typename T, typename P, typename C>
void pyTemplateProperty(C &prop) {
    prop.def_property("value", [](P &p) { return p.get(); }, [](P &p, T t) { p.set(t); })
        .def("__repr__", [](P &v) { return inviwo::toString(v.get()); });
}

template <typename PropertyType, typename T>
struct OrdinalPropertyIterator {
    PropertyType *property_;
    T cur;
    T inc;

    T begin;
    T end;

    OrdinalPropertyIterator(PropertyType *prop)
        : property_(prop)
        , cur(prop->getMinValue())
        , inc(prop->getIncrement())
        , begin(prop->getMinValue())
        , end(prop->getMaxValue()) {
        property_->set(property_->getMinValue());
    }

    OrdinalPropertyIterator(PropertyType *prop, T begin, T end)
        : property_(prop), cur(begin), inc(prop->getIncrement()), begin(begin), end(end) {
        property_->set(property_->getMinValue());
    }

    OrdinalPropertyIterator(PropertyType *prop, T begin, T end, T inc)
        : property_(prop), cur(begin), inc(inc), begin(begin), end(end) {
        property_->set(property_->getMinValue());
    }

    OrdinalPropertyIterator *iter() { return this; }

    T next() {
        if (cur > end) {
            throw pybind11::stop_iteration();
        } else {
            property_->set(cur);
            cur += inc;
            return cur - inc;
        }
    }
};

template <typename T, typename P, typename M, typename PC>
void addOrdinalPropertyIterator(M &m, PC &pc, std::true_type) {
    auto itclassname = Defaultvalues<T>::getName() + "PropertyIterator";

    using IT = OrdinalPropertyIterator<P, T>;

    pybind11::class_<IT>(m, itclassname.c_str())
        .def(pybind11::init<P *>())
        .def("__next__", &IT::next)
        .def("__iter__", &IT::iter);

    pc.def("__iter__", [&](P *p) { return IT(p); });
    pc.def("foreach", [&](P *p, T begin, T end) { return IT(p, begin, end); });
    pc.def("foreach", [&](P *p, T begin, T end, T inc) { return IT(p, begin, end, inc); });
}

template <typename T, typename P, typename M, typename PC>
void addOrdinalPropertyIterator(M &m, PC &pc, std::false_type) {}

template <typename T, typename P, typename M, typename PC>
void addOrdinalPropertyIterator(M &m, PC &pc) {
    addOrdinalPropertyIterator<T, P>(
        m, pc, typename std::conditional<util::rank<T>::value == 0, std::true_type,
                                         std::false_type>::type());
}

struct OrdinalPropertyHelper {

    template <typename T>
    auto operator()(pybind11::module &m) {
        using P = OrdinalProperty<T>;

        auto classname = Defaultvalues<T>::getName() + "Property";

        pybind11::class_<P, Property, std::unique_ptr<P, HasOwnerDeleter<P>>> pyOrdinal(
            m, classname.c_str());
        pyOrdinal
            .def("__init__",
                 [](P &instance, const std::string &identifier, const std::string &displayName,
                    const T &value = Defaultvalues<T>::getVal(),
                    const T &minValue = Defaultvalues<T>::getMin(),
                    const T &maxValue = Defaultvalues<T>::getMax(),
                    const T &increment = Defaultvalues<T>::getInc()) {
                     new (&instance)
                         P(identifier, displayName, value, minValue, maxValue, increment);
                 })

            .def_property("minValue", &P::getMinValue, &P::setMinValue)
            .def_property("maxValue", &P::getMaxValue, &P::setMaxValue)
            .def_property("increment", &P::getIncrement, &P::setIncrement);
        pyTemplateProperty<T, P>(pyOrdinal);

        addOrdinalPropertyIterator<T, P>(m, pyOrdinal);

        return pyOrdinal;
    }
};

struct MinMaxHelper {

    template <typename T>
    auto operator()(pybind11::module &m) {
        using P = MinMaxProperty<T>;
        using range_type = glm::tvec2<T, glm::defaultp>;

        auto classname = Defaultvalues<T>::getName() + "MinMaxProperty";

        pybind11::class_<P, Property, std::unique_ptr<P, HasOwnerDeleter<P>>> pyOrdinal(
            m, classname.c_str());
        pyOrdinal
            .def("__init__",
                 [](P &instance, const std::string &identifier, const std::string &displayName,
                    const T &valueMin = Defaultvalues<T>::getMin(),
                    const T &valueMax = Defaultvalues<T>::getMax(),
                    const T &rangeMin = Defaultvalues<T>::getMin(),
                    const T &rangeMax = Defaultvalues<T>::getMax(),
                    const T &increment = Defaultvalues<T>::getInc(), const T &minSeperation = 0) {
                     new (&instance) P(identifier, displayName, valueMin, valueMax, rangeMin,
                                       rangeMax, increment, minSeperation);
                 })
            .def_property("rangeMin", &P::getRangeMin, &P::setRangeMin)
            .def_property("rangeMax", &P::getRangeMax, &P::setRangeMax)
            .def_property("increment", &P::getIncrement, &P::setIncrement)
            .def_property("minSeparation", &P::getMinSeparation, &P::setMinSeparation)
            .def_property("range", &P::getRange, &P::setRange);

        pyTemplateProperty<range_type, P>(pyOrdinal);

        return pyOrdinal;
    }
};

struct OptionPropertyHelper {

    template <typename T>
    auto operator()(pybind11::module &m) {
        using namespace inviwo;
        using P = TemplateOptionProperty<T>;
        using O = OptionPropertyOption<T>;

        auto classname = "OptionProperty" + Defaultvalues<T>::getName();
        auto optionclassname = Defaultvalues<T>::getName() + "Option";

        pybind11::class_<O>(m, optionclassname.c_str())
            .def(pybind11::init<>())
            .def(pybind11::init<std::string, std::string, T>())
            .def_readwrite("id", &O::id_)
            .def_readwrite("name", &O::name_)
            .def_readwrite("value", &O::value_);

        pybind11::class_<P, BaseOptionProperty, std::unique_ptr<P, HasOwnerDeleter<P>>> pyOption(
            m, classname.c_str());
        pyOption.def(pybind11::init<std::string, std::string>())
            .def("addOption", [](P *p, std::string id, std::string displayName,
                                 T t) { p->addOption(id, displayName, t); })

            .def_property_readonly("values", &P::getValues)
            .def("removeOption", &P::removeOption)

            .def_property("value", [](P *p) { return p->get(); }, [](P *p, T &t) { p->set(t); })

            //     .def_property("selectedOption", &P::getSelectedOption, &P::setSelectedOption)
            .def_property("selectedValue", &P::getSelectedValue, &P::setSelectedValue)

            .def("replaceOptions",
                 [](P *p, std::vector<std::string> ids, std::vector<std::string> displayNames,
                    std::vector<T> values) { p->replaceOptions(ids, displayNames, values); })

            .def("replaceOptions",
                 [](P *p, std::vector<OptionPropertyOption<T>> options) {
                     p->replaceOptions(options);
                 })

            ;

        return pyOption;
    }
};

void exposeProperties(pybind11::module &m);
}

#endif  // IVW_PYPROPERTIES_H
