/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2022 Inviwo Foundation
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

#include <inviwopy/pyproperties.h>
#include <inviwopy/pyflags.h>
#include <inviwopy/pypropertytypehook.h>

#include <inviwo/core/properties/propertyfactory.h>
#include <inviwo/core/properties/constraintbehavior.h>
#include <inviwo/core/properties/buttonproperty.h>
#include <inviwo/core/properties/buttongroupproperty.h>
#include <inviwo/core/properties/transferfunctionproperty.h>
#include <inviwo/core/properties/isovalueproperty.h>
#include <inviwo/core/properties/stringproperty.h>
#include <inviwo/core/properties/fileproperty.h>
#include <inviwo/core/properties/directoryproperty.h>
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/properties/propertyeditorwidget.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/ordinalrefproperty.h>
#include <inviwo/core/properties/minmaxproperty.h>
#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/util/defaultvalues.h>
#include <inviwo/core/util/stdextensions.h>
#include <inviwo/core/util/colorconversion.h>
#include <inviwo/core/datastructures/tfprimitive.h>

#include <pybind11/stl.h>
#include <pybind11/functional.h>
#include <fmt/format.h>

namespace py = pybind11;

namespace inviwo {

template <typename T, typename P, typename C>
void pyTemplateProperty(C& prop) {
    prop.def_property(
            "value", [](P& p) { return p.get(); }, [](P& p, T t) { p.set(t); })
        .def("__repr__", [](P& v) { return inviwo::toString(v.get()); });
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
            .def("__repr__", [](P& v) { return inviwo::toString(v.get()); });

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
            .def(py::init([](std::string_view identifier, std::string_view name, Document help,
                             std::function<T()> get, std::function<void(const T&)> set,
                             const std::pair<T, ConstraintBehavior>& min,
                             const std::pair<T, ConstraintBehavior>& max, const T& increment,
                             InvalidationLevel invalidationLevel, PropertySemantics semantics) {
                     return new P(identifier, name, std::move(help), std::move(get), std::move(set),
                                  min, max, increment, invalidationLevel, semantics);
                 }),
                 py::arg("identifier"), py::arg("name"), py::arg("help"), py::arg("get"),
                 py::arg("set"),
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
        prop.def(py::init([](std::string_view identifier, std::string_view name, Document help,
                             const T& valueMin, const T& valueMax, const T& rangeMin,
                             const T& rangeMax, const T& increment, const T& minSeparation,
                             InvalidationLevel invalidationLevel, PropertySemantics semantics) {
                     return new P(identifier, name, std::move(help), valueMin, valueMax, rangeMin,
                                  rangeMax, increment, minSeparation, invalidationLevel, semantics);
                 }),
                 py::arg("identifier"), py::arg("name"), py::arg("help") = Document{},
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

void exposeProperties(py::module& m) {

    py::enum_<ConstraintBehavior>(m, "ConstraintBehavior")
        .value("Editable", ConstraintBehavior::Editable)
        .value("Mutable", ConstraintBehavior::Mutable)
        .value("Immutable", ConstraintBehavior::Immutable)
        .value("Ignore", ConstraintBehavior::Ignore);

    py::enum_<InvalidationLevel>(m, "InvalidationLevel")
        .value("Valid", InvalidationLevel::Valid)
        .value("InvalidOutput", InvalidationLevel::InvalidOutput)
        .value("InvalidResources", InvalidationLevel::InvalidResources);

    auto listPropertyUIFlag = py::enum_<ListPropertyUIFlag>(m, "ListPropertyUIFlag")
                                  .value("Static", ListPropertyUIFlag::Static)
                                  .value("Add", ListPropertyUIFlag::Add)
                                  .value("Remove", ListPropertyUIFlag::Remove);
    exposeFlags<ListPropertyUIFlag>(m, listPropertyUIFlag, "ListPropertyUIFlags");

    py::class_<PropertySemantics>(m, "PropertySemantics")
        .def(py::init())
        .def(py::init<std::string>(), py::arg("semantic"))
        .def("getString", &PropertySemantics::getString)
        // clang-format off
        .def_property_readonly_static("Default", [](py::object) { return PropertySemantics::Default; })
        .def_property_readonly_static("Text", [](py::object) { return PropertySemantics::Text; })
        .def_property_readonly_static("SpinBox", [](py::object) { return PropertySemantics::SpinBox; })
        .def_property_readonly_static("Color", [](py::object) { return PropertySemantics::Color; })
        .def_property_readonly_static("LightPosition", [](py::object) { return PropertySemantics::LightPosition; })
        .def_property_readonly_static("TextEditor", [](py::object) { return PropertySemantics::TextEditor; })
        .def_property_readonly_static("Multiline", [](py::object) { return PropertySemantics::Multiline; })
        .def_property_readonly_static("ImageEditor", [](py::object) { return PropertySemantics::ImageEditor; })
        .def_property_readonly_static("ShaderEditor", [](py::object) { return PropertySemantics::ShaderEditor; })
        .def_property_readonly_static("PythonEditor", [](py::object) { return PropertySemantics::PythonEditor; })
        // clang-format on
        .def("__repr__", [](const PropertySemantics& s) {
            return fmt::format("<PropertySemantics: '{}'>", s.getString());
        });

    py::class_<PropertyFactory>(m, "PropertyFactory")
        .def("hasKey", [](PropertyFactory* pf, std::string key) { return pf->hasKey(key); })
        .def_property_readonly("keys", [](PropertyFactory* pf) { return pf->getKeys(); })
        .def("create", [](PropertyFactory* pf, std::string key) { return pf->create(key); });

    py::class_<PropertyWidget>(m, "PropertyWidget", py::multiple_inheritance{})
        .def_property_readonly("property", &PropertyWidget::getProperty,
                               py::return_value_policy::reference)

        .def("getEditorWidget", &PropertyWidget::getEditorWidget)
        .def("hasEditorWidget", &PropertyWidget::hasEditorWidget)
        .def("updateFromProperty", &PropertyWidget::updateFromProperty)

        .def("address", [](PropertyWidget* w) {
            return reinterpret_cast<std::intptr_t>(static_cast<void*>(w));
        });

    py::class_<PropertyEditorWidget>(m, "PropertyEditorWidget", py::multiple_inheritance{})
        .def_property("visible", &PropertyEditorWidget::isVisible,
                      &PropertyEditorWidget::setVisible)
        .def_property("dimensions", &PropertyEditorWidget::getDimensions,
                      &PropertyEditorWidget::setDimensions)
        .def_property("position", &PropertyEditorWidget::getPosition,
                      &PropertyEditorWidget::setPosition)
        .def("address", [](PropertyEditorWidget* w) {
            return reinterpret_cast<std::intptr_t>(static_cast<void*>(w));
        });

    py::class_<Property>(m, "Property")
        .def_property("identifier", &Property::getIdentifier, &Property::setIdentifier)
        .def_property("displayName", &Property::getDisplayName, &Property::setDisplayName)
        .def_property("readOnly", &Property::getReadOnly, &Property::setReadOnly)
        .def_property("visible", &Property::getVisible, &Property::setVisible)
        .def_property("semantics", &Property::getSemantics, &Property::setSemantics)
        .def_property(
            "help", [](const Property& p) { return p.getHelp(); }, &Property::setHelp)
        .def_property_readonly("classIdentifier", &Property::getClassIdentifier)
        .def_property_readonly("classIdentifierForWidget", &Property::getClassIdentifierForWidget)
        .def_property_readonly("path", &Property::getPath)
        .def_property("invalidationLevel", &Property::getInvalidationLevel,
                      &Property::setInvalidationLevel)
        .def_property_readonly("widgets", &Property::getWidgets)
        .def_property_readonly("isModified", &Property::isModified)
        .def("hasWidgets", &Property::hasWidgets)
        .def("setCurrentStateAsDefault", &Property::setCurrentStateAsDefault)
        .def("resetToDefaultState", &Property::resetToDefaultState)
        .def("onChange", [](Property* p, std::function<void()> func) { p->onChange(func); })
        .def("visibilityDependsOn",
             [](Property* p, Property* other, std::function<bool(Property&)> func) {
                 p->visibilityDependsOn(*other, func);
             })
        .def("readonlyDependsOn",
             [](Property* p, Property* other, std::function<bool(Property&)> func) {
                 p->readonlyDependsOn(*other, func);
             })
        .def("getHelp", static_cast<Document& (Property::*)()>(&Property::getHelp))
        .def("getDescription", &Property::getDescription);

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
    using MinMaxPropertyTypes = std::tuple<float, double, size_t, glm::i64, int>;
    using OrdinalPropetyTypes = std::tuple<float, int, size_t, glm::i64, double, vec2, vec3, vec4,
                                           dvec2, dvec3, dvec4, ivec2, ivec3, ivec4, size2_t,
                                           size3_t, size4_t, mat2, mat3, mat4, dmat2, dmat3, dmat4>;

    util::for_each_type<OrdinalPropetyTypes>{}(OrdinalPropertyHelper{}, m);
    util::for_each_type<OrdinalPropetyTypes>{}(OrdinalRefPropertyHelper{}, m);
    util::for_each_type<OptionPropertyTypes>{}(OptionPropertyHelper{}, m);
    util::for_each_type<MinMaxPropertyTypes>{}(MinMaxHelper{}, m);

    py::class_<TransferFunctionProperty, Property>(m, "TransferFunctionProperty")
        .def(py::init([](std::string_view identifier, std::string_view displayName, Document help,
                         const TransferFunction& value, VolumeInport* volumeInport,
                         InvalidationLevel invalidationLevel, PropertySemantics semantics) {
                 return new TransferFunctionProperty(identifier, displayName, std::move(help),
                                                     value, volumeInport, invalidationLevel,
                                                     semantics);
             }),
             py::arg("identifier"), py::arg("displayName"), py::arg("help"), py::arg("value"),
             py::arg("inport") = nullptr,
             py::arg("invalidationLevel") = InvalidationLevel::InvalidOutput,
             py::arg("semantics") = PropertySemantics::Default)
        .def(py::init([](std::string_view identifier, std::string_view displayName,
                         const TransferFunction& value, VolumeInport* volumeInport,
                         InvalidationLevel invalidationLevel, PropertySemantics semantics) {
                 return new TransferFunctionProperty(identifier, displayName, value, volumeInport,
                                                     invalidationLevel, semantics);
             }),
             py::arg("identifier"), py::arg("displayName"), py::arg("value"),
             py::arg("inport") = nullptr,
             py::arg("invalidationLevel") = InvalidationLevel::InvalidOutput,
             py::arg("semantics") = PropertySemantics::Default)
        .def_property("mask", &TransferFunctionProperty::getMask,
                      &TransferFunctionProperty::setMask)
        .def_property("zoomH", &TransferFunctionProperty::getZoomH,
                      &TransferFunctionProperty::setZoomH)
        .def_property("zoomV", &TransferFunctionProperty::getZoomV,
                      &TransferFunctionProperty::setZoomV)
        .def("clear", [](TransferFunctionProperty& tp) { tp.get().clear(); })
        .def_property(
            "value",
            py::cpp_function(
                [](TransferFunctionProperty& tp) -> TransferFunction& { return tp.get(); },
                py::return_value_policy::reference_internal),
            py::overload_cast<const TransferFunction&>(&TransferFunctionProperty::set))

        .def("add", [](TransferFunctionProperty& tp, double value,
                       const vec4& color) { tp.get().add(value, color); })
        .def("add", [](TransferFunctionProperty& tp, const dvec2& pos) { tp.get().add(pos); })
        .def("add", [](TransferFunctionProperty& tp, const TFPrimitiveData& v) { tp.get().add(v); })
        .def("add", [](TransferFunctionProperty& tp,
                       const std::vector<TFPrimitiveData>& values) { tp.get().add(values); })
        .def("setValues",
             [](TransferFunctionProperty& tp, const std::vector<TFPrimitiveData>& values) {
                 tp.get().clear();
                 tp.get().add(values);
             })
        .def("getValues",
             [](TransferFunctionProperty& tp) -> std::vector<TFPrimitiveData> {
                 return tp.get().get();
             })
        .def("__repr__", [](const TransferFunctionProperty& tp) {
            std::ostringstream oss;
            oss << "<TransferFunctionProperty:  " << tp.get().size() << " TF points";
            for (auto& p : tp.get()) {
                oss << "\n    " << p.getPosition() << ", " << color::rgba2hex(p.getColor());
            }
            oss << ">";
            return oss.str();
        });

    py::class_<IsoValueProperty, Property>(m, "IsoValueProperty")
        .def(py::init([](std::string_view identifier, std::string_view displayName, Document help,
                         const IsoValueCollection& value, VolumeInport* volumeInport,
                         InvalidationLevel invalidationLevel, PropertySemantics semantics) {
                 return new IsoValueProperty(identifier, displayName, std::move(help), value,
                                             volumeInport, invalidationLevel, semantics);
             }),
             py::arg("identifier"), py::arg("displayName"), py::arg("help"), py::arg("value"),
             py::arg("inport") = nullptr,
             py::arg("invalidationLevel") = InvalidationLevel::InvalidOutput,
             py::arg("semantics") = PropertySemantics::Default)
        .def(py::init([](std::string_view identifier, std::string_view displayName,
                         const IsoValueCollection& value, VolumeInport* volumeInport,
                         InvalidationLevel invalidationLevel, PropertySemantics semantics) {
                 return new IsoValueProperty(identifier, displayName, value, volumeInport,
                                             invalidationLevel, semantics);
             }),
             py::arg("identifier"), py::arg("displayName"), py::arg("value"),
             py::arg("inport") = nullptr,
             py::arg("invalidationLevel") = InvalidationLevel::InvalidOutput,
             py::arg("semantics") = PropertySemantics::Default)
        .def_property("zoomH", &IsoValueProperty::getZoomH, &IsoValueProperty::setZoomH)
        .def_property("zoomV", &IsoValueProperty::getZoomV, &IsoValueProperty::setZoomV)
        .def("clear", [](IsoValueProperty& ivp) { ivp.get().clear(); })
        .def_property(
            "value",
            py::cpp_function([](IsoValueProperty& tp) -> IsoValueCollection& { return tp.get(); },
                             py::return_value_policy::reference_internal),
            py::overload_cast<const IsoValueCollection&>(&IsoValueProperty::set))
        .def("add", [](IsoValueProperty& ivp, double value,
                       const vec4& color) { ivp.get().add(value, color); })
        .def("add", [](IsoValueProperty& ivp, const dvec2& pos) { ivp.get().add(pos); })
        .def("add", [](IsoValueProperty& ivp, const TFPrimitiveData& v) { ivp.get().add(v); })
        .def("add", [](IsoValueProperty& ivp,
                       const std::vector<TFPrimitiveData>& values) { ivp.get().add(values); })
        .def("setValues",
             [](IsoValueProperty& ivp, const std::vector<TFPrimitiveData>& values) {
                 ivp.get().clear();
                 ivp.get().add(values);
             })
        .def("getValues",
             [](IsoValueProperty& ivp) -> std::vector<TFPrimitiveData> { return ivp.get().get(); })
        .def("__repr__", [](const IsoValueProperty& ivp) {
            std::ostringstream oss;
            oss << "<IsoValueProperty:  " << ivp.get().size() << " isovalues";
            for (auto& p : ivp.get()) {
                oss << "\n    " << p.getPosition() << ", " << color::rgba2hex(p.getColor());
            }
            oss << ">";
            return oss.str();
        });

    py::class_<StringProperty, Property> strProperty(m, "StringProperty");
    strProperty
        .def(py::init([](std::string_view identifier, std::string_view displayName, Document help,
                         std::string_view value, InvalidationLevel invalidationLevel,
                         PropertySemantics semantics) {
                 return new StringProperty(identifier, displayName, std::move(help), value,
                                           invalidationLevel, semantics);
             }),
             py::arg("identifier"), py::arg("displayName"), py::arg("help"), py::arg("value") = "",
             py::arg("invalidationLevel") = InvalidationLevel::InvalidOutput,
             py::arg("semantics") = PropertySemantics::Default)
        .def(py::init([](std::string_view identifier, std::string_view displayName,
                         std::string_view value, InvalidationLevel invalidationLevel,
                         PropertySemantics semantics) {
                 return new StringProperty(identifier, displayName, value, invalidationLevel,
                                           semantics);
             }),
             py::arg("identifier"), py::arg("displayName"), py::arg("value") = "",
             py::arg("invalidationLevel") = InvalidationLevel::InvalidOutput,
             py::arg("semantics") = PropertySemantics::Default);
    pyTemplateProperty<std::string, StringProperty>(strProperty);

    py::enum_<AcceptMode>(m, "AcceptMode")
        .value("Open", AcceptMode::Open)
        .value("Save", AcceptMode::Save);

    py::enum_<FileMode>(m, "FileMode")
        .value("AnyFile", FileMode::AnyFile)
        .value("ExistingFile", FileMode::ExistingFile)
        .value("Directory", FileMode::Directory)
        .value("ExistingFiles", FileMode::ExistingFiles)
        .value("DirectoryOnly", FileMode::DirectoryOnly);

    py::class_<FileExtension>(m, "FileExtension")
        .def(py::init<>())
        .def(py::init<std::string, std::string>(), py::arg("ext"), py::arg("desc"))
        .def("toString", &FileExtension::toString)
        .def("empty", &FileExtension::empty)
        .def("matchesAll", &FileExtension::matchesAll)
        .def("matches", &FileExtension::matches)
        .def_static("all", &FileExtension::all)
        .def_readwrite("extension", &FileExtension::extension_)
        .def_readwrite("description", &FileExtension::description_);

    py::class_<FileProperty, Property> fileProperty(m, "FileProperty");
    fileProperty
        .def(py::init([](std::string_view identifier, std::string_view displayName, Document help,
                         std::string_view value, std::string_view contentType,
                         InvalidationLevel invalidationLevel, PropertySemantics semantics) {
                 return new FileProperty(identifier, displayName, std::move(help), value,
                                         contentType, invalidationLevel, semantics);
             }),
             py::arg("identifier"), py::arg("displayName"), py::arg("help") = Document{},
             py::arg("value") = "", py::arg("contentType") = "default",
             py::arg("invalidationLevel") = InvalidationLevel::InvalidOutput,
             py::arg("semantics") = PropertySemantics::Default)
        .def(py::init([](std::string_view identifier, std::string_view displayName,
                         std::string_view value, std::string_view contentType,
                         InvalidationLevel invalidationLevel, PropertySemantics semantics) {
                 return new FileProperty(identifier, displayName, value, contentType,
                                         invalidationLevel, semantics);
             }),
             py::arg("identifier"), py::arg("displayName"), py::arg("value") = "",
             py::arg("contentType") = "default",
             py::arg("invalidationLevel") = InvalidationLevel::InvalidOutput,
             py::arg("semantics") = PropertySemantics::Default)
        .def("requestFile", &FileProperty::requestFile)
        .def("addNameFilter",
             static_cast<void (FileProperty::*)(std::string_view)>(&FileProperty::addNameFilter))
        .def("addNameFilter",
             static_cast<void (FileProperty::*)(FileExtension)>(&FileProperty::addNameFilter))
        .def("clearNameFilters", &FileProperty::clearNameFilters)

        .def("getNameFilters", &FileProperty::getNameFilters)
        .def_property("acceptMode", &FileProperty::getAcceptMode, &FileProperty::setAcceptMode)
        .def_property("fileMode", &FileProperty::getFileMode, &FileProperty::setFileMode)
        .def_property("contentType", &FileProperty::getContentType, &FileProperty::setContentType)
        .def_property("selectedExtension", &FileProperty::getSelectedExtension,
                      &FileProperty::setSelectedExtension);
    pyTemplateProperty<std::string, FileProperty>(fileProperty);

    py::class_<DirectoryProperty, FileProperty> dirProperty(m, "DirectoryProperty");
    dirProperty
        .def(py::init([](std::string_view identifier, std::string_view displayName, Document help,
                         std::string_view value, std::string_view contentType,
                         InvalidationLevel invalidationLevel, PropertySemantics semantics) {
                 return new DirectoryProperty(identifier, displayName, std::move(help), value,
                                              contentType, invalidationLevel, semantics);
             }),
             py::arg("identifier"), py::arg("displayName"), py::arg("help"), py::arg("value") = "",
             py::arg("contentType") = "default",
             py::arg("invalidationLevel") = InvalidationLevel::InvalidOutput,
             py::arg("semantics") = PropertySemantics::Default)
        .def(py::init([](std::string_view identifier, std::string_view displayName,
                         std::string_view value, std::string_view contentType,
                         InvalidationLevel invalidationLevel, PropertySemantics semantics) {
                 return new DirectoryProperty(identifier, displayName, value, contentType,
                                              invalidationLevel, semantics);
             }),
             py::arg("identifier"), py::arg("displayName"), py::arg("value") = "",
             py::arg("contentType") = "default",
             py::arg("invalidationLevel") = InvalidationLevel::InvalidOutput,
             py::arg("semantics") = PropertySemantics::Default);
    pyTemplateProperty<std::string, DirectoryProperty>(dirProperty);

    py::class_<BoolProperty, Property> boolProperty(m, "BoolProperty");
    boolProperty
        .def(py::init([](std::string_view identifier, std::string_view displayName, Document help,
                         bool value, InvalidationLevel invalidationLevel,
                         PropertySemantics semantics) {
                 return new BoolProperty(identifier, displayName, std::move(help), value,
                                         invalidationLevel, semantics);
             }),
             py::arg("identifier"), py::arg("displayName"), py::arg("help") = Document{},
             py::arg("value") = false,
             py::arg("invalidationLevel") = InvalidationLevel::InvalidOutput,
             py::arg("semantics") = PropertySemantics::Default)
        .def(py::init([](std::string_view identifier, std::string_view displayName, bool value,
                         InvalidationLevel invalidationLevel, PropertySemantics semantics) {
                 return new BoolProperty(identifier, displayName, value, invalidationLevel,
                                         semantics);
             }),
             py::arg("identifier"), py::arg("displayName"), py::arg("value") = false,
             py::arg("invalidationLevel") = InvalidationLevel::InvalidOutput,
             py::arg("semantics") = PropertySemantics::Default)
        .def("__bool__", &BoolProperty::get);

    pyTemplateProperty<bool, BoolProperty>(boolProperty);

    py::class_<ButtonProperty, Property>(m, "ButtonProperty")
        .def(py::init([](std::string_view identifier, std::string_view displayName,
                         InvalidationLevel invalidationLevel, PropertySemantics semantics) {
                 return new ButtonProperty(identifier, displayName, invalidationLevel, semantics);
             }),
             py::arg("identifier"), py::arg("displayName"),
             py::arg("invalidationLevel") = InvalidationLevel::InvalidOutput,
             py::arg("semantics") = PropertySemantics::Default)
        .def(py::init([](std::string_view identifier, std::string_view displayName, Document help,
                         std::function<void()> action, InvalidationLevel invalidationLevel,
                         PropertySemantics semantics) {
                 return new ButtonProperty(identifier, displayName, std::move(help), action,
                                           invalidationLevel, semantics);
             }),
             py::arg("identifier"), py::arg("displayName"), py::arg("help"), py::arg("action"),
             py::arg("invalidationLevel") = InvalidationLevel::InvalidOutput,
             py::arg("semantics") = PropertySemantics::Default)
        .def(py::init([](std::string_view identifier, std::string_view displayName,
                         std::function<void()> action, InvalidationLevel invalidationLevel,
                         PropertySemantics semantics) {
                 return new ButtonProperty(identifier, displayName, action, invalidationLevel,
                                           semantics);
             }),
             py::arg("identifier"), py::arg("displayName"), py::arg("action"),
             py::arg("invalidationLevel") = InvalidationLevel::InvalidOutput,
             py::arg("semantics") = PropertySemantics::Default)
        .def("press", &ButtonProperty::pressButton);

    py::class_<ButtonGroupProperty::Button>(m, "ButtonGroupPropertyButton")
        .def(py::init<std::optional<std::string>, std::optional<std::string>,
                      std::optional<std::string>, std::function<void()>>());

    py::class_<ButtonGroupProperty, Property>(m, "ButtonGroupProperty")
        .def(py::init([](std::string_view identifier, std::string_view displayName,
                         InvalidationLevel invalidationLevel, PropertySemantics semantics) {
                 return new ButtonGroupProperty(identifier, displayName, invalidationLevel,
                                                semantics);
             }),
             py::arg("identifier"), py::arg("displayName"),
             py::arg("invalidationLevel") = InvalidationLevel::InvalidOutput,
             py::arg("semantics") = PropertySemantics::Default)
        .def(py::init([](std::string_view identifier, std::string_view displayName, Document help,
                         std::vector<ButtonGroupProperty::Button> buttons,
                         InvalidationLevel invalidationLevel, PropertySemantics semantics) {
                 return new ButtonGroupProperty(identifier, displayName, std::move(help),
                                                std::move(buttons), invalidationLevel, semantics);
             }),
             py::arg("identifier"), py::arg("displayName"), py::arg("help"), py::arg("buttons"),
             py::arg("invalidationLevel") = InvalidationLevel::InvalidOutput,
             py::arg("semantics") = PropertySemantics::Default)
        .def(py::init([](std::string_view identifier, std::string_view displayName,
                         std::vector<ButtonGroupProperty::Button> buttons,
                         InvalidationLevel invalidationLevel, PropertySemantics semantics) {
                 return new ButtonGroupProperty(identifier, displayName, std::move(buttons),
                                                invalidationLevel, semantics);
             }),
             py::arg("identifier"), py::arg("displayName"), py::arg("buttons"),
             py::arg("invalidationLevel") = InvalidationLevel::InvalidOutput,
             py::arg("semantics") = PropertySemantics::Default)
        .def("press", &ButtonGroupProperty::pressButton);
}

}  // namespace inviwo
