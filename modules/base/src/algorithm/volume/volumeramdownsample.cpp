/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2025 Inviwo Foundation
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

#include <modules/base/algorithm/volume/volumeramdownsample.h>

#include <inviwo/core/datastructures/volume/volumeram.h>  // for VolumeRAM
#include <inviwo/core/util/formatdispatching.h>           // for PrecisionValueType
#include <inviwo/core/util/glmutils.h>                    // for same_extent
#include <inviwo/core/util/glmvec.h>                      // for size3_t
#include <inviwo/core/util/indexmapper.h>                 // for IndexMapper, IndexMapper3D

#include <cstddef>  // for size_t

#include <glm/vec2.hpp>  // for operator*
#include <glm/vec3.hpp>  // for operator*, vec<>::(anonymous)
#include <glm/vec4.hpp>  // for operator*

#ifdef IVW_USE_OPENMP
#include <omp.h>
#endif

namespace inviwo::util {

std::shared_ptr<VolumeRAM> volumeDownsample(const VolumeRAM* volume, size3_t strides,
                                            DownsamplingMode mode) {
    switch (mode) {
        case DownsamplingMode::Strided:
            return volumeStridedDownsample(volume, strides);
        case DownsamplingMode::Averaged:
            return volumeAveragedDownsample(volume, strides);
        default:
            return volumeStridedDownsample(volume, strides);
    }
}

std::shared_ptr<VolumeRAM> volumeStridedDownsample(const VolumeRAM* volume, size3_t strides) {
    return volume->dispatch<std::shared_ptr<VolumeRAM>>(
        [&strides](auto srcVol) -> std::shared_ptr<VolumeRAM> {
            using ValueType = util::PrecisionValueType<decltype(srcVol)>;

            // calculate new size
            const size3_t srcDims{srcVol->getDimensions()};
            const size3_t destDims{srcDims / strides};

            auto destVol = std::make_shared<VolumeRAMPrecision<ValueType>>(destDims);

            const auto src = srcVol->getDataTyped();
            auto dst = destVol->getDataTyped();

            const util::IndexMapper3D sourceMapper(srcDims);
            const util::IndexMapper3D destMapper(destDims);

#ifdef IVW_USE_OPENMP
#pragma omp parallel for
#endif
            for (long long z_ = 0; z_ < static_cast<long long>(destDims.z); ++z_) {
                const auto z = static_cast<size_t>(z_);  // OpenMP requires signed integral type
                for (size_t y = 0; y < destDims.y; ++y) {
                    for (size_t x = 0; x < destDims.x; ++x) {
                        const size_t px{x * strides.x};
                        const size_t py{y * strides.y};
                        const size_t pz{z * strides.z};

                        dst[destMapper(x, y, z)] = src[sourceMapper(px, py, pz)];
                    }
                }
            }
            return destVol;
        });
}

std::shared_ptr<VolumeRAM> volumeAveragedDownsample(const VolumeRAM* volume, size3_t strides) {
    return volume->dispatch<std::shared_ptr<VolumeRAM>>(
        [&strides](auto srcVol) -> std::shared_ptr<VolumeRAM> {
            using ValueType = util::PrecisionValueType<decltype(srcVol)>;

            // use a double type to perform the summation
            using P = typename util::same_extent<ValueType, double>::type;

            // calculate new size
            const size3_t srcDims{srcVol->getDimensions()};
            const size3_t destDims{srcDims / strides};

            auto destVol = std::make_shared<VolumeRAMPrecision<ValueType>>(destDims);

            const auto src = srcVol->getDataTyped();
            auto dst = destVol->getDataTyped();

            const util::IndexMapper3D sourceMapper(srcDims);
            const util::IndexMapper3D destMapper(destDims);
            const double samplesInv = 1.0 / static_cast<double>(glm::compMul(strides));

#ifdef IVW_USE_OPENMP
#pragma omp parallel for
#endif
            for (long long z_ = 0; z_ < static_cast<long long>(destDims.z); ++z_) {
                const auto z = static_cast<size_t>(z_);  // OpenMP requires signed integral type
                for (size_t y = 0; y < destDims.y; ++y) {
                    for (size_t x = 0; x < destDims.x; ++x) {
                        const size_t px{x * strides.x};
                        const size_t py{y * strides.y};
                        const size_t pz{z * strides.z};
                        P val{0.0};

                        for (size_t oz = 0; oz < strides.z; ++oz) {
                            for (size_t oy = 0; oy < strides.y; ++oy) {
                                for (size_t ox = 0; ox < strides.x; ++ox) {
                                    val += static_cast<P>(
                                        src[sourceMapper(px + ox, py + oy, pz + oz)]);
                                }
                            }
                        }

                        dst[destMapper(x, y, z)] = static_cast<ValueType>(val * samplesInv);
                    }
                }
            }

            return destVol;
        });
}

}  // namespace inviwo::util
