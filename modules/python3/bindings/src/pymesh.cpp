/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2025 Inviwo Foundation
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

#include <inviwopy/pymesh.h>
#include <inviwopy/pynetwork.h>
#include <inviwopy/pyglmtypes.h>

#include <pybind11/functional.h>
#include <pybind11/stl_bind.h>
#include <pybind11/stl.h>
#include <pybind11/stl/filesystem.h>
#include <pybind11/numpy.h>

#include <inviwo/core/util/formatdispatching.h>
#include <inviwo/core/datastructures/geometry/mesh.h>
#include <inviwo/core/datastructures/geometry/basicmesh.h>
#include <inviwo/core/ports/meshport.h>
#include <modules/python3/pyportutils.h>
#include <modules/python3/opaquetypes.h>
#include <modules/python3/polymorphictypehooks.h>

#include <fmt/format.h>

// expose STL container
PYBIND11_MAKE_OPAQUE(std::vector<std::shared_ptr<inviwo::Mesh>>)

namespace inviwo {

namespace py = pybind11;

void exposeMesh(pybind11::module& m) {

    py::classh<Mesh::MeshInfo>(m, "MeshInfo")
        .def(py::init<>())
        .def(py::init<DrawType, ConnectivityType>(), py::arg("dt"), py::arg("ct"))
        .def("__repr__",
             [](Mesh::MeshInfo& mi) {
                 std::ostringstream oss;
                 oss << "MeshInfo (" << mi.dt << ", " << mi.ct << ")";
                 return oss.str();
             })
        .def_readwrite("dt", &Mesh::MeshInfo::dt)
        .def_readwrite("ct", &Mesh::MeshInfo::ct);

    py::classh<Mesh::BufferInfo>(m, "BufferInfo")
        .def(py::init<>())
        .def(py::init<BufferType>())
        .def(py::init<BufferType, int>())
        .def("__repr__",
             [](Mesh::BufferInfo& bi) {
                 std::ostringstream oss;
                 oss << "BufferInfo (" << bi.type << ", " << bi.location << ")";
                 return oss.str();
             })
        .def_readwrite("type", &Mesh::BufferInfo::type)
        .def_readwrite("location", &Mesh::BufferInfo::location);

    auto getBuffers = [](auto buffers, auto& parent) {
        pybind11::list list;
        for (auto& buffer : buffers) {
            auto tupl = py::tuple(2);
            tupl[0] = py::cast(buffer.first);
            tupl[1] = py::cast(buffer.second.get(), py::return_value_policy::reference_internal,
                               py::cast(parent));
            list.append(tupl);
        }
        return list;
    };

    py::classh<Mesh>(m, "Mesh")
        .def(py::init<>())
        .def(py::init<DrawType, ConnectivityType>(), py::arg("dt") = DrawType::Points,
             py::arg("ct") = ConnectivityType::None)
        .def(py::init<Mesh::MeshInfo>(), py::arg("meshInfo"))
        .def_property_readonly("defaultMeshInfo", &Mesh::getDefaultMeshInfo)

        .def_property("modelMatrix", &Mesh::getModelMatrix, &Mesh::setModelMatrix)
        .def_property("worldMatrix", &Mesh::getWorldMatrix, &Mesh::setWorldMatrix)
        .def_property("basis", &Mesh::getBasis, &Mesh::setBasis)
        .def_property("offset", &Mesh::getOffset, &Mesh::setOffset)

        .def("addBuffer", [](Mesh* m, Mesh::BufferInfo info,
                             std::shared_ptr<BufferBase> att) { m->addBuffer(info, att); })
        .def("addBuffer", [](Mesh* m, BufferType type,
                             std::shared_ptr<BufferBase> att) { m->addBuffer(type, att); })
        .def("removeBuffer", [](Mesh* m, size_t idx) { m->removeBuffer(idx); })

        .def("replaceBuffer",
             [](Mesh* m, size_t idx, Mesh::BufferInfo info, std::shared_ptr<BufferBase> att) {
                 m->replaceBuffer(idx, info, att);
             })
        .def(
            "addIndexBuffer",
            [](Mesh* mesh, DrawType dt, ConnectivityType ct) {
                mesh->addIndexBuffer(dt, ct);
                return mesh->getIndexBuffers().back().second.get();
            },
            py::arg("dt"), py::arg("ct"), py::return_value_policy::reference)
        .def("addIndices", [](Mesh* m, Mesh::MeshInfo info,
                              std::shared_ptr<IndexBuffer> ind) { m->addIndices(info, ind); })
        .def("addIndicies",
             [](Mesh* m, Mesh::MeshInfo info, std::shared_ptr<IndexBuffer> ind) {
                 log::info("Mesh::addIndicies is deprecated, use addIndices");
                 m->addIndices(info, std::move(ind));
             })
        .def("removeIndexBuffer", [](Mesh* m, size_t idx) { m->removeIndexBuffer(idx); })

        .def("reserveSizeInVertexBuffer", &Mesh::reserveSizeInVertexBuffer)
        .def("reserveIndexBuffers", &Mesh::reserveIndexBuffers)

        .def_property_readonly("buffers",
                               [&](Mesh* mesh) { return getBuffers(mesh->getBuffers(), mesh); })
        .def_property_readonly(
            "indexBuffers", [&](Mesh* mesh) { return getBuffers(mesh->getIndexBuffers(), mesh); })
        .def("__repr__", [](const Mesh& self) {
            std::ostringstream ossBuffers;
            ossBuffers << "\n  <Buffers (" << self.getNumberOfBuffers() << ")";
            for (const auto& elem : self.getBuffers()) {
                ossBuffers << "\n    " << elem.first << ", " << elem.second->getBufferUsage()
                           << " (" << elem.second->getSize() << ")";
            }
            ossBuffers << ">";

            if (self.getNumberOfIndicies() > 0) {
                const size_t maxLines = 10;
                const size_t numIndexBuffers = self.getNumberOfIndicies();

                ossBuffers << "\n  <Indexbuffers (" << self.getNumberOfIndicies() << ")";
                size_t line = 0;
                for (const auto& elem : self.getIndexBuffers()) {
                    ossBuffers << "\n    " << elem.first.dt << ", " << elem.first.ct << " ("
                               << elem.second->getSize() << ")";
                    ++line;
                    if (line >= maxLines) break;
                }
                if (line < numIndexBuffers) {
                    ossBuffers << "\n    ... (" << (numIndexBuffers - line)
                               << " additional buffers)";
                }
                ossBuffers << ">";
            }
            auto meshinfo = self.getDefaultMeshInfo();
            return fmt::format("<Mesh:\n  default mesh info = MeshInfo({}, {}){}>",
                               toString(meshinfo.dt), toString(meshinfo.ct), ossBuffers.str());
        });

    auto buffertraits = m.def_submodule("buffertraits", "Module containing mesh buffertraits");

    py::classh<buffertraits::PositionsBuffer3D>(buffertraits, "PositionsBuffer3D")
        .def("getVertices", &buffertraits::PositionsBuffer3D::getEditableVertices)
        .def("setVertexPosition", &buffertraits::PositionsBuffer3D::setVertexPosition);

    py::classh<buffertraits::PositionsBuffer2D>(buffertraits, "PositionsBuffer2D")
        .def("getVertices", &buffertraits::PositionsBuffer2D::getEditableVertices)
        .def("setVertexPosition", &buffertraits::PositionsBuffer2D::setVertexPosition);

    py::classh<buffertraits::PositionsBuffer1D>(buffertraits, "PositionsBuffer1D")
        .def("getVertices", &buffertraits::PositionsBuffer1D::getEditableVertices)
        .def("setVertexPosition", &buffertraits::PositionsBuffer1D::setVertexPosition);

    py::classh<buffertraits::NormalBuffer>(buffertraits, "NormalBuffer")
        .def("getNormals", &buffertraits::NormalBuffer::getEditableNormals)
        .def("setVertexNormal", &buffertraits::NormalBuffer::setVertexNormal);

    py::classh<buffertraits::ColorsBuffer>(buffertraits, "ColorsBuffer")
        .def("getColors", &buffertraits::ColorsBuffer::getEditableColors)
        .def("setVertexColor", &buffertraits::ColorsBuffer::setVertexColor);

    py::classh<buffertraits::TexCoordBuffer<3>>(buffertraits, "TexCoordBuffer3D")
        .def("getTexCoords", &buffertraits::TexCoordBuffer<3>::getEditableTexCoords)
        .def("setVertexTexCoord", &buffertraits::TexCoordBuffer<3>::setVertexTexCoord);

    py::classh<buffertraits::TexCoordBuffer<2>>(buffertraits, "TexCoordBuffer2D")
        .def("getTexCoords", &buffertraits::TexCoordBuffer<2>::getEditableTexCoords)
        .def("setVertexTexCoord", &buffertraits::TexCoordBuffer<2>::setVertexTexCoord);

    py::classh<buffertraits::TexCoordBuffer<1>>(buffertraits, "TexCoordBuffer1D")
        .def("getTexCoords", &buffertraits::TexCoordBuffer<1>::getEditableTexCoords)
        .def("setVertexTexCoord", &buffertraits::TexCoordBuffer<1>::setVertexTexCoord);

    py::classh<buffertraits::CurvatureBuffer>(buffertraits, "CurvatureBuffer")
        .def("getCurvatures", &buffertraits::CurvatureBuffer::getEditableCurvatures)
        .def("setVertexCurvature", &buffertraits::CurvatureBuffer::setVertexCurvature);

    py::classh<buffertraits::IndexBuffer>(buffertraits, "IndexBuffer")
        .def("getIndex", &buffertraits::IndexBuffer::getEditableIndex)
        .def("setVertexIndex", &buffertraits::IndexBuffer::setVertexIndex);

    py::classh<buffertraits::RadiiBuffer>(buffertraits, "RadiiBuffer")
        .def("getRadii", &buffertraits::RadiiBuffer::getEditableRadii)
        .def("setVertexRadius", &buffertraits::RadiiBuffer::setVertexRadius);

    py::classh<buffertraits::PickingBuffer>(buffertraits, "PickingBuffer")
        .def("getPicking", &buffertraits::PickingBuffer::getEditablePicking)
        .def("setVertexPicking", &buffertraits::PickingBuffer::setVertexPicking);

    py::classh<buffertraits::ScalarMetaBuffer>(buffertraits, "ScalarMetaBuffer")
        .def("getScalarMeta", &buffertraits::ScalarMetaBuffer::getEditableScalarMeta)
        .def("setVertexScalarMeta", &buffertraits::ScalarMetaBuffer::setVertexScalarMeta);

    py::classh<BasicMesh, Mesh, buffertraits::PositionsBuffer, buffertraits::NormalBuffer,
               buffertraits::TexCoordBuffer<3>, buffertraits::ColorsBuffer>(m, "BasicMesh")
        .def(py::init<>())
        .def(py::init<DrawType, ConnectivityType>(), py::arg("dt") = DrawType::Points,
             py::arg("ct") = ConnectivityType::None)
        .def(py::init<DrawType, ConnectivityType, const std::vector<BasicMesh::Vertex>&,
                      const std::vector<std::uint32_t>&>(),
             py::arg("dt"), py::arg("ct"), py::arg("vertices"), py::arg("indices"))
        .def("addVertices", &BasicMesh::addVertices)

        .def("addVertex", [](BasicMesh& self, vec3 pos, vec3 norm, vec3 texCoord,
                             vec4 color) { return self.addVertex(pos, norm, texCoord, color); })
        .def("setVertex", [](BasicMesh& self, size_t i, vec3 pos, vec3 norm, vec3 texCoord,
                             vec4 color) { self.setVertex(i, pos, norm, texCoord, color); })
        .def("setVertex", [](BasicMesh& self, size_t i,
                             const BasicMesh::Vertex& vertex) { self.setVertex(i, vertex); })
        .def("addVertex", [](BasicMesh& self, const BasicMesh::Vertex& vertex) {
            return self.addVertex(vertex);
        });

    py::classh<ColoredMesh, Mesh, buffertraits::PositionsBuffer, buffertraits::ColorsBuffer>(
        m, "ColoredMesh")
        .def(py::init<>())
        .def(py::init<DrawType, ConnectivityType>(), py::arg("dt") = DrawType::Points,
             py::arg("ct") = ConnectivityType::None)
        .def(py::init<DrawType, ConnectivityType, const std::vector<ColoredMesh::Vertex>&,
                      const std::vector<std::uint32_t>&>(),
             py::arg("dt"), py::arg("ct"), py::arg("vertices"), py::arg("indices"))
        .def("addVertices", &ColoredMesh::addVertices)

        .def("addVertex",
             [](ColoredMesh& self, vec3 pos, vec4 color) { return self.addVertex(pos, color); })
        .def("setVertex", [](ColoredMesh& self, size_t i, vec3 pos,
                             vec4 color) { self.setVertex(i, pos, color); })
        .def("setVertex", [](ColoredMesh& self, size_t i,
                             const ColoredMesh::Vertex& vertex) { self.setVertex(i, vertex); })
        .def("addVertex", [](ColoredMesh& self, const ColoredMesh::Vertex& vertex) {
            return self.addVertex(vertex);
        });

    py::classh<SphereMesh, Mesh, buffertraits::PositionsBuffer, buffertraits::RadiiBuffer,
               buffertraits::ColorsBuffer>(m, "SphereMesh")
        .def(py::init<>())
        .def(py::init<DrawType, ConnectivityType>(), py::arg("dt") = DrawType::Points,
             py::arg("ct") = ConnectivityType::None)
        .def(py::init<DrawType, ConnectivityType, const std::vector<SphereMesh::Vertex>&,
                      const std::vector<std::uint32_t>&>(),
             py::arg("dt"), py::arg("ct"), py::arg("vertices"), py::arg("indices"))
        .def("addVertices", &SphereMesh::addVertices)

        .def("addVertex", [](SphereMesh& self, vec3 pos, float radii,
                             vec4 color) { return self.addVertex(pos, radii, color); })
        .def("setVertex", [](SphereMesh& self, size_t i, vec3 pos, float radii,
                             vec4 color) { self.setVertex(i, pos, radii, color); })
        .def("setVertex", [](SphereMesh& self, size_t i,
                             const SphereMesh::Vertex& vertex) { self.setVertex(i, vertex); })
        .def("addVertex", [](SphereMesh& self, const SphereMesh::Vertex& vertex) {
            return self.addVertex(vertex);
        });

    py::classh<PosTexColorMesh, Mesh, buffertraits::PositionsBuffer,
               buffertraits::TexCoordBuffer<3>, buffertraits::ColorsBuffer>(m, "PosTexColorMesh")
        .def(py::init<>())
        .def(py::init<DrawType, ConnectivityType>(), py::arg("dt") = DrawType::Points,
             py::arg("ct") = ConnectivityType::None)
        .def(py::init<DrawType, ConnectivityType, const std::vector<PosTexColorMesh::Vertex>&,
                      const std::vector<std::uint32_t>&>(),
             py::arg("dt"), py::arg("ct"), py::arg("vertices"), py::arg("indices"))
        .def("addVertices", &PosTexColorMesh::addVertices)

        .def("addVertex", [](PosTexColorMesh& self, vec3 pos, vec3 texCoord,
                             vec4 color) { return self.addVertex(pos, texCoord, color); })
        .def("setVertex", [](PosTexColorMesh& self, size_t i, vec3 pos, vec3 texCoord,
                             vec4 color) { self.setVertex(i, pos, texCoord, color); })
        .def("setVertex", [](PosTexColorMesh& self, size_t i,
                             const PosTexColorMesh::Vertex& vertex) { self.setVertex(i, vertex); })
        .def("addVertex", [](PosTexColorMesh& self, const PosTexColorMesh::Vertex& vertex) {
            return self.addVertex(vertex);
        });

    using MeshSequence = std::vector<std::shared_ptr<Mesh>>;
    py::bind_vector<MeshSequence>(m, "MeshSequence");

    exposeStandardDataPorts<Mesh>(m, "Mesh");
    exposeStandardDataPorts<MeshSequence>(m, "MeshSequence");
}

}  // namespace inviwo
