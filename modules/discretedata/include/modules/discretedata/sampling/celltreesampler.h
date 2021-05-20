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
#include <inviwo/core/util/interpolation.h>
#include <inviwo/core/util/spatialsampler.h>

namespace inviwo {
namespace discretedata {

template <unsigned int SpatialDims, unsigned int DataDims, typename T>
class CellTreeSampler : public SpatialSampler<SpatialDims, DataDims, T> {
public:
    CellTreeSampler(std::shared_ptr<const Connectivity> grid,
                    std::shared_ptr<const DataChannel<double, SpatialDims>> coords,
                    std::shared_ptr<const DataChannel<T, DataDims>> data);
    // CellTreeSampler(const Volume& vol, CoordinateSpace space = CoordinateSpace::Data);
    virtual ~CellTreeSampler() = default;

    CellTreeSampler& operator=(const CellTreeSampler&) = delete;

    virtual Vector<DataDims, T> sampleDataSpace(
        const Vector<SpatialDims, double>& pos) const override;
    virtual bool withinBoundsDataSpace(const Vector<SpatialDims, double>& pos) const override;

protected:
    // Vector<DataDims, double> getVoxel(const size3_t& pos) const;

    std::shared_ptr<CellTree<SpatialDims>> tree_;
    std::shared_ptr<const DataChannel<double, DataDims>> coordinates_;
    std::shared_ptr<const DataChannel<T, DataDims>> data_;
};

using CellTreeSampler2D = CellTreeSampler<2, 2, double>;
using CellTreeSampler3D = CellTreeSampler<3, 3, double>;
using CellTreeSampler4D = CellTreeSampler<4, 4, double>;

}  // namespace discretedata
}  // namespace inviwo

#include "celltreesampler.inl"
