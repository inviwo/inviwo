/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2019 Inviwo Foundation
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

#ifndef IVW_LAYERRAMSUBSET_H
#define IVW_LAYERRAMSUBSET_H

#include <modules/base/basemoduledefine.h>
#include <inviwo/core/common/inviwo.h>

#include <inviwo/core/datastructures/image/layer.h>
#include <inviwo/core/datastructures/image/layerram.h>
#include <inviwo/core/datastructures/image/layerramprecision.h>
#include <inviwo/core/util/glm.h>

#include <algorithm>

namespace inviwo {

namespace util {

/**
 * \brief extracts a subregion from a layer and returns it as a new layer
 *
 * This function extracts a subregion given by offset and extent from the input layer.
 * If border clamping is enabled, the output region will be clamped to lie completely within the
 * source layer. Otherwise (default), the areas outside the source layer will be filled with
 * zeros.
 *
 * @param in       input layer
 * @param offset   subregion offset in input layer
 * @param extent   extent (width and height) of subregion
 * @param clampBorderOutsideImage    if true, the output region is clamped to the layer boundaries
 * @return std::shared_ptr<LayerRAM>
 */
IVW_MODULE_BASE_API std::shared_ptr<LayerRAM> layerSubSet(const Layer* in, ivec2 offset,
                                                          size2_t extent,
                                                          bool clampBorderOutsideImage = false);

/**
 * \brief extracts a subregion from a layer and converts it into a new layer
 *
 * This function extracts a subregion given by offset and extent from the input layer. The values
 * will be converted to type T using util::glm_convert_normalized.
 * If border clamping is enabled, the output region will be clamped to lie completely within the
 * source layer. Otherwise (default), the areas outside the source layer will be filled with
 * zeros.
 *
 * @param in       input layer
 * @param offset   subregion offset in input layer
 * @param extent   extent (width and height) of subregion
 * @param clampBorderOutsideImage    if true, the output region is clamped to the layer boundaries
 * @return std::shared_ptr<LayerRAMPrecision<T>>
 */
template <typename T>
std::shared_ptr<LayerRAMPrecision<T>> layerSubSet(const Layer* in, ivec2 offset, size2_t extent,
                                                  bool clampBorderOutsideImage = false);

namespace detail {

template <typename T>
void conversionCopy(const T* src, T* dst, size_t len) {
    std::copy(src, src + len, dst);
}

template <typename To, typename From>
void conversionCopy(const From* src, To* dst, size_t len) {
    for (size_t i = 0; i < len; i++) {
        dst[i] = util::glm_convert_normalized<To, From>(src[i]);
    }
}

template <typename T, typename U = T>
std::shared_ptr<LayerRAMPrecision<U>> extractLayerSubSet(const LayerRAMPrecision<T>* inLayer,
                                                         ivec2 offset, size2_t extent,
                                                         bool clampBorderOutsideImage) {

    // determine parameters
    const ivec2 srcDim(inLayer->getDimensions());

    // adjust the output dimensions to match the intersection of output and input regions
    const ivec2 srcOffset(glm::max(ivec2(0), offset));
    const ivec2 dstOffset = clampBorderOutsideImage ? ivec2(0) : (glm::max(ivec2(0), -offset));
    // clamp copy extent to source layer
    const ivec2 copyExtent = glm::min(ivec2(extent) - dstOffset, srcDim - srcOffset);
    const ivec2 dstDim = clampBorderOutsideImage ? copyExtent : ivec2(extent);

    // allocate space
    auto newLayer = std::make_shared<LayerRAMPrecision<U>>(dstDim);

    const auto src = inLayer->getDataTyped();
    auto dst = newLayer->getDataTyped();

    if (!clampBorderOutsideImage) {
        // clear entire layer as only parts will be copied
        std::fill(dst, dst + dstDim.x * dstDim.y, U(0));
    }
    // memcpy each row to form sub layer
#pragma omp parallel for
    for (int j = 0; j < copyExtent.y; j++) {
        size_t srcPos = (j + srcOffset.y) * srcDim.x + srcOffset.x;
        size_t dstPos = (j + dstOffset.y) * dstDim.x + dstOffset.x;
        conversionCopy(src + srcPos, dst + dstPos, static_cast<size_t>(copyExtent.x));
    }

    return newLayer;
}

}  // namespace detail

}  // namespace util

template <typename T>
std::shared_ptr<LayerRAMPrecision<T>> util::layerSubSet(const Layer* in, ivec2 offset,
                                                        size2_t extent,
                                                        bool clampBorderOutsideImage) {

    return in->getRepresentation<LayerRAM>()->dispatch<std::shared_ptr<LayerRAMPrecision<T>>>(
        [offset, extent, clampBorderOutsideImage](auto layerpr) {
            using ValueType = util::PrecisionValueType<decltype(layerpr)>;

            return util::detail::extractLayerSubSet<ValueType, T>(layerpr, offset, extent,
                                                                  clampBorderOutsideImage);
        });
}

}  // namespace inviwo

#endif  // IVW_LAYERRAMSUBSET_H
