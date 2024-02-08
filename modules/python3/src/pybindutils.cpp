/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2024 Inviwo Foundation
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

#include <inviwo/core/datastructures/buffer/buffer.h>                   // for BufferBase, Buffer
#include <inviwo/core/datastructures/image/layer.h>                     // for Layer
#include <inviwo/core/datastructures/image/layerram.h>                  // IWYU pragma: keep
#include <inviwo/core/datastructures/representationconverter.h>         // for RepresentationCon...
#include <inviwo/core/datastructures/representationconverterfactory.h>  // for RepresentationCon...
#include <inviwo/core/datastructures/volume/volume.h>                   // for Volume
#include <inviwo/core/util/assertion.h>                                 // for ivwAssert
#include <inviwo/core/util/formatdispatching.h>                         // for dispatch, All
#include <inviwo/core/util/formats.h>                                   // for NumericType, Data...
#include <inviwo/core/util/glmvec.h>                                    // for size2_t, size3_t
#include <inviwo/core/util/exception.h>                                 // for Exception

#include <cstring>        // for memcpy
#include <unordered_map>  // for unordered_map
#include <unordered_set>  // for unordered_set

namespace inviwo {
template <typename T>
class VolumeRAMPrecision;

namespace pyutil {

pybind11::dtype toNumPyFormat(const DataFormatBase* df) {
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

const DataFormatBase* getDataFormat(pybind11::ssize_t components, pybind11::array& arr) {
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
    auto format = DataFormatBase::get(numType, static_cast<size_t>(components),
                                      static_cast<size_t>(arr.itemsize() * 8));
    if (!format) {
        throw pybind11::value_error(
            "Could not map the type of the given array to an inviwo format");
    }
    return format;
}

std::unique_ptr<BufferBase> createBuffer(pybind11::array& arr) {
    auto ndim = arr.ndim();
    ivwAssert(ndim == 1 || ndim == 2, "ndims must be either 1 or 2");
    auto df = pyutil::getDataFormat(ndim == 1 ? 1 : arr.shape(1), arr);

    if (pybind11::array::c_style == (arr.flags() & pybind11::array::c_style)) {
        return dispatching::singleDispatch<std::unique_ptr<BufferBase>, dispatching::filter::All>(
            df->getId(), [&]<typename Type>() {
                auto buf = std::make_unique<Buffer<Type>>(arr.shape(0));
                memcpy(buf->getEditableRAMRepresentation()->getData(), arr.data(0), arr.nbytes());
                return buf;
            });
    } else {
        throw Exception(
            "Unable to create a Buffer from array: The array is not in contiguous C order. Use "
            "numpy.ascontiguousarray() or similar to ensure this.",
            IVW_CONTEXT_CUSTOM("pybindutils"));
    }
}

std::unique_ptr<Layer> createLayer(pybind11::array& arr) {
    auto ndim = arr.ndim();
    ivwAssert(ndim == 2 || ndim == 3, "Ndims must be either 2 or 3");
    auto df = pyutil::getDataFormat(ndim == 2 ? 1 : arr.shape(2), arr);

    if (pybind11::array::c_style == (arr.flags() & pybind11::array::c_style)) {
        return dispatching::singleDispatch<std::unique_ptr<Layer>, dispatching::filter::All>(
            df->getId(), [&]<typename Type>() {
                const size2_t dims(arr.shape(1), arr.shape(0));
                auto layerRAM = std::make_shared<LayerRAMPrecision<Type>>(dims);
                memcpy(layerRAM->getData(), arr.data(0), arr.nbytes());
                return std::make_unique<Layer>(layerRAM);
            });
    } else {
        throw Exception(
            "Unable to create a Layer from array: The array is not in contiguous C order. Use "
            "numpy.ascontiguousarray() or similar to ensure this.",
            IVW_CONTEXT_CUSTOM("pybindutils"));
    }
}

std::unique_ptr<Volume> createVolume(pybind11::array& arr) {
    auto ndim = arr.ndim();
    ivwAssert(ndim == 3 || ndim == 4, "Ndims must be either 3 or 4");
    auto df = pyutil::getDataFormat(ndim == 3 ? 1 : arr.shape(3), arr);

    if (pybind11::array::c_style == (arr.flags() & pybind11::array::c_style)) {
        return dispatching::singleDispatch<std::unique_ptr<Volume>, dispatching::filter::All>(
            df->getId(), [&]<typename Type>() {
                const size3_t dims(arr.shape(2), arr.shape(1), arr.shape(0));
                auto volumeRAM = std::make_shared<VolumeRAMPrecision<Type>>(dims);
                memcpy(volumeRAM->getData(), arr.data(0), arr.nbytes());
                return std::make_unique<Volume>(volumeRAM);
            });
    } else {
        throw Exception(
            "Unable to create a Volume from array: The array is not in contiguous C order. Use "
            "numpy.ascontiguousarray() or similar to ensure this.",
            IVW_CONTEXT_CUSTOM("pybindutils"));
    }
}

}  // namespace pyutil

}  // namespace inviwo
