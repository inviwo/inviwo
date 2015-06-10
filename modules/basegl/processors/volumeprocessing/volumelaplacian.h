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

#ifndef IVW_VOLUMELAPLACIAN_H
#define IVW_VOLUMELAPLACIAN_H

#include <modules/basegl/baseglmoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/ports/volumeport.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/datastructures/volume/volumeram.h>
#include <inviwo/core/datastructures/volume/volumeramprecision.h>
#include <limits>

namespace inviwo {

/** \docpage{org.inviwo.VolumeLaplacian, Volume Laplacian}
 * ![](org.inviwo.VolumeLaplacian.png?classIdentifier=org.inviwo.VolumeLaplacian)
 *
 * ...
 * 
 * 
 * 
 *
 */
class IVW_MODULE_BASEGL_API VolumeLaplacian : public Processor {
public:
    VolumeLaplacian();
    virtual ~VolumeLaplacian() {}

    InviwoProcessorInfo();

    virtual void process();

protected:
    template <typename T>
    void processRepresentation();

private:
    template <typename T>
    struct offset {
        offset(T w, ivec3 p) : weight(w), shift(0) {}
        T weight;
        ivec3 shift;
    };

    struct Dispatcher {
        using type = Volume*;
        template <class T>
        Volume* dispatch(const Volume* volume);
    };


    VolumeInport inport_;
    VolumeOutport outport_;
};

template <class DataType>
Volume* inviwo::VolumeLaplacian::Dispatcher::dispatch(const Volume* volume) {
    using T = typename DataType::type;

    LogInfo("Calculating");
    const VolumeRAMPrecision<T>* inRep =
        static_cast<const VolumeRAMPrecision<T>*>(
            volume->getRepresentation<VolumeRAM>()
        );

    Volume* newData = volume->clone();

    VolumeRAMPrecision<T>* outRep =
        static_cast<VolumeRAMPrecision<T>*>(
            newData->getEditableRepresentation<VolumeRAM>()
        );

    ivec3 dim = static_cast<ivec3>(volume->getDimensions());
    
    const T* in = static_cast<const T*>(inRep->getData());
    T* out = static_cast<T*>(outRep->getData());

    T value;
    ivec3 pos(0);

    std::vector<offset<T> > kernel;
    kernel.push_back(offset<T>(T(-6), ivec3(0, 0, 0)));
    kernel.push_back(offset<T>(T(1), ivec3(1, 0, 0)));
    kernel.push_back(offset<T>(T(1), ivec3(-1, 0, 0)));
    kernel.push_back(offset<T>(T(1), ivec3(0, 1, 0)));
    kernel.push_back(offset<T>(T(1), ivec3(0, -1, 0)));
    kernel.push_back(offset<T>(T(1), ivec3(0, 0, 1)));
    kernel.push_back(offset<T>(T(1), ivec3(0, 0, -1)));

    //T minval(std::numeric_limits<T>::max());
    //T maxval(std::numeric_limits<T>::min());
    T mean(0);
    size_t count = 0;

    for (pos.z = 0; pos.z < dim.z; pos.z++) {
        for (pos.y = 0; pos.y < dim.y; pos.y++) {
            for (pos.x = 0; pos.x < dim.x; pos.x++) {
                value = T(0);
                for (const auto& k : kernel) {
                    value += k.weight * in[VolumeRAM::periodicPosToIndex(pos + k.shift, dim)];
                }
                out[VolumeRAM::periodicPosToIndex(pos, dim)] = value;
//                minval = glm::min(minval, value);
//                maxval = glm::max(maxval, value);
                mean += value;
                ++count;
            }
        }
    }
   

    //mean/=count;
    LogInfo("Done: count " << count);

    return newData;
}

} // namespace

#endif // IVW_VOLUMELAPLACIAN_H

