/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2023-2025 Inviwo Foundation
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

#include <inviwo/python3gl/layerpytoglconverter.h>

#include <modules/opengl/texture/texture2d.h>

namespace inviwo {

std::shared_ptr<LayerGL> LayerPy2GLConverter::createFrom(
    std::shared_ptr<const LayerPy> source) const {
    const pybind11::gil_scoped_acquire guard{};

    auto destination = std::make_shared<LayerGL>(source->getDimensions(), source->getLayerType(),
                                                 source->getDataFormat(), source->getSwizzleMask(),
                                                 source->getInterpolation(), source->getWrapping());

    const auto tex = destination->getTexture();
    auto srcSize = static_cast<size_t>(source->data().nbytes());
    auto dstSize = tex->getNumberOfValues() * tex->getSizeInBytes();

    if (srcSize == dstSize &&
        pybind11::array::c_style == (source->data().flags() & pybind11::array::c_style)) {
        auto src = source->data().data(0);
        destination->getTexture()->initialize(src);
    } else {
        throw Exception("Unable to convert from LayerPy to LayerGL");
    }

    return destination;
}

void LayerPy2GLConverter::update(std::shared_ptr<const LayerPy> source,
                                 std::shared_ptr<LayerGL> destination) const {
    const pybind11::gil_scoped_acquire guard{};

    destination->setDimensions(source->getDimensions());
    destination->setSwizzleMask(source->getSwizzleMask());
    destination->setInterpolation(source->getInterpolation());
    destination->setWrapping(source->getWrapping());

    const auto tex = destination->getTexture();
    auto srcSize = static_cast<size_t>(source->data().nbytes());
    auto dstSize = tex->getNumberOfValues() * tex->getSizeInBytes();

    if (srcSize == dstSize &&
        pybind11::array::c_style == (source->data().flags() & pybind11::array::c_style)) {
        auto src = source->data().data(0);
        destination->getTexture()->upload(src);
    } else {
        throw Exception("Unable to convert from LayerPy to LayerGL");
    }
}

std::shared_ptr<LayerPy> LayerGL2PyConverter::createFrom(
    std::shared_ptr<const LayerGL> source) const {
    const pybind11::gil_scoped_acquire guard{};
    auto destination = std::make_shared<LayerPy>(source->getDimensions(), source->getLayerType(),
                                                 source->getDataFormat(), source->getSwizzleMask(),
                                                 source->getInterpolation(), source->getWrapping());

    const auto tex = source->getTexture();
    auto dstSize = static_cast<size_t>(destination->data().nbytes());
    auto srcSize = tex->getNumberOfValues() * tex->getSizeInBytes();

    if (srcSize == dstSize &&
        pybind11::array::c_style == (destination->data().flags() & pybind11::array::c_style)) {
        auto dst = destination->data().mutable_data(0);
        source->getTexture()->download(dst);
    } else {
        throw Exception("Unable to convert from LayerGL to LayerPy");
    }

    return destination;
}

void LayerGL2PyConverter::update(std::shared_ptr<const LayerGL> source,
                                 std::shared_ptr<LayerPy> destination) const {
    const pybind11::gil_scoped_acquire guard{};
    destination->setDimensions(source->getDimensions());
    destination->setSwizzleMask(source->getSwizzleMask());
    destination->setInterpolation(source->getInterpolation());
    destination->setWrapping(source->getWrapping());

    const auto tex = source->getTexture();
    auto dstSize = static_cast<size_t>(destination->data().nbytes());
    auto srcSize = tex->getNumberOfValues() * tex->getSizeInBytes();

    if (srcSize == dstSize &&
        pybind11::array::c_style == (destination->data().flags() & pybind11::array::c_style)) {
        auto dst = destination->data().mutable_data(0);
        source->getTexture()->download(dst);
    } else {
        throw Exception("Unable to convert from LayerGL to LayerPy");
    }
}

}  // namespace inviwo
