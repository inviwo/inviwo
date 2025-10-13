/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2025 Inviwo Foundation
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

#include <inviwo/core/datastructures/image/layer.h>     // IWYU pragma: keep
#include <inviwo/core/datastructures/image/layerram.h>  // for LayerRAMPrecision
#include <inviwo/core/util/glmconvert.h>                // for glm_convert_normalized
#include <inviwo/core/util/glmmat.h>                    // for mat3
#include <inviwo/core/util/glmvec.h>                    // for size3_t, dvec3
#include <inviwo/core/util/indexmapper.h>               // for IndexMapper, IndexMapper3D
#include <modules/base/algorithm/algorithmoptions.h>  // for IgnoreSpecialValues, IgnoreSpecialV...
#include <modules/base/algorithm/dataminmax.h>        // for dataMinMax

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
 * Convenience function for generating layers
 * @param dimensions Layer dimensions
 * @param basis      Layer basis, offset automatically set to center the layer around origo
 * @param function   Functor called for each texel of the layer. T(const size2_t& ind).
 */
template <typename Functor>
std::unique_ptr<Layer> generateLayer(const size2_t& dimensions, const mat3& basis,
                                     Functor&& function) {
    using T = decltype(function(dimensions));

    auto ram = std::make_shared<LayerRAMPrecision<T>>(dimensions);
    auto data = ram->getView();
    IndexMapper2D im(dimensions);

    for (size_t y = 0; y < dimensions.y; ++y) {
        for (size_t x = 0; x < dimensions.x; ++x) {
            const size2_t index{x, y};
            data[im(index)] = function(index);
        }
    }

    auto minmax = util::dataMinMax(data.data(), glm::compMul(dimensions), IgnoreSpecialValues::Yes);

    auto layer = std::make_unique<Layer>(ram);
    layer->setBasis(basis);
    layer->setOffset(-0.5f * (basis[0] + basis[1]));
    layer->dataMap.dataRange.x = glm::compMin(minmax.first);
    layer->dataMap.dataRange.y = glm::compMax(minmax.second);
    layer->dataMap.valueRange = layer->dataMap.dataRange;
    layer->setSwizzleMask(swizzlemasks::defaultData(layer->getDataFormat()->getComponents()));
    return layer;
}

/**
 * Center texel equal to value, all others are set to zero
 */
template <typename T = float>
std::unique_ptr<Layer> makeSingleTexelLayer(const size2_t& size, const dvec4& value = dvec4{1.0}) {
    const size2_t mid{(size - size2_t{1u}) / size_t{2}};
    return generateLayer(size, mat3{1.0f}, [&mid, &value](const size2_t& ind) {
        if (ind == mid) {
            return glm_convert_normalized<T>(value);
        } else {
            return glm_convert_normalized<T>(0.0);
        }
    });
}

template <typename T = float>
std::unique_ptr<Layer> makeUniformLayer(const size2_t& size, const dvec4& value = dvec4{1.0}) {
    return generateLayer(size, mat3{1.0f},
                         [&value](const size2_t&) { return glm_convert_normalized<T>(value); });
}

/**
 * Radially symmetric density centered in the Layer decaying with the distance from the center
 */
template <typename T = float>
std::unique_ptr<Layer> makeRadialLayer(const size2_t& size) {
    const dvec2 rsize{size};
    const dvec2 center{rsize / 2.0};
    const auto r0 = glm::length(rsize);
    return generateLayer(size, mat3{1.0f}, [&center, &r0](const size2_t& ind) {
        const auto pos = dvec2(ind) + dvec2{0.5};
        return glm_convert_normalized<T>(r0 / (r0 + glm::length2(center - pos)));
    });
}

/**
 * A quickly oscillating density between 0 and 1
 */
template <typename T = float>
std::unique_ptr<Layer> makeRippleLayer(const size2_t& size) {
    const dvec2 rsize{size};
    const dvec2 center = (rsize / 2.0);
    const double r0 = glm::length(rsize);
    return generateLayer(size, mat3{1.0f}, [&center, &r0, &rsize](const size2_t& ind) {
        const auto pos = dvec2{ind} + dvec2{0.5};
        const auto r = glm::length2(center - pos);
        return glm_convert_normalized<T>(
            0.5 + 0.5 * std::sin(rsize.x * 0.5 * glm::pi<double>() * r / r0));
    });
}

}  // namespace util

}  // namespace inviwo
