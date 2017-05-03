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

#include <modules/python3/interface/pybuffer.h>

#include <inviwo/core/util/formatdispatching.h>

#include <inviwo/core/datastructures/buffer/buffer.h>
#include <inviwo/core/datastructures/buffer/bufferram.h>
#include <inviwo/core/datastructures/buffer/bufferramprecision.h>
#include <inviwo/core/ports/bufferport.h>
#include <inviwo/core/util/glm.h>
#include <inviwo/core/util/formats.h>
#include <inviwo/core/util/stdextensions.h>

#include <modules/python3/interface/inviwopy.h>
#include <modules/python3/interface/pynetwork.h>
#include <modules/python3/interface/pyglmtypes.h>

#include <pybind11/pybind11.h>
#include <pybind11/common.h>
#include <pybind11/numpy.h>


namespace py = pybind11;

namespace inviwo {



    struct BufferRAMHelper{
        template <typename DataFormat>
        auto operator()(pybind11::module &m) {
            using T = typename DataFormat::type;
            std::ostringstream className;
            className << "Buffer" << DataFormat::str();
            py::class_<Buffer<T>,BufferBase>(m, className.str().c_str())
                .def(py::init<size_t>())
                ;
        }
    };



void exposeBuffer(py::module &m) {

    py::class_<BufferBase>(m, "Buffer")
        .def_property_readonly("size", &BufferBase::getSize)
        .def_property_readonly("data", [&](BufferBase &buffer) -> py::array {
            auto func = [&](auto pBuffer) -> py::array {
                using ValueType = util::PrecsionValueType<decltype(pBuffer)>;
                using ComponentType = typename util::value_type<ValueType>::type;

                ComponentType *data = (ComponentType *) pBuffer->getData();

                std::vector<size_t> shape = {pBuffer->getSize(),
                                             pBuffer->getDataFormat()->getComponents()};

                std::vector<size_t> strides = {
                    sizeof(ComponentType) * pBuffer->getDataFormat()->getComponents(),
                    sizeof(ComponentType)};

                bool readOnly = false;
                if (readOnly) {
                    return py::array_t<ComponentType>(shape, strides, (ComponentType *)data);
                } else {
                    return py::array_t<ComponentType>(shape, strides, (ComponentType *)data,
                                                      py::cast<>(1));
                }

            };

            return buffer.getRepresentation<BufferRAM>()->dispatch<py::array>(func);
        });

    util::for_each_type<DefaultDataFormats>{}(BufferRAMHelper{}, m);

    exposeOutport<BufferOutport>(m, "Buffer");
}
}  // namespace
