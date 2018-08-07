/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018 Inviwo Foundation
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

#include <inviwo/core/util/formats.h>
#include <inviwo/core/datastructures/image/layerram.h>
#include <inviwo/core/datastructures/image/layerramprecision.h>

namespace inviwo {

/**
 * \class LayerRAMSubSet
 * \brief extracts a subregion from a layer and returns a new layer.
 */
class IVW_MODULE_BASE_API LayerRAMSubSet {
public:
    /**
     * \brief extracts the subregion of a layer
     *
     * This function extracts a subregion given by offset and extent from the input layer.
     * If border clamping is enabled, the output region will be clamped to lie completely within the
     * source layer. Otherwise (default), the areas outside the source layer will be filled with
     * zeros.
     *
     * @param in       input layer
     * @param offset   subregion offset in input layer
     * @param extent   extent (width and height) of subregion
     * @param clampBorderOutsideImage    if true, the output region will be clamped to the layer
     * boundaries
     * @param dstFormat    data format of the destination layer, if not set output will match input
     * format Currently, only conversion to DataVec4UInt8 is supported.
     * @return std::shared_ptr<LayerRAM>
     *
     * \see VolumeRAMSubSet
     */
    static std::shared_ptr<LayerRAM> apply(const LayerRepresentation* in, ivec2 offset,
                                           size2_t extent, bool clampBorderOutsideImage = false,
                                           const DataFormatBase* dstFormat = nullptr);
};

namespace detail {

struct IVW_MODULE_BASE_API LayerRAMSubSetDispatcher {
    using type = std::shared_ptr<LayerRAM>;
    template <typename Result, typename T>
    std::shared_ptr<LayerRAM> operator()(const LayerRepresentation* in, ivec2 offset,
                                         size2_t extent, bool clampBorderOutsideImage);
};

template <class Tdst>
struct IVW_MODULE_BASE_API LayerRAMSubSetConvertDispatcher {
    using type = std::shared_ptr<LayerRAM>;
    template <typename Result, typename T>
    std::shared_ptr<LayerRAM> operator()(const LayerRepresentation* in, ivec2 offset,
                                         size2_t extent, bool clampBorderOutsideImage);
};

template <typename Result, typename DataType>
std::shared_ptr<LayerRAM> LayerRAMSubSetDispatcher::operator()(const LayerRepresentation* in,
                                                               ivec2 offset, size2_t extent,
                                                               bool clampBorderOutsideImage) {
    using T = typename DataType::type;

    const LayerRAMPrecision<T>* layer = dynamic_cast<const LayerRAMPrecision<T>*>(in);
    if (!layer) return nullptr;

    // determine parameters
    const ivec2 srcDim(layer->getDimensions());

    ivec2 srcOffset(glm::max(ivec2(0), offset));
    ivec2 dstOffset(glm::max(ivec2(0), -offset));
    ivec2 dstDim(extent);
    ivec2 copyExtent(ivec2(extent) - dstOffset);

    // clamp to source layer
    copyExtent = glm::min(copyExtent, srcDim - srcOffset);

    if (clampBorderOutsideImage) {
        // adjust the output dimensions to match the intersection of output and input regions
        dstDim = copyExtent;
        dstOffset = ivec2(0);
    }

    // allocate space
    auto newLayer = std::make_shared<LayerRAMPrecision<T> >(dstDim);

    const T* src = static_cast<const T*>(layer->getData());
    T* dst = static_cast<T*>(newLayer->getData());
    if (!clampBorderOutsideImage) {
        // clear entire layer as only parts will be copied
        std::fill(dst, dst + dstDim.x * dstDim.y, T(0));
    }
    // memcpy each row to form sub layer
#pragma omp parallel for
    for (int j = 0; j < copyExtent.y; j++) {
        size_t srcPos = (j + srcOffset.y) * srcDim.x + srcOffset.x;
        size_t dstPos = (j + dstOffset.y) * dstDim.x + dstOffset.x;
        std::copy(src + srcPos, src + srcPos + copyExtent.x, dst + dstPos);
    }

    return newLayer;
}

template <class Tdst>
template <typename Result, typename DataType>
std::shared_ptr<LayerRAM> LayerRAMSubSetConvertDispatcher<Tdst>::operator()(
    const LayerRepresentation* in, ivec2 offset, size2_t extent, bool clampBorderOutsideImage) {
    using T = typename DataType::type;

    const LayerRAMPrecision<T>* layer = dynamic_cast<const LayerRAMPrecision<T>*>(in);
    if (!layer) return nullptr;

    // determine parameters
    const ivec2 srcDim(layer->getDimensions());

    ivec2 srcOffset(glm::max(ivec2(0), offset));
    ivec2 dstOffset(glm::max(ivec2(0), -offset));
    ivec2 dstDim(extent);
    ivec2 copyExtent(ivec2(extent) - dstOffset);

    // clamp to source layer
    copyExtent = glm::min(copyExtent, srcDim - srcOffset);

    if (clampBorderOutsideImage) {
        // adjust the output dimensions to match the intersection of output and input regions
        dstDim = copyExtent;
        dstOffset = ivec2(0);
    }

    // allocate space
    auto newLayer = std::make_shared<LayerRAMPrecision<Tdst> >(dstDim);

    const T* src = static_cast<const T*>(layer->getData());
    Tdst* dst = static_cast<Tdst*>(newLayer->getData());
    if (!clampBorderOutsideImage) {
        // clear entire layer as only parts will be copied
        std::fill(dst, dst + dstDim.x * dstDim.y, Tdst(0));
    }
    // memcpy each row to form sub layer
#pragma omp parallel for
    for (int j = 0; j < copyExtent.y; j++) {
        size_t srcPos = (j + srcOffset.y) * srcDim.x + srcOffset.x;
        size_t dstPos = (j + dstOffset.y) * dstDim.x + dstOffset.x;
        for (int i = 0; i < copyExtent.x; i++) {
            dst[dstPos + i] = util::glm_convert_normalized<Tdst, T>(src[srcPos + i]);
        }
    }

    return newLayer;
}

}  // namespace detail

}  // namespace inviwo

#endif  // IVW_LAYERRAMSUBSET_H
