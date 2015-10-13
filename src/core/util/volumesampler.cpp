/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015 Inviwo Foundation
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

#include <inviwo/core/util/volumesampler.h>

namespace inviwo {

VolumeSampler::VolumeSampler(const VolumeRAM *ram) : vol_(ram), dims_(ram->getDimensions()) {}

VolumeSampler::VolumeSampler(const Volume *vol)
    : VolumeSampler(vol->getRepresentation<VolumeRAM>()) {}

VolumeSampler::~VolumeSampler() {}

dvec4 VolumeSampler::sample(const dvec3 &pos) const {
    dvec3 samplePos = pos * dvec3(dims_-size3_t(1));
    size3_t indexPos = size3_t(samplePos);
    dvec3 interpolants = samplePos - dvec3(indexPos);

    dvec4 samples[8];
    samples[0] = getVoxel(indexPos);
    samples[1] = getVoxel(indexPos + size3_t(1, 0, 0));
    samples[2] = getVoxel(indexPos + size3_t(0, 1, 0));
    samples[3] = getVoxel(indexPos + size3_t(1, 1, 0));

    samples[4] = getVoxel(indexPos + size3_t(0, 0, 1));
    samples[5] = getVoxel(indexPos + size3_t(1, 0, 1));
    samples[6] = getVoxel(indexPos + size3_t(0, 1, 1));
    samples[7] = getVoxel(indexPos + size3_t(1, 1, 1));

    return Interpolation<dvec4>::trilinear(samples, interpolants);
}

inviwo::dvec4 VolumeSampler::getVoxel(const size3_t &pos) const
{
    auto p = glm::clamp(pos, size3_t(0), dims_ - size3_t(1));
    return vol_->getValueAsVec4Double(p);
}

}  // namespace
