/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2025 Inviwo Foundation
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

#include <modules/base/pythonbindings/properties/pybaseproperties.h>

#include <pybind11/pybind11.h>
#include <pybind11/functional.h>
#include <pybind11/stl.h>

#include <inviwo/core/datastructures/image/image.h>
#include <inviwo/core/datastructures/geometry/mesh.h>

#include <modules/base/properties/basisproperty.h>
#include <modules/base/properties/datarangeproperty.h>
#include <modules/base/properties/imageinformationproperty.h>
#include <modules/base/properties/layerinformationproperty.h>
#include <modules/base/properties/meshinformationproperty.h>
#include <modules/base/properties/transformlistproperty.h>
#include <modules/base/properties/valueaxisproperty.h>
#include <modules/base/properties/volumeinformationproperty.h>

#include <modules/python3/opaquetypes.h>
#include <modules/python3/polymorphictypehooks.h>

namespace inviwo {

void exposeBaseProperties(pybind11::module& m) {
    namespace py = pybind11;

    py::enum_<BasisProperty::BasisPropertyMode>(m, "BasisPropertyMode")
        .value("General", BasisProperty::BasisPropertyMode::General)
        .value("Orthogonal", BasisProperty::BasisPropertyMode::Orthogonal);

    py::classh<BasisProperty, CompositeProperty>{m, "BasisProperty"}
        .def(py::init([](std::string_view identifier, std::string_view displayName, Document help,
                         InvalidationLevel invalidationLevel, PropertySemantics semantics) {
                 return new BasisProperty{identifier, displayName, std::move(help),
                                          invalidationLevel, std::move(semantics)};
             }),
             py::arg("identifier"), py::arg("displayName"), py::arg("help"),
             py::arg("invalidationLevel") = InvalidationLevel::InvalidOutput,
             py::arg("semantics") = PropertySemantics::Default)
        .def(py::init([](std::string_view identifier, std::string_view displayName,
                         InvalidationLevel invalidationLevel, PropertySemantics semantics) {
                 return new BasisProperty{identifier, displayName, invalidationLevel,
                                          std::move(semantics)};
             }),
             py::arg("identifier"), py::arg("displayName"),
             py::arg("invalidationLevel") = InvalidationLevel::InvalidOutput,
             py::arg("semantics") = PropertySemantics::Default)
        .def("updateForNewEntity",
             py::overload_cast<const mat4&, size3_t, bool>(&BasisProperty::updateForNewEntity))
        .def("updateForNewEntity", py::overload_cast<const StructuredGridEntity<3>&, bool>(
                                       &BasisProperty::updateForNewEntity))
        .def("updateForNewEntity", py::overload_cast<const StructuredGridEntity<2>&, bool>(
                                       &BasisProperty::updateForNewEntity))
        .def("updateForNewEntity",
             py::overload_cast<const SpatialEntity&, bool>(&BasisProperty::updateForNewEntity))
        .def("updateEntity", &BasisProperty::updateEntity)
        .def_property_readonly("basisAndOffset", &BasisProperty::getBasisAndOffset);

    py::classh<DataRangeProperty, CompositeProperty>{m, "DataRangeProperty"}
        .def(py::init([](std::string_view identifier, std::string_view displayName,
                         bool customRanges, InvalidationLevel invalidationLevel,
                         PropertySemantics semantics) {
                 return new DataRangeProperty{identifier, displayName, customRanges,
                                              invalidationLevel, std::move(semantics)};
             }),
             py::arg("identifier"), py::arg("displayName"), py::arg("customRanges") = true,
             py::arg("invalidationLevel") = InvalidationLevel::InvalidOutput,
             py::arg("semantics") = PropertySemantics::Default)
        .def(py::init([](std::string_view identifier, std::string_view displayName,
                         VolumeInport& port, bool customRanges, InvalidationLevel invalidationLevel,
                         PropertySemantics semantics) {
                 return new DataRangeProperty{
                     identifier,   displayName,       port,
                     customRanges, invalidationLevel, std::move(semantics)};
             }),
             py::arg("identifier"), py::arg("displayName"), py::arg("port"),
             py::arg("customRanges") = true,
             py::arg("invalidationLevel") = InvalidationLevel::InvalidOutput,
             py::arg("semantics") = PropertySemantics::Default)
        .def(py::init([](std::string_view identifier, std::string_view displayName,
                         LayerInport& port, bool customRanges, InvalidationLevel invalidationLevel,
                         PropertySemantics semantics) {
                 return new DataRangeProperty{
                     identifier,   displayName,       port,
                     customRanges, invalidationLevel, std::move(semantics)};
             }),
             py::arg("identifier"), py::arg("displayName"), py::arg("port"),
             py::arg("customRanges") = true,
             py::arg("invalidationLevel") = InvalidationLevel::InvalidOutput,
             py::arg("semantics") = PropertySemantics::Default)
        .def("updateFromVolume", &DataRangeProperty::updateFromVolume)
        .def("updateFromLayer", &DataRangeProperty::updateFromLayer)
        .def_property("dataRange", &DataRangeProperty::getDataRange,
                      &DataRangeProperty::setDataRange)
        .def_property("valueRange", &DataRangeProperty::getValueRange,
                      &DataRangeProperty::setValueRange)
        .def_property_readonly("customDataRange", &DataRangeProperty::getCustomDataRange)
        .def_property_readonly("customValueRange", &DataRangeProperty::getCustomValueRange)
        .def_property_readonly("customRangeEnabled", &DataRangeProperty::getCustomRangeEnabled);

    py::classh<ValueAxisProperty, CompositeProperty>{m, "ValueAxisProperty"}
        .def(py::init([](std::string_view identifier, std::string_view displayName, bool customAxis,
                         InvalidationLevel invalidationLevel, PropertySemantics semantics) {
                 return new ValueAxisProperty{identifier, displayName, customAxis,
                                              invalidationLevel, std::move(semantics)};
             }),
             py::arg("identifier"), py::arg("displayName"), py::arg("customAxis") = true,
             py::arg("invalidationLevel") = InvalidationLevel::InvalidOutput,
             py::arg("semantics") = PropertySemantics::Default)
        .def(py::init([](std::string_view identifier, std::string_view displayName,
                         VolumeInport& port, bool customAxis, InvalidationLevel invalidationLevel,
                         PropertySemantics semantics) {
                 return new ValueAxisProperty{identifier, displayName,       port,
                                              customAxis, invalidationLevel, std::move(semantics)};
             }),
             py::arg("identifier"), py::arg("displayName"), py::arg("port"),
             py::arg("customAxis") = true,
             py::arg("invalidationLevel") = InvalidationLevel::InvalidOutput,
             py::arg("semantics") = PropertySemantics::Default)
        .def(py::init([](std::string_view identifier, std::string_view displayName,
                         LayerInport& port, bool customAxis, InvalidationLevel invalidationLevel,
                         PropertySemantics semantics) {
                 return new ValueAxisProperty{identifier, displayName,       port,
                                              customAxis, invalidationLevel, std::move(semantics)};
             }),
             py::arg("identifier"), py::arg("displayName"), py::arg("port"),
             py::arg("customAxis") = true,
             py::arg("invalidationLevel") = InvalidationLevel::InvalidOutput,
             py::arg("semantics") = PropertySemantics::Default)
        .def("updateFromVolume", &ValueAxisProperty::updateFromVolume)
        .def("updateFromLayer", &ValueAxisProperty::updateFromLayer)
        .def_property("valueName", &ValueAxisProperty::getValueName,
                      &ValueAxisProperty::setValueName)
        .def_property("valueUnit", &ValueAxisProperty::getValueUnit,
                      &ValueAxisProperty::setValueUnit)
        .def_property_readonly("customValueName", &ValueAxisProperty::getCustomValueName)
        .def_property_readonly("customValueUnit", &ValueAxisProperty::getCustomValueUnit)
        .def_property_readonly("customAxisEnabled", &ValueAxisProperty::getCustomAxisEnabled);

    py::classh<LayerInformationProperty, BoolCompositeProperty>{m, "LayerInformationProperty"}
        .def(py::init([](std::string_view identifier, std::string_view displayName,
                         InvalidationLevel invalidationLevel, PropertySemantics semantics) {
                 return new LayerInformationProperty{identifier, displayName, invalidationLevel,
                                                     std::move(semantics)};
             }),
             py::arg("identifier"), py::arg("displayName"),
             py::arg("invalidationLevel") = InvalidationLevel::InvalidOutput,
             py::arg("semantics") = PropertySemantics::Default)
        .def("updateForNewLayer", &LayerInformationProperty::updateForNewLayer)
        .def("updateLayer", &LayerInformationProperty::updateLayer);

    py::classh<VolumeInformationProperty, BoolCompositeProperty>{m, "VolumeInformationProperty"}
        .def(py::init([](std::string_view identifier, std::string_view displayName,
                         InvalidationLevel invalidationLevel, PropertySemantics semantics) {
                 return new VolumeInformationProperty{identifier, displayName, invalidationLevel,
                                                      std::move(semantics)};
             }),
             py::arg("identifier"), py::arg("displayName"),
             py::arg("invalidationLevel") = InvalidationLevel::InvalidOutput,
             py::arg("semantics") = PropertySemantics::Default)
        .def("updateForNewVolume", &VolumeInformationProperty::updateForNewVolume)
        .def("updateVolume", &VolumeInformationProperty::updateVolume);

    py::classh<ImageInformationProperty, CompositeProperty>{m, "ImageInformationProperty"}
        .def(py::init([](std::string_view identifier, std::string_view displayName,
                         InvalidationLevel invalidationLevel, PropertySemantics semantics) {
                 return new ImageInformationProperty{identifier, displayName, invalidationLevel,
                                                     std::move(semantics)};
             }),
             py::arg("identifier"), py::arg("displayName"),
             py::arg("invalidationLevel") = InvalidationLevel::InvalidOutput,
             py::arg("semantics") = PropertySemantics::Default)
        .def("updateForNewMesh", &ImageInformationProperty::updateForNewImage);

    py::classh<MeshInformationProperty, CompositeProperty>{m, "MeshInformationProperty"}
        .def(py::init([](std::string_view identifier, std::string_view displayName,
                         InvalidationLevel invalidationLevel, PropertySemantics semantics) {
                 return new MeshInformationProperty{identifier, displayName, invalidationLevel,
                                                    std::move(semantics)};
             }),
             py::arg("identifier"), py::arg("displayName"),
             py::arg("invalidationLevel") = InvalidationLevel::InvalidOutput,
             py::arg("semantics") = PropertySemantics::Default)
        .def("updateForNewMesh", &MeshInformationProperty::updateForNewMesh);

    py::classh<TransformListProperty, CompositeProperty>{m, "TransformListProperty"}
        .def(py::init([](std::string_view identifier, std::string_view displayName, Document help,
                         InvalidationLevel invalidationLevel, PropertySemantics semantics) {
                 return new TransformListProperty{identifier, displayName, std::move(help),
                                                  invalidationLevel, std::move(semantics)};
             }),
             py::arg("identifier"), py::arg("displayName"), py::arg("help"),
             py::arg("invalidationLevel") = InvalidationLevel::InvalidOutput,
             py::arg("semantics") = PropertySemantics::Default)
        .def(py::init([](std::string_view identifier, std::string_view displayName,
                         InvalidationLevel invalidationLevel, PropertySemantics semantics) {
                 return new TransformListProperty{identifier, displayName, invalidationLevel,
                                                  std::move(semantics)};
             }),
             py::arg("identifier"), py::arg("displayName"),
             py::arg("invalidationLevel") = InvalidationLevel::InvalidOutput,
             py::arg("semantics") = PropertySemantics::Default)
        .def_property_readonly("matrix", &TransformListProperty::getMatrix);
}

}  // namespace inviwo
