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

#ifndef IVW_PYBINDUTILS_H
#define IVW_PYBINDUTILS_H

#include <modules/python3/python3moduledefine.h>
#include <inviwo/core/common/inviwo.h>

#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>

#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/network/processornetwork.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/util/formats.h>

namespace inviwo {

class BufferBase;
class Layer;
class Volume;

namespace pyutil {

IVW_MODULE_PYTHON3_API pybind11::dtype toNumPyFormat(const DataFormatBase *df);
IVW_MODULE_PYTHON3_API const DataFormatBase *getDataFomrat(size_t components, pybind11::array &arr);
IVW_MODULE_PYTHON3_API std::shared_ptr<BufferBase> createBuffer(pybind11::array &arr);
IVW_MODULE_PYTHON3_API std::shared_ptr<Layer> createLayer(pybind11::array &arr);
IVW_MODULE_PYTHON3_API std::shared_ptr<Volume> createVolume(pybind11::array &arr);


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


template<typename T> 
pybind11::array toNpArray(const std::vector<T> &v){
    auto df = DataFormat<T>::get();
    auto componentSize = df->getSize();

    std::vector<size_t> shape = { v.size(), df->getComponents() };
    std::vector<size_t> strides = { componentSize * df->getComponents(), componentSize };

    if(shape.back()==1){
        shape.pop_back();
        strides.pop_back();
    }

    bool readOnly = false;
    if (readOnly) {
        return pybind11::array(pyutil::toNumPyFormat(df), shape, strides, v.data());
    }
    else {
        return pybind11::array(/*pyutil::toNumPyFormat(df)*/ pybind11::dtype::of<DataFormat<T>::primitive>(), shape, strides, v.data(), pybind11::cast<>(1));
    }
}

}  // namespace pyutil

}  // namespace inviwo

#endif  // IVW_NUMPYUTILS_H
