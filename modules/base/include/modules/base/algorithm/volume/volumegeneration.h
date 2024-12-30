/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2024 Inviwo Foundation
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

#include <inviwo/core/datastructures/volume/volume.h>  // IWYU pragma: keep
#include <inviwo/core/util/glmconvert.h>               // for glm_convert_normalized
#include <inviwo/core/util/glmmat.h>                   // for mat3
#include <inviwo/core/util/glmvec.h>                   // for size3_t, dvec3
#include <inviwo/core/util/indexmapper.h>              // for IndexMapper, IndexMapper3D
#include <inviwo/core/util/volumeramutils.h>           // for forEachVoxelParallel
#include <modules/base/algorithm/algorithmoptions.h>   // for IgnoreSpecialValues, IgnoreSpecialV...
#include <modules/base/algorithm/dataminmax.h>         // for dataMinMax

#include <array>          // for array
#include <bitset>         // for bitset, __bitset<>::reference, bits...
#include <cmath>          // for sin
#include <cstddef>        // for size_t
#include <memory>         // for unique_ptr, make_shared, make_unique
#include <unordered_map>  // for unordered_map

#include <glm/ext/scalar_constants.hpp>  // for pi
#include <glm/geometric.hpp>             // for length
#include <glm/gtx/component_wise.hpp>    // for compMul
#include <glm/gtx/norm.hpp>              // for length2
#include <glm/mat3x3.hpp>                // for mat<>::col_type
#include <glm/vec3.hpp>                  // for operator+, operator-, operator/, vec

namespace inviwo {
template <typename T>
class VolumeRAMPrecision;

namespace util {

/**
 * Convenience function for generating volumes
 * @param dimensions Volume grid dimensions
 * @param basis Volume basis, offset automatically set to center the volume around origo
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

    auto minmax = util::dataMinMax(data, glm::compMul(dimensions), IgnoreSpecialValues::Yes);

    auto volume = std::make_unique<Volume>(ram);
    volume->setBasis(basis);
    volume->setOffset(-0.5f * (basis[0] + basis[1] + basis[2]));
    volume->dataMap.dataRange.x = glm::compMin(minmax.first);
    volume->dataMap.dataRange.y = glm::compMax(minmax.second);
    volume->dataMap.valueRange = volume->dataMap.dataRange;
    return volume;
}

/**
 * Center voxel equal to value all other 0
 */
template <typename T = float>
std::unique_ptr<Volume> makeSingleVoxelVolume(const size3_t& size,
                                              const dvec4& value = dvec4{1.0}) {
    const size3_t mid{(size - size3_t{1u}) / size_t{2}};
    return generateVolume(size, mat3(1.0), [&](const size3_t& ind) {
        if (ind == mid) {
            return glm_convert_normalized<T>(value);
        } else {
            return glm_convert_normalized<T>(0.0);
        }
    });
}

template <typename T = float>
std::unique_ptr<Volume> makeUniformVolume(const size3_t& size, const dvec4& value = dvec4{1.0}) {
    return generateVolume(size, mat3(1.0),
                          [&](const size3_t&) { return glm_convert_normalized<T>(value); });
}

/**
 * Spherically symmetric density centered in the volume decaying radially with the distance from the
 * center
 */
template <typename T = float>
std::unique_ptr<Volume> makeSphericalVolume(const size3_t& size) {
    const dvec3 rsize{size};
    const dvec3 center = (rsize / 2.0);
    const auto r0 = glm::length(rsize);
    return generateVolume(size, mat3(1.0), [&](const size3_t& ind) {
        const auto pos = dvec3(ind) + dvec3{0.5};
        return glm_convert_normalized<T>(r0 / (r0 + glm::length2(center - pos)));
    });
}

/**
 * A quickly oscillating density between 0 and 1
 */
template <typename T = float>
std::unique_ptr<Volume> makeRippleVolume(const size3_t& size) {
    const dvec3 rsize{size};
    const dvec3 center = (rsize / 2.0);
    const double r0 = glm::length(rsize);
    return generateVolume(size, mat3(1.0), [&](const size3_t& ind) {
        const auto pos = dvec3(ind) + dvec3{0.5};
        const auto r = glm::length2(center - pos);
        return glm_convert_normalized<T>(
            0.5 + 0.5 * std::sin(rsize.x * 0.5 * glm::pi<double>() * r / r0));
    });
}

/**
 * A 2x2x2 volume corresponding to a marching cube case
 */
template <typename T = float>
std::unique_ptr<Volume> makeMarchingCubeVolume(const size_t& index) {
    std::bitset<8> corners(index);
    const std::array<size3_t, 8> vertices = {
        {{0, 0, 0}, {1, 0, 0}, {1, 1, 0}, {0, 1, 0}, {0, 0, 1}, {1, 0, 1}, {1, 1, 1}, {0, 1, 1}}};
    std::unordered_map<size3_t, T> map;
    for (int i = 0; i < 8; ++i) {
        map[vertices[i]] = glm_convert_normalized<T>(corners[i] ? 1.0 : 0.0);
    }
    return generateVolume({2, 2, 2}, mat3(1.0), [&](const size3_t& ind) { return map[ind]; });
}

}  // namespace util

}  // namespace inviwo
