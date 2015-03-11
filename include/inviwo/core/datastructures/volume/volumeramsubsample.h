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

#include <inviwo/core/datastructures/volume/volumeram.h>
#include <inviwo/core/datastructures/volume/volumeoperation.h>

namespace inviwo {

class IVW_CORE_API VolumeRAMSubSample : public VolumeOperation {

//friend class VolumeHalfSampleCalculator;

public:
    enum FACTOR{
        HALF=2
    };

    VolumeRAMSubSample(const VolumeRepresentation* in, FACTOR factor) : VolumeOperation(in), factor_(factor) {}
    virtual ~VolumeRAMSubSample() {}

    template<typename T, size_t B>
    void evaluate();

    static inline VolumeRAM* apply(const VolumeRepresentation* in, FACTOR factor) {
        VolumeRAMSubSample subsampleOP = VolumeRAMSubSample(in, factor);
        in->performOperation(&subsampleOP);
        return subsampleOP.getOutput<VolumeRAM>();
    }

protected:
    template<typename T, typename D>
    void halfsample();

private:
    FACTOR factor_;
};

template<typename T>
class VolumeRAMPrecision;

template<typename T, size_t B>
class VolumeRAMCustomPrecision;

/*template<typename T>
class VolumeHalfSampleCalculator {
public:
    static void calculate(VolumeRAMSubSample* op, T* dst, const T* src, uvec3 dstDims, uvec3 srcDims){
        op->halfsample<T, DataFLOAT64::type>(dst, src, dstDims, srcDims);
    };
};

template<typename T>
class VolumeHalfSampleCalculator<glm::detail::tvec2<T, glm::defaultp> > {
public:
    static void calculate(VolumeRAMSubSample* op, T* dst, const T* src, uvec3 dstDims, uvec3 srcDims){
        op->halfsample<T, DataVec2FLOAT64::type>(dst, src, dstDims, srcDims);
    };
};

template<typename T>
class VolumeHalfSampleCalculator<glm::detail::tvec3<T, glm::defaultp> > {
public:
    static void calculate(VolumeRAMSubSample* op, T* dst, const T* src, uvec3 dstDims, uvec3 srcDims){
        op->halfsample<T, DataVec3FLOAT64::type>(dst, src, dstDims, srcDims);
    };
};

template<typename T>
class VolumeHalfSampleCalculator<glm::detail::tvec4<T, glm::defaultp> > {
public:
    static void calculate(VolumeRAMSubSample* op, T* dst, const T* src, uvec3 dstDims, uvec3 srcDims){
        op->halfsample<T, DataVec4FLOAT64::type>(dst, src, dstDims, srcDims);
    };
};*/

template<typename T, size_t B>
void VolumeRAMSubSample::evaluate() {
    const VolumeRAMPrecision<T>* volume = dynamic_cast<const VolumeRAMPrecision<T>*>(getInputVolume());

    if (!volume) {
        setOutput(NULL);
        return;
    }

    uvec3 dataDims = volume->getDimensions();
    size_t sXY = static_cast<size_t>(dataDims.x*dataDims.y);
    size_t sX = static_cast<size_t>(dataDims.x);

    //calculate new size
    uvec3 newDims = dataDims / static_cast<unsigned int>(factor_);
    size_t dXY = static_cast<size_t>(newDims.x*newDims.y);
    size_t dX = static_cast<size_t>(newDims.x);

    //allocate space
    VolumeRAMPrecision<T>* newVolume = new VolumeRAMCustomPrecision<T, B>(newDims);

    //get data pointers
    const T* src = reinterpret_cast<const T*>(volume->getData());
    T* dst = reinterpret_cast<T*>(newVolume->getData());
    const DataFormatBase* format = volume->getDataFormat();

    //Half sampling
    if(factor_ == HALF){
        //VolumeHalfSampleCalculator<T>::calculate(this, dst, src, dataDims, newDims);
        #define sumCurVal(v) curVal = v; val += format->valueToVec4Double(&curVal)*0.125;
        for (int z=0; z < static_cast<int>(newDims.z); ++z) {
            for (int y=0; y < static_cast<int>(newDims.y); ++y) {
                #pragma omp parallel for
                for (int x=0; x < static_cast<int>(newDims.x); ++x) {
                    size_t px = static_cast<size_t>(x*2);
                    size_t py = static_cast<size_t>(y*2);
                    size_t pz = static_cast<size_t>(z*2);
                    dvec4 val = dvec4(0.0);
                    T curVal;
                    sumCurVal(src[(pz*sXY) + (py*sX) + px]) 
                    sumCurVal(src[(pz*sXY) + (py*sX) + (px+1)])
                    sumCurVal(src[(pz*sXY) + ((py+1)*sX) + px])
                    sumCurVal(src[(pz*sXY) + ((py+1)*sX) + (px+1)])
                    sumCurVal(src[((pz+1)*sXY) + (py*sX) + px])
                    sumCurVal(src[((pz+1)*sXY) + (py*sX) + (px+1)])
                    sumCurVal(src[((pz+1)*sXY) + ((py+1)*sX) + px])
                    sumCurVal(src[((pz+1)*sXY) + ((py+1)*sX) + (px+1)])
                    format->vec4DoubleToValue(val, &curVal);
                    dst[(z*dXY) + (y*dX) + x] = curVal;
                }
            }
        }
    }

    setOutput(newVolume);
}

template<typename T, typename D>
void halfsample(T* dst, const T* src, uvec3 dstDims, uvec3 srcDims){
    size_t sXY = static_cast<size_t>(srcDims.x*srcDims.y);
    size_t sX = static_cast<size_t>(srcDims.x);
    size_t dXY = static_cast<size_t>(dstDims.x*dstDims.y);
    size_t dX = static_cast<size_t>(dstDims.x);

    int x,y,z;
    #pragma omp parallel for
    for (z=0; z < static_cast<int>(dstDims.z); ++z) {
        for (y=0; y < static_cast<int>(dstDims.y); ++y) {
            for (x=0; x < static_cast<int>(dstDims.x); ++x) {
                size_t px = static_cast<size_t>(x*2);
                size_t py = static_cast<size_t>(y*2);
                size_t pz = static_cast<size_t>(z*2);
                D val = D(src[(pz*sXY) + (py*sX) + px])*0.125;
                val += D(src[(pz*sXY) + (py*sX) + (px+1)])*0.125;
                val += D(src[(pz*sXY) + ((py+1)*sX) + px])*0.125;
                val += D(src[(pz*sXY) + ((py+1)*sX) + (px+1)])*0.125;
                val += D(src[((pz+1)*sXY) + (py*sX) + px])*0.125;
                val += D(src[((pz+1)*sXY) + (py*sX) + (px+1)])*0.125;
                val += D(src[((pz+1)*sXY) + ((py+1)*sX) + px])*0.125;
                val += D(src[((pz+1)*sXY) + ((py+1)*sX) + (px+1)])*0.125;
                dst[(z*dXY) + (y*dX) + x] = T(val);
            }
        }
    }
}


} // namespace

#endif // IVW_VOLUMERAMSUBSAMPLE_H
