/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2024 Inviwo Foundation
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

#include <modules/base/algorithm/volume/volumeramsubset.h>

#include <inviwo/core/datastructures/volume/volumeborder.h>          // for VolumeBorders
#include <inviwo/core/datastructures/volume/volumeram.h>             // for VolumeRAM
#include <inviwo/core/datastructures/volume/volumerepresentation.h>  // for VolumeRepresentation
#include <inviwo/core/util/formatdispatching.h>                      // for dispatch, All
#include <inviwo/core/util/formats.h>                                // for DataFormatBase
#include <inviwo/core/util/glmvec.h>                                 // for size3_t, ivec3

#include <cstring>  // for size_t, memcpy

#include <glm/common.hpp>  // for max, min
#include <glm/vec3.hpp>    // for operator-, operator+

#ifdef IVW_USE_OPENMP
#include <omp.h>
#endif

namespace inviwo {

namespace {

constexpr auto dispatcher = []<typename T>(
                                const VolumeRepresentation* in, size3_t dim, size3_t offset,
                                const VolumeBorders& border,
                                bool clampBorderOutsideVolume) -> std::shared_ptr<VolumeRAM> {
    const VolumeRAMPrecision<T>* volume = dynamic_cast<const VolumeRAMPrecision<T>*>(in);
    if (!volume) return nullptr;

    // determine parameters
    const size3_t dataDims{volume->getDimensions()};
    const size3_t copyDataDims{static_cast<size3_t>(glm::max(
        static_cast<ivec3>(dim) -
            glm::max(static_cast<ivec3>(offset + dim) - static_cast<ivec3>(dataDims), ivec3(0)),
        ivec3(0)))};

    ivec3 newOffset_Dims = static_cast<ivec3>(glm::min(offset, dataDims) - border.llf);
    VolumeBorders trueBorder = VolumeBorders();
    VolumeBorders correctBorder = border;

    if (clampBorderOutsideVolume) {
        correctBorder.llf += static_cast<size3_t>(-glm::min(newOffset_Dims, ivec3(0, 0, 0)));
        correctBorder.urb += static_cast<size3_t>(
            -glm::min(static_cast<ivec3>(dataDims) -
                          static_cast<ivec3>(offset + copyDataDims + correctBorder.urb),
                      ivec3(0, 0, 0)));
        newOffset_Dims = static_cast<ivec3>(offset - correctBorder.llf);
    } else {
        trueBorder.llf = static_cast<size3_t>(-glm::min(newOffset_Dims, ivec3(0, 0, 0)));
        trueBorder.urb = static_cast<size3_t>(
            glm::max(static_cast<ivec3>(offset + copyDataDims + correctBorder.urb) -
                         static_cast<ivec3>(dataDims),
                     ivec3(0, 0, 0)));
    }

    size3_t newOffset_DimsU = static_cast<size3_t>(glm::max(newOffset_Dims, ivec3(0, 0, 0)));
    size_t initialStartPos = (newOffset_DimsU.z * (dataDims.x * dataDims.y)) +
                             (newOffset_DimsU.y * dataDims.x) + newOffset_DimsU.x;
    size3_t dimsWithBorder = dim + correctBorder.llf + correctBorder.urb;
    size3_t copyDimsWithoutBorder = static_cast<size3_t>(
        glm::max(static_cast<ivec3>(copyDataDims + correctBorder.llf + correctBorder.urb) -
                     static_cast<ivec3>(trueBorder.llf) - static_cast<ivec3>(trueBorder.urb),
                 ivec3(1, 1, 1)));
    // per row
    size_t dataSize =
        copyDimsWithoutBorder.x * static_cast<size_t>(volume->getDataFormat()->getSize());
    // allocate space
    auto newVolume = std::make_shared<VolumeRAMPrecision<T>>(
        dim + correctBorder.llf + correctBorder.urb, in->getSwizzleMask(), in->getInterpolation());

    Wrapping3D wrapping{in->getWrapping()};
    for (int i = 0; i < 3; ++i) {
        // if output dimensions are different from input or a border is present, disable wrapping
        if ((in->getDimensions()[i] != dim[i]) || (correctBorder.llf[i] != 0) ||
            (correctBorder.urb[i] != 0)) {
            wrapping[i] = Wrapping::Clamp;
        }
    }
    newVolume->setWrapping(wrapping);

    const T* src = static_cast<const T*>(volume->getData());
    T* dst = static_cast<T*>(newVolume->getData());
    // memcpy each row for every slice to form sub volume

    for (int i = 0; i < static_cast<int>(copyDimsWithoutBorder.z); i++) {
#ifdef IVW_USE_OPENMP
#pragma omp parallel for
#endif
        for (int j = 0; j < static_cast<int>(copyDimsWithoutBorder.y); j++) {
            size_t volumePos = (j * dataDims.x) + (i * dataDims.x * dataDims.y);
            size_t subVolumePos = ((j + trueBorder.llf.y) * dimsWithBorder.x) +
                                  ((i + trueBorder.llf.z) * dimsWithBorder.x * dimsWithBorder.y) +
                                  trueBorder.llf.x;
            std::memcpy(dst + subVolumePos, (src + volumePos + initialStartPos), dataSize);
        }
    }

    return newVolume;
};

}  // namespace

std::shared_ptr<VolumeRAM> VolumeRAMSubSet::apply(const VolumeRepresentation* in, size3_t dim,
                                                  size3_t offset,
                                                  const VolumeBorders& border /*= VolumeBorders()*/,
                                                  bool clampBorderOutsideVolume /*= true*/) {
    return dispatching::singleDispatch<std::shared_ptr<VolumeRAM>, dispatching::filter::All>(
        in->getDataFormat()->getId(), dispatcher, in, dim, offset, border,
        clampBorderOutsideVolume);
}

}  // namespace inviwo
