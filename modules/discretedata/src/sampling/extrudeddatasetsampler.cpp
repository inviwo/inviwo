/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2020 Inviwo Foundation
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

#include <modules/discretedata/sampling/extrudeddatasetsampler.h>

namespace inviwo {
namespace discretedata {

std::shared_ptr<DataSetSamplerBase> createExtrudedDataSetSampler(
    std::shared_ptr<const DataSetSamplerBase> baseSampler,
    const std::shared_ptr<const DataChannel<double, 1>>& extraDimension,
    const std::shared_ptr<const ExtrudedGrid>& grid) {
    detail::CreateExtrudedDataSetSamplerHelper dispatcher;
    return channeldispatching::dispatchNumber<std::shared_ptr<DataSetSamplerBase>, 1, 2>(
        baseSampler->getDimension(), dispatcher, baseSampler, extraDimension, grid);
}

using ExtrudedDataSetSampler2D = ExtrudedDataSetSampler<2>;
using ExtrudedDataSetSampler3D = ExtrudedDataSetSampler<3>;
// using ExtrudedDataSetSampler4D = ExtrudedDataSetSampler<4>;

}  // namespace discretedata
}  // namespace inviwo
