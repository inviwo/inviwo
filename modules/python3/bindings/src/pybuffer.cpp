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

#include <inviwopy/pybuffer.h>

#include <inviwo/core/util/formatdispatching.h>

#include <inviwo/core/datastructures/buffer/buffer.h>
#include <inviwo/core/datastructures/buffer/bufferram.h>
#include <inviwo/core/datastructures/buffer/bufferramprecision.h>
#include <inviwo/core/ports/bufferport.h>
#include <inviwo/core/util/glm.h>
#include <inviwo/core/util/formats.h>
#include <inviwo/core/util/stdextensions.h>

#include <inviwopy/inviwopy.h>
#include <inviwopy/pynetwork.h>
#include <inviwopy/pyglmtypes.h>
#include <modules/python3/pybindutils.h>
#include <modules/python3/pyportutils.h>

#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>

#include <fmt/format.h>

namespace inviwo {

struct BufferRAMHelper {
    template <typename DataFormat>
    auto operator()(pybind11::module &m) {
        namespace py = pybind11;
        using T = typename DataFormat::type;

        std::string className = fmt::format("Buffer{}", DataFormat::str());
        py::class_<Buffer<T, BufferTarget::Data>, BufferBase,
                   std::shared_ptr<Buffer<T, BufferTarget::Data>>>(m, className.c_str())
            .def(py::init<size_t>())
            .def(py::init<size_t, BufferUsage>());

        className = fmt::format("Index{}", DataFormat::str());
        py::class_<Buffer<T, BufferTarget::Index>, BufferBase,
                   std::shared_ptr<Buffer<T, BufferTarget::Index>>>(m, className.c_str())
            .def(py::init<size_t>())
            .def(py::init<size_t, BufferUsage>());
    }
};

void exposeBuffer(pybind11::module &m) {
    namespace py = pybind11;

    py::enum_<BufferType>(m, "BufferType")
        .value("PositionAttrib", BufferType::PositionAttrib)
        .value("NormalAttrib", BufferType::NormalAttrib)
        .value("ColorAttrib", BufferType::ColorAttrib)
        .value("TexcoordAttrib", BufferType::TexcoordAttrib)
        .value("CurvatureAttrib", BufferType::CurvatureAttrib)
        .value("IndexAttrib", BufferType::IndexAttrib)
        .value("RadiiAttrib", BufferType::RadiiAttrib)
        .value("PickingAttrib", BufferType::PickingAttrib)
        .value("ScalarMetaAttrib", BufferType::ScalarMetaAttrib)
        .value("NumberOfBufferTypes", BufferType::NumberOfBufferTypes);

    py::enum_<BufferUsage>(m, "BufferUsage")
        .value("Static", BufferUsage::Static)
        .value("Dynamic", BufferUsage::Dynamic);

    py::enum_<BufferTarget>(m, "BufferTarget")
        .value("Data", BufferTarget::Data)
        .value("Index", BufferTarget::Index);

    py::enum_<DrawType>(m, "DrawType")
        .value("NotSpecified", DrawType::NotSpecified)
        .value("Points", DrawType::Points)
        .value("Lines", DrawType::Lines)
        .value("Triangles", DrawType::Triangles);

    py::enum_<ConnectivityType>(m, "ConnectivityType")
        .value("None_", ConnectivityType::None)
        .value("Strip", ConnectivityType::Strip)
        .value("Loop", ConnectivityType::Loop)
        .value("Fan", ConnectivityType::Fan)
        .value("Adjacency", ConnectivityType::Adjacency)
        .value("StripAdjacency", ConnectivityType::StripAdjacency);

    py::class_<BufferBase, std::shared_ptr<BufferBase>>(m, "Buffer")
        .def(py::init([](py::array data) { return pyutil::createBuffer(data).release(); }))
        .def("clone", [](BufferBase &self) { return self.clone(); })
        .def_property("size", &BufferBase::getSize, &BufferBase::setSize)
        .def_property("data",
                      [&](BufferBase *buffer) -> py::array {
                          auto df = buffer->getDataFormat();
                          std::vector<size_t> shape = {buffer->getSize()};
                          std::vector<size_t> strides = {df->getSize()};

                          if (df->getComponents() > 1) {
                              shape.push_back(df->getComponents());
                              strides.push_back(df->getSize() / df->getComponents());
                          }

                          auto data = buffer->getEditableRepresentation<BufferRAM>()->getData();
                          return py::array(pyutil::toNumPyFormat(df), shape, strides, data,
                                           py::cast<>(1));
                      },
                      [](BufferBase *buffer, py::array data) {
                          auto rep = buffer->getEditableRepresentation<BufferRAM>();
                          pyutil::checkDataFormat<1>(rep->getDataFormat(), rep->getSize(), data);

                          memcpy(rep->getData(), data.data(0), data.nbytes());
                      })
        .def("__repr__", [](const BufferBase &self) {
            return fmt::format("<Buffer:\n  target = {}\n  usage = {}\n  format = {}\n  size = {}>",
                               toString(self.getBufferTarget()), toString(self.getBufferUsage()),
                               self.getDataFormat()->getString(), self.getSize());
        });

    util::for_each_type<DefaultDataFormats>{}(BufferRAMHelper{}, m);

    exposeStandardDataPorts<BufferBase>(m, "Buffer");
}
}  // namespace inviwo
