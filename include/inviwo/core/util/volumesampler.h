/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015-2024 Inviwo Foundation
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

#pragma once

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/util/indexmapper.h>

#include <inviwo/core/util/interpolation.h>
#include <inviwo/core/datastructures/volume/volume.h>
#include <inviwo/core/datastructures/volume/volumeram.h>

#include <inviwo/core/util/spatialsampler.h>

namespace inviwo {

/**
 * \class VolumeSampler
 */
template <typename ReturnType = dvec4>
class VolumeSampler : public SpatialSampler<ReturnType> {
public:
    VolumeSampler(std::shared_ptr<const Volume> vol, CoordinateSpace space = CoordinateSpace::Data);
    VolumeSampler(const Volume& vol, CoordinateSpace space = CoordinateSpace::Data);
    VolumeSampler& operator=(const VolumeSampler&) = default;

    virtual ~VolumeSampler() = default;

protected:
    virtual ReturnType sampleDataSpace(const dvec3& pos) const override;
    virtual bool withinBoundsDataSpace(const dvec3& pos) const override;
    ReturnType getVoxel(const size3_t& pos) const;

    std::shared_ptr<const Volume> volume_;
    const VolumeRAM* ram_;
    size3_t dims_;
};

template <size_t N = 4>
using VolumeDoubleSampler = VolumeSampler<util::glmtype_t<double, N>>;

template <typename ReturnType>
VolumeSampler<ReturnType>::VolumeSampler(std::shared_ptr<const Volume> vol, CoordinateSpace space)
    : VolumeSampler(*vol, space) {
    volume_ = vol;
}

template <typename ReturnType>
VolumeSampler<ReturnType>::VolumeSampler(const Volume& vol, CoordinateSpace space)
    : SpatialSampler<ReturnType>(vol, space)
    , ram_(vol.getRepresentation<VolumeRAM>())
    , dims_(vol.getDimensions()) {}

template <typename ReturnType>
auto VolumeSampler<ReturnType>::sampleDataSpace(const dvec3& pos) const -> ReturnType {
    if (!withinBoundsDataSpace(pos)) {
        return ReturnType(0.0);
    }
    const dvec3 samplePos = pos * dvec3(dims_ - size3_t(1));
    const size3_t indexPos = size3_t(samplePos);
    const dvec3 interpolants = samplePos - dvec3(indexPos);

    ReturnType samples[8];
    samples[0] = getVoxel(indexPos);
    samples[1] = getVoxel(indexPos + size3_t(1, 0, 0));
    samples[2] = getVoxel(indexPos + size3_t(0, 1, 0));
    samples[3] = getVoxel(indexPos + size3_t(1, 1, 0));

    samples[4] = getVoxel(indexPos + size3_t(0, 0, 1));
    samples[5] = getVoxel(indexPos + size3_t(1, 0, 1));
    samples[6] = getVoxel(indexPos + size3_t(0, 1, 1));
    samples[7] = getVoxel(indexPos + size3_t(1, 1, 1));

    return Interpolation<ReturnType, double>::trilinear(samples, interpolants);
}

template <>
inline double VolumeSampler<double>::getVoxel(const size3_t& pos) const {
    const auto p = glm::clamp(pos, size3_t(0), dims_ - size3_t(1));
    return ram_->getAsDouble(p);
}

template <>
inline dvec2 VolumeSampler<dvec2>::getVoxel(const size3_t& pos) const {
    const auto p = glm::clamp(pos, size3_t(0), dims_ - size3_t(1));
    return ram_->getAsDVec2(p);
}

template <>
inline dvec3 VolumeSampler<dvec3>::getVoxel(const size3_t& pos) const {
    const auto p = glm::clamp(pos, size3_t(0), dims_ - size3_t(1));
    return ram_->getAsDVec3(p);
}

template <>
inline dvec4 VolumeSampler<dvec4>::getVoxel(const size3_t& pos) const {
    const auto p = glm::clamp(pos, size3_t(0), dims_ - size3_t(1));
    return ram_->getAsDVec4(p);
}

template <typename ReturnType>
bool VolumeSampler<ReturnType>::withinBoundsDataSpace(const dvec3& pos) const {
    return !(glm::any(glm::lessThan(pos, dvec3(0.0))) ||
             glm::any(glm::greaterThan(pos, dvec3(1.0))));
}

}  // namespace inviwo
