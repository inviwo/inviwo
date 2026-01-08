/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2025-2026 Inviwo Foundation
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

#include <inviwopy/pyspatialdata.h>

#include <pybind11/stl_bind.h>
#include <pybind11/functional.h>
#include <pybind11/stl.h>

#include <inviwo/core/datastructures/spatialdata.h>
#include <inviwo/core/util/glmvec.h>
#include <inviwo/core/util/glmmat.h>
#include <inviwo/core/util/glmfmt.h>

#include <modules/python3/opaquetypes.h>
#include <modules/python3/polymorphictypehooks.h>

#include <fmt/format.h>
#include <fmt/ostream.h>

namespace inviwo {

namespace {

template <unsigned int N>
void structuredGridHelper(pybind11::module& m, const std::string& name) {
    namespace py = pybind11;

    py::classh<StructuredGridEntity<N>, SpatialEntity>(m, name.c_str())
        .def_property_readonly("dimensions", &StructuredGridEntity<N>::getDimensions)
        .def_property_readonly("indexMatrix", &StructuredGridEntity<N>::getIndexMatrix)
        .def("__repr__", [name](const StructuredGridEntity<N>& entity) {
            return fmt::format(
                "<{}:\n"
                "modelMatrix = {}\n"
                "worldMatrix = {}>",
                name, entity.getModelMatrix(), entity.getWorldMatrix());
        });
}

}  // namespace

void exposeSpatialData(pybind11::module& m) {
    namespace py = pybind11;

    py::classh<SpatialEntity>(m, "SpatialEntity")
        .def_property("modelMatrix", &SpatialEntity::getModelMatrix, &SpatialEntity::setModelMatrix)
        .def_property("worldMatrix", &SpatialEntity::getWorldMatrix, &SpatialEntity::setWorldMatrix)
        .def_property("basis", &SpatialEntity::getBasis, &SpatialEntity::setBasis)
        .def_property("offset", &SpatialEntity::getOffset, &SpatialEntity::setOffset)
        .def("getAxis", &SpatialEntity::getAxis)
        .def("__repr__", [](const SpatialEntity& entity) {
            return fmt::format(
                "<SpatialEntity:\n"
                "modelMatrix = {}\n"
                "worldMatrix = {}>",
                entity.getModelMatrix(), entity.getWorldMatrix());
        });

    py::classh<SpatialIdentity, SpatialEntity>(m, "SpatialIdentity")
        .def("__repr__", [](const SpatialEntity& entity) {
            return fmt::format(
                "<SpatialIdentity:\n"
                "modelMatrix = {}\n"
                "worldMatrix = {}>",
                entity.getModelMatrix(), entity.getWorldMatrix());
        });

    structuredGridHelper<2>(m, "StructuredGridEntity2D");
    structuredGridHelper<3>(m, "StructuredGridEntity3D");
}

}  // namespace inviwo
