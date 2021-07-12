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

template <unsigned int SpatialDims>
DataSetSampler<SpatialDims>::DataSetSampler(
    std::shared_ptr<const Connectivity> grid,
    std::shared_ptr<const DataChannel<double, SpatialDims>> coordinates,
    const Interpolant<SpatialDims>& interpolant)
    : DataSetSamplerBase(grid, std::static_pointer_cast<const Channel>(coordinates))
    , interpolant_(interpolant.copy()) {
    std::cout << "" << std::endl;
    if (!coordinates_) std::cout << "Oopsie doo coordinates!" << std::endl;
    if (!grid_) std::cout << "Oopsie doo grid!" << std::endl;
    // if (coordinates_->getGridPrimitiveType() != GridPrimitive::Vertex ||
    //     coordinates_->getNumComponents() != SpatialDims ||
    //     coordinates_->size() != grid->getNumElements()) {
    //     LogError("Incompatible grid and coordinate channel given, aborting.");
    //     return;
    // }
}

template <unsigned int SpatialDims>
DataSetSampler<SpatialDims>::~DataSetSampler() {
    delete interpolant_;
}

template <unsigned int SpatialDims>
DataSetSampler<SpatialDims>::DataSetSampler(DataSetSampler&& tree)
    : DataSetSampler(tree.grid_, tree.coordinates_) {}

template <unsigned int SpatialDims>
SpatialEntity<SpatialDims>* DataSetSampler<SpatialDims>::clone() const {
    std::cout << "Cloned DataSetSampler (being a good spatial entity)" << std::endl;
    return new DataSetSampler<SpatialDims>(*this);
}

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
