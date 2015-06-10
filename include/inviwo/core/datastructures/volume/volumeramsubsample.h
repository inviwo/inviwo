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

#include <inviwo/core/datastructures/volume/volumeramprecision.h>

namespace inviwo {

class IVW_CORE_API VolumeRAMSubSample {
public:
    enum class FACTOR : size_t { HALF = 2 };
    static VolumeRAM* apply(const VolumeRepresentation* in, FACTOR factor);
};

namespace detail {

struct IVW_CORE_API VolumeRAMSubSampleDispatcher {
    using type = VolumeRAM*;
    template <class T>
    VolumeRAM* dispatch(const VolumeRepresentation* in, VolumeRAMSubSample::FACTOR factor);
};

template <class DataType>
VolumeRAM* VolumeRAMSubSampleDispatcher::dispatch(const VolumeRepresentation* in,  VolumeRAMSubSample::FACTOR factor) {
    using T = typename DataType::type;
    using P = typename util::same_extent<T, double>::type;

    const VolumeRAMPrecision<T>* volume = dynamic_cast<const VolumeRAMPrecision<T>*>(in);
    if (!volume) return nullptr;
 
    const size3_t dataDims{volume->getDimensions()};
    const size_t sXY{dataDims.x*dataDims.y};
    const size_t sX{dataDims.x};

    //calculate new size
    const size3_t newDims{dataDims / static_cast<size_t>(factor)};
    const size_t dXY{newDims.x*newDims.y};
    const size_t dX{newDims.x};

    //allocate space
    VolumeRAMPrecision<T>* newVolume = new VolumeRAMPrecision<T>(newDims);

    //get data pointers
    const T* src = static_cast<const T*>(volume->getData());
    T* dst = static_cast<T*>(newVolume->getData());

    //Half sampling
    if (factor == VolumeRAMSubSample::FACTOR::HALF) {
        for (long long z=0; z < static_cast<long long>(newDims.z); ++z) {
            for (long long y=0; y < static_cast<long long>(newDims.y); ++y) {
                #pragma omp parallel for
                for (long long x=0; x < static_cast<long long>(newDims.x); ++x) {
                    const long long px{x*2};
                    const long long py{y*2};
                    const long long pz{z*2};
                    P val{0.0};
                    val += src[(pz*sXY) + (py*sX) + px];
                    val += src[(pz*sXY) + (py*sX) + (px+1)];
                    val += src[(pz*sXY) + ((py+1)*sX) + px];
                    val += src[(pz*sXY) + ((py+1)*sX) + (px+1)];
                    val += src[((pz+1)*sXY) + (py*sX) + px];
                    val += src[((pz+1)*sXY) + (py*sX) + (px+1)];
                    val += src[((pz+1)*sXY) + ((py+1)*sX) + px];
                    val += src[((pz+1)*sXY) + ((py+1)*sX) + (px+1)];

                    #include <warn/push>
                    #include <warn/ignore/conversion>
                    dst[(z*dXY) + (y*dX) + x] = static_cast<T>(val*0.125);
                    #include <warn/pop>
                }
            }
        }
    }
    return newVolume;
}

} // namespace

} // namespace

#endif // IVW_VOLUMERAMSUBSAMPLE_H
