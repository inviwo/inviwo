/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2020 Inviwo Foundation
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

#include <modules/vectorfieldvisualization/util/unsteadyspatialsampler.h>

namespace inviwo {

UnsteadVolumeDoubleSampler::UnsteadVolumeDoubleSampler(std::shared_ptr<const Volume> vol,
                                                       CoordinateSpace space, bool periodicTime,
                                                       float maxTime)
    : VolumeDoubleSampler(*vol, space), periodicTime_(periodicTime), maxTime_(maxTime) {
    volume_ = vol;
}

UnsteadVolumeDoubleSampler::UnsteadVolumeDoubleSampler(const Volume& vol, CoordinateSpace space,
                                                       bool periodicTime, float maxTime)
    : VolumeDoubleSampler<3>(vol, space), periodicTime_(periodicTime), maxTime_(maxTime) {}

Vector<3, double> UnsteadVolumeDoubleSampler::sampleDataSpace(const dvec3& posIn) const {
    if (!withinBoundsDataSpace(posIn)) {
        return Vector<3, double>(0.0);
    }
    dvec3 pos = posIn;
    if (periodicTime_) pos.z -= int(pos.z);
    auto steadyVec = this->VolumeDoubleSampler<3>::sampleDataSpace(pos);
    steadyVec[2] = 1.0;
    return steadyVec;
}

bool UnsteadVolumeDoubleSampler::withinBoundsDataSpace(const dvec3& posIn) const {
    dvec3 pos = posIn;
    if (periodicTime_ && maxTime_ != 0 && posIn.z > maxTime_) return false;
    if (periodicTime_) pos.z -= int(pos.z);

    bool inBBox = this->VolumeDoubleSampler<3>::withinBoundsDataSpace(pos);
    if (!inBBox) return false;

    const dvec3 samplePos = pos * dvec3(dims_ - size3_t(1));
    const size3_t indexPos = size3_t(samplePos);
    for (int x = 0; x < 2; ++x) {
        for (int y = 0; y < 2; ++y) {
            for (int z = 0; z < 2; ++z) {
                auto voxelVal = getVoxel(indexPos + size3_t(1, 0, 0));
                if (voxelVal.x != 0.0 || voxelVal.y != 0.0) return true;
            }
        }
    }
    return false;
}

}  // namespace inviwo
