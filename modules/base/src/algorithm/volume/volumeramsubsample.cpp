/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2019 Inviwo Foundation
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

#include <modules/base/algorithm/volume/volumeramsubsample.h>
#include <inviwo/core/datastructures/volume/volumeram.h>
#include <inviwo/core/datastructures/volume/volumeramprecision.h>
#include <inviwo/core/util/indexmapper.h>

namespace inviwo {

std::shared_ptr<VolumeRAM> util::volumeSubSample(const VolumeRAM* volume, size3_t f) {
    return volume->dispatch<std::shared_ptr<VolumeRAM>>(
        [&f](auto srcVol) -> std::shared_ptr<VolumeRAM> {
            using ValueType = util::PrecisionValueType<decltype(srcVol)>;

            // use a double type to perform the summation
            using P = typename util::same_extent<ValueType, double>::type;

            // calculate new size
            const size3_t srcDims{srcVol->getDimensions()};
            const size3_t destDims{srcDims / f};

            // allocate space
            auto destVol = std::make_shared<VolumeRAMPrecision<ValueType>>(destDims);

            // get data pointers
            const auto src = srcVol->getDataTyped();
            auto dst = destVol->getDataTyped();

            util::IndexMapper3D o(srcDims);
            util::IndexMapper3D n(destDims);

            const double samplesInv = 1.0 / (f.x * f.y * f.z);
#pragma omp parallel for
            for (long long z_ = 0; z_ < static_cast<long long>(destDims.z); ++z_) {
                const size_t z = static_cast<size_t>(z_);  // OpenMP need signed integral type.
                for (size_t y = 0; y < destDims.y; ++y) {
                    for (size_t x = 0; x < destDims.x; ++x) {
                        const size_t px{x * f.x};
                        const size_t py{y * f.y};
                        const size_t pz{z * f.z};
                        P val{0.0};

                        for (size_t oz = 0; oz < f.z; ++oz) {
                            for (size_t oy = 0; oy < f.y; ++oy) {
                                for (size_t ox = 0; ox < f.x; ++ox) {
                                    val += src[o(px + ox, py + oy, pz + oz)];
                                }
                            }
                        }

#include <warn/push>
#include <warn/ignore/conversion>
                        dst[n(x, y, z)] = static_cast<ValueType>(val * samplesInv);
#include <warn/pop>
                    }
                }
            }

            return destVol;
        });
}

}  // namespace inviwo
