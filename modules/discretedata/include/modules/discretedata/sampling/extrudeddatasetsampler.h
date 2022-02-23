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

#include <modules/discretedata/sampling/datasetsampler.h>
// #include <modules/discretedata/connectivity/extrudedgrid.h>
#include <modules/discretedata/connectivity/pointcloud.h>

namespace inviwo {
namespace discretedata {

/**
 * \brief Base class for sampling a grid connectivity.
 **/
template <unsigned int SpatialDims, unsigned int BaseDims>
class ExtrudedDataSetSampler : public DataSetSampler<SpatialDims> {
    // friend class ExtrudedDataSetSamplerBuilder;
    static_assert(SpatialDims > BaseDims,
                  "Needs to have a higher dimension than the base sampler.");

public:
    ExtrudedDataSetSampler(std::shared_ptr<const DataSetSampler<BaseDims>> baseSampler,
                           std::array<const std::shared_ptr<const DataChannel<double, 1>>,
                                      SpatialDims - BaseDims>&& extraDimensions);

    virtual ~ExtrudedDataSetSampler();

    virtual unsigned int getDimension() const { return SpatialDims; }

    virtual ind locateAndSampleCell(
        const std::array<float, SpatialDims>& pos, std::vector<double>& returnWeights,
        std::vector<ind>& returnVertices,
        InterpolationType interpolationType = InterpolationType::Ignore) const;
    void setInterpolant(const Interpolant<SpatialDims>& interpolant);
    const Interpolant<SpatialDims>& getInterpolant() const;
    virtual std::array<float, SpatialDims> getMin() const { return coordsMin_; };
    virtual std::array<float, SpatialDims> getMax() const { return coordsMax_; };

protected:
    const std::shared_ptr<const DataSetSampler<BaseDims>> baseSampler_;
    const std::array<const std::shared_ptr<const DataChannel<double, 1>>, SpatialDims - BaseDims>
        extraDimensions_;
};

template <unsigned int SpatialDims, unsigned int BaseDims>
ExtrudedDataSetSampler<SpatialDims, BaseDims>::ExtrudedDataSetSampler(
    std::shared_ptr<const DataSetSampler<BaseDims>> baseSampler,
    std::array<const std::shared_ptr<const DataChannel<double, 1>>, SpatialDims - BaseDims>&&
        extraDimensions)
    : DataSetSampler<SpatialDims>()
    , baseSampler_(baseSampler)
    , extraDimensions_(std::move(extraDimensions)) {}

}  // namespace discretedata
}  // namespace inviwo

#include "extrudeddatasetsampler.inl"
