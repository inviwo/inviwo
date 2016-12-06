/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2016 Inviwo Foundation
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
#include <inviwo/core/ports/meshport.h>
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/properties/buttonproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/stringproperty.h>
#include <inviwo/core/processors/progressbarowner.h>
#include <inviwo/core/datastructures/volume/volumeramprecision.h>
#include <inviwo/core/util/indexmapper.h>

#ifndef __clang__
#include <omp.h>
#endif

namespace inviwo {


/** \docpage{org.inviwo.DistanceTransformRAM, Distance Transform}
 * ![](org.inviwo.DistanceTransformRAM.png?classIdentifier=org.inviwo.DistanceTransformRAM)
 *
 * Computes the distance transform of a binary volume dataset using the data range as low 
 * and high value. The result is the distance from each voxel to the closest feature 
 * (high value). It uses the Saito's algorithm to compute the Euclidean distance.
 * 
 * ### Inports
 *   * __inputVolume__ Binary input volume
 * 
 * ### Outports
 *   * __outputVolume__ Scalar volume representing the distance transform (Uint16)
 * 
 * ### Properties
 *   * __Enabled__              Enables the computation. If disabled, the output is identical 
 *                              to the input volume.
 *   * __Squared Distance__     Use squared distances instead of Euclidean distance.
 *   * __Scaling Factor__       Scales the resulting distances.
 *   * __Update Distance Map__  Triggers a re-computation of the distance transform
 *
 */
class IVW_MODULE_BASE_API DistanceTransformRAM : public Processor, public ProgressBarOwner {
public:  
    DistanceTransformRAM();
    virtual ~DistanceTransformRAM();

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

protected:
    virtual void process() override;

private:
    using int64 = glm::int64;
    using i64vec3 = glm::tvec3<int64>;

    void updateOutport();
    void paramChanged();

    template <typename T, typename U>
    void computeDistanceTransform(const VolumeRAMPrecision<T> *inVolume,
                                  VolumeRAMPrecision<U> *outDistanceField, const size3_t upsample);

    VolumeInport volumePort_;
    VolumeOutport outport_;
    std::shared_ptr<VolumeRAMPrecision<float>> dstRepr_;
    std::shared_ptr<Volume> volDist_;

    BoolProperty transformEnabled_;
    BoolProperty resultSquaredDist_; // determines whether output uses squared euclidean distances
    FloatProperty resultDistScale_;  // scaling factor for distances
    IntSize3Property upsample_;      // Upscale the output field 
    ButtonProperty btnForceUpdate_;

    size3_t volDim_;
    bool distTransformDirty_;

    int numThreads_;
};

/**
 *	implementation of Euclidean Distance Transform according to Saito's algorithm
 *  T. Saito and J.I. Toriwaki. New algorithms for Euclidean distance transformations
 *  of an n-dimensional digitized picture with applications. Pattern Recognition, 27(11).
 *  pp. 1551-1565, 1994.
 */
template <typename T, typename U>
void DistanceTransformRAM::computeDistanceTransform(const VolumeRAMPrecision<T> *inVolume,
                                                    VolumeRAMPrecision<U> *outDistanceField,
                                                    const size3_t upsample) {

    auto square = [](auto a) { return a * a; };

    const T* src = inVolume->getDataTyped();
    U* dst = outDistanceField->getDataTyped();

    const i64vec3 srcDim{inVolume->getDimensions()};
    const i64vec3 dstDim{outDistanceField->getDimensions()};
    const i64vec3 sm{upsample};

    if (srcDim*sm != dstDim) {
        throw Exception("DistanceTransformRAM: Dimensions does not match src = " +
                            toString(srcDim) + " dst = " + toString(dstDim) + " scaling = " +
                            toString(sm),
                        IvwContext);
    }

    

    const U lowVal{0};
    const U highVal{static_cast<U>(3*glm::compMax(dstDim)*glm::compMax(dstDim))};
    
    double totalTime = 0.0;
    Clock clock;
    clock.start();

    util::IndexMapper<3, int64> srcInd(srcDim);
    util::IndexMapper<3, int64> dstInd(dstDim);

    // prepare data based on volume source data
    
    #pragma omp parallel for
    for (int64 z = 0; z < dstDim.z; ++z) {
        for (int64 y = 0; y < dstDim.y; ++y) {
            for (int64 x = 0; x < dstDim.x; ++x) {
                if (util::glm_convert_normalized<double>(
                        src[srcInd(x / sm.x, y / sm.y, z / sm.z)]) > 0.5) {
                    dst[dstInd(x, y, z)] = lowVal;
                } else {
                    dst[dstInd(x, y, z)] = highVal;
                }
            }
        }
    }

    clock.tick();
    LogInfo("preparation done. " << clock.getElapsedMiliseconds() << " ms");
    totalTime += clock.getElapsedSeconds();
    
    // first pass, forward and backward scan along x
    // result: min distance in x direction
    clock.start();
    #pragma omp parallel for
    for (int64 z = 0; z < dstDim.z; ++z) {
        for (int64 y = 0; y < dstDim.y; ++y) {
            // forward
            U minIndex = 1;
            bool feature = (dst[dstInd(0, y, z)] == lowVal);
            for (int64 x = 1; x < dstDim.x; ++x) {
                if (dst[dstInd(x, y, z)] == lowVal) {
                    minIndex = 0;
                    feature = true;
                } else if (feature) {
                    dst[dstInd(x, y, z)] = minIndex * minIndex;
                }
                ++minIndex;
            }
            // backward
            minIndex = 1;
            feature = (dst[dstInd(dstDim.x - 1, y, z)] == lowVal);
            for (int64 x = dstDim.x - 2; x >= 0; --x) {
                if (dst[dstInd(x, y, z)] == lowVal) {
                    minIndex = 0;
                    feature = true;
                } else if (feature) {
                    dst[dstInd(x, y, z)] = std::min<U>(dst[dstInd(x, y, z)], minIndex * minIndex);
                }
                ++minIndex;
            }
        }
    }

    clock.tick();
    updateProgress(progressBar_.getProgress() + 1.0f / 3.0f);
    LogInfo("x finished. " << clock.getElapsedSeconds() << " sec");
    totalTime += clock.getElapsedSeconds();

    // second pass, scan y direction
    // for each voxel v(x,y,z) find min_i(data(x,i,z) + (y - i)^2), 0 <= i < dimY
    // result: min distance in x and y direction
    clock.start();
    #pragma omp parallel
    {
        std::vector<U> colTmp;
        colTmp.resize(dstDim.y);
        #pragma omp for
        for (int64 z = 0; z < dstDim.z; ++z) {
            for (int64 x = 0; x < dstDim.x; ++x) {

                // cache column data into temporary buffer
                for (int64 y = 0; y < dstDim.y; ++y) {
                    colTmp[y] = dst[dstInd(x, y, z)];
                }

                for (int64 y = 0; y < dstDim.y; ++y) {
                    // find minimum in y direction
                    auto minVal = highVal;
                    for (int64 i = 0; i < dstDim.y; ++i) {
                        auto val = colTmp[i] + static_cast<U>(square(y - i));
                        minVal = std::min<U>(minVal, val);
                    }
                    dst[dstInd(x, y, z)] = minVal;
                }
            }
        }
    }

    clock.tick();
    updateProgress(progressBar_.getProgress() + 1.0f / 3.0f);
    LogInfo("y finished. " << clock.getElapsedSeconds() << " sec");
    totalTime += clock.getElapsedSeconds();

    // third pass, scan z direction
    // for each voxel v(x,y,z) find min_i(data(x,y,i) + (z - i)^2), 0 <= i < dimZ
    // result: min distance in x and y direction
    clock.start();
    #pragma omp parallel
    {
        std::vector<U> colTmp;
        colTmp.resize(dstDim.z);
        #pragma omp for
        for (int64 y = 0; y < dstDim.y; ++y) {
            for (int64 x = 0; x < dstDim.x; ++x) {
                // cache column data into temporary buffer
                for (int64 z = 0; z < dstDim.z; ++z) {
                    colTmp[z] = (dst[dstInd(x, y, z)]);
                }

                for (int64 z = 0; z < dstDim.z; ++z) {
                    // find minimum in z direction
                    auto minVal = highVal;
                    for (int64 i = 0; i < dstDim.z; ++i) {
                        auto val = colTmp[i] + static_cast<U>(square(z - i));
                        minVal = std::min<U>(minVal, val);
                    }
                    dst[dstInd(x, y, z)] = minVal;
                }
            }
        }
    }
    clock.tick();
    updateProgress(progressBar_.getProgress() + 1.0f / 3.0f);
    LogInfo("z finished. " << clock.getElapsedSeconds() << " sec");
    totalTime += clock.getElapsedSeconds();
        

    // scale data
    clock.start();
    const int64 volSize = dstDim.x * dstDim.y * dstDim.z;
    auto scale = resultDistScale_.get();
    if (resultSquaredDist_.get()) {
        #pragma omp parallel for
        for (int64 i=0; i < volSize; ++i) {
            dst[i] = static_cast<U>(dst[i] * scale);
        }
    } else {
        // update data to regular distances by applying the square root
        #pragma omp parallel for
        for (int64 i = 0; i < volSize; ++i) {
            dst[i] = static_cast<U>(std::sqrt(static_cast<double>(dst[i])) * scale);
        }
    }
    clock.tick();
    LogInfo("normalization done. " << clock.getElapsedSeconds() << " sec");
    totalTime += clock.getElapsedSeconds();

    LogInfo("Total Time: " << totalTime);
}

} // namespace

#endif // IVW_DISTANCETRANSFORMRAM_H
