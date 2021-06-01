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
#include <modules/discretedata/channels/channeldispatching.h>
#include <inviwo/core/util/zip.h>

#include <stack>

namespace inviwo {
namespace discretedata {

namespace dd_detail {
struct SetInterpolantDispatcher {
    template <typename Result, ind N>
    Result operator()(const InterpolantBase& interpolant, DataSetSamplerBase* sampler) {
        auto* usableInterpolant = dynamic_cast<const Interpolant<unsigned(N)>*>(&interpolant);
        auto usableSampler = dynamic_cast<DataSetSampler<unsigned(N)>*>(sampler);
        if (!usableInterpolant || !usableSampler) return false;

        usableSampler->setInterpolant(*usableInterpolant);
        return true;
    }
};

struct GetInterpolantDispatcher {
    template <typename Result, ind N>
    Result operator()(const DataSetSamplerBase* sampler) {
        auto usableSampler = dynamic_cast<const DataSetSampler<unsigned(N)>*>(sampler);

        return static_cast<const InterpolantBase&>((usableSampler->getInterpolant()));
    }
};
}  // namespace dd_detail

bool DataSetSamplerBase::setInterpolant(const InterpolantBase& interpolant) {
    dd_detail::SetInterpolantDispatcher dispatcher;
    return channeldispatching::dispatchNumber<bool, 1, DISCRETEDATA_MAX_NUM_DIMENSIONS>(
        getDimension(), dispatcher, interpolant, this);
}

const InterpolantBase& DataSetSamplerBase::getInterpolantBase() const {
    dd_detail::GetInterpolantDispatcher dispatcher;
    return channeldispatching::dispatchNumber<const InterpolantBase&, 1,
                                              DISCRETEDATA_MAX_NUM_DIMENSIONS>(getDimension(),
                                                                               dispatcher, this);
}

template <unsigned int SpatialDims>
DataSetSampler<SpatialDims>::DataSetSampler(
    std::shared_ptr<const Connectivity> grid,
    std::shared_ptr<const DataChannel<double, SpatialDims>> coordinates,
    const Interpolant<SpatialDims>& interpolant)
    : grid_(grid), coordinates_(coordinates), interpolant_(interpolant.copy()) {
    if (coordinates_->getGridPrimitiveType() != GridPrimitive::Vertex ||
        coordinates_->getNumComponents() != SpatialDims ||
        coordinates_->size() != grid->getNumElements()) {
        LogError("Incompatible grid and coordinate channel given, aborting.");
        return;
    }
}

template <unsigned int SpatialDims>
DataSetSampler<SpatialDims>::~DataSetSampler() {
    delete interpolant_;
}

template <unsigned int SpatialDims>
DataSetSampler<SpatialDims>::DataSetSampler(DataSetSampler&& tree)
    : grid_(tree.grid_), coordinates_(tree.coordinates_) {}

template <unsigned int SpatialDims>
void DataSetSampler<SpatialDims>::setInterpolant(const Interpolant<SpatialDims>& interpolant) {
    interpolant_ = interpolant.copy();
}

template <unsigned int SpatialDims>
const Interpolant<SpatialDims>& DataSetSampler<SpatialDims>::getInterpolant() const {
    return *interpolant_;
}

}  // namespace discretedata
}  // namespace inviwo
