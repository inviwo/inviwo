/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 * Version 0.9
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

#ifndef IVW_VOLUMERAMSLICE_H
#define IVW_VOLUMERAMSLICE_H

#include <inviwo/core/datastructures/image/layerramprecision.h>
#include <inviwo/core/datastructures/volume/volumeram.h>
#include <inviwo/core/datastructures/volume/volumeoperation.h>
#include <inviwo/core/datastructures/geometry/geometrytype.h>

namespace inviwo {

class IVW_CORE_API VolumeRAMSlice : public VolumeOperation {
public:
    VolumeRAMSlice(const VolumeRepresentation* in, CoordinateEnums::CartesianCoordinateAxis sliceAlongAxis, unsigned int sliceNum)
        : VolumeOperation(in), sliceAlongAxis_(sliceAlongAxis), sliceNum_(sliceNum) {}
    virtual ~VolumeRAMSlice() {}

    template<typename T, size_t B>
    void evaluate();

    static inline LayerRAM* apply(const VolumeRepresentation* in, CoordinateEnums::CartesianCoordinateAxis sliceAlongAxis, unsigned int sliceNum) {
        VolumeRAMSlice sliceOP = VolumeRAMSlice(in, sliceAlongAxis, sliceNum);
        in->performOperation(&sliceOP);
        return sliceOP.getOutput<LayerRAM>();
    }

private:
    CoordinateEnums::CartesianCoordinateAxis sliceAlongAxis_;
    unsigned int sliceNum_;
};

template<typename T>
class VolumeRAMPrecision;

template<typename T, size_t B>
class VolumeRAMCustomPrecision;

template<typename T, size_t B>
void VolumeRAMSlice::evaluate() {
    const VolumeRAMPrecision<T>* volume = dynamic_cast<const VolumeRAMPrecision<T>*>(getInputVolume());

    if (!volume) {
        setOutput(NULL);
        return;
    }

    uvec3 dataDims = volume->getDimensions();
    if (sliceAlongAxis_ == CoordinateEnums::X){ // Along z axis (ZY Plane)
        if (sliceNum_ >= dataDims.x){
            setOutput(NULL);
            return;
        }

        //allocate space
        LayerRAMPrecision<T>* sliceImage;
        if (volume->getDataFormat()->getBitsAllocated() != B)
            sliceImage = new LayerRAMCustomPrecision<T, B>(dataDims.zy());
        else
            sliceImage = new LayerRAMPrecision<T>(dataDims.zy());

        const T* src = reinterpret_cast<const T*>(volume->getData());
        T* dst = reinterpret_cast<T*>(sliceImage->getData());

        //copy data
        size_t offsetVolume;
        size_t offsetImage;
        for (size_t i=0; i < dataDims.z; i++) {
            for (size_t j=0; j < dataDims.y; j++) {
                offsetVolume = (i*dataDims.x*dataDims.y) + (j*dataDims.x) + sliceNum_;
                offsetImage = (j*dataDims.z)+i;
                dst[offsetImage] = src[offsetVolume];
            }
        }
        setOutput(sliceImage);
    } else if(sliceAlongAxis_ == CoordinateEnums::Y){ // Along y axis (XZ plane)
        if (sliceNum_ >= dataDims.y){
            setOutput(NULL);
            return;
        }

        //allocate space
        LayerRAMPrecision<T>* sliceImage;
        if (volume->getDataFormat()->getBitsAllocated() != B)
            sliceImage = new LayerRAMCustomPrecision<T, B>(dataDims.xz());
        else
            sliceImage = new LayerRAMPrecision<T>(dataDims.xz());

        const T* src = reinterpret_cast<const T*>(volume->getData());
        T* dst = reinterpret_cast<T*>(sliceImage->getData());

        //copy data
        size_t dataSize = dataDims.x*static_cast<size_t>(volume->getDataFormat()->getBytesAllocated());
        size_t initialStartPos = sliceNum_*dataDims.x;
        size_t offsetVolume;
        size_t offsetImage;
        for (size_t j=0; j < dataDims.z; j++) {
            offsetVolume = (j*dataDims.x*dataDims.y) + initialStartPos;
            offsetImage = j*dataDims.x;
            memcpy(dst + offsetImage, src + offsetVolume, dataSize);
        }
        setOutput(sliceImage);
    } else if(sliceAlongAxis_ == CoordinateEnums::Z){ // Along z axis (XY Plane)
        if (sliceNum_ >= dataDims.z){
            setOutput(NULL);
            return;
        }

        //allocate space
        LayerRAMPrecision<T>* sliceImage;
        if (volume->getDataFormat()->getBitsAllocated() != B)
            sliceImage = new LayerRAMCustomPrecision<T, B>(dataDims.xy());
        else
            sliceImage = new LayerRAMPrecision<T>(dataDims.xy());

        const T* src = reinterpret_cast<const T*>(volume->getData());
        T* dst = reinterpret_cast<T*>(sliceImage->getData());

        //copy data
        size_t dataSize = dataDims.x*static_cast<size_t>(volume->getDataFormat()->getBytesAllocated());
        size_t initialStartPos = sliceNum_*dataDims.x*dataDims.y;
        size_t offset;
        for (size_t j=0; j < dataDims.y; j++) {
            offset = (j*dataDims.x);
            memcpy(dst + offset, (src + offset + initialStartPos), dataSize);
        }
        setOutput(sliceImage);
    }
}

} // namespace

#endif // IVW_VOLUMERAMSLICE_H
