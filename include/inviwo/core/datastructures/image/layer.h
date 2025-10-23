/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2025 Inviwo Foundation
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
#include <inviwo/core/datastructures/datatraits.h>
#include <inviwo/core/datastructures/spatialdata.h>
#include <inviwo/core/datastructures/image/imagetypes.h>
#include <inviwo/core/datastructures/image/layerrepresentation.h>
#include <inviwo/core/datastructures/image/layerconfig.h>
#include <inviwo/core/datastructures/datamapper.h>
#include <inviwo/core/datastructures/unitsystem.h>
#include <inviwo/core/datastructures/histogramtools.h>
#include <inviwo/core/datastructures/datasequence.h>

#include <inviwo/core/io/datareader.h>
#include <inviwo/core/io/datawriter.h>
#include <inviwo/core/ports/datainport.h>
#include <inviwo/core/ports/dataoutport.h>

namespace inviwo {

class DataFormatBase;

/**
 * \ingroup datastructures
 */
class IVW_CORE_API Layer : public Data<Layer, LayerRepresentation>, public StructuredGridEntity<2> {
public:
    using Config = LayerConfig;

    explicit Layer(size2_t defaultDimensions = LayerConfig::defaultDimensions,
                   const DataFormatBase* defaultFormat = LayerConfig::defaultFormat,
                   LayerType type = LayerConfig::defaultType,
                   const SwizzleMask& defaultSwizzleMask = LayerConfig::defaultSwizzleMask,
                   InterpolationType interpolation = LayerConfig::defaultInterpolation,
                   const Wrapping2D& wrapping = LayerConfig::defaultWrapping);
    explicit Layer(const LayerConfig& config);
    explicit Layer(std::shared_ptr<LayerRepresentation>);
    Layer(const Layer&) = default;
    Layer(Layer&&) = default;
    Layer& operator=(const Layer& that) = default;
    Layer& operator=(Layer&& that) = default;
    virtual Layer* clone() const override;
    virtual ~Layer() override = default;

    /**
     * Create a layer based on \p rhs without copying any representations. State from @p rhs can be
     * overridden by the @p config
     * @param rhs             source layer providing the necessary information like dimensions,
     *                        swizzle masks, interpolation, spatial transformations, etc.
     * @param noData          Tag type to indicate that representations should not be copied from
     *                        rhs
     * @param config          custom parameters overriding values from @p rhs
     */
    Layer(const Layer& rhs, NoData noData, const LayerConfig& config = {});

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
     * \brief Computes the spacing to be used for gradient computation. Also works for layers with
     * non-orthogonal basis.
     *
     * For orthogonal lattices this will be equal to the world space texel spacing.
     * For non-orthogonal lattices it will be the longer of the axes projected
     * onto the world space axes.
     *
     *        World space
     *
     *         b ^           ^
     *          /            |
     * y ^     /             dy
     *   |    /  texel       |
     *   |   /__________>a   \/
     *   |   <----dx--->
     *   |____________> x
     *
     *
     * The actual gradient spacing vectors are given by
     * mat3{ gradientSpacing.x,        0,                0,
     *             0,            gradientSpacing.y,      0,
     *             0,                  0,           gradientSpacing.y
     * }
     * However, we do not return the zeroes.
     *
     * To get the spacing in texture space use:
     * mat3{glm::scale(worldToTextureMatrix, getWorldSpaceGradientSpacing())};
     * @return Step size for gradient computation in world space.
     */
    vec3 getWorldSpaceGradientSpacing() const;

    /**
     * returns the axis information corresponding to \p index
     * @throws RangeException if \p index is out of bounds, i.e. \p index >= 2
     */
    virtual const Axis* getAxis(size_t index) const override;

    LayerConfig config() const;

    DataMapper dataMap;
    std::array<Axis, 2> axes;

    [[nodiscard]] HistogramCache::Result calculateHistograms(
        const std::function<void(const std::vector<Histogram1D>&)>& whenDone) const;
    void discardHistograms();

private:
    friend class LayerRepresentation;

    LayerType defaultLayerType_;
    size2_t defaultDimensions_;
    const DataFormatBase* defaultDataFormat_;
    SwizzleMask defaultSwizzleMask_;
    InterpolationType defaultInterpolation_;
    Wrapping2D defaultWrapping_;
    HistogramCache histograms_;
};

namespace util {
IVW_CORE_API Document layerInfo(const Layer& layer);
}  // namespace util

template <>
struct DataTraits<Layer> {
    static constexpr std::string_view classIdentifier() { return "org.inviwo.Layer"; }
    static constexpr std::string_view dataName() { return "Layer"; }
    static constexpr uvec3 colorCode() { return {95, 204, 114}; }
    static Document info(const Layer& layer) { return util::layerInfo(layer); }
};

using LayerSequence = DataSequence<Layer>;

// https://docs.microsoft.com/en-us/cpp/cpp/general-rules-and-limitations?view=vs-2017
extern template class IVW_CORE_TMPL_EXP DataReaderType<Layer>;
extern template class IVW_CORE_TMPL_EXP DataWriterType<Layer>;

extern template class IVW_CORE_TMPL_EXP DataInport<Layer>;
extern template class IVW_CORE_TMPL_EXP DataInport<Layer, 0, false>;
extern template class IVW_CORE_TMPL_EXP DataInport<Layer, 0, true>;
extern template class IVW_CORE_TMPL_EXP DataInport<DataSequence<Layer>>;
extern template class IVW_CORE_TMPL_EXP DataInport<DataSequence<Layer>, 0, false>;
extern template class IVW_CORE_TMPL_EXP DataInport<DataSequence<Layer>, 0, true>;
extern template class IVW_CORE_TMPL_EXP DataOutport<Layer>;
extern template class IVW_CORE_TMPL_EXP DataOutport<DataSequence<Layer>>;

}  // namespace inviwo
