/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2017 Inviwo Foundation
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

#include <modules/python3/pybindutils.h>

#include <inviwo/core/util/glm.h>
#include <inviwo/core/util/formats.h>

#include <inviwo/core/datastructures/buffer/buffer.h>
#include <inviwo/core/datastructures/buffer/bufferram.h>
#include <inviwo/core/datastructures/buffer/bufferramprecision.h>
#include <inviwo/core/util/stdextensions.h>

namespace inviwo {

namespace pyutil {

const DataFormatBase *getDataFomrat(size_t components, pybind11::array &arr) {
    auto f = arr.request().format;

    if (f == "e") {
        return DataFormatBase::get(NumericType::Float, components, 16);
    } else if (f == "f") {
        return DataFormatBase::get(NumericType::Float, components, 32);
    } else if (f == "d") {
        return DataFormatBase::get(NumericType::Float, components, 64);
    } else if (f == "b") {
        return DataFormatBase::get(NumericType::SignedInteger, components, 8);
    } else if (f == "h") {
        return DataFormatBase::get(NumericType::SignedInteger, components, 16);
    } else if (f == "l") {
        return DataFormatBase::get(NumericType::SignedInteger, components, 32);
    } else if (f == "q") {
        return DataFormatBase::get(NumericType::SignedInteger, components, 64);
    } else if (f == "B") {
        return DataFormatBase::get(NumericType::SignedInteger, components, 8);
    } else if (f == "H") {
        return DataFormatBase::get(NumericType::SignedInteger, components, 16);
    } else if (f == "L") {
        return DataFormatBase::get(NumericType::SignedInteger, components, 32);
    } else if (f == "Q") {
        return DataFormatBase::get(NumericType::SignedInteger, components, 64);
    } else {
        LogInfoCustom("pyutil::dispatch", "Unknown type: " << f);
        throw Exception("pyutil::dispatch: Unknown type", IvwContextCustom("pyutil::dispatch"));
    }
}

struct BufferFromArrayDistpatcher {
    using type = std::shared_ptr<BufferBase>;

    template <typename T>
    std::shared_ptr<BufferBase> dispatch(pybind11::array &arr) {
        using Type = T::type;
        auto buf = std::make_shared<Buffer<Type>>(arr.shape(0));
        memcpy(buf->getEditableRAMRepresentation()->getData(), arr.data(0), arr.nbytes());
        return buf;
    }
};


struct LayerFromArrayDistpatcher {
    using type = std::shared_ptr<Layer>;

    template <typename T>
    std::shared_ptr<Layer> dispatch(pybind11::array &arr) {
        using Type = T::type;
        size2_t dims(arr.shape(0), arr.shape(1));

        auto vol = std::make_shared<Layer>(dims, DataFormat<Type>::get());

        memcpy(vol->getEditableRepresentation<LayerRAM>()->getData(), arr.data(0), arr.nbytes());
        return vol;
    }


};



struct VolumeFromArrayDistpatcher {
    using type = std::shared_ptr<Volume>;

    template <typename T>
    std::shared_ptr<Volume> dispatch(pybind11::array &arr) {
        using Type = T::type;
        size3_t dims(arr.shape(0), arr.shape(1), arr.shape(2));

        auto vol = std::make_shared<Volume>(dims, DataFormat<Type>::get());
        
        memcpy(vol->getEditableRepresentation<VolumeRAM>()->getData(), arr.data(0), arr.nbytes());
        return vol;
    }


};

std::shared_ptr<BufferBase> createBuffer(pybind11::array &arr) {
    auto ndim = arr.ndim();
    ivwAssert(ndim == 1 || ndim == 2, "ndims must be either 1 or 2");
    auto df = pyutil::getDataFomrat(ndim == 1 ? 1 : arr.shape(1), arr);
    BufferFromArrayDistpatcher bufferFromArrayDistpatcher;
    return df->dispatch(bufferFromArrayDistpatcher, arr);
}


std::shared_ptr<Layer> createLayer(pybind11::array &arr) {
    auto ndim = arr.ndim();
    ivwAssert(ndim == 2 || ndim == 3, "Ndims must be either 2 or 3");
    auto df = pyutil::getDataFomrat(ndim == 2 ? 1 : arr.shape(2), arr);
    LayerFromArrayDistpatcher bufferFromArrayDistpatcher;
    return df->dispatch(bufferFromArrayDistpatcher, arr);
}

std::shared_ptr<Volume> createVolume(pybind11::array &arr) {
    auto ndim = arr.ndim();
    ivwAssert(ndim == 3||ndim == 4 , "Ndims must be either 3 or 4");
    auto df = pyutil::getDataFomrat(ndim == 3 ? 1 : arr.shape(3), arr);
    VolumeFromArrayDistpatcher bufferFromArrayDistpatcher;
    return df->dispatch(bufferFromArrayDistpatcher, arr);
}

}  // namespace pyutil
}  // namespace inviwo
