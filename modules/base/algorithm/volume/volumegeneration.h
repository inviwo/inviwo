/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017 Inviwo Foundation
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

#ifndef IVW_VOLUMEGENERATION_H
#define IVW_VOLUMEGENERATION_H

#include <modules/base/basemoduledefine.h>
#include <inviwo/core/common/inviwo.h>

#include <inviwo/core/datastructures/volume/volume.h>
#include <inviwo/core/datastructures/volume/volumeramprecision.h>
#include <inviwo/core/util/volumeramutils.h>
#include <inviwo/core/util/indexmapper.h>

namespace inviwo {

namespace util {

/**
 * Convenience function for generating volumes
 * @param dimensions Volume grid dimensions
 * @param basisAndOffset Volume basis and offset matrix
 * @param function Functor called for each volume voxel. T(const size3_t& ind).
 */
template <typename Functor>
std::unique_ptr<Volume> generateVolume(const size3_t& dimensions, const mat3& basis,
                                       Functor&& function) {
    using T = decltype(function(dimensions));

    auto ram = std::make_shared<VolumeRAMPrecision<T>>(dimensions);
    auto data = ram->getDataTyped();
    IndexMapper3D im(dimensions);

    forEachVoxelParallel(*ram, [&](const size3_t& ind) { data[im(ind)] = function(ind); });

    auto minmax = std::minmax_element(data, data + glm::compMul(dimensions));

    auto volume = std::make_unique<Volume>(ram);
    volume->setBasis(basis);
    volume->setOffset(-0.5f * (basis[0] + basis[1] + basis[2]));
    volume->dataMap_.dataRange.x = util::glm_convert<double>(*minmax.first);
    volume->dataMap_.dataRange.y = util::glm_convert<double>(*minmax.second);
    volume->dataMap_.valueRange = volume->dataMap_.dataRange;
    return volume;
}

IVW_MODULE_BASE_API std::unique_ptr<Volume> makeSingleVoxelVolume(const size3_t& size);

IVW_MODULE_BASE_API std::unique_ptr<Volume> makeSphericalVolume(const size3_t& size);

IVW_MODULE_BASE_API std::unique_ptr<Volume> makeRippleVolume(const size3_t& size);

IVW_MODULE_BASE_API std::unique_ptr<Volume> makeMarchingCubeVolume(const size_t& index);

}  // namespace util

}  // namespace inviwo

#endif  // IVW_VOLUMEGENERATION_H
