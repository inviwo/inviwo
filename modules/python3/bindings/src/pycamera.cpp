/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2020-2024 Inviwo Foundation
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

#include <inviwopy/pycamera.h>

#include <inviwo/core/datastructures/camera/camera.h>
#include <inviwo/core/datastructures/camera/orthographiccamera.h>
#include <inviwo/core/datastructures/camera/perspectivecamera.h>
#include <inviwo/core/datastructures/camera/skewedperspectivecamera.h>

#include <pybind11/stl.h>
#include <pybind11/functional.h>

namespace inviwo {

void exposeCamera(pybind11::module& m) {
    namespace py = pybind11;

    py::class_<Camera>(m, "Camera")
        .def("clone", &Camera::clone)
        .def("updateFrom", &Camera::updateFrom)
        .def_property("lookFrom", &Camera::getLookFrom, &Camera::setLookFrom)
        .def_property("lookTo", &Camera::getLookTo, &Camera::setLookTo)
        .def_property("lookUp", &Camera::getLookUp, &Camera::setLookUp)
        .def_property("aspectRatio", &Camera::getAspectRatio, &Camera::setAspectRatio)
        .def_property("nearPlaneDist", &Camera::getNearPlaneDist, &Camera::setNearPlaneDist)
        .def_property("farPlaneDist", &Camera::getFarPlaneDist, &Camera::setFarPlaneDist)
        .def_property_readonly("viewMatrix", &Camera::getViewMatrix)
        .def_property_readonly("projectionMatrix", &Camera::getProjectionMatrix)
        .def_property_readonly("inverseViewMatrix", &Camera::getInverseViewMatrix)
        .def_property_readonly("inverseProjectionMatrix", &Camera::getInverseProjectionMatrix)
        .def("getDirection", &Camera::getDirection)
        .def("getWorldPosFromNormalizedDeviceCoords",
             &Camera::getWorldPosFromNormalizedDeviceCoords)
        .def("getClipPosFromNormalizedDeviceCoords", &Camera::getClipPosFromNormalizedDeviceCoords)
        .def("getNormalizedDeviceFromNormalizedScreenAtFocusPointDepth",
             &Camera::getNormalizedDeviceFromNormalizedScreenAtFocusPointDepth);

    py::class_<PerspectiveCamera, Camera>(m, "PerspectiveCamera")
        .def(py::init<vec3, vec3, vec3, float, float, float, float>(),
             py::arg("lookFrom") = vec3(0.0f, 0.0f, 2.0f), py::arg("lookTo") = vec3(0.0f),
             py::arg("lookUp") = vec3(0.0f, 1.0f, 0.0f), py::arg("nearPlane") = 0.01f,
             py::arg("farPlane") = 10000.0f, py::arg("aspectRatio") = 1.f,
             py::arg("fieldOfView") = 60.f)
        .def_property("fovy", &PerspectiveCamera::getFovy, &PerspectiveCamera::setFovy);

    py::class_<OrthographicCamera, Camera>(m, "OrthographicCamera")
        .def(py::init<vec3, vec3, vec3, float, float, float, float>(),
             py::arg("lookFrom") = vec3(0.0f, 0.0f, 2.0f), py::arg("lookTo") = vec3(0.0f),
             py::arg("lookUp") = vec3(0.0f, 1.0f, 0.0f), py::arg("nearPlane") = 0.01f,
             py::arg("farPlane") = 10000.0f, py::arg("aspectRatio") = 1.f, py::arg("width") = 60.f)
        .def_property("width", &OrthographicCamera::getWidth, &OrthographicCamera::setWidth);

    py::class_<SkewedPerspectiveCamera, Camera>(m, "SkewedPerspectiveCamera")
        .def(py::init<vec3, vec3, vec3, float, float, float, float, vec2>(),
             py::arg("lookFrom") = vec3(0.0f, 0.0f, 2.0f), py::arg("lookTo") = vec3(0.0f),
             py::arg("lookUp") = vec3(0.0f, 1.0f, 0.0f), py::arg("nearPlane") = 0.01f,
             py::arg("farPlane") = 10000.0f, py::arg("aspectRatio") = 1.f,
             py::arg("fieldOfView") = 60.f, py::arg("frustumOffset") = vec2(0.0f, 0.0f))
        .def_property("fovy", &SkewedPerspectiveCamera::getFovy, &SkewedPerspectiveCamera::setFovy)
        .def_property("offset", &SkewedPerspectiveCamera::getOffset,
                      &SkewedPerspectiveCamera::setOffset);
}

}  // namespace inviwo
