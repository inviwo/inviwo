/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2020 Inviwo Foundation
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

#include <modules/discretedata/sampling/datasetspatialsampler.h>

namespace inviwo {
namespace discretedata {

template <unsigned int SpatialDims, unsigned int DataDims, typename T>
class ExtendedSpatialSampler : public SpatialSampler<SpatialDims, DataDims, T> {
public:
    ExtendedSpatialSampler(std::shared_ptr<const DataSetSamplerBase> sampler,
                           InterpolationType interpolationType,
                           std::shared_ptr<const Channel> dataChannel,
                           std::shared_ptr<const Channel> data);
    virtual ~ExtendedSpatialSampler() = default;
    // virtual SpatialEntity<SpatialDims>* clone() const override;

    virtual Vector<DataDims, T> sampleDataSpace(
        const Vector<SpatialDims, double>& pos) const override;
    virtual bool withinBoundsDataSpace(const Vector<SpatialDims, double>& pos) const override;
    InterpolationType interpolationType_;

protected:
    std::shared_ptr<const DataSetSampler<SpatialDims>> sampler_;
    std::shared_ptr<const DataChannel<T, DataDims>> data_;
};

using ExtendedSpatialSampler2D = ExtendedSpatialSampler<2, 2, double>;
using ExtendedSpatialSampler3D = ExtendedSpatialSampler<3, 3, double>;
using ExtendedSpatialSampler4D = ExtendedSpatialSampler<4, 4, double>;

}  // namespace discretedata
}  // namespace inviwo

#include "datasetspatialsampler.inl"
