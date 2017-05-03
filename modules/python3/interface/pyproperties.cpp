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

#include <inviwo/core/util/stdextensions.h>

namespace py = pybind11;

namespace inviwo {

struct propertyDelete {
    void operator()(Property *p) {
        if (p && p->getOwner() == nullptr) delete p;
    }
};

void exposeProperties(py::module &m) {

    py::class_<PropertyFactory>(m, "PropertyFactory")
        .def("hasKey", [](PropertyFactory *pf, std::string key) { return pf->hasKey(key); })
        .def_property_readonly("keys", [](PropertyFactory *pf) { return pf->getKeys(); })
        .def("create", [](PropertyFactory *pf, std::string key) {
            return propertyToPyObject(pf->create(key).release());

        });

    py::class_<PropertyWidget>(m, "PropertyWidget")
        .def_property_readonly("editorWidget", &PropertyWidget::getEditorWidget,
                               py::return_value_policy::reference)
        .def_property_readonly("property", &PropertyWidget::getProperty,
                               py::return_value_policy::reference);

    py::class_<PropertyEditorWidget>(m, "PropertyEditorWidget")
        .def_property("visibility", &PropertyEditorWidget::isVisible,
                      &PropertyEditorWidget::setVisibility)
        .def_property("dimensions", &PropertyEditorWidget::getDimensions,
                      &PropertyEditorWidget::setDimensions)
        .def_property("position", &PropertyEditorWidget::getPosition,
                      &PropertyEditorWidget::setPosition)
        //.def_property("dockStatus", &PropertyEditorWidget::getDockStatus,
        //&PropertyEditorWidget::setDockStatus) //TODO expose dock status
        .def_property("sticky", &PropertyEditorWidget::isSticky, &PropertyEditorWidget::setSticky);

    py::class_<Property, PropertyPtr<Property>>(m, "Property")
        .def_property("identifier", &Property::getIdentifier, &Property::setIdentifier)
        .def_property("displayName", &Property::getDisplayName, &Property::setDisplayName)
        .def_property("readOnly", &Property::getReadOnly, &Property::setReadOnly)
        .def_property("semantics", &Property::getSemantics,
                      &Property::setSemantics)  // TODO expose semantics
        .def_property_readonly("classIdentifierForWidget", &Property::getClassIdentifierForWidget)
        .def_property_readonly("path", &Property::getPath)
        .def_property_readonly("invalidationLevel", &Property::getInvalidationLevel)
        .def_property_readonly("widgets", &Property::getWidgets)
        .def("updateWidgets", &Property::updateWidgets)
        .def("hasWidgets", &Property::hasWidgets)
        .def("setCurrentStateAsDefault", &Property::setCurrentStateAsDefault)
        .def("resetToDefaultState", &Property::resetToDefaultState);

    py::class_<CompositeProperty, Property, PropertyOwner, PropertyPtr<CompositeProperty>>(
        m, "CompositeProperty")
        .def("__getattr__", &getPropertyById<CompositeProperty>,
             py::return_value_policy::reference);

    py::class_<BaseOptionProperty, Property, PropertyPtr<BaseOptionProperty>>(m, "BaseOptionProperty")
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
        .def_property_readonly("displayName", &BaseOptionProperty::getDisplayNames)

        ;
    using OptionPropetyTypes = std::tuple<double, float, int, std::string>;
    using MinMaxPropertyTypes = std::tuple<float, double, size_t, glm::i64, int>;
    using OrdinalPropetyTypes = std::tuple<float, int, size_t, glm::i64, double, vec2, vec3, vec4,
                                           dvec2, dvec3, dvec4, ivec2, ivec3, ivec4, size2_t,
                                           size3_t, size4_t, mat2, mat3, mat4, dmat2, dmat3, dmat4>;

    util::for_each_type<OrdinalPropetyTypes>{}(OrdinalPropertyHelper{}, m);
    util::for_each_type<OptionPropetyTypes>{}(OptionPropertyHelper{}, m);
    util::for_each_type<MinMaxPropertyTypes>{}(MinMaxHelper{}, m);

    py::class_<ButtonProperty, Property, PropertyPtr<ButtonProperty>>(m, "ButtonProperty")
        .def("press", &ButtonProperty::pressButton);

    py::class_<CameraProperty, CompositeProperty, PropertyPtr<CameraProperty>>(m, "CameraProperty")
        .def_property("lookFrom", &CameraProperty::getLookFrom, &CameraProperty::setLookFrom , py::return_value_policy::copy)
        .def_property("lookTo", &CameraProperty::getLookTo, &CameraProperty::setLookTo , py::return_value_policy::copy)
        .def_property("lookUp", &CameraProperty::getLookUp, &CameraProperty::setLookUp , py::return_value_policy::copy)
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

    py::class_<TransferFunctionProperty, Property, PropertyPtr<TransferFunctionProperty>>(
        m, "TransferFunctionProperty")
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
        .def("addPoint", [](TransferFunctionProperty &tp, vec2 pos, vec3 color) {
            tp.get().addPoint(pos, vec4(color, pos.y));
        });

    py::class_<StringProperty, Property, PropertyPtr<StringProperty>> strProperty(m, "StringProperty");
    pyTemplateProperty<std::string, StringProperty>(strProperty);

    py::class_<FileProperty, Property, PropertyPtr<FileProperty>> fileProperty(m, "FileProperty");
    pyTemplateProperty<std::string, FileProperty>(fileProperty);

    py::class_<DirectoryProperty, Property, PropertyPtr<DirectoryProperty>> dirProperty(
        m, "DirectoryProperty");
    pyTemplateProperty<std::string, DirectoryProperty>(dirProperty);

    py::class_<BoolProperty, Property, PropertyPtr<BoolProperty>> boolProperty(m, "BoolProperty");
    pyTemplateProperty<bool, BoolProperty>(boolProperty);
}
}  // namespace
