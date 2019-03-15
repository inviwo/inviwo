/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2019 Inviwo Foundation
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
#include <inviwo/core/util/formatdispatching.h>

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

const DataFormatBase *getDataFormat(size_t components, pybind11::array &arr) {
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
    auto format = DataFormatBase::get(numType, components, arr.itemsize() * 8);
    if (!format) {
        throw pybind11::value_error("Could not map the type of the given array to a inviwo format");
    }
    return format;
}

struct BufferFromArrayDispatcher {
    using type = std::unique_ptr<BufferBase>;

    template <typename Result, typename T>
    std::unique_ptr<BufferBase> operator()(pybind11::array &arr) {
        using Type = typename T::type;
        auto buf = std::make_unique<Buffer<Type>>(arr.shape(0));
        memcpy(buf->getEditableRAMRepresentation()->getData(), arr.data(0), arr.nbytes());
        return buf;
    }
};

struct LayerFromArrayDispatcher {
    using type = std::unique_ptr<Layer>;

    template <typename Result, typename T>
    std::unique_ptr<Layer> operator()(pybind11::array &arr) {
        using Type = typename T::type;
        size2_t dims(arr.shape(0), arr.shape(1));
        auto layerRAM = std::make_shared<LayerRAMPrecision<Type>>(dims);
        memcpy(layerRAM->getData(), arr.data(0), arr.nbytes());
        return std::make_unique<Layer>(layerRAM);
    }
};

struct VolumeFromArrayDispatcher {
    using type = std::unique_ptr<Volume>;

    template <typename Result, typename T>
    std::unique_ptr<Volume> operator()(pybind11::array &arr) {
        using Type = typename T::type;
        size3_t dims(arr.shape(0), arr.shape(1), arr.shape(2));
        auto volumeRAM = std::make_shared<VolumeRAMPrecision<Type>>(dims);
        memcpy(volumeRAM->getData(), arr.data(0), arr.nbytes());
        return std::make_unique<Volume>(volumeRAM);
    }
};

std::unique_ptr<BufferBase> createBuffer(pybind11::array &arr) {
    auto ndim = arr.ndim();
    ivwAssert(ndim == 1 || ndim == 2, "ndims must be either 1 or 2");
    auto df = pyutil::getDataFormat(ndim == 1 ? 1 : arr.shape(1), arr);
    BufferFromArrayDispatcher dispatcher;
    return dispatching::dispatch<std::unique_ptr<BufferBase>, dispatching::filter::All>(
        df->getId(), dispatcher, arr);
}

std::unique_ptr<Layer> createLayer(pybind11::array &arr) {
    auto ndim = arr.ndim();
    ivwAssert(ndim == 2 || ndim == 3, "Ndims must be either 2 or 3");
    auto df = pyutil::getDataFormat(ndim == 2 ? 1 : arr.shape(2), arr);
    LayerFromArrayDispatcher dispatcher;
    return dispatching::dispatch<std::unique_ptr<Layer>, dispatching::filter::All>(df->getId(),
                                                                                   dispatcher, arr);
}

std::unique_ptr<Volume> createVolume(pybind11::array &arr) {
    auto ndim = arr.ndim();
    ivwAssert(ndim == 3 || ndim == 4, "Ndims must be either 3 or 4");
    auto df = pyutil::getDataFormat(ndim == 3 ? 1 : arr.shape(3), arr);
    VolumeFromArrayDispatcher dispatcher;
    return dispatching::dispatch<std::unique_ptr<Volume>, dispatching::filter::All>(
        df->getId(), dispatcher, arr);
}

}  // namespace pyutil
}  // namespace inviwo
