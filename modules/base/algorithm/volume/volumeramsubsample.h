/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2015 Inviwo Foundation
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

#ifndef IVW_VOLUMERAMSUBSAMPLE_H
#define IVW_VOLUMERAMSUBSAMPLE_H

#include <modules/base/basemoduledefine.h>
#include <inviwo/core/datastructures/volume/volumeramprecision.h>
#include <inviwo/core/util/indexmapper.h>

namespace inviwo {

class IVW_MODULE_BASE_API VolumeRAMSubSample {
public:
    enum class Factor : size_t {None = 1, Half = 2, Third = 3, Fourth = 4 };
    static VolumeRAM* apply(const VolumeRepresentation* in, Factor factor);
};

namespace detail {

struct IVW_MODULE_BASE_API VolumeRAMSubSampleDispatcher {
    using type = VolumeRAM*;
    template <class T>
    VolumeRAM* dispatch(const VolumeRepresentation* in, VolumeRAMSubSample::Factor factor);
};

template <class DataType>
VolumeRAM* VolumeRAMSubSampleDispatcher::dispatch(const VolumeRepresentation* in, 
                                                  VolumeRAMSubSample::Factor factor) {
    using T = typename DataType::type;
    using P = typename util::same_extent<T, double>::type;

    const VolumeRAMPrecision<T>* volume = dynamic_cast<const VolumeRAMPrecision<T>*>(in);
    if (!volume) return nullptr;
 
    // calculate new size
    const size3_t dataDims{volume->getDimensions()};
    const size_t f = static_cast<size_t>(factor);
    const size3_t newDims{dataDims / f};

    // allocate space
    VolumeRAMPrecision<T>* newVolume = new VolumeRAMPrecision<T>(newDims);

    // get data pointers
    const T* src = static_cast<const T*>(volume->getData());
    T* dst = static_cast<T*>(newVolume->getData());

    util::IndexMapper o(dataDims);
    util::IndexMapper n(newDims);

    const double samplesInv = 1.0/(f*f*f);
    for (size_t z=0; z < newDims.z; ++z) {
        for (size_t y=0; y < newDims.y; ++y) {
            #pragma omp parallel for
            for (long long xomp=0; xomp < static_cast<long long>(newDims.x); ++xomp) {
                const size_t x = static_cast<size_t>(xomp); // OpenMP need signed integral type.
                const size_t px{x*f};
                const size_t py{y*f};
                const size_t pz{z*f};
                P val{0.0};

                for (size_t oz = 0; oz < f; ++oz) {
                    for (size_t oy = 0; oy < f; ++oy) {
                        for (size_t ox = 0; ox < f; ++ox) {
                            val += src[o(px + ox, py + oy, pz + oz)];
                        }
                    }
                }
 
                #include <warn/push>
                #include <warn/ignore/conversion>
                dst[n(x,y,z)] = static_cast<T>(val*samplesInv);
                #include <warn/pop>
            }
        }
    }

    return newVolume;
}

} // namespace

} // namespace

#endif // IVW_VOLUMERAMSUBSAMPLE_H
