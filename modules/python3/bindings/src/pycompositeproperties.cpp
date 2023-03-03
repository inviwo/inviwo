/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2023 Inviwo Foundation
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

#include <inviwopy/pycompositeproperties.h>

#include <pybind11/functional.h>
#include <pybind11/stl.h>

#include <inviwopy/pyflags.h>
#include <inviwopy/pypropertytypehook.h>

#include <inviwo/core/properties/constraintbehavior.h>
#include <inviwo/core/properties/isotfproperty.h>
#include <inviwo/core/properties/boolcompositeproperty.h>
#include <inviwo/core/properties/listproperty.h>
#include <inviwo/core/properties/filepatternproperty.h>

#include <fmt/format.h>

namespace py = pybind11;

namespace inviwo {

void exposeCompositeProperties(py::module& m) {

    py::class_<CompositeProperty, PropertyOwner, Property>(
        m, "CompositeProperty", py::multiple_inheritance{}, py::dynamic_attr{})
        .def(py::init([](std::string_view identifier, std::string_view displayName,
                         InvalidationLevel invalidationLevel, PropertySemantics semantics) {
                 return new CompositeProperty(identifier, displayName, invalidationLevel,
                                              semantics);
             }),
             py::arg("identifier"), py::arg("displayName"),
             py::arg("invalidationLevel") = InvalidationLevel::InvalidResources,
             py::arg("semantics") = PropertySemantics::Default)
        .def("setCollapsed", py::overload_cast<bool>(&CompositeProperty::setCollapsed))
        .def("isCollapsed", &CompositeProperty::isCollapsed)
        .def_property("collapsed", &BoolCompositeProperty::isCollapsed,
                      py::overload_cast<bool>(&CompositeProperty::setCollapsed));

    py::class_<BoolCompositeProperty, CompositeProperty>(m, "BoolCompositeProperty")
        .def(py::init([](std::string_view identifier, std::string_view displayName, bool checked,
                         InvalidationLevel invalidationLevel, PropertySemantics semantics) {
                 return new BoolCompositeProperty(identifier, displayName, checked,
                                                  invalidationLevel, semantics);
             }),
             py::arg("identifier"), py::arg("displayName"), py::arg("checked"),
             py::arg("invalidationLevel") = InvalidationLevel::InvalidResources,
             py::arg("semantics") = PropertySemantics::Default)
        .def("isChecked", &BoolCompositeProperty::isChecked)
        .def("setChecked", &BoolCompositeProperty::setChecked)
        .def_property("checked", &BoolCompositeProperty::isChecked,
                      &BoolCompositeProperty::setChecked)
        .def("__bool__", &BoolCompositeProperty::isChecked);

    py::class_<ListProperty, CompositeProperty>(m, "ListProperty")
        .def(py::init([](std::string_view identifier, std::string_view displayName,
                         size_t maxNumberOfElements, ListPropertyUIFlags uiFlags,
                         InvalidationLevel invalidationLevel, PropertySemantics semantics) {
                 return new ListProperty(identifier, displayName, maxNumberOfElements, uiFlags,
                                         invalidationLevel, semantics);
             }),
             py::arg("identifier"), py::arg("displayName"), py::arg("maxNumberOfElements") = 0,
             py::arg("uiFlags") = ListPropertyUIFlag::Add | ListPropertyUIFlag::Remove,
             py::arg("invalidationLevel") = InvalidationLevel::InvalidResources,
             py::arg("semantics") = PropertySemantics::Default)

        .def_property("maxNumberOfElements", &ListProperty::getMaxNumberOfElements,
                      &ListProperty::setMaxNumberOfElements)
        .def("constructProperty", &ListProperty::constructProperty)
        .def_property_readonly("prefabCount", &ListProperty::getPrefabCount)
        .def("clear", &ListProperty::clear)
        .def("addPrefab",
             [](ListProperty& list, const Property& prefab) {
                 list.addPrefab(std::unique_ptr<Property>(prefab.clone()));
             })
        .def("getPrefab", [](ListProperty& list, size_t idx) -> Property* {
            return list.getPrefabs()[idx].get();
        });

    py::class_<IsoTFProperty, CompositeProperty>(m, "IsoTFProperty")
        .def(py::init([](std::string_view identifier, std::string_view displayName,
                         const IsoValueCollection& isovalues, const TransferFunction& tf,
                         VolumeInport* volumeInport, InvalidationLevel invalidationLevel,
                         PropertySemantics semantics) {
                 return new IsoTFProperty(identifier, displayName, isovalues, tf, volumeInport,
                                          invalidationLevel, semantics);
             }),
             py::arg("identifier"), py::arg("displayName"), py::arg("isovalues"), py::arg("tf"),
             py::arg("inport") = nullptr,
             py::arg("invalidationLevel") = InvalidationLevel::InvalidResources,
             py::arg("semantics") = PropertySemantics::Default)
        .def(py::init([](std::string_view identifier, std::string_view displayName,
                         VolumeInport* volumeInport, InvalidationLevel invalidationLevel,
                         PropertySemantics semantics) {
                 return new IsoTFProperty(identifier, displayName, volumeInport, invalidationLevel,
                                          semantics);
             }),
             py::arg("identifier"), py::arg("displayName"), py::arg("inport"),
             py::arg("invalidationLevel") = InvalidationLevel::InvalidResources,
             py::arg("semantics") = PropertySemantics::Default)
        .def_property_readonly(
            "isovalues",
            py::cpp_function([](IsoTFProperty& tp) -> IsoValueProperty& { return tp.isovalues_; },
                             py::return_value_policy::reference_internal))
        .def_property_readonly(
            "tf",
            py::cpp_function([](IsoTFProperty& tp) -> TransferFunctionProperty& { return tp.tf_; },
                             py::return_value_policy::reference_internal))
        .def_property("mask", &IsoTFProperty::getMask, &IsoTFProperty::setMask)
        .def_property("zoomH", &IsoTFProperty::getZoomH, &IsoTFProperty::setZoomH)
        .def_property("zoomV", &IsoTFProperty::getZoomV, &IsoTFProperty::setZoomV);

    py::class_<FilePatternProperty, CompositeProperty>(m, "FilePatternProperty")
        .def(py::init([](std::string_view identifier, std::string_view displayName,
                         std::string_view pattern, std::string_view directory,
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
                               [](FilePatternProperty* p) {
                                   return std::make_tuple(p->getMinRange(), p->getMaxRange());
                               })
        .def_property("selectedExtension", &FilePatternProperty::getSelectedExtension,
                      &FilePatternProperty::setSelectedExtension)
        .def("addNameFilter", static_cast<void (FilePatternProperty::*)(std::string)>(
                                  &FilePatternProperty::addNameFilter))
        .def("addNameFilter", static_cast<void (FilePatternProperty::*)(FileExtension)>(
                                  &FilePatternProperty::addNameFilter))
        .def("clearNameFilters", &FilePatternProperty::clearNameFilters);
}

}  // namespace inviwo
