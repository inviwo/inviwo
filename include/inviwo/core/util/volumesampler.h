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

#ifndef IVW_VOLUMESAMPLER_H
#define IVW_VOLUMESAMPLER_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/util/indexmapper.h>


#include <inviwo/core/util/interpolation.h>
#include <inviwo/core/datastructures/volume/volume.h>
#include <inviwo/core/datastructures/volume/volumeram.h>

namespace inviwo {

class Volume;
class VolumeRAM;

/**
 * \class VolumeSampler
 *
 * \brief VERY_BRIEFLY_DESCRIBE_THE_CLASS
 *
 * DESCRIBE_THE_CLASS
 */
class IVW_CORE_API VolumeSampler {
public:
    VolumeSampler(const VolumeRAM *ram);
    VolumeSampler(const Volume *vol);
    virtual ~VolumeSampler();

    dvec4 sample(const dvec3 &pos) const;
    dvec4 sample(double x, double y, double z) const { return sample(dvec3(x, y, z)); }
    dvec4 sample(const vec3 &pos) const { return sample(dvec3(pos)); }

private:
    dvec4 getVoxel(const size3_t &pos)const;
    const VolumeRAM *vol_;
    size3_t dims_;
};

template <typename T,typename P>
class TemplateVolumeSampler {
public:
    TemplateVolumeSampler(const VolumeRAM *ram)
        : data_(static_cast<const T*>(ram->getData())), dims_(ram->getDimensions()), ic_(dims_) {}

    TemplateVolumeSampler(const Volume *vol)
        : TemplateVolumeSampler(vol->getRepresentation<VolumeRAM>()) {}

    virtual ~TemplateVolumeSampler(){}

    T sample(const vec3 &pos) { return sample(dvec3(pos)); }
    T sample(double x, double y, double z) { return sample(dvec3(x, y, z)); }
    T sample(const dvec3 &pos) {
        dvec3 samplePos = pos * dvec3(dims_ - size3_t(1));
        size3_t indexPos = size3_t(samplePos);
        dvec3 interpolants = samplePos - dvec3(indexPos);

        T samples[8];
        samples[0] = getVoxel(indexPos);
        samples[1] = getVoxel(indexPos + size3_t(1, 0, 0));
        samples[2] = getVoxel(indexPos + size3_t(0, 1, 0));
        samples[3] = getVoxel(indexPos + size3_t(1, 1, 0));
        samples[4] = getVoxel(indexPos + size3_t(0, 0, 1));
        samples[5] = getVoxel(indexPos + size3_t(1, 0, 1));
        samples[6] = getVoxel(indexPos + size3_t(0, 1, 1));
        samples[7] = getVoxel(indexPos + size3_t(1, 1, 1));

        return Interpolation<T,P>::trilinear(samples, interpolants);
    }

private:
    T getVoxel(const size3_t &pos) {
        auto p = glm::clamp(pos, size3_t(0), dims_ - size3_t(1));
        return  data_[ic_(p)];
    }
    
    const T *data_;
    size3_t dims_;
    util::IndexMapper3D ic_;
};

}  // namespace

#endif  // IVW_VOLUMESAMPLER_H
