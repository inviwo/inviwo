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

#include <modules/python3/interface/pymesh.h>

#include <inviwo/core/util/formatdispatching.h>

#include <modules/python3/interface/inviwopy.h>
#include <modules/python3/interface/pynetwork.h>
#include <modules/python3/interface/pyglmtypes.h>

#include <inviwo/core/datastructures/geometry/mesh.h>
#include <inviwo/core/datastructures/geometry/basicmesh.h>
#include <inviwo/core/ports/meshport.h>

#include <pybind11/pybind11.h>
#include <pybind11/common.h>
#include <pybind11/numpy.h>

namespace py = pybind11;

namespace inviwo {

void exposeMesh(py::module &m) {
    py::class_<Mesh::MeshInfo>(m, "MeshInfo")
        .def(py::init<>())
        .def(py::init<DrawType, ConnectivityType>())
        .def("__str__",
             [](Mesh::MeshInfo &mi) {
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
        .def("__str__",
             [](Mesh::BufferInfo &bi) {
                 std::ostringstream oss;
                 oss << "BufferInfo (" << bi.type << ", " << bi.location << ")";
                 return oss.str();
             })
        .def_readwrite("type", &Mesh::BufferInfo::type)
        .def_readwrite("location", &Mesh::BufferInfo::location);

    auto getBuffers = [](auto &buffers) {
        pybind11::list list;
        for (auto &buffer : buffers) {
            auto tupl = py::tuple(2);
            tupl[0] = py::cast(buffer.first);
            tupl[1] = py::cast(buffer.second.get(), py::return_value_policy::reference);
            list.append(tupl);
        }
        return list;
    };

    py::class_<Mesh>(m, "Mesh")
        .def(py::init<>())
        .def_property_readonly("dataInfo", &Mesh::getDataInfo)
        .def("addBuffer", [](Mesh *m, Mesh::BufferInfo info,
                             std::shared_ptr<BufferBase> att) { m->addBuffer(info, att); })
        .def("addBuffer", [](Mesh *m, BufferType type,
                             std::shared_ptr<BufferBase> att) { m->addBuffer(type, att); })

        .def("setBuffer", [](Mesh *m, size_t idx, Mesh::BufferInfo info,
                             std::shared_ptr<BufferBase> att) { m->setBuffer(idx, info, att); })
        .def("addIndicies", [](Mesh *m, Mesh::MeshInfo info,
                               std::shared_ptr<IndexBuffer> ind) { m->addIndicies(info, ind); })

        .def("reserveSizeInVertexBuffer", &Mesh::reserveSizeInVertexBuffer)
        .def("reserveIndexBuffers", &Mesh::reserveIndexBuffers)

        .def_property_readonly("buffers", [&](Mesh *m) { return getBuffers(m->getBuffers());})
        .def_property_readonly("indexBuffers", [&](Mesh *m) { return getBuffers(m->getIndexBuffers());})
            
        ;

    py::class_<BasicMesh::Vertex>(m, "BasicMeshVertex")
        .def(py::init<>())
        .def("__init__",
             [](BasicMesh::Vertex &instance, vec3 pos, vec3 normal, vec3 tex, vec4 color) {
                 new (&instance) BasicMesh::Vertex{pos, normal, tex, color};
             });

    py::class_<BasicMesh, Mesh>(m, "BasicMesh")
        .def(pybind11::init<>())
        .def(py::init<>())
        .def("addVertex", &BasicMesh::addVertex)
        .def("addVertices", &BasicMesh::addVertices)

        .def("setVertex", &BasicMesh::setVertex)
        .def("setVertexPosition", &BasicMesh::setVertexPosition)
        .def("setVertexNormal", &BasicMesh::setVertexNormal)
        .def("setVertexTexCoord", &BasicMesh::setVertexTexCoord)
        .def("setVertexColor", &BasicMesh::setVertexColor)

        //.def("addIndexBuffer", &BasicMesh::addIndexBuffer2, py::return_value_policy::reference)
        .def("addIndexBuffer",
             [](BasicMesh *mesh, DrawType dt, ConnectivityType ct) {
                 mesh->addIndexBuffer(dt, ct);
                 return mesh->getIndexBuffers().back().second.get();
             },
             py::return_value_policy::reference)

        .def("getVertices", &BasicMesh::getEditableVertices, py::return_value_policy::reference)
        .def("getTexCoords", &BasicMesh::getEditableTexCoords, py::return_value_policy::reference)
        .def("getColors", &BasicMesh::getEditableColors, py::return_value_policy::reference)
        .def("getNormals", &BasicMesh::getEditableNormals, py::return_value_policy::reference);

    exposeOutport<MeshOutport>(m, "Mesh");
}
}  // namespace
