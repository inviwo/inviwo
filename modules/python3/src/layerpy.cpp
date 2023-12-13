/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2023 Inviwo Foundation
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

#include <modules/python3/layerpy.h>

#include <inviwo/core/datastructures/image/imagetypes.h>

#include <inviwo/core/datastructures/image/imageram.h>
#include <inviwo/core/datastructures/image/imagerepresentation.h>
#include <inviwo/core/util/exception.h>
#include <inviwo/core/util/formatdispatching.h>
#include <inviwo/core/util/formats.h>
#include <inviwo/core/util/glmutils.h>
#include <inviwo/core/util/glmvec.h>
#include <inviwo/core/util/sourcecontext.h>
#include <modules/python3/pybindutils.h>

#include <cstring>
#include <string>
#include <string_view>
#include <type_traits>

#include <glm/vec3.hpp>

namespace inviwo {

namespace {
const DataFormatBase* format(pybind11::array data) {
    const auto ndim = data.ndim();
    if (!(ndim == 2 || ndim == 3)) {
        throw Exception("Ndims must be either 2 or 3, with 1 <= size[2] <= 4",
                        IVW_CONTEXT_CUSTOM("LayerPy"));
    }
    return pyutil::getDataFormat(ndim == 2 ? 1 : data.shape(2), data);
}
}  // namespace

LayerPy::LayerPy(pybind11::array data, LayerType type, const SwizzleMask& swizzleMask,
                 InterpolationType interpolation, const Wrapping2D& wrapping)
    : LayerRepresentation(type)
    , swizzleMask_{swizzleMask}
    , interpolation_{interpolation}
    , wrapping_{wrapping}
    , data_{data}
    , dims_{data_.shape(1), data_.shape(0)} {}

LayerPy::LayerPy(size2_t dimensions, LayerType type, const DataFormatBase* format,
                 const SwizzleMask& swizzleMask, InterpolationType interpolation,
                 const Wrapping2D& wrapping)
    : LayerRepresentation(type)
    , swizzleMask_{swizzleMask}
    , interpolation_{interpolation}
    , wrapping_{wrapping}
    , data_{pybind11::array(pyutil::toNumPyFormat(format),
                            pybind11::array::ShapeContainer{dimensions.y, dimensions.x,
                                                            getDataFormat()->getComponents()})}
    , dims_{dimensions} {}

LayerPy::~LayerPy() {}

LayerPy* LayerPy::clone() const { return new LayerPy(*this); }

std::type_index LayerPy::getTypeIndex() const { return std::type_index(typeid(LayerPy)); }

void LayerPy::setDimensions(size2_t dimensions) {
    if (dimensions != dims_) {
        data_ = pybind11::array(data_.dtype(),
                                pybind11::array::ShapeContainer{dimensions.y, dimensions.x,
                                                                getDataFormat()->getComponents()});
        dims_ = dimensions;
    }
}

const DataFormatBase* LayerPy::getDataFormat() const {
    return format(data_);
}

const size2_t& LayerPy::getDimensions() const { return dims_; }

void LayerPy::setSwizzleMask(const SwizzleMask& mask) { swizzleMask_ = mask; }

SwizzleMask LayerPy::getSwizzleMask() const { return swizzleMask_; }

void LayerPy::setInterpolation(InterpolationType interpolation) { interpolation_ = interpolation; }

InterpolationType LayerPy::getInterpolation() const { return interpolation_; }

void LayerPy::setWrapping(const Wrapping2D& wrapping) { wrapping_ = wrapping; }

Wrapping2D LayerPy::getWrapping() const { return wrapping_; }

bool LayerPy::copyRepresentationsTo(LayerRepresentation*) const { return false; }

std::shared_ptr<LayerPy> LayerRAM2PyConverter::createFrom(
    std::shared_ptr<const LayerRAM> source) const {

    pybind11::array data = source->dispatch<pybind11::array>([](auto lr) {
        using ValueType = util::PrecisionValueType<decltype(lr)>;
        using CompType = typename util::value_type<ValueType>::type;
        constexpr size_t extent = util::extent<ValueType>::value;

        const auto dims = lr->getDimensions();

        auto shape = extent == 1 ? pybind11::array::ShapeContainer{dims.y, dims.x}
                                 : pybind11::array::ShapeContainer{dims.y, dims.x, extent};
        pybind11::array_t<CompType> data{shape};

        if (pybind11::array::c_style == (data.flags() & pybind11::array::c_style)) {
            std::memcpy(data.mutable_data(0), lr->getData(), data.nbytes());
        } else {
            throw Exception(
                "Unable to convert from LayerRM to LayerPy: numpy array is not C-contiguous.",
                IVW_CONTEXT_CUSTOM("LayerRAM2PyConverter"));
        }

        return data;
    });

    auto destination =
        std::make_shared<LayerPy>(data, source->getLayerType(), source->getSwizzleMask(),
                                  source->getInterpolation(), source->getWrapping());
    return destination;
}

void LayerRAM2PyConverter::update(std::shared_ptr<const LayerRAM> source,
                                  std::shared_ptr<LayerPy> destination) const {
    destination->setDimensions(source->getDimensions());
    destination->setSwizzleMask(source->getSwizzleMask());
    destination->setInterpolation(source->getInterpolation());
    destination->setWrapping(source->getWrapping());

    if (pybind11::array::c_style == (destination->data().flags() & pybind11::array::c_style)) {
        std::memcpy(destination->data().mutable_data(0), source->getData(),
                    destination->data().nbytes());
    } else {
        throw Exception(
            "Unable to convert from LayerRAM to LayerPy: numpy array is not C-contiguous.",
            IVW_CONTEXT);
    }
}

std::shared_ptr<LayerRAM> LayerPy2RAMConverter::createFrom(
    std::shared_ptr<const LayerPy> source) const {

    auto destination =
        createLayerRAM(source->getDimensions(), source->getLayerType(), source->getDataFormat(),
                       source->getSwizzleMask(), source->getInterpolation(), source->getWrapping());

    auto dst = destination->getData();
    auto src = source->data().data(0);
    auto size = source->data().nbytes();

    if (pybind11::array::c_style == (source->data().flags() & pybind11::array::c_style)) {
        std::memcpy(dst, src, size);
    } else {
        throw Exception(
            "Unable to convert from LayerPy to LayerRAM: numpy array is not C-contiguous.",
            IVW_CONTEXT);
    }

    return destination;
}

void LayerPy2RAMConverter::update(std::shared_ptr<const LayerPy> source,
                                  std::shared_ptr<LayerRAM> destination) const {
    destination->setDimensions(source->getDimensions());
    destination->setSwizzleMask(source->getSwizzleMask());
    destination->setInterpolation(source->getInterpolation());
    destination->setWrapping(source->getWrapping());

    auto dst = destination->getData();
    auto src = source->data().data(0);
    auto size = source->data().nbytes();

    if (pybind11::array::c_style == (source->data().flags() & pybind11::array::c_style)) {
        std::memcpy(dst, src, size);
    } else {
        throw Exception(
            "Unable to convert from LayerPy to LayerRAM: numpy array is not C-contiguous.",
            IVW_CONTEXT);
    }
}

}  // namespace inviwo
