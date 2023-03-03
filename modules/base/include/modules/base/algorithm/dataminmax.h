/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2023 Inviwo Foundation
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

#include <modules/base/basemoduledefine.h>  // for IVW_MODULE_BASE_API

#include <inviwo/core/util/formats.h>                 // for DataFormat
#include <inviwo/core/util/glm.h>                     // for isfinite
#include <inviwo/core/util/glmcomp.h>                 // for glmcomp
#include <inviwo/core/util/glmconvert.h>              // for glm_convert
#include <inviwo/core/util/glmutils.h>                // for is_floating_point, flat_extent
#include <inviwo/core/util/glmvec.h>                  // for dvec4
#include <modules/base/algorithm/algorithmoptions.h>  // for IgnoreSpecialValues, IgnoreSpecialV...

#include <algorithm>    // for max, min
#include <cstddef>      // for size_t
#include <numeric>      // for accumulate
#include <type_traits>  // for enable_if
#include <utility>      // for pair

#include <glm/common.hpp>  // for max, min

namespace inviwo {

class BufferBase;
class BufferRAM;
class Layer;
class LayerRAM;
class Volume;
class VolumeRAM;

namespace util {

IVW_MODULE_BASE_API std::pair<dvec4, dvec4> volumeMinMax(
    const VolumeRAM* volume, IgnoreSpecialValues ignore = IgnoreSpecialValues::No);

IVW_MODULE_BASE_API std::pair<dvec4, dvec4> layerMinMax(
    const LayerRAM* layer, IgnoreSpecialValues ignore = IgnoreSpecialValues::No);

IVW_MODULE_BASE_API std::pair<dvec4, dvec4> bufferMinMax(
    const BufferRAM* layer, IgnoreSpecialValues ignore = IgnoreSpecialValues::No);

IVW_MODULE_BASE_API std::pair<dvec4, dvec4> volumeMinMax(
    const Volume* volume, IgnoreSpecialValues ignore = IgnoreSpecialValues::No);

IVW_MODULE_BASE_API std::pair<dvec4, dvec4> layerMinMax(
    const Layer* layer, IgnoreSpecialValues ignore = IgnoreSpecialValues::No);

IVW_MODULE_BASE_API std::pair<dvec4, dvec4> bufferMinMax(
    const BufferBase* buffer, IgnoreSpecialValues ignore = IgnoreSpecialValues::No);

namespace detail {

// Specialization for double, float, half
template <typename ValueType,
          typename std::enable_if<util::is_floating_point<ValueType>::value, int>::type = 0>
std::pair<dvec4, dvec4> dataMinMax(const ValueType* data, size_t size,
                                   IgnoreSpecialValues ignore = IgnoreSpecialValues::No) {
    using Res = std::pair<ValueType, ValueType>;
    Res minmax{DataFormat<ValueType>::max(), DataFormat<ValueType>::lowest()};

    if (ignore == IgnoreSpecialValues::Yes) {
        minmax = std::accumulate(
            data, data + size, minmax, [](const Res& mm, const ValueType& v) -> Res {
                Res res(mm);
                for (size_t i = 0; i < util::flat_extent<ValueType>::value; ++i) {
                    if (util::isfinite(util::glmcomp(v, i))) {
                        util::glmcomp(res.first, i) =
                            std::min(util::glmcomp(mm.first, i), util::glmcomp(v, i));
                        util::glmcomp(res.second, i) =
                            std::max(util::glmcomp(mm.second, i), util::glmcomp(v, i));
                    }
                }
                return res;
            });
    } else {
        minmax = std::accumulate(data, data + size, minmax,
                                 [](const Res& mm, const ValueType& v) -> Res {
                                     return {glm::min(mm.first, v), glm::max(mm.second, v)};
                                 });
    }

    return {util::glm_convert<dvec4>(minmax.first), util::glm_convert<dvec4>(minmax.second)};
}
// Specialization for integer types. They do not have special values
template <typename ValueType,
          typename std::enable_if<!util::is_floating_point<ValueType>::value, int>::type = 0>
std::pair<dvec4, dvec4> dataMinMax(const ValueType* data, size_t size,
                                   IgnoreSpecialValues = IgnoreSpecialValues::No) {
    using Res = std::pair<ValueType, ValueType>;
    Res minmax{DataFormat<ValueType>::max(), DataFormat<ValueType>::lowest()};
    minmax =
        std::accumulate(data, data + size, minmax, [](const Res& mm, const ValueType& v) -> Res {
            return {glm::min(mm.first, v), glm::max(mm.second, v)};
        });

    return {util::glm_convert<dvec4>(minmax.first), util::glm_convert<dvec4>(minmax.second)};
}

}  // namespace detail

/**
 * Compute component-wise minimum and maximum values scalar and glm::vec types.
 *
 * @param data pointer to values
 * @param size of data
 * @param ignore infinite and NaN
 * @return minimum and maximum values of each component and zero for non-existing components
 */
template <typename ValueType>
std::pair<dvec4, dvec4> dataMinMax(const ValueType* data, size_t size,
                                   IgnoreSpecialValues ignore = IgnoreSpecialValues::No) {
    return detail::dataMinMax<ValueType>(data, size, ignore);
}

}  // namespace util

}  // namespace inviwo
