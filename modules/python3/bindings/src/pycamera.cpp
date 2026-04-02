/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2020-2026 Inviwo Foundation
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

#include <pybind11/functional.h>
#include <pybind11/stl.h>
#include <pybind11/stl/filesystem.h>
#include <pybind11/native_enum.h>

#include <inviwo/core/datastructures/camera/camera.h>
#include <inviwo/core/datastructures/camera/orthographiccamera.h>
#include <inviwo/core/datastructures/camera/perspectivecamera.h>

#include <inviwo/core/datastructures/camera/plotcamera.h>
#include <inviwo/core/datastructures/camera/skewedperspectivecamera.h>

#include <modules/python3/opaquetypes.h>
#include <modules/python3/polymorphictypehooks.h>

namespace inviwo {

void exposeCamera(pybind11::module& m) {
    namespace py = pybind11;

    auto zo = py::classh<ZoomOptions>(m, "ZoomOptions")
                  .def_readwrite("factor", &ZoomOptions::factor)
                  .def_readwrite("origin", &ZoomOptions::origin)
                  .def_readwrite("bounded", &ZoomOptions::bounded)
                  .def_readwrite("boundingBox", &ZoomOptions::boundingBox);

    py::native_enum<ZoomOptions::Bounded>(zo, "Bounded", "enum.Enum")
        .value("Yes", ZoomOptions::Bounded::Yes)
        .value("No", ZoomOptions::Bounded::No)
        .export_values()
        .finalize();

    py::classh<Camera>(m, "Camera")
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
        .def("zoom", &Camera::zoom, py::arg("options"))
        .def("getLookRight", &Camera::getLookRight)
        .def("getDirection", &Camera::getDirection)
        .def("getWorldPosFromNormalizedDeviceCoords",
             &Camera::getWorldPosFromNormalizedDeviceCoords)
        .def("getClipPosFromNormalizedDeviceCoords", &Camera::getClipPosFromNormalizedDeviceCoords)
        .def("getNormalizedDeviceFromNormalizedScreenAtFocusPointDepth",
             &Camera::getNormalizedDeviceFromNormalizedScreenAtFocusPointDepth);

    py::classh<PerspectiveCamera, Camera>(m, "PerspectiveCamera")
        .def(py::init<dvec3, dvec3, dvec3, double, double, double, double>(),
             py::arg("lookFrom") = cameradefaults::lookFrom,
             py::arg("lookTo") = cameradefaults::lookTo, py::arg("lookUp") = cameradefaults::lookUp,
             py::arg("nearPlane") = cameradefaults::nearPlane,
             py::arg("farPlane") = cameradefaults::farPlane,
             py::arg("aspectRatio") = cameradefaults::aspectRatio,
             py::arg("fieldOfView") = cameradefaults::fieldOfView)
        .def_property("fovy", &PerspectiveCamera::getFovy, &PerspectiveCamera::setFovy);

    py::classh<OrthographicCamera, Camera>(m, "OrthographicCamera")
        .def(py::init<dvec3, dvec3, dvec3, double, double, double, double>(),
             py::arg("lookFrom") = cameradefaults::lookFrom,
             py::arg("lookTo") = cameradefaults::lookTo, py::arg("lookUp") = cameradefaults::lookUp,
             py::arg("nearPlane") = cameradefaults::nearPlane,
             py::arg("farPlane") = cameradefaults::farPlane,
             py::arg("aspectRatio") = cameradefaults::aspectRatio,
             py::arg("width") = cameradefaults::width)
        .def_property("width", &OrthographicCamera::getWidth, &OrthographicCamera::setWidth);

    py::classh<SkewedPerspectiveCamera, Camera>(m, "SkewedPerspectiveCamera")
        .def(py::init<dvec3, dvec3, dvec3, double, double, double, double, vec2>(),
             py::arg("lookFrom") = cameradefaults::lookFrom,
             py::arg("lookTo") = cameradefaults::lookTo, py::arg("lookUp") = cameradefaults::lookUp,
             py::arg("nearPlane") = cameradefaults::nearPlane,
             py::arg("farPlane") = cameradefaults::farPlane,
             py::arg("aspectRatio") = cameradefaults::aspectRatio,
             py::arg("fieldOfView") = cameradefaults::fieldOfView,
             py::arg("frustumOffset") = dvec2(0.0, 0.0))
        .def_property("fovy", &SkewedPerspectiveCamera::getFovy, &SkewedPerspectiveCamera::setFovy)
        .def_property("offset", &SkewedPerspectiveCamera::getOffset,
                      &SkewedPerspectiveCamera::setOffset);

    py::classh<PlotCamera, Camera>(m, "PlotCamera")
        .def(py::init<dvec3, dvec3, dvec3, double, double, double, dvec2>(),
             py::arg("lookFrom") = cameradefaults::lookFrom,
             py::arg("lookTo") = cameradefaults::lookTo, py::arg("lookUp") = cameradefaults::lookUp,
             py::arg("nearPlane") = cameradefaults::nearPlane,
             py::arg("farPlane") = cameradefaults::farPlane,
             py::arg("aspectRatio") = cameradefaults::aspectRatio,
             py::arg("size") = dvec2{300, 300})
        .def_property("size", &PlotCamera::getSize, &PlotCamera::setSize);
}

}  // namespace inviwo
