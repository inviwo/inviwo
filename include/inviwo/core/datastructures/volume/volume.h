/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2024 Inviwo Foundation
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
#include <inviwo/core/datastructures/histogramtools.h>
#include <inviwo/core/datastructures/image/imagetypes.h>
#include <inviwo/core/datastructures/datamapper.h>
#include <inviwo/core/datastructures/representationtraits.h>
#include <inviwo/core/datastructures/volume/volumerepresentation.h>
#include <inviwo/core/datastructures/unitsystem.h>
#include <inviwo/core/metadata/metadataowner.h>
#include <inviwo/core/util/glmvec.h>
#include <inviwo/core/util/document.h>
#include <inviwo/core/io/datareader.h>
#include <inviwo/core/io/datawriter.h>
#include <inviwo/core/ports/datainport.h>
#include <inviwo/core/ports/dataoutport.h>

#include <optional>

namespace inviwo {

class Camera;

struct IVW_CORE_API VolumeConfig {
    std::optional<size3_t> dimensions = std::nullopt;
    const DataFormatBase* format = nullptr;
    std::optional<SwizzleMask> swizzleMask = std::nullopt;
    std::optional<InterpolationType> interpolation = std::nullopt;
    std::optional<Wrapping3D> wrapping = std::nullopt;
    std::optional<Axis> xAxis = std::nullopt;
    std::optional<Axis> yAxis = std::nullopt;
    std::optional<Axis> zAxis = std::nullopt;
    std::optional<Axis> valueAxis = std::nullopt;
    std::optional<dvec2> dataRange = std::nullopt;
    std::optional<dvec2> valueRange = std::nullopt;
    std::optional<mat4> model = std::nullopt;
    std::optional<mat4> world = std::nullopt;

    static constexpr auto defaultDimensions = size3_t(128, 128, 128);
    static inline const DataFormatBase* defaultFormat = DataUInt8::get();
    static constexpr auto defaultSwizzleMask = swizzlemasks::rgba;
    static constexpr auto defaultInterpolation = InterpolationType::Linear;
    static constexpr auto defaultWrapping = wrapping3d::clampAll;
    static inline const auto defaultXAxis = Axis{"x", Unit{}};
    static inline const auto defaultYAxis = Axis{"y", Unit{}};
    static inline const auto defaultZAxis = Axis{"z", Unit{}};
    static inline const auto defaultValueAxis = Axis{};
    static dvec2 defaultDataRange(const DataFormatBase* format = defaultFormat) {
        return DataMapper::defaultDataRangeFor(format);
    }
    static dvec2 defaultValueRange(const DataFormatBase* format = defaultFormat) {
        return defaultDataRange(format);
    }
    static constexpr auto defaultModel = mat4{1.0f};
    static constexpr auto defaultWorld = mat4{1.0f};

    DataMapper dataMap() {
        auto dataFormat = format ? format : VolumeConfig::defaultFormat;
        return DataMapper{dataRange.value_or(defaultDataRange(dataFormat)),
                          valueRange.value_or(defaultValueRange(dataFormat)),
                          valueAxis.value_or(defaultValueAxis)};
    }

    VolumeConfig& updateFrom(const VolumeConfig& config) {
        static constexpr auto update = [](auto& dest, const auto& src) {
            if (src) {
                dest = src.value();
            }
        };
        update(dimensions, config.dimensions);
        if (config.format) format = config.format;
        update(swizzleMask, config.swizzleMask);
        update(interpolation, config.interpolation);
        update(wrapping, config.wrapping);
        update(xAxis, config.xAxis);
        update(yAxis, config.yAxis);
        update(zAxis, config.zAxis);
        update(valueAxis, config.valueAxis);
        update(dataRange, config.dataRange);
        update(valueRange, config.valueRange);
        update(model, config.model);
        update(world, config.world);
    }
};

/**
 * \ingroup datastructures
 *
 * \class Volume
 * Data structure for volumetric data in form of a structured three-dimensional grid. Basis and
 * offset determine the position and extent of the volume in model space. Skewed volumes are
 * represented by providing a non-orthogonal basis.
 *
 * In case a volume was loaded via VolumeSource or Volume, the filename of the source data is
 * available via MetaData.
 */
class IVW_CORE_API Volume : public Data<Volume, VolumeRepresentation>,
                            public StructuredGridEntity<3>,
                            public MetaDataOwner,
                            public HistogramSupplier {
public:
    explicit Volume(size3_t defaultDimensions = VolumeConfig::defaultDimensions,
                    const DataFormatBase* defaultFormat = VolumeConfig::defaultFormat,
                    const SwizzleMask& defaultSwizzleMask = VolumeConfig::defaultSwizzleMask,
                    InterpolationType interpolation = VolumeConfig::defaultInterpolation,
                    const Wrapping3D& wrapping = VolumeConfig::defaultWrapping);
    explicit Volume(VolumeConfig config);
    explicit Volume(std::shared_ptr<VolumeRepresentation>);
    Volume(const Volume&) = default;
    /**
     * Create a volume based on \p rhs without copying any data. If \p defaultFormat is a nullptr,
     * the format of \p rhs is used. This applies to all std::optional arguments, too. If not
     * supplied, the corresponding value is taken from \p rhs.
     * @param rhs             source volume providing the necessary information like dimensions,
     *                        swizzle masks, interpolation, spatial transformations, etc.
     * @param dims            custom dimension of the new volume, otherwise rhs.getDimensions()
     * @param defaultFormat   data format of the new volume. If equal to nullptr, the format of \p
     *                        rhs is used instead.
     * @param defaultSwizzleMask   custom swizzle mask, otherwise rhs.getSwizzleMask()
     * @param interpolation   custom interpolation, otherwise rhs.getInterpolation()
     * @param wrapping        custom texture wrapping, otherwise rhs.getWrapping()
     */
    Volume(const Volume& rhs, NoData, VolumeConfig config = {});
    Volume& operator=(const Volume& that) = default;
    virtual Volume* clone() const override;
    virtual ~Volume();
    Document getInfo() const;

    /**
     * Resize to dimension. This is destructive, the data will not be
     * preserved.
     * @note Resizes the last valid representation and erases all representations.
     * Last valid representation will remain valid after changing the dimension.
     */
    virtual void setDimensions(const size3_t& dim);
    virtual size3_t getDimensions() const override;

    /**
     * Set the default data format. Existing representations will not be affected.
     * @note Only useful before any representations have been created.
     * @see DataFormatBase
     * @param format The format of the data.
     */
    void setDataFormat(const DataFormatBase* format);
    const DataFormatBase* getDataFormat() const;

    /**
     * \brief update the swizzle mask of the color channels when sampling the volume
     *
     * @param mask new swizzle mask
     */
    void setSwizzleMask(const SwizzleMask& mask);
    SwizzleMask getSwizzleMask() const;

    void setInterpolation(InterpolationType interpolation);
    InterpolationType getInterpolation() const;

    void setWrapping(const Wrapping3D& wrapping);
    Wrapping3D getWrapping() const;

    /**
     * \brief Computes the spacing to be used for gradient computation. Also works for volume with
     * non-orthogonal basis.
     *
     * For orthogonal lattices this will be equal to the world space voxel spacing.
     * For non-orthogonal lattices it will be the longest of the axes projected
     * onto the world space axes.
     *
     *        World space
     *
     *         b ^           ^
     *          /            |
     * y ^     /             dy
     *   |    /  Voxel       |
     *   |   /__________>a   \/
     *   |   <----dx--->
     *   |____________> x
     *
     *
     * The actual gradient spacing vectors are given by
     * mat3{ gradientSpacing.x,        0,                    0,
     *             0,            gradientSpacing.y,          0,
     *             0,                  0,              gradientSpacing.z }
     * However, we do not return the zeroes.
     *
     * To get the spacing in texture space use:
     * mat3(glm::scale(worldToTextureMatrix, getWorldSpaceGradientSpacing()));
     * @return Step size for gradient computation in world space.
     */
    vec3 getWorldSpaceGradientSpacing() const;

    /**
     * @copydoc SpatialEntity::getAxis
     */
    virtual const Axis* getAxis(size_t index) const override;

    DataMapper dataMap_;
    std::array<Axis, 3> axes;

    static uvec3 colorCode;
    static const std::string classIdentifier;
    static const std::string dataName;

    template <typename Kind>
    const typename representation_traits<Volume, Kind>::type* getRep() const;

    std::shared_ptr<HistogramCalculationState> calculateHistograms(size_t bins = 2048) const;

    VolumeConfig config() const;

protected:
    size3_t defaultDimensions_;
    const DataFormatBase* defaultDataFormat_;
    SwizzleMask defaultSwizzleMask_;
    InterpolationType defaultInterpolation_;
    Wrapping3D defaultWrapping_;
};

template <typename Kind>
const typename representation_traits<Volume, Kind>::type* Volume::getRep() const {
    static_assert(
        !std::is_same<typename representation_traits<Volume, Kind>::type, std::nullptr_t>::value,
        "No representation of specified kind found");
    return getRepresentation<typename representation_traits<Volume, Kind>::type>();
}

using VolumeSequence = std::vector<std::shared_ptr<Volume>>;

extern template class IVW_CORE_TMPL_EXP DataReaderType<Volume>;
extern template class IVW_CORE_TMPL_EXP DataWriterType<Volume>;
extern template class IVW_CORE_TMPL_EXP DataReaderType<VolumeSequence>;

extern template class IVW_CORE_TMPL_EXP DataInport<Volume>;
extern template class IVW_CORE_TMPL_EXP DataOutport<Volume>;
extern template class IVW_CORE_TMPL_EXP DataInport<VolumeSequence>;
extern template class IVW_CORE_TMPL_EXP DataOutport<VolumeSequence>;

}  // namespace inviwo
