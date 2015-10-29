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

#include <inviwo/core/util/volumevectorsampler.h>

namespace inviwo {

VolumeVectorSampler::VolumeVectorSampler(std::shared_ptr<const std::vector<std::shared_ptr<Volume>>> volumeVector) {
    for (const auto &vol : (*volumeVector.get())) {
        samplers_.push_back(VolumeSampler(vol.get()));
    }
}

VolumeVectorSampler::~VolumeVectorSampler()  {
    
}

dvec4 VolumeVectorSampler::sample(const dvec4 &pos) const
{
    dvec3 spatialPos = pos.xyz();
    double t = pos.w;

    int tIndex = static_cast<int>(t);
    double tInterpolant = t - static_cast<float>(tIndex);

    dvec4 v0, v1;
    v0 = getVoxel(spatialPos, tIndex);
    v1 = getVoxel(spatialPos, tIndex + 1);

    return Interpolation<dvec4>::linear(v0, v1, tInterpolant);
}

dvec4 VolumeVectorSampler::getVoxel(const dvec3 &pos, int T) const
{
    T = glm::clamp(T, 0, static_cast<int>(samplers_.size()) - 1);
    return samplers_[T].sample(pos);
}

} // namespace

