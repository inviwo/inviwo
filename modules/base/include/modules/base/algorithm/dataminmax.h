/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2026 Inviwo Foundation
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

#include <modules/base/basemoduledefine.h>

#include <inviwo/core/util/formats.h>
#include <inviwo/core/util/glm.h>
#include <inviwo/core/util/glmcomp.h>
#include <inviwo/core/util/glmconvert.h>
#include <inviwo/core/util/glmutils.h>
#include <inviwo/core/util/glmvec.h>
#include <modules/base/algorithm/algorithmoptions.h>

#include <algorithm>
#include <cstddef>
#include <numeric>
#include <type_traits>
#include <utility>

#include <glm/common.hpp>
#include <glm/vector_relational.hpp>

namespace inviwo {

class BufferBase;
class BufferRAM;
class Layer;
class LayerRAM;
class Volume;
class VolumeRAM;

namespace util {

IVW_MODULE_BASE_API std::pair<dvec4, dvec4> volumeMinMax(const VolumeRAM* volume,
                                                         IgnoreValues ignore = {});

IVW_MODULE_BASE_API std::pair<dvec4, dvec4> layerMinMax(const LayerRAM* layer,
                                                        IgnoreValues ignore = {});

IVW_MODULE_BASE_API std::pair<dvec4, dvec4> bufferMinMax(const BufferRAM* layer,
                                                         IgnoreValues ignore = {});

IVW_MODULE_BASE_API std::pair<dvec4, dvec4> volumeMinMax(const Volume* volume,
                                                         IgnoreValues ignore = {});

IVW_MODULE_BASE_API std::pair<dvec4, dvec4> layerMinMax(const Layer* layer,
                                                        IgnoreValues ignore = {});

IVW_MODULE_BASE_API std::pair<dvec4, dvec4> bufferMinMax(const BufferBase* buffer,
                                                         IgnoreValues ignore = {});

IVW_MODULE_BASE_API std::pair<dvec4, dvec4> volumeMinMax(const VolumeRAM* volume,
                                                         IgnoreSpecialValues ignore);

IVW_MODULE_BASE_API std::pair<dvec4, dvec4> layerMinMax(const LayerRAM* layer,
                                                        IgnoreSpecialValues ignore);

IVW_MODULE_BASE_API std::pair<dvec4, dvec4> bufferMinMax(const BufferRAM* layer,
                                                         IgnoreSpecialValues ignore);

IVW_MODULE_BASE_API std::pair<dvec4, dvec4> volumeMinMax(const Volume* volume,
                                                         IgnoreSpecialValues ignore);

IVW_MODULE_BASE_API std::pair<dvec4, dvec4> layerMinMax(const Layer* layer,
                                                        IgnoreSpecialValues ignore);

IVW_MODULE_BASE_API std::pair<dvec4, dvec4> bufferMinMax(const BufferBase* buffer,
                                                         IgnoreSpecialValues ignore);

namespace detail {

template <typename ValueType>
std::pair<dvec4, dvec4> dataMinMax(const ValueType* data, size_t size, IgnoreValues ignore = {}) {
    using Res = std::pair<ValueType, ValueType>;
    const auto init = Res{DataFormat<ValueType>::max(), DataFormat<ValueType>::lowest()};

    const auto calc = [&](auto maskFun) {
        return std::accumulate(
            data, data + size, init, [&](const Res& mm, const ValueType& v) -> Res {
                const auto mask = maskFun(v);
                const auto vMaskedMin = glm::mix(v, mm.first, mask);
                const auto vMaskedMax = glm::mix(v, mm.second, mask);
                return {glm::min(mm.first, vMaskedMin), glm::max(mm.second, vMaskedMax)};
            });
    };

    const auto minmax = [&]() {
        if constexpr (std::is_floating_point_v<util::value_type_t<ValueType>>) {
            if (ignore.special == IgnoreSpecialValues::Yes && ignore.floatingPoint) {
                const auto skip =
                    ValueType{static_cast<util::value_type_t<ValueType>>(*ignore.floatingPoint)};
                return calc([skip](const auto& v) {
                    return glm::not_(util::isfinite(v)) || glm::equal(v, skip);
                });

            } else if (ignore.special == IgnoreSpecialValues::Yes && !ignore.floatingPoint) {
                return calc([](const auto& v) { return glm::not_(util::isfinite(v)); });

            } else if (ignore.special == IgnoreSpecialValues::No && ignore.floatingPoint) {
                const auto skip =
                    ValueType{static_cast<util::value_type_t<ValueType>>(*ignore.floatingPoint)};
                return calc([skip](const auto& v) { return glm::equal(v, skip); });
            }

        } else if constexpr (std::is_signed_v<util::value_type_t<ValueType>>) {
            if (ignore.signedInteger) {
                const auto skip =
                    ValueType{static_cast<util::value_type_t<ValueType>>(*ignore.signedInteger)};
                return calc([skip](const auto& v) { return glm::equal(v, skip); });
            }

        } else {
            if (ignore.unsignedInteger) {
                const auto skip =
                    ValueType{static_cast<util::value_type_t<ValueType>>(*ignore.unsignedInteger)};
                return calc([skip](const auto& v) { return glm::equal(v, skip); });
            }
        }

        return std::accumulate(data, data + size, init,
                               [](const Res& mm, const ValueType& v) -> Res {
                                   return {glm::min(mm.first, v), glm::max(mm.second, v)};
                               });
    }();

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
std::pair<dvec4, dvec4> dataMinMax(const ValueType* data, size_t size, IgnoreValues ignore = {}) {
    return detail::dataMinMax<ValueType>(data, size, ignore);
}

template <typename ValueType>
std::pair<dvec4, dvec4> dataMinMax(const ValueType* data, size_t size, IgnoreSpecialValues ignore) {
    return detail::dataMinMax<ValueType>(data, size, {.special = ignore});
}

}  // namespace util

}  // namespace inviwo
