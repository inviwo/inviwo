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

#include <inviwo/core/datastructures/image/image.h>
#include <inviwo/core/datastructures/image/layer.h>
#include <inviwo/core/datastructures/image/layerram.h>
#include <inviwo/core/datastructures/image/layerramprecision.h>

#include <inviwo/core/datastructures/buffer/buffer.h>
#include <inviwo/core/datastructures/buffer/bufferram.h>
#include <inviwo/core/datastructures/buffer/bufferramprecision.h>

#include <inviwo/core/datastructures/volume/volume.h>
#include <inviwo/core/datastructures/volume/volumeram.h>
#include <inviwo/core/datastructures/volume/volumeramprecision.h>

#include <inviwo/core/util/stdextensions.h>

namespace inviwo {

namespace pyutil {

pybind11::dtype toNumPyFormat(const DataFormatBase *df) {
    std::string format;
    switch (df->getNumericType()) {
        case inviwo::NumericType::Float:
            format = "float";
            break;
        case inviwo::NumericType::SignedInteger:
            format = "int";
            break;
        case inviwo::NumericType::UnsignedInteger:
        default:
            format = "uint";
            break;
    }

    format += std::to_string(df->getPrecision());

    return pybind11::dtype(format);
}

const DataFormatBase *getDataFomrat(size_t components, pybind11::array &arr) {
    auto k = arr.dtype().kind();
    auto numType = [&]() {
        if (k == 'f')
            return NumericType::Float;
        else if (k == 'i')
            return NumericType::SignedInteger;
        else if (k == 'u')
            return NumericType::UnsignedInteger;
        return NumericType::NotSpecialized;
    }();
    return DataFormatBase::get(numType, components, arr.itemsize() * 8);
}

struct BufferFromArrayDistpatcher {
    using type = std::shared_ptr<BufferBase>;

    template <typename T>
    std::shared_ptr<BufferBase> dispatch(pybind11::array &arr) {
        using Type = typename T::type;
        auto buf = std::make_shared<Buffer<Type>>(arr.shape(0));
        memcpy(buf->getEditableRAMRepresentation()->getData(), arr.data(0), arr.nbytes());
        return buf;
    }
};

struct LayerFromArrayDistpatcher {
    using type = std::shared_ptr<Layer>;

    template <typename T>
    std::shared_ptr<Layer> dispatch(pybind11::array &arr) {
        using Type = typename T::type;
        size2_t dims(arr.shape(0), arr.shape(1));
        auto layerRAM = std::make_shared<LayerRAMPrecision<Type>>(dims);
        memcpy(layerRAM->getData(), arr.data(0), arr.nbytes());
        return std::make_shared<Layer>(layerRAM);
    }
};

struct VolumeFromArrayDistpatcher {
    using type = std::shared_ptr<Volume>;

    template <typename T>
    std::shared_ptr<Volume> dispatch(pybind11::array &arr) {
        using Type = typename T::type;
        size3_t dims(arr.shape(0), arr.shape(1), arr.shape(2));
        auto volumeRAM = std::make_shared<VolumeRAMPrecision<Type>>(dims);
        memcpy(volumeRAM->getData(), arr.data(0), arr.nbytes());
        return std::make_shared<Volume>(volumeRAM);
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
    ivwAssert(ndim == 3 || ndim == 4, "Ndims must be either 3 or 4");
    auto df = pyutil::getDataFomrat(ndim == 3 ? 1 : arr.shape(3), arr);
    VolumeFromArrayDistpatcher bufferFromArrayDistpatcher;
    return df->dispatch(bufferFromArrayDistpatcher, arr);
}

}  // namespace pyutil
}  // namespace inviwo
