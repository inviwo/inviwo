/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2024 Inviwo Foundation
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

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/datastructures/data.h>
#include <inviwo/core/datastructures/spatialdata.h>
#include <inviwo/core/datastructures/image/imagetypes.h>
#include <inviwo/core/datastructures/image/layerrepresentation.h>
#include <inviwo/core/datastructures/datamapper.h>
#include <inviwo/core/datastructures/unitsystem.h>

#include <inviwo/core/io/datareader.h>
#include <inviwo/core/io/datawriter.h>

#include <optional>

namespace inviwo {

class DataFormatBase;

/**
 * \ingroup datastructures
 */
class IVW_CORE_API Layer : public Data<Layer, LayerRepresentation>, public StructuredGridEntity<2> {
public:
    explicit Layer(size2_t defaultDimensions = size2_t(8, 8),
                   const DataFormatBase* defaultFormat = DataVec4UInt8::get(),
                   LayerType type = LayerType::Color,
                   const SwizzleMask& defaultSwizzleMask = swizzlemasks::rgba,
                   InterpolationType interpolation = InterpolationType::Linear,
                   const Wrapping2D& wrapping = wrapping2d::clampAll);
    explicit Layer(std::shared_ptr<LayerRepresentation>);
    Layer(const Layer&) = default;
    /**
     * Create a layer based on \p rhs without copying any data. If \p defaultFormat is a nullptr,
     * the format of \p rhs is used. This applies to all std::optional arguments, too. If not
     * supplied, the corresponding value is taken from \p rhs.
     * @param rhs             source layer providing the necessary information like dimensions,
     *                        swizzle masks, interpolation, spatial transformations, etc.
     * @param dims            custom dimension of the new layer, otherwise rhs.getDimensions()
     * @param defaultFormat   data format of the new layer. If equal to nullptr, the format of \p
     *                        rhs is used instead.
     * @param defaultSwizzleMask   custom swizzle mask, otherwise rhs.getSwizzleMask()
     * @param interpolation   custom interpolation, otherwise rhs.getInterpolation()
     * @param wrapping        custom texture wrapping, otherwise rhs.getWrapping()

     */
    Layer(const Layer& rhs, NoData, std::optional<size2_t> dims = {},
          const DataFormatBase* defaultFormat = nullptr, std::optional<LayerType> type = {},
          std::optional<SwizzleMask> defaultSwizzleMask = {},
          std::optional<InterpolationType> interpolation = {},
          std::optional<Wrapping2D> wrapping = {});
    Layer& operator=(const Layer& that) = default;
    virtual Layer* clone() const override;
    virtual ~Layer() = default;

    LayerType getLayerType() const;

    /**
     * Resize to dimension. This is destructive, the data will not be
     * preserved. Use copyRepresentationsTo to update the data.
     * @note Resizes the last valid representation and erases all other representations.
     * Last valid representation will remain valid after changing the dimension.
     */
    virtual void setDimensions(const size2_t& dim);
    virtual size2_t getDimensions() const override;

    /**
     * Set the format of the data.
     * @see DataFormatBase
     * @param format The format of the data.
     */
    // clang-format off
    [[ deprecated("use LayerRepresentation::setDataFormat() instead (deprecated since 2019-06-26)")]]
    void setDataFormat(const DataFormatBase* format);
    const DataFormatBase* getDataFormat() const;
    // clang-format on

    /**
     * \brief update the swizzle mask of the channels for sampling color layers
     * The swizzle mask is only affecting Color layers.
     *
     * @param mask new swizzle mask
     */
    void setSwizzleMask(const SwizzleMask& mask);
    SwizzleMask getSwizzleMask() const;

    void setInterpolation(InterpolationType interpolation);
    InterpolationType getInterpolation() const;

    void setWrapping(const Wrapping2D& wrapping);
    Wrapping2D getWrapping() const;

    /**
     * \brief encode the layer contents to a buffer considering the requested image format
     *
     * @param fileExtension   file extension of the requested image format
     * @return encoded layer contents as std::vector
     */
    std::unique_ptr<std::vector<unsigned char>> getAsCodedBuffer(
        const std::string& fileExtension) const;

    /**
     * returns the axis information corresponding to \p index
     * @throws RangeException if \p index is out of bounds, i.e. \p index >= 2
     */
    virtual const Axis* getAxis(size_t index) const override;

    DataMapper dataMap;
    std::array<Axis, 2> axes;

private:
    friend class LayerRepresentation;

    LayerType defaultLayerType_;
    size2_t defaultDimensions_;
    const DataFormatBase* defaultDataFormat_;
    SwizzleMask defaultSwizzleMask_;
    InterpolationType defaultInterpolation_;
    Wrapping2D defaultWrapping_;
};

// https://docs.microsoft.com/en-us/cpp/cpp/general-rules-and-limitations?view=vs-2017
extern template class IVW_CORE_TMPL_EXP DataReaderType<Layer>;
extern template class IVW_CORE_TMPL_EXP DataWriterType<Layer>;

}  // namespace inviwo
