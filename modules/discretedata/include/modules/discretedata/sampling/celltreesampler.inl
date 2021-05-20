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

#include <modules/discretedata/sampling/celltree.h>
#include <modules/discretedata/util/spatialentitychannel.h>
#include <inviwo/core/util/interpolation.h>
#include <inviwo/core/util/spatialsampler.h>

namespace inviwo {
namespace discretedata {

template <unsigned int SpatialDims, unsigned int DataDims, typename T>
CellTreeSampler<SpatialDims, DataDims, T>::CellTreeSampler(
    std::shared_ptr<const Connectivity> grid,
    std::shared_ptr<const DataChannel<double, SpatialDims>> coords,
    std::shared_ptr<const DataChannel<T, DataDims>> data)
    : tree_(grid, coords)
    , coordinates_(coords)
    , data_(data)
    , SpatialSampler<SpatialDims, DataDims, T>(SpatialEntityChannel<T, SpatialDims>(coords)) {}

// template <unsigned int SpatialDims, unsigned int DataDims, typename T>
// CellTreeSampler<SpatialDims, DataDims, T>::CellTreeSampler(const Volume& vol,
//                                                            CoordinateSpace space) {}

template <unsigned int SpatialDims, unsigned int DataDims, typename T>
Vector<DataDims, T> CellTreeSampler<SpatialDims, DataDims, T>::sampleDataSpace(
    const Vector<SpatialDims, double>& pos) const {}

template <unsigned int SpatialDims, unsigned int DataDims, typename T>
bool CellTreeSampler<SpatialDims, DataDims, T>::withinBoundsDataSpace(
    const Vector<SpatialDims, double>& pos) const {}

// template <unsigned int SpatialDims, unsigned int DataDims, typename T>
// Vector<DataDims, double> CellTreeSampler<SpatialDims, DataDims, T>::getVoxel(
//     const size3_t& pos) const {}

}  // namespace discretedata
}  // namespace inviwo
