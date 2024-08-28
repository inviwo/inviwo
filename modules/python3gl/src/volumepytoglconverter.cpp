/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2021-2024 Inviwo Foundation
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

#include <inviwo/python3gl/volumepytoglconverter.h>

#include <modules/opengl/texture/texture3d.h>

#include <pybind11/pybind11.h>

namespace inviwo {

std::shared_ptr<VolumeGL> VolumePy2GLConverter::createFrom(
    std::shared_ptr<const VolumePy> volumeSrc) const {
    pybind11::gil_scoped_acquire gil;

    auto volumeDst = std::make_shared<VolumeGL>(
        volumeSrc->getDimensions(), volumeSrc->getDataFormat(), volumeSrc->getSwizzleMask(),
        volumeSrc->getInterpolation(), volumeSrc->getWrapping(), false);

    const auto tex = volumeDst->getTexture();
    auto srcSize = static_cast<size_t>(volumeSrc->data().nbytes());
    auto dstSize = tex->getNumberOfValues() * tex->getSizeInBytes();

    if (srcSize == dstSize &&
        pybind11::array::c_style == (volumeSrc->data().flags() & pybind11::array::c_style)) {
        auto src = volumeSrc->data().data(0);
        volumeDst->getTexture()->initialize(src);
    } else {
        throw Exception("Unable to convert from VolumePy to VolumeGL", IVW_CONTEXT);
    }

    return volumeDst;
}

void VolumePy2GLConverter::update(std::shared_ptr<const VolumePy> volumeSrc,
                                  std::shared_ptr<VolumeGL> volumeDst) const {
    pybind11::gil_scoped_acquire gil;

    volumeDst->setDimensions(volumeSrc->getDimensions());
    volumeDst->setSwizzleMask(volumeSrc->getSwizzleMask());
    volumeDst->setInterpolation(volumeSrc->getInterpolation());
    volumeDst->setWrapping(volumeSrc->getWrapping());

    const auto tex = volumeDst->getTexture();
    auto srcSize = static_cast<size_t>(volumeSrc->data().nbytes());
    auto dstSize = tex->getNumberOfValues() * tex->getSizeInBytes();

    if (srcSize == dstSize &&
        pybind11::array::c_style == (volumeSrc->data().flags() & pybind11::array::c_style)) {
        auto src = volumeSrc->data().data(0);
        volumeDst->getTexture()->upload(src);
    } else {
        throw Exception("Unable to convert from VolumePy to VolumeGL", IVW_CONTEXT);
    }
}

std::shared_ptr<VolumePy> VolumeGL2PyConverter::createFrom(
    std::shared_ptr<const VolumeGL> volumeSrc) const {
    pybind11::gil_scoped_acquire gil;
    auto volumeDst = std::make_shared<VolumePy>(
        volumeSrc->getDimensions(), volumeSrc->getDataFormat(), volumeSrc->getSwizzleMask(),
        volumeSrc->getInterpolation(), volumeSrc->getWrapping());

    const auto tex = volumeSrc->getTexture();
    auto dstSize = static_cast<size_t>(volumeDst->data().nbytes());
    auto srcSize = tex->getNumberOfValues() * tex->getSizeInBytes();

    if (srcSize == dstSize &&
        pybind11::array::c_style == (volumeDst->data().flags() & pybind11::array::c_style)) {
        auto dst = volumeDst->data().mutable_data(0);
        volumeSrc->getTexture()->download(dst);
    } else {
        throw Exception("Unable to convert from VolumeGL to VolumePy", IVW_CONTEXT);
    }

    return volumeDst;
}

void VolumeGL2PyConverter::update(std::shared_ptr<const VolumeGL> volumeSrc,
                                  std::shared_ptr<VolumePy> volumeDst) const {
    pybind11::gil_scoped_acquire gil;

    volumeDst->setDimensions(volumeSrc->getDimensions());
    volumeDst->setSwizzleMask(volumeSrc->getSwizzleMask());
    volumeDst->setInterpolation(volumeSrc->getInterpolation());
    volumeDst->setWrapping(volumeSrc->getWrapping());

    const auto tex = volumeSrc->getTexture();
    auto dstSize = static_cast<size_t>(volumeDst->data().nbytes());
    auto srcSize = tex->getNumberOfValues() * tex->getSizeInBytes();

    if (srcSize == dstSize &&
        pybind11::array::c_style == (volumeDst->data().flags() & pybind11::array::c_style)) {
        auto dst = volumeDst->data().mutable_data(0);
        volumeSrc->getTexture()->download(dst);
    } else {
        throw Exception("Unable to convert from VolumeGL to VolumePy", IVW_CONTEXT);
    }
}

}  // namespace inviwo
