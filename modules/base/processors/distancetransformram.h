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

#ifndef IVW_DISTANCETRANSFORMRAM_H
#define IVW_DISTANCETRANSFORMRAM_H

#include <modules/base/basemoduledefine.h>

#include <inviwo/core/util/assertion.h>
#include <inviwo/core/util/clock.h>

#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/ports/volumeport.h>
#include <inviwo/core/ports/geometryport.h>
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/properties/buttonproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/stringproperty.h>
#include <inviwo/core/processors/progressbarowner.h>
#include <inviwo/core/datastructures/volume/volumeram.h>

#ifndef __clang__
#include <omp.h>
#endif

namespace inviwo {

class DistanceTransformRAM : public Processor, public ProgressBarOwner {
public:
    DistanceTransformRAM();
    ~DistanceTransformRAM();

    InviwoProcessorInfo();

    void initialize();
    void deinitialize();

protected:
    virtual void process();

private:
    void updateOutport();
    void paramChanged();

    template <typename T, size_t B>
    void computeDistanceTransform(); 

    VolumeInport volumePort_;
    VolumeOutport outport_;

    BoolProperty transformEnabled_;
    BoolProperty resultSquaredDist_; //determines whether output uses squared euclidean distances
    FloatProperty resultDistScale_;  // scaling factor for distances


    ButtonProperty btnForceUpdate_;

    uvec3 volDim_;
    bool dirty_;
    bool distTransformDirty_;

    int numThreads_;
};

template <typename Type>
Type Square(Type a) { return (a * a); }

template <typename T, size_t B>
void DistanceTransformRAM::computeDistanceTransform() {
    const VolumeRAM *srcVol = volumePort_.getData()->getRepresentation<VolumeRAM>();
    VolumeRAM *vol = outport_.getData()->getEditableRepresentation<VolumeRAM>();

    uvec3 dataDim = vol->getDimensions();
    if (dataDim != srcVol->getDimensions())
        return;

    T lowVal = static_cast<T>(0);
    //T highVal = (vol->getDataFormat()->getNumericType() == FLOAT_TYPE ? static_cast<T>(1.0e10) : std::numeric_limits<T>::max());
    T highVal = std::numeric_limits<T>::max();
    T *data = static_cast<T *>(vol->getData());
    // alternative data access
    //VolumeRAMPrecision<T>* volume = dynamic_cast<VolumeRAMPrecision<T> *>(vol);
    //T *data2 = static_cast<T *>(volume->getData());

    // implementation of Euclidean Distance Transform according to Saito's algorithm
    //  T. Saito and J.I. Toriwaki. New algorithms for Euclidean distance transformations 
    //    of an n-dimensional digitized picture with applications. Pattern Recognition, 27(11). 
    //    pp. 1551–1565, 1994.

    double totalTime = 0.0;

    Clock clock;
    clock.start();

    // prepare data based on volume source data
#pragma omp parallel for
    for (int z=0; z<static_cast<int>(dataDim.z); ++z) {
        for (int y=0; y<static_cast<int>(dataDim.y); ++y) {
            for (std::size_t x=0; x<dataDim.x; ++x) {
                uvec3 pos(x, y, z);
                if (srcVol->getValueAsSingleDouble(pos) > 0.5) {
                    // set distance to zero
                    data[(z * dataDim.y + y) * dataDim.x + x] = lowVal;
                }
                else {
                    data[(z * dataDim.y + y) * dataDim.x + x] = highVal;
                }
            }
        }
    }
    clock.stop();
    LogInfo("preparation done. " << clock.getElapsedMiliseconds() << " ms");
    totalTime += clock.getElapsedSeconds();
    
    // first pass, forward and backward scan along x
    // result: min distance in x direction
    clock.start();
#pragma omp parallel for
    for (int z=0; z<static_cast<int>(dataDim.z); ++z) {
        for (int y=0; y<static_cast<int>(dataDim.y); ++y) {
            unsigned int offset = (z * dataDim.y + y) * dataDim.x;

            // forward
            unsigned int minIndex = 1;
            bool feature = (data[offset] == lowVal);
            for (unsigned int x=1; x<dataDim.x; ++x) {
                unsigned int index = offset + x;
                if (data[index] == lowVal) {
                    minIndex = 0;
                    feature = true;
                }
                else if (feature) { 
                    data[index] = static_cast<T>(minIndex * minIndex);
                }
                ++minIndex;
            }
            // backward
            minIndex = 1;
            feature = (data[offset + dataDim.x - 1] == lowVal);
            for (unsigned int x=dataDim.x - 1; x>0; --x) {
                unsigned int index = offset + x - 1;
                if (data[index] == lowVal) {
                    minIndex = 0;
                    feature = true;
                }
                else if (feature) { 
                    data[index] = std::min(data[index], static_cast<T>(minIndex * minIndex));
                }
                ++minIndex;
            }
        }
    }
    clock.stop();
    updateProgress(progressBar_.getProgress() + 1.0f / 3.0f);
    LogInfo("x finished. " << clock.getElapsedSeconds() << " sec");
    totalTime += clock.getElapsedSeconds();

    // second pass, scan y direction
    //   for each voxel v(x,y,z) find min_i(data(x,i,z) + (y - i)^2), 0 <= i < dimY
    // result: min distance in x and y direction
    clock.start();
#pragma omp parallel
    {
        std::vector<std::size_t> colResult;
        std::vector<std::size_t> colTmp;
        colResult.resize(dataDim.y);
        colTmp.resize(dataDim.y);
#pragma omp for
        for (int z=0; z<static_cast<int>(dataDim.z); ++z) {
            for (int x=0; x<static_cast<int>(dataDim.x); ++x) {
                unsigned int offset = z * dataDim.y * dataDim.x + x;

                // cache column data into temporary buffer
                for (unsigned int y=0; y<dataDim.y; ++y) {
                    colTmp[y] = static_cast<std::size_t>(data[offset + y * dataDim.x]);
                }

                for (unsigned int y=0; y<dataDim.y; ++y) {
                    // find minimum in y direction
                    std::size_t minVal = static_cast<std::size_t>(highVal);
                    for (unsigned int i=0; i<dataDim.y; ++i) {
                        std::size_t val = colTmp[i] + static_cast<std::size_t>(Square((glm::i64)y - (glm::i64)i));
                        minVal = std::min(minVal, val);
                    }
                    data[offset + y * dataDim.x] = static_cast<T>(minVal);
                }
            }
        }
    }
    clock.stop();
    updateProgress(progressBar_.getProgress() + 1.0f / 3.0f);
    LogInfo("y finished. " << clock.getElapsedSeconds() << " sec");
    totalTime += clock.getElapsedSeconds();

    // third pass, scan z direction
    //   for each voxel v(x,y,z) find min_i(data(x,y,i) + (z - i)^2), 0 <= i < dimZ
    // result: min distance in x and y direction
    clock.start();
#pragma omp parallel
    {
        std::vector<std::size_t> colResult;
        std::vector<std::size_t> colTmp;
        colResult.resize(dataDim.z);
        colTmp.resize(dataDim.z);
#pragma omp for
        for (int y=0; y<static_cast<int>(dataDim.y); ++y) {
            for (int x=0; x<static_cast<int>(dataDim.x); ++x) {
                unsigned int offset = y * dataDim.x + x;

                // cache column data into temporary buffer
                for (unsigned int z=0; z<dataDim.z; ++z) {
                    colTmp[z] = static_cast<std::size_t>(data[offset + z * dataDim.y * dataDim.x]);
                }

                for (unsigned int z=0; z<dataDim.z; ++z) {
                    // find minimum in z direction
                    std::size_t minVal = static_cast<std::size_t>(highVal);
                    for (unsigned int i=0; i<dataDim.z; ++i) {
                        std::size_t val = colTmp[i] + static_cast<std::size_t>(Square((glm::i64)z - (glm::i64)i));
                        minVal = std::min(minVal, val);
                    }
                    data[offset + z * dataDim.y * dataDim.x] = static_cast<T>(minVal);
                }
            }
        }
    }
    clock.stop();
    updateProgress(progressBar_.getProgress() + 1.0f / 3.0f);
    LogInfo("z finished. " << clock.getElapsedSeconds() << " sec");
    totalTime += clock.getElapsedSeconds();
        
    if (std::abs(resultDistScale_.get() - 1.0f) > glm::epsilon<float>()) {
        // scale data
        clock.start();
        std::size_t volSize = dataDim.x * dataDim.y * dataDim.z;
        float scale = resultDistScale_.get();
        if (resultSquaredDist_.get()) {
#pragma omp parallel for
            for (glm::i64 i=0; i<static_cast<glm::i64>(volSize); ++i) {
                data[i] = static_cast<T>(data[i] * scale);
            }
        }
        else {
            // update data to regular distances by applying the square root
#pragma omp parallel for
            for (glm::i64 i=0; i<static_cast<glm::i64>(volSize); ++i) {
                data[i] = static_cast<T>(std::sqrt(static_cast<float>(data[i])) * scale);
            }
        }
        clock.stop();
        LogInfo("normalization done. " << clock.getElapsedSeconds() << " sec");
        totalTime += clock.getElapsedSeconds();
    }

    LogInfo("Total Time: " << totalTime);
}

} // namespace

#endif // IVW_DISTANCETRANSFORMRAM_H
