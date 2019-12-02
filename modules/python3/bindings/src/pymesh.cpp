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

#include <inviwopy/pymesh.h>

#include <inviwo/core/util/formatdispatching.h>

#include <inviwopy/inviwopy.h>
#include <inviwopy/pynetwork.h>
#include <inviwopy/pyglmtypes.h>

#include <modules/python3/pyportutils.h>
#include <inviwo/core/datastructures/geometry/mesh.h>
#include <inviwo/core/datastructures/geometry/basicmesh.h>
#include <inviwo/core/ports/meshport.h>

#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>

#include <fmt/format.h>

namespace inviwo {

namespace py = pybind11;

void exposeMesh(pybind11::module& m) {
    py::class_<Mesh::MeshInfo>(m, "MeshInfo")
        .def(py::init<>())
        .def(py::init<DrawType, ConnectivityType>())
        .def("__repr__",
             [](Mesh::MeshInfo& mi) {
                 std::ostringstream oss;
                 oss << "MeshInfo (" << mi.dt << ", " << mi.ct << ")";
                 return oss.str();
             })
        .def_readwrite("dt", &Mesh::MeshInfo::dt)
        .def_readwrite("ct", &Mesh::MeshInfo::ct);

    py::class_<Mesh::BufferInfo>(m, "BufferInfo")
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

    py::class_<Mesh, std::shared_ptr<Mesh>>(m, "Mesh")
        .def(py::init<>())
        .def(py::init<DrawType, ConnectivityType>(), py::arg("dt"), py::arg("ct"))
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
        .def("addIndexBuffer",
             [](Mesh* mesh, DrawType dt, ConnectivityType ct) {
                 mesh->addIndexBuffer(dt, ct);
                 return mesh->getIndexBuffers().back().second.get();
             },
             py::return_value_policy::reference)
        .def("addIndices", [](Mesh* m, Mesh::MeshInfo info,
                              std::shared_ptr<IndexBuffer> ind) { m->addIndices(info, ind); })
        .def("addIndicies",
             [](Mesh* m, Mesh::MeshInfo info, std::shared_ptr<IndexBuffer> ind) {
                 LogInfoCustom("inviwopy.data.Mesh",
                               "Mesh::addIndicies is deprecated, use addIndices");
                 m->addIndices(info, ind);
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

    py::class_<BasicMesh::Vertex>(m, "BasicMeshVertex")
        .def(py::init<>())
        .def(py::init([](vec3 pos, vec3 normal, vec3 tex, vec4 color) {
            return BasicMesh::Vertex{pos, normal, tex, color};
        }));

    py::class_<BasicMesh, Mesh, std::shared_ptr<BasicMesh>>(m, "BasicMesh")
        .def(py::init<>())
        .def("addVertices", &BasicMesh::addVertices)

        .def("addVertex", [](BasicMesh& self, vec3 pos, vec3 norm, vec3 texCoord,
                             vec4 color) { return self.addVertex(pos, norm, texCoord, color); })
        .def("addVertex",
             [](BasicMesh& self, const BasicMesh::Vertex& vertex) {
                 return self.addVertex(std::get<0>(vertex), std::get<1>(vertex),
                                       std::get<2>(vertex), std::get<3>(vertex));
             })

        .def("setVertex", [](BasicMesh& self, size_t i, vec3 pos, vec3 norm, vec3 texCoord,
                             vec4 color) { self.setVertex(i, pos, norm, texCoord, color); })
        .def("setVertex",
             [](BasicMesh& self, size_t i, const BasicMesh::Vertex& vertex) {
                 self.setVertex(i, std::get<0>(vertex), std::get<1>(vertex), std::get<2>(vertex),
                                std::get<3>(vertex));
             })
        .def("setVertexPosition", &BasicMesh::setVertexPosition)
        .def("setVertexNormal", &BasicMesh::setVertexNormal)
        .def("setVertexTexCoord", &BasicMesh::setVertexTexCoord)
        .def("setVertexColor", &BasicMesh::setVertexColor)

        .def("addIndexBuffer",
             [](BasicMesh* mesh, DrawType dt, ConnectivityType ct) {
                 mesh->addIndexBuffer(dt, ct);
                 return mesh->getIndexBuffers().back().second.get();
             },
             py::return_value_policy::reference)

        .def("getVertices", &BasicMesh::getEditableVertices, py::return_value_policy::reference)
        .def("getTexCoords", &BasicMesh::getEditableTexCoords, py::return_value_policy::reference)
        .def("getColors", &BasicMesh::getEditableColors, py::return_value_policy::reference)
        .def("getNormals", &BasicMesh::getEditableNormals, py::return_value_policy::reference);

    exposeStandardDataPorts<Mesh>(m, "Mesh");
}

}  // namespace inviwo
