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

#include <modules/discretedata/connectivity/connectivity.h>
#include <modules/discretedata/channels/datachannel.h>
#include <modules/discretedata/sampling/interpolant.h>
#include <inviwo/core/datastructures/geometry/mesh.h>

namespace inviwo {
namespace discretedata {
class DataSetSamplerBase {
public:
    DataSetSamplerBase(std::shared_ptr<const Connectivity> grid,
                       std::shared_ptr<const Channel> coordinates);
    virtual unsigned int getDimension() const = 0;
    bool setInterpolant(const InterpolantBase& interpolant);
    // InterpolantBase* getInterpolantBase();
    const InterpolantBase& getInterpolantBase() const;
    virtual std::string getIdentifier() const = 0;
    virtual Mesh* getDebugMesh() const { return nullptr; }

public:
    const std::shared_ptr<const Connectivity> grid_;
    const std::shared_ptr<const Channel> coordinates_;
};
/**
 * \brief Base class for sampling a grid connectivity.
 **/
template <unsigned int SpatialDims>
class DataSetSampler : public DataSetSamplerBase, public SpatialEntity<SpatialDims> {
protected:
    DataSetSampler(std::shared_ptr<const Connectivity> grid,
                   std::shared_ptr<const DataChannel<double, SpatialDims>> coordinates,
                   const Interpolant<SpatialDims>& interpolant,
                   const std::array<float, SpatialDims>& coordsMin,
                   const std::array<float, SpatialDims>& coordsMax);

public:
    virtual ~DataSetSampler();
    DataSetSampler(DataSetSampler&& sampler);
    DataSetSampler(DataSetSampler& sampler) = delete;
    DataSetSampler& operator=(DataSetSampler&& tree) = delete;
    DataSetSampler& operator=(DataSetSampler& tree) = delete;

    virtual unsigned int getDimension() const { return SpatialDims; }

    virtual ind locateAndSampleCell(
        const std::array<float, SpatialDims>& pos, std::vector<double>& returnWeights,
        std::vector<ind>& returnVertices,
        InterpolationType interpolationType = InterpolationType::Ignore) const = 0;
    void setInterpolant(const Interpolant<SpatialDims>& interpolant);
    const Interpolant<SpatialDims>& getInterpolant() const;
    virtual std::array<float, SpatialDims> getMin() const { return coordsMin_; };
    virtual std::array<float, SpatialDims> getMax() const { return coordsMax_; };

protected:
    Interpolant<SpatialDims>* interpolant_;
    const std::array<float, SpatialDims> coordsMin_, coordsMax_;
};

}  // namespace discretedata
}  // namespace inviwo

#include "datasetsampler.inl"
