/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2019 Inviwo Foundation
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

#ifndef IVW_TEMPLATESAMPLER_H
#define IVW_TEMPLATESAMPLER_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/inviwo.h>

#include <inviwo/core/util/indexmapper.h>

#include <inviwo/core/util/interpolation.h>
#include <inviwo/core/datastructures/volume/volume.h>
#include <inviwo/core/datastructures/volume/volumeram.h>

#include <inviwo/core/util/spatialsampler.h>

namespace inviwo {

namespace detail {

template <typename T>
constexpr size_t components() {
    return util::rank<T>::value == 0 ? 1 : util::extent<T, 0>::value;
}

template <typename T>
using componentType =
    typename std::conditional<util::rank<T>::value == 0, T, typename T::value_type>::type;

}  // namespace detail

template <typename DataType, typename P, typename T = detail::componentType<DataType>,
          unsigned int DataDims = detail::components<DataType>()>
class TemplateVolumeSampler : public SpatialSampler<3, DataDims, T> {
public:
    static_assert(DataDims > 0, "zero extent");

    TemplateVolumeSampler(std::shared_ptr<const Volume> sharedVolume,
                          CoordinateSpace space = CoordinateSpace::Data);

    TemplateVolumeSampler(const Volume &sharedVolume,
                          CoordinateSpace space = CoordinateSpace::Data);
    virtual ~TemplateVolumeSampler() = default;

    virtual Vector<DataDims, T> sampleDataSpace(const dvec3 &pos) const override;

private:
    Vector<DataDims, T> getVoxel(const size3_t &pos) const;
    virtual bool withinBoundsDataSpace(const dvec3 &pos) const override;

    const DataType *data_;
    size3_t dims_;
    util::IndexMapper3D ic_;
    std::shared_ptr<const Volume> sharedVolume_;
};

template <typename DataType, typename P, typename T, unsigned int DataDims>
TemplateVolumeSampler<DataType, P, T, DataDims>::TemplateVolumeSampler(
    std::shared_ptr<const Volume> sharedVolume, CoordinateSpace space)
    : TemplateVolumeSampler(*sharedVolume, space) {
    sharedVolume_ = sharedVolume;
}

template <typename DataType, typename P, typename T, unsigned int DataDims>
TemplateVolumeSampler<DataType, P, T, DataDims>::TemplateVolumeSampler(const Volume &volume,
                                                                       CoordinateSpace space)
    : SpatialSampler<3, DataDims, T>(volume, space)
    , data_(static_cast<const DataType *>(volume.getRepresentation<VolumeRAM>()->getData()))
    , dims_(volume.getRepresentation<VolumeRAM>()->getDimensions())
    , ic_(dims_) {}

template <typename DataType, typename P, typename T, unsigned int DataDims>
bool TemplateVolumeSampler<DataType, P, T, DataDims>::withinBoundsDataSpace(
    const dvec3 &pos) const {
    if (glm::any(glm::lessThan(pos, dvec3(0.0)))) {
        return false;
    }
    if (glm::any(glm::greaterThan(pos, dvec3(1.0)))) {
        return false;
    }
    return true;
}

template <typename DataType, typename P, typename T, unsigned int DataDims>
Vector<DataDims, T> TemplateVolumeSampler<DataType, P, T, DataDims>::sampleDataSpace(
    const dvec3 &pos) const {
    if (!withinBoundsDataSpace(pos)) {
        return Vector<DataDims, T>{0};
    }
    
    //const dvec3 samplePos = pos * dvec3(dims_ - size3_t(1));
    const dvec3 samplePos = pos * dvec3(dims_) - 0.5;
    const size3_t indexPos = size3_t(samplePos);
    const dvec3 interpolants = samplePos - dvec3(indexPos);

    Vector<DataDims, T> samples[8];
    samples[0] = getVoxel(indexPos);
    samples[1] = getVoxel(indexPos + size3_t(1, 0, 0));
    samples[2] = getVoxel(indexPos + size3_t(0, 1, 0));
    samples[3] = getVoxel(indexPos + size3_t(1, 1, 0));

    if (interpolants.z < std::numeric_limits<double>::epsilon()) {
        return Interpolation<Vector<DataDims, T>, P>::bilinear(samples, dvec2(interpolants));
    }

    samples[4] = getVoxel(indexPos + size3_t(0, 0, 1));
    samples[5] = getVoxel(indexPos + size3_t(1, 0, 1));
    samples[6] = getVoxel(indexPos + size3_t(0, 1, 1));
    samples[7] = getVoxel(indexPos + size3_t(1, 1, 1));

    return Interpolation<Vector<DataDims, T>, P>::trilinear(samples, interpolants);
}

template <typename DataType, typename P, typename T, unsigned int DataDims>
Vector<DataDims, T> TemplateVolumeSampler<DataType, P, T, DataDims>::getVoxel(
    const size3_t &pos) const {
    return static_cast<const Vector<DataDims, T>>(data_[ic_(pos)]);
}

}  // namespace inviwo

#endif  // IVW_TEMPLATESAMPLER_H
