/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2019 Inviwo Foundation
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
#include <inviwo/core/properties/propertyfactory.h>

#include <inviwopy/inviwopy.h>

#include <inviwo/core/properties/cameraproperty.h>
#include <inviwo/core/properties/buttonproperty.h>
#include <inviwo/core/properties/buttongroupproperty.h>
#include <inviwo/core/properties/transferfunctionproperty.h>
#include <inviwo/core/properties/isovalueproperty.h>
#include <inviwo/core/properties/isotfproperty.h>
#include <inviwo/core/properties/stringproperty.h>
#include <inviwo/core/properties/fileproperty.h>
#include <inviwo/core/properties/directoryproperty.h>
#include <inviwo/core/properties/filepatternproperty.h>
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/properties/propertyeditorwidget.h>

#include <inviwo/core/util/stdextensions.h>
#include <inviwo/core/util/colorconversion.h>
#include <inviwo/core/datastructures/tfprimitive.h>
//#include <inviwo/core/datastructures/tfprimitiveset.h>

#include <pybind11/functional.h>

#include <fmt/format.h>

namespace py = pybind11;

namespace inviwo {

template <typename P, typename... Extra>
using PyPropertyClass = py::class_<P, Extra..., PropertyPtr<P>>;

void exposeProperties(py::module &m) {

    py::enum_<InvalidationLevel>(m, "InvalidationLevel")
        .value("Valid", InvalidationLevel::Valid)
        .value("InvalidOutput", InvalidationLevel::InvalidOutput)
        .value("InvalidResources", InvalidationLevel::InvalidResources);

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
        .def("__repr__", [](const PropertySemantics &s) {
            return fmt::format("<PropertySemantics: '{}'>", s.getString());
        });

    py::class_<PropertyFactory>(m, "PropertyFactory")
        .def("hasKey", [](PropertyFactory *pf, std::string key) { return pf->hasKey(key); })
        .def_property_readonly("keys", [](PropertyFactory *pf) { return pf->getKeys(); })
        .def("create", [](PropertyFactory *pf, std::string key) { return pf->create(key); });

    py::class_<PropertyWidget>(m, "PropertyWidget")
        .def_property_readonly("editorWidget", &PropertyWidget::getEditorWidget,
                               py::return_value_policy::reference)
        .def_property_readonly("property", &PropertyWidget::getProperty,
                               py::return_value_policy::reference);

    py::class_<PropertyEditorWidget>(m, "PropertyEditorWidget")
        .def_property("visible", &PropertyEditorWidget::isVisible,
                      &PropertyEditorWidget::setVisible)
        .def_property("dimensions", &PropertyEditorWidget::getDimensions,
                      &PropertyEditorWidget::setDimensions)
        .def_property("position", &PropertyEditorWidget::getPosition,
                      &PropertyEditorWidget::setPosition);

    PyPropertyClass<Property>(m, "Property")
        .def_property("identifier", &Property::getIdentifier, &Property::setIdentifier)
        .def_property("displayName", &Property::getDisplayName, &Property::setDisplayName)
        .def_property("readOnly", &Property::getReadOnly, &Property::setReadOnly)
        .def_property("visible", &Property::getVisible, &Property::setVisible)
        .def_property("semantics", &Property::getSemantics, &Property::setSemantics)
        .def_property_readonly("classIdentifierForWidget", &Property::getClassIdentifierForWidget)
        .def_property_readonly("path", &Property::getPath)
        .def_property_readonly("invalidationLevel", &Property::getInvalidationLevel)
        .def_property_readonly("widgets", &Property::getWidgets)
        .def_property_readonly("isModified", &Property::isModified)
        .def("hasWidgets", &Property::hasWidgets)
        .def("setCurrentStateAsDefault", &Property::setCurrentStateAsDefault)
        .def("resetToDefaultState", &Property::resetToDefaultState)
        .def("onChange", [](Property *p, std::function<void()> func) { p->onChange(func); });

    PyPropertyClass<CompositeProperty, Property, PropertyOwner>(m, "CompositeProperty")
        .def(py::init([](const std::string &identifier, const std::string &displayName,
                         InvalidationLevel invalidationLevel, PropertySemantics semantics) {
                 return new CompositeProperty(identifier, displayName, invalidationLevel,
                                              semantics);
             }),
             py::arg("identifier"), py::arg("displayName"),
             py::arg("invalidationLevel") = InvalidationLevel::InvalidResources,
             py::arg("semantics") = PropertySemantics::Default)
        .def("setCollapsed", &CompositeProperty::setCollapsed)
        .def("isCollapsed", &CompositeProperty::isCollapsed)
        .def("__getattr__",
             [](PropertyOwner &po, const std::string &key) {
                 if (auto prop = po.getPropertyByIdentifier(key)) {
                     return prop;
                 } else {
                     throw py::attribute_error{
                         "CompositeProperty (" + joinString(po.getPath(), ".") +
                         ") does not have a property with identifier: '" + key + "'"};
                 }
             },
             py::return_value_policy::reference);

    PyPropertyClass<BaseOptionProperty, Property>(m, "BaseOptionProperty")
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
        .def_property_readonly("displayName", &BaseOptionProperty::getDisplayNames);

    using OptionPropetyTypes = std::tuple<double, float, int, std::string>;
    using MinMaxPropertyTypes = std::tuple<float, double, size_t, glm::i64, int>;
    using OrdinalPropetyTypes = std::tuple<float, int, size_t, glm::i64, double, vec2, vec3, vec4,
                                           dvec2, dvec3, dvec4, ivec2, ivec3, ivec4, size2_t,
                                           size3_t, size4_t, mat2, mat3, mat4, dmat2, dmat3, dmat4>;

    util::for_each_type<OrdinalPropetyTypes>{}(OrdinalPropertyHelper{}, m);
    util::for_each_type<OptionPropetyTypes>{}(OptionPropertyHelper{}, m);
    util::for_each_type<MinMaxPropertyTypes>{}(MinMaxHelper{}, m);

    PyPropertyClass<CameraProperty, CompositeProperty>(m, "CameraProperty")
        .def(py::init([](const std::string &identifier, const std::string &displayName, vec3 eye,
                         vec3 center, vec3 lookUp, Inport *inport,
                         InvalidationLevel invalidationLevel, PropertySemantics semantics) {
                 return new CameraProperty(identifier, displayName, eye, center, lookUp, inport,
                                           invalidationLevel, semantics);
             }),
             py::arg("identifier"), py::arg("displayName"), py::arg("eye") = vec3(0.0f, 0.0f, 2.0f),
             py::arg("center") = vec3(0.0f), py::arg("lookUp") = vec3(0.0f, 1.0f, 0.0f),
             py::arg("inport") = nullptr,
             py::arg("invalidationLevel") = InvalidationLevel::InvalidResources,
             py::arg("semantics") = PropertySemantics::Default)
        .def_property("lookFrom", &CameraProperty::getLookFrom, &CameraProperty::setLookFrom,
                      py::return_value_policy::copy)
        .def_property("lookTo", &CameraProperty::getLookTo, &CameraProperty::setLookTo,
                      py::return_value_policy::copy)
        .def_property("lookUp", &CameraProperty::getLookUp, &CameraProperty::setLookUp,
                      py::return_value_policy::copy)
        .def_property_readonly("lookRight", &CameraProperty::getLookRight)
        .def_property("aspectRatio", &CameraProperty::getAspectRatio,
                      &CameraProperty::setAspectRatio)
        .def_property("nearPlane", &CameraProperty::getNearPlaneDist,
                      &CameraProperty::setNearPlaneDist)
        .def_property("farPlane", &CameraProperty::getFarPlaneDist,
                      &CameraProperty::setFarPlaneDist)
        .def("setLook", &CameraProperty::setLook)
        .def_property_readonly("lookFromMinValue", &CameraProperty::getLookFromMinValue)
        .def_property_readonly("lookFromMaxValue", &CameraProperty::getLookFromMaxValue)
        .def_property_readonly("lookToMinValue", &CameraProperty::getLookToMinValue)
        .def_property_readonly("lookToMaxValue", &CameraProperty::getLookToMaxValue)
        .def("getWorldPosFromNormalizedDeviceCoords",
             &CameraProperty::getWorldPosFromNormalizedDeviceCoords)
        .def("getClipPosFromNormalizedDeviceCoords",
             &CameraProperty::getClipPosFromNormalizedDeviceCoords)
        .def("getNormalizedDeviceFromNormalizedScreenAtFocusPointDepth",
             &CameraProperty::getNormalizedDeviceFromNormalizedScreenAtFocusPointDepth)
        .def_property_readonly("viewMatrix", &CameraProperty::viewMatrix)
        .def_property_readonly("projectionMatrix", &CameraProperty::projectionMatrix)
        .def_property_readonly("inverseViewMatrix", &CameraProperty::inverseViewMatrix)
        .def_property_readonly("inverseProjectionMatrix", &CameraProperty::inverseProjectionMatrix);

    PyPropertyClass<TransferFunctionProperty>(m, "TransferFunctionProperty")
        .def(py::init([](const std::string &identifier, const std::string &displayName,
                         const TransferFunction &value, VolumeInport *volumeInport,
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
        .def("save",
             [](TransferFunctionProperty *tf, std::string filename) { tf->get().save(filename); })
        .def("load",
             [](TransferFunctionProperty *tf, std::string filename) { tf->get().load(filename); })
        .def("clear", [](TransferFunctionProperty &tp) { tp.get().clear(); })
        .def_property(
            "value",
            py::cpp_function(
                [](TransferFunctionProperty &tp) -> TransferFunction & { return tp.get(); },
                py::return_value_policy::reference_internal),
            py::overload_cast<const TransferFunction &>(&TransferFunctionProperty::set))

        .def("add", [](TransferFunctionProperty &tp, double value,
                       const vec4 &color) { tp.get().add(value, color); })
        .def("add", [](TransferFunctionProperty &tp, const dvec2 &pos) { tp.get().add(pos); })
        .def("add", [](TransferFunctionProperty &tp, const TFPrimitiveData &v) { tp.get().add(v); })
        .def("add", [](TransferFunctionProperty &tp,
                       const std::vector<TFPrimitiveData> &values) { tp.get().add(values); })
        .def("setValues",
             [](TransferFunctionProperty &tp, const std::vector<TFPrimitiveData> &values) {
                 tp.get().clear();
                 tp.get().add(values);
             })
        .def("getValues",
             [](TransferFunctionProperty &tp) -> std::vector<TFPrimitiveData> {
                 return tp.get().get();
             })
        .def("__repr__", [](const TransferFunctionProperty &tp) {
            std::ostringstream oss;
            oss << "<TransferFunctionProperty:  " << tp.get().size() << " TF points";
            for (auto &p : tp.get()) {
                oss << "\n    " << p.getPosition() << ", " << color::rgba2hex(p.getColor());
            }
            oss << ">";
            return oss.str();
        });

    PyPropertyClass<IsoValueProperty>(m, "IsoValueProperty")
        .def(py::init([](const std::string &identifier, const std::string &displayName,
                         const IsoValueCollection &value, VolumeInport *volumeInport,
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
        .def("save", [](IsoValueProperty *ivp, std::string filename) { ivp->get().save(filename); })
        .def("load", [](IsoValueProperty *ivp, std::string filename) { ivp->get().load(filename); })
        .def("clear", [](IsoValueProperty &ivp) { ivp.get().clear(); })
        .def_property(
            "value",
            py::cpp_function([](IsoValueProperty &tp) -> IsoValueCollection & { return tp.get(); },
                             py::return_value_policy::reference_internal),
            py::overload_cast<const IsoValueCollection &>(&IsoValueProperty::set))
        .def("add", [](IsoValueProperty &ivp, double value,
                       const vec4 &color) { ivp.get().add(value, color); })
        .def("add", [](IsoValueProperty &ivp, const dvec2 &pos) { ivp.get().add(pos); })
        .def("add", [](IsoValueProperty &ivp, const TFPrimitiveData &v) { ivp.get().add(v); })
        .def("add", [](IsoValueProperty &ivp,
                       const std::vector<TFPrimitiveData> &values) { ivp.get().add(values); })
        .def("setValues",
             [](IsoValueProperty &ivp, const std::vector<TFPrimitiveData> &values) {
                 ivp.get().clear();
                 ivp.get().add(values);
             })
        .def("getValues",
             [](IsoValueProperty &ivp) -> std::vector<TFPrimitiveData> { return ivp.get().get(); })
        .def("__repr__", [](const IsoValueProperty &ivp) {
            std::ostringstream oss;
            oss << "<IsoValueProperty:  " << ivp.get().size() << " isovalues";
            for (auto &p : ivp.get()) {
                oss << "\n    " << p.getPosition() << ", " << color::rgba2hex(p.getColor());
            }
            oss << ">";
            return oss.str();
        });

    PyPropertyClass<IsoTFProperty>(m, "IsoTFProperty")
        .def(py::init([](const std::string &identifier, const std::string &displayName,
                         const IsoValueCollection &isovalues, const TransferFunction &tf,
                         VolumeInport *volumeInport, InvalidationLevel invalidationLevel,
                         PropertySemantics semantics) {
                 return new IsoTFProperty(identifier, displayName, isovalues, tf, volumeInport,
                                          invalidationLevel, semantics);
             }),
             py::arg("identifier"), py::arg("displayName"), py::arg("isovalues"), py::arg("tf"),
             py::arg("inport") = nullptr,
             py::arg("invalidationLevel") = InvalidationLevel::InvalidResources,
             py::arg("semantics") = PropertySemantics::Default)
        .def(py::init([](const std::string &identifier, const std::string &displayName,
                         VolumeInport *volumeInport, InvalidationLevel invalidationLevel,
                         PropertySemantics semantics) {
                 return new IsoTFProperty(identifier, displayName, volumeInport, invalidationLevel,
                                          semantics);
             }),
             py::arg("identifier"), py::arg("displayName"), py::arg("inport"),
             py::arg("invalidationLevel") = InvalidationLevel::InvalidResources,
             py::arg("semantics") = PropertySemantics::Default)
        .def_property_readonly(
            "isovalues",
            py::cpp_function([](IsoTFProperty &tp) -> IsoValueProperty & { return tp.isovalues_; },
                             py::return_value_policy::reference_internal))
        .def_property_readonly(
            "tf",
            py::cpp_function([](IsoTFProperty &tp) -> TransferFunctionProperty & { return tp.tf_; },
                             py::return_value_policy::reference_internal))
        .def_property("mask", &IsoTFProperty::getMask, &IsoTFProperty::setMask)
        .def_property("zoomH", &IsoTFProperty::getZoomH, &IsoTFProperty::setZoomH)
        .def_property("zoomV", &IsoTFProperty::getZoomV, &IsoTFProperty::setZoomV);

    PyPropertyClass<StringProperty, Property> strProperty(m, "StringProperty");
    strProperty.def(py::init([](const std::string &identifier, const std::string &displayName,
                                const std::string &value, InvalidationLevel invalidationLevel,
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

    PyPropertyClass<FileProperty, Property> fileProperty(m, "FileProperty");
    fileProperty
        .def(py::init([](const std::string &identifier, const std::string &displayName,
                         const std::string &value, const std::string &contentType,
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
             static_cast<void (FileProperty::*)(std::string)>(&FileProperty::addNameFilter))
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

    PyPropertyClass<DirectoryProperty, FileProperty> dirProperty(m, "DirectoryProperty");
    dirProperty.def(py::init([](const std::string &identifier, const std::string &displayName,
                                const std::string &value, const std::string &contentType,
                                InvalidationLevel invalidationLevel, PropertySemantics semantics) {
                        return new DirectoryProperty(identifier, displayName, value, contentType,
                                                     invalidationLevel, semantics);
                    }),
                    py::arg("identifier"), py::arg("displayName"), py::arg("value") = "",
                    py::arg("contentType") = "default",
                    py::arg("invalidationLevel") = InvalidationLevel::InvalidOutput,
                    py::arg("semantics") = PropertySemantics::Default);
    pyTemplateProperty<std::string, DirectoryProperty>(dirProperty);

    PyPropertyClass<FilePatternProperty, CompositeProperty>(m, "FilePatternProperty")
        .def(py::init([](const std::string &identifier, const std::string &displayName,
                         const std::string &pattern, const std::string &directory,
                         InvalidationLevel invalidationLevel, PropertySemantics semantics) {
                 return new FilePatternProperty(identifier, displayName, pattern, directory,
                                                invalidationLevel, semantics);
             }),
             py::arg("identifier"), py::arg("displayName"), py::arg("pattern") = "",
             py::arg("directory") = "",
             py::arg("invalidationLevel") = InvalidationLevel::InvalidOutput,
             py::arg("semantics") = PropertySemantics::Default)
        .def_property_readonly("filePattern", &FilePatternProperty::getFilePattern)
        .def_property_readonly("filePatternPath", &FilePatternProperty::getFilePatternPath)
        .def_property_readonly("fileList", &FilePatternProperty::getFileList)
        .def_property_readonly("fileIndices", &FilePatternProperty::getFileIndices)
        .def_property_readonly("outOfRangeMatches", &FilePatternProperty::hasOutOfRangeMatches)
        .def_property_readonly("rangeSelection", &FilePatternProperty::hasRangeSelection)
        .def_property_readonly("range",
                               [](FilePatternProperty *p) {
                                   return std::make_tuple(p->getMinRange(), p->getMaxRange());
                               })
        .def_property("selectedExtension", &FilePatternProperty::getSelectedExtension,
                      &FilePatternProperty::setSelectedExtension)
        .def("addNameFilter", static_cast<void (FilePatternProperty::*)(std::string)>(
                                  &FilePatternProperty::addNameFilter))
        .def("addNameFilter", static_cast<void (FilePatternProperty::*)(FileExtension)>(
                                  &FilePatternProperty::addNameFilter))
        .def("clearNameFilters", &FilePatternProperty::clearNameFilters);

    PyPropertyClass<BoolProperty, Property> boolProperty(m, "BoolProperty");
    boolProperty.def(
        py::init([](const std::string &identifier, const std::string &displayName, bool value,
                    InvalidationLevel invalidationLevel, PropertySemantics semantics) {
            return new BoolProperty(identifier, displayName, value, invalidationLevel, semantics);
        }),
        py::arg("identifier"), py::arg("displayName"), py::arg("value") = false,
        py::arg("invalidationLevel") = InvalidationLevel::InvalidOutput,
        py::arg("semantics") = PropertySemantics::Default);
    pyTemplateProperty<bool, BoolProperty>(boolProperty);

    PyPropertyClass<ButtonProperty, Property>(m, "ButtonProperty")
        .def(py::init([](const std::string &identifier, const std::string &displayName,
                         InvalidationLevel invalidationLevel, PropertySemantics semantics) {
                 return new ButtonProperty(identifier, displayName, invalidationLevel, semantics);
             }),
             py::arg("identifier"), py::arg("displayName"),
             py::arg("invalidationLevel") = InvalidationLevel::InvalidOutput,
             py::arg("semantics") = PropertySemantics::Default)
        .def(py::init([](const std::string &identifier, const std::string &displayName,
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

    PyPropertyClass<ButtonGroupProperty, Property>(m, "ButtonGroupProperty")
        .def(py::init([](const std::string &identifier, const std::string &displayName,
                         InvalidationLevel invalidationLevel, PropertySemantics semantics) {
                 return new ButtonGroupProperty(identifier, displayName, invalidationLevel,
                                                semantics);
             }),
             py::arg("identifier"), py::arg("displayName"),
             py::arg("invalidationLevel") = InvalidationLevel::InvalidOutput,
             py::arg("semantics") = PropertySemantics::Default)
        .def(py::init([](const std::string &identifier, const std::string &displayName,
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
