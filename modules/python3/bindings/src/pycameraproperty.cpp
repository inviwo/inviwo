/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2024 Inviwo Foundation
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

#include <inviwopy/pycameraproperty.h>

#include <inviwo/core/properties/cameraproperty.h>
#include <inviwo/core/algorithm/camerautils.h>
#include <inviwo/core/ports/inport.h>

#include <modules/python3/polymorphictypehooks.h>

#include <pybind11/functional.h>
#include <fmt/format.h>

namespace py = pybind11;

namespace inviwo {

void exposeCameraProperty(pybind11::module& main, pybind11::module& properties) {

    auto cameraModule = main.def_submodule("camerautil", "Camera utilities");

    py::enum_<camerautil::Side>(cameraModule, "Side")
        .value("XNegative", camerautil::Side::XNegative)
        .value("XPositive", camerautil::Side::XPositive)
        .value("YNegative", camerautil::Side::YNegative)
        .value("YPositive", camerautil::Side::YPositive)
        .value("ZNegative", camerautil::Side::ZNegative)
        .value("ZPositive", camerautil::Side::ZPositive);

    py::enum_<camerautil::UpdateNearFar>(cameraModule, "UpdateNearFar")
        .value("Yes", camerautil::UpdateNearFar::Yes)
        .value("No", camerautil::UpdateNearFar::No);

    py::enum_<camerautil::UpdateLookRanges>(cameraModule, "UpdateLookRanges")
        .value("Yes", camerautil::UpdateLookRanges::Yes)
        .value("No", camerautil::UpdateLookRanges::No);

    cameraModule.def("setCameraLookRanges", &camerautil::setCameraLookRanges);
    cameraModule.def("computeCameraNearFar", &camerautil::computeCameraNearFar);
    cameraModule.def("setCameraNearFar", &camerautil::setCameraNearFar);

    cameraModule.def(
        "setCameraView",
        [](CameraProperty& cam, const mat4& boundingBox, float fitRatio,
           camerautil::UpdateNearFar updateNearFar, camerautil::UpdateLookRanges updateLookRanges) {
            camerautil::setCameraView(cam, boundingBox, fitRatio, updateNearFar, updateLookRanges);
        },
        py::arg("cameraProperty"), py::arg("boundingBox"), py::arg("fitRatio") = 1.05f,
        py::arg("updateNearFar") = camerautil::UpdateNearFar::No,
        py::arg("updateLookRanges") = camerautil::UpdateLookRanges::No);

    cameraModule.def(
        "setCameraView",
        [](CameraProperty& cam, const mat4& boundingBox, camerautil::Side side, float fitRatio,
           camerautil::UpdateNearFar updateNearFar, camerautil::UpdateLookRanges updateLookRanges) {
            camerautil::setCameraView(cam, boundingBox, side, fitRatio, updateNearFar,
                                      updateLookRanges);
        },
        py::arg("cameraProperty"), py::arg("boundingBox"), py::arg("side"),
        py::arg("fitRatio") = 1.05f, py::arg("updateNearFar") = camerautil::UpdateNearFar::No,
        py::arg("updateLookRanges") = camerautil::UpdateLookRanges::No);

    cameraModule.def(
        "setCameraView",
        [](CameraProperty& cam, const mat4& boundingBox, vec3 viewDir, vec3 lookUp, float fitRatio,
           camerautil::UpdateNearFar updateNearFar, camerautil::UpdateLookRanges updateLookRanges) {
            camerautil::setCameraView(cam, boundingBox, viewDir, lookUp, fitRatio, updateNearFar,
                                      updateLookRanges);
        },
        py::arg("cameraProperty"), py::arg("boundingBox"), py::arg("viewDir"), py::arg("lookUp"),
        py::arg("fitRatio") = 1.05f, py::arg("updateNearFar") = camerautil::UpdateNearFar::No,
        py::arg("updateLookRanges") = camerautil::UpdateLookRanges::No);

    py::class_<CameraProperty, CompositeProperty>(properties, "CameraProperty")
        .def(py::init([](const std::string& identifier, const std::string& displayName, vec3 eye,
                         vec3 center, vec3 lookUp, Inport* inport,
                         InvalidationLevel invalidationLevel, PropertySemantics semantics) {
                 return new CameraProperty(identifier, displayName, eye, center, lookUp, inport,
                                           invalidationLevel, semantics);
             }),
             py::arg("identifier"), py::arg("displayName"), py::arg("eye") = vec3(0.0f, 0.0f, 2.0f),
             py::arg("center") = vec3(0.0f), py::arg("lookUp") = vec3(0.0f, 1.0f, 0.0f),
             py::arg("inport") = nullptr,
             py::arg("invalidationLevel") = InvalidationLevel::InvalidResources,
             py::arg("semantics") = PropertySemantics::Default)
        .def_property_readonly("camera",
                               static_cast<Camera& (CameraProperty::*)()>(&CameraProperty::get))
        .def_property_readonly("value",
                               static_cast<Camera& (CameraProperty::*)()>(&CameraProperty::get))
        .def_property(
            "lookFrom", &CameraProperty::getLookFrom,
            [](CameraProperty* cam, vec3 val) { cam->setLookFrom(val); },
            py::return_value_policy::copy)
        .def_property(
            "lookTo", &CameraProperty::getLookTo,
            [](CameraProperty* cam, vec3 val) { cam->setLookTo(val); },
            py::return_value_policy::copy)
        .def_property(
            "lookUp", &CameraProperty::getLookUp,
            [](CameraProperty* cam, vec3 val) { cam->setLookUp(val); },
            py::return_value_policy::copy)
        .def_property_readonly("lookRight", &CameraProperty::getLookRight)
        .def_property("aspectRatio", &CameraProperty::getAspectRatio,
                      &CameraProperty::setAspectRatio)
        .def_property("nearPlane", &CameraProperty::getNearPlaneDist,
                      &CameraProperty::setNearPlaneDist)
        .def_property("farPlane", &CameraProperty::getFarPlaneDist,
                      &CameraProperty::setFarPlaneDist)
        .def("setLook",
             [](CameraProperty* cam, vec3 from, vec3 to, vec3 up) { cam->setLook(from, to, up); })
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
        .def("fitData", &CameraProperty::fitData)
        .def("setView", &CameraProperty::setView)
        .def("flipUp", &CameraProperty::flipUp)
        .def("setNearFar", &CameraProperty::setNearFar)
        .def("setLookRange", &CameraProperty::setLookRange);
}

}  // namespace inviwo
