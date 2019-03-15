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

#ifndef IVW_PYBINDUTILS_H
#define IVW_PYBINDUTILS_H

#include <modules/python3/python3moduledefine.h>
#include <inviwo/core/common/inviwo.h>

// push/pop warning state to prevent disabling some warnings by pybind headers
#include <warn/push>
#include <warn/ignore/shadow>
#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include <warn/pop>

#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/network/processornetwork.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/util/formats.h>
#include <inviwo/core/util/stringconversion.h>

namespace inviwo {

class BufferBase;
class Layer;
class Volume;

namespace pyutil {

IVW_MODULE_PYTHON3_API pybind11::dtype toNumPyFormat(const DataFormatBase *df);
IVW_MODULE_PYTHON3_API const DataFormatBase *getDataFormat(size_t components, pybind11::array &arr);
IVW_MODULE_PYTHON3_API std::unique_ptr<BufferBase> createBuffer(pybind11::array &arr);
IVW_MODULE_PYTHON3_API std::unique_ptr<Layer> createLayer(pybind11::array &arr);
IVW_MODULE_PYTHON3_API std::unique_ptr<Volume> createVolume(pybind11::array &arr);

template <int Dim>
void checkDataFormat(const DataFormatBase *format, const Vector<Dim, size_t> &dim,
                     const pybind11::array &data) {
    namespace py = pybind11;
    const auto expectedType = pyutil::toNumPyFormat(format);
    if (!data.dtype().is(expectedType)) {
        throw py::value_error("Invalid data format, got: '" + data.dtype().cast<std::string>() +
                              "' expected: '" + expectedType.cast<std::string>() + "'");
    }

    const auto expectedComponents = format->getComponents();

    std::vector<size_t> expectedDimensions;
    for (int i = 0; i < Dim; i++) {
        expectedDimensions.push_back(util::glmcomp(dim, i));
    }
    if (expectedComponents != 1) {
        expectedDimensions.push_back(expectedComponents);
    }

    if (static_cast<size_t>(data.ndim()) != expectedDimensions.size()) {
        throw py::value_error("Invalid data rank, get: '" + toString(data.ndim()) +
                              "' expected: '" + toString(expectedDimensions.size()) + "'");
    }

    for (size_t i = 0; i < expectedDimensions.size(); i++) {
        if (static_cast<size_t>(data.shape(i)) != expectedDimensions[i]) {
            throw py::value_error("Invalid data dimensions, got '" + toString(data.shape(i)) +
                                  "' expected: '" + toString(expectedDimensions[i]) + "'");
        }
    }
}

template <typename T>
pybind11::dtype toNumPyFormat() {
    return toNumPyFormat(DataFormat<T>::get());
}
template <typename T>
T toPyBindObjectBorrow(PyObject *obj) {
    return pybind11::reinterpret_borrow<T>(pybind11::handle(obj));
}

template <typename T>
T toPyBindObjectSteal(PyObject *obj) {
    return pybind11::reinterpret_steal<T>(pybind11::handle(obj));
}

template <typename T>
pybind11::array toNpArray(const std::vector<T> &v) {
    auto df = DataFormat<T>::get();
    auto componentSize = df->getSize();

    std::vector<size_t> shape = {v.size(), df->getComponents()};
    std::vector<size_t> strides = {componentSize * df->getComponents(), componentSize};

    if (shape.back() == 1) {
        shape.pop_back();
        strides.pop_back();
    }

    bool readOnly = false;
    if (readOnly) {
        return pybind11::array(pyutil::toNumPyFormat(df), shape, strides, v.data());
    } else {
        return pybind11::array(
            /*pyutil::toNumPyFormat(df)*/ pybind11::dtype::of<typename DataFormat<T>::primitive>(),
            shape, strides, v.data(), pybind11::cast<>(1));
    }
}

}  // namespace pyutil

}  // namespace inviwo

#endif  // IVW_NUMPYUTILS_H
