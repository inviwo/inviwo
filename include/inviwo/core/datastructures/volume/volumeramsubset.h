/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2015 Inviwo Foundation
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

#ifndef IVW_VOLUMERAMSUBSET_H
#define IVW_VOLUMERAMSUBSET_H

#include <inviwo/core/datastructures/volume/volumeram.h>
#include <inviwo/core/datastructures/volume/volumeborder.h>
#include <inviwo/core/datastructures/volume/volumeoperation.h>

namespace inviwo {

class IVW_CORE_API VolumeRAMSubSet : public VolumeOperation {
public:
    VolumeRAMSubSet(const VolumeRepresentation* in, uvec3 dim, uvec3 offset, const VolumeBorders& border,
                    bool clampBorderOutsideVolume)
        : VolumeOperation(in), newDim_(dim), newOffset_(offset), newBorder_(border), clampBorderOutsideVolume_(clampBorderOutsideVolume) {}
    virtual ~VolumeRAMSubSet() {}

    template<typename T, size_t B>
    void evaluate();

    static inline VolumeRAM* apply(const VolumeRepresentation* in, uvec3 dim, uvec3 offset,
                                   const VolumeBorders& border = VolumeBorders(), bool clampBorderOutsideVolume = true) {
        VolumeRAMSubSet subsetOP = VolumeRAMSubSet(in, dim, offset, border, clampBorderOutsideVolume);
        in->performOperation(&subsetOP);
        return subsetOP.getOutput<VolumeRAM>();
    }

private:
    uvec3 newDim_;
    uvec3 newOffset_;
    VolumeBorders newBorder_;
    bool clampBorderOutsideVolume_;
};

template<typename T>
class VolumeRAMPrecision;

template<typename T, size_t B>
class VolumeRAMCustomPrecision;

template<typename T, size_t B>
void VolumeRAMSubSet::evaluate() {
    const VolumeRAMPrecision<T>* volume = dynamic_cast<const VolumeRAMPrecision<T>*>(getInputVolume());

    if (!volume) {
        setOutput(NULL);
        return;
    }

    uvec3 dataDims = volume->getDimensions();

    if (newOffset_.x > dataDims.x && newOffset_.y > dataDims.y && newOffset_.z > dataDims.z) {
        setOutput(NULL);
        return;
    }

    // determine parameters
    uvec3 copyDataDims = static_cast<uvec3>(glm::max(static_cast<ivec3>(newDim_) - glm::max(static_cast<ivec3>
                                            (newOffset_+newDim_)-static_cast<ivec3>(dataDims), ivec3(0,0,0)), ivec3(0,0,0)));
    ivec3 newOffset_Dims = static_cast<ivec3>(glm::min(newOffset_, dataDims)-newBorder_.llf);
    VolumeBorders trueBorder = VolumeBorders();
    VolumeBorders correctBorder = newBorder_;

    if (clampBorderOutsideVolume_) {
        correctBorder.llf += static_cast<uvec3>(-glm::min(newOffset_Dims, ivec3(0,0,0)));
        correctBorder.urb += static_cast<uvec3>(-glm::min(static_cast<ivec3>(dataDims)-static_cast<ivec3>(newOffset_+copyDataDims
                                                +correctBorder.urb), ivec3(0,0,0)));
        newOffset_Dims = static_cast<ivec3>(newOffset_-correctBorder.llf);
    }
    else {
        trueBorder.llf = static_cast<uvec3>(-glm::min(newOffset_Dims, ivec3(0,0,0)));
        trueBorder.urb = static_cast<uvec3>(glm::max(static_cast<ivec3>(newOffset_+copyDataDims+correctBorder.urb)-static_cast<ivec3>(dataDims),
                                            ivec3(0,0,0)));
    }

    uvec3 newOffset_DimsU = static_cast<uvec3>(glm::max(newOffset_Dims, ivec3(0,0,0)));
    size_t initialStartPos = (newOffset_DimsU.z * (dataDims.x*dataDims.y))+(newOffset_DimsU.y * dataDims.x) + newOffset_DimsU.x;
    uvec3 dimsWithBorder = newDim_+correctBorder.llf+correctBorder.urb;
    uvec3 copyDimsWithoutBorder = static_cast<uvec3>(glm::max(static_cast<ivec3>(copyDataDims+correctBorder.llf+correctBorder.urb)
                                  -static_cast<ivec3>(trueBorder.llf)-static_cast<ivec3>(trueBorder.urb), ivec3(1,1,1)));
    // per row
    size_t dataSize = copyDimsWithoutBorder.x*static_cast<size_t>(volume->getDataFormat()->getBytesAllocated());
    //allocate space
    VolumeRAMPrecision<T>* newVolume;

    if (volume->getDataFormat()->getBitsAllocated() != B)
        newVolume = new VolumeRAMCustomPrecision<T, B>(newDim_+correctBorder.llf+correctBorder.urb);
    else
        newVolume = new VolumeRAMPrecision<T>(newDim_+correctBorder.llf+correctBorder.urb);

    //newVolume->clear();
    const T* src = reinterpret_cast<const T*>(volume->getData());
    T* dst = reinterpret_cast<T*>(newVolume->getData());
    // memcpy each row for every slice to form sub volume

    for (int i=0; i < static_cast<int>(copyDimsWithoutBorder.z); i++) {
        #pragma omp parallel for
        for (int j=0; j < static_cast<int>(copyDimsWithoutBorder.y); j++) {
            size_t volumePos = (j*dataDims.x) + (i*dataDims.x*dataDims.y);
            size_t subVolumePos = ((j+trueBorder.llf.y)*dimsWithBorder.x) + ((i+trueBorder.llf.z)*dimsWithBorder.x*dimsWithBorder.y) + trueBorder.llf.x;
            memcpy(dst + subVolumePos, (src + volumePos + initialStartPos), dataSize);
        }
    }

    setOutput(newVolume);
}

} // namespace

#endif // IVW_VOLUMERAMSUBSET_H
