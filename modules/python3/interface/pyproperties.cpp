/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2018 Inviwo Foundation
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

#include <modules/python3/interface/pyproperties.h>
#include <inviwo/core/properties/propertyfactory.h>

#include <modules/python3/interface/inviwopy.h>

#include <inviwo/core/properties/cameraproperty.h>
#include <inviwo/core/properties/buttonproperty.h>
#include <inviwo/core/properties/transferfunctionproperty.h>
#include <inviwo/core/properties/stringproperty.h>
#include <inviwo/core/properties/fileproperty.h>
#include <inviwo/core/properties/directoryproperty.h>
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/properties/propertyeditorwidget.h>

#include <inviwo/core/util/stdextensions.h>

#include <pybind11/functional.h>

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
        .def(py::init<std::string>())
        .def("getString", &PropertySemantics::getString);

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
        .def_property("semantics", &Property::getSemantics, &Property::setSemantics)
        .def_property_readonly("classIdentifierForWidget", &Property::getClassIdentifierForWidget)
        .def_property_readonly("path", &Property::getPath)
        .def_property_readonly("invalidationLevel", &Property::getInvalidationLevel)
        .def_property_readonly("widgets", &Property::getWidgets)
        .def("hasWidgets", &Property::hasWidgets)
        .def("setCurrentStateAsDefault", &Property::setCurrentStateAsDefault)
        .def("resetToDefaultState", &Property::resetToDefaultState)
        .def("onChange",
             [](Property *p, std::function<void()> func) {
                 p->onChange(func);
             });

    PyPropertyClass<CompositeProperty, Property, PropertyOwner>(m, "CompositeProperty");

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
        .def_property_readonly("inverseProjectionMatrix", &CameraProperty::inverseProjectionMatrix)
        .def("adjustCameraToData", &CameraProperty::adjustCameraToData)
        .def("resetAdjustCameraToData", &CameraProperty::resetAdjustCameraToData);

    PyPropertyClass<TransferFunctionProperty>(m, "TransferFunctionProperty")
        .def(py::init([](const std::string &identifier, const std::string &displayName,
                         const TransferFunction &value, VolumeInport *volumeInport,
                         InvalidationLevel invalidationLevel, PropertySemantics semantics) {
                 return new TransferFunctionProperty(identifier, displayName, value, volumeInport,
                                                     invalidationLevel, semantics);
             }),
             py::arg("identifier"), py::arg("displayName"),
             py::arg("value"),
             py::arg("inport") = nullptr,
             py::arg("invalidationLevel") = InvalidationLevel::InvalidResources,
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
        .def("clear", [](TransferFunctionProperty &tp) { tp.get().clearPoints(); })
        .def("addPoint", [](TransferFunctionProperty &tp, vec2 pos,
                            vec3 color) { tp.get().addPoint(pos.x, vec4(color, pos.y)); })
        .def("addPoint", [](TransferFunctionProperty &tp, float pos, vec4 color) {
            tp.get().addPoint(pos, color);
        });

    PyPropertyClass<StringProperty, Property> strProperty(m, "StringProperty");
    strProperty.def(py::init([](const std::string &identifier, const std::string &displayName,
                                const std::string &value, InvalidationLevel invalidationLevel,
                                PropertySemantics semantics) {
                        return new StringProperty(identifier, displayName, value, invalidationLevel,
                                                  semantics);
                    }),
                    py::arg("identifier"), py::arg("displayName"), py::arg("value") = "",
                    py::arg("invalidationLevel") = InvalidationLevel::InvalidResources,
                    py::arg("semantics") = PropertySemantics::Default);
    pyTemplateProperty<std::string, StringProperty>(strProperty);

    PyPropertyClass<FileProperty, Property> fileProperty(m, "FileProperty");
    fileProperty.def(py::init([](const std::string &identifier, const std::string &displayName,
                                 const std::string &value, const std::string &contentType,
                                 InvalidationLevel invalidationLevel, PropertySemantics semantics) {
                         return new FileProperty(identifier, displayName, value, contentType,
                                                 invalidationLevel, semantics);
                     }),
                     py::arg("identifier"), py::arg("displayName"), py::arg("value") = "",
                     py::arg("contentType") = "default",
                     py::arg("invalidationLevel") = InvalidationLevel::InvalidResources,
                     py::arg("semantics") = PropertySemantics::Default);
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
                    py::arg("invalidationLevel") = InvalidationLevel::InvalidResources,
                    py::arg("semantics") = PropertySemantics::Default);
    pyTemplateProperty<std::string, DirectoryProperty>(dirProperty);

    PyPropertyClass<BoolProperty, Property> boolProperty(m, "BoolProperty");
    boolProperty.def(
        py::init([](const std::string &identifier, const std::string &displayName, bool value,
                    InvalidationLevel invalidationLevel, PropertySemantics semantics) {
            return new BoolProperty(identifier, displayName, value, invalidationLevel, semantics);
        }),
        py::arg("identifier"), py::arg("displayName"), py::arg("value") = false,
        py::arg("invalidationLevel") = InvalidationLevel::InvalidResources,
        py::arg("semantics") = PropertySemantics::Default);
    pyTemplateProperty<bool, BoolProperty>(boolProperty);

    PyPropertyClass<ButtonProperty, Property>(m, "ButtonProperty")
        .def(py::init([](const std::string &identifier, const std::string &displayName,
                         InvalidationLevel invalidationLevel, PropertySemantics semantics) {
                 return new ButtonProperty(identifier, displayName, invalidationLevel, semantics);
             }),
             py::arg("identifier"), py::arg("displayName"),
             py::arg("invalidationLevel") = InvalidationLevel::InvalidResources,
             py::arg("semantics") = PropertySemantics::Default)
        .def("press", &ButtonProperty::pressButton);
}

}  // namespace inviwo
