/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2019 Inviwo Foundation
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

#ifndef IVW_VOLUME_H
#define IVW_VOLUME_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/datastructures/data.h>
#include <inviwo/core/datastructures/spatialdata.h>
#include <inviwo/core/datastructures/datamapper.h>
#include <inviwo/core/datastructures/representationtraits.h>
#include <inviwo/core/datastructures/volume/volumerepresentation.h>
#include <inviwo/core/metadata/metadataowner.h>
#include <inviwo/core/util/document.h>
#include <inviwo/core/io/datareader.h>
#include <inviwo/core/io/datawriter.h>
#include <inviwo/core/ports/datainport.h>
#include <inviwo/core/ports/dataoutport.h>

namespace inviwo {

class CameraProperty;

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
                            public MetaDataOwner {
public:
    Volume(size3_t dimensions = size3_t(128, 128, 128),
           const DataFormatBase* format = DataUInt8::get());
    Volume(std::shared_ptr<VolumeRepresentation>);
    Volume(const Volume&) = default;
    Volume& operator=(const Volume& that) = default;
    virtual Volume* clone() const override;
    virtual ~Volume();
    Document getInfo() const;

    size3_t getDimensions() const override;

    /**
     * Resize to dimension. This is destructive, the data will not be
     * preserved.
     * @note Resizes the last valid representation and erases all representations.
     * Last valid representation will remain valid after changing the dimension.
     */
    void setDimensions(const size3_t& dim) override;

    void setOffset(const vec3& offset);
    vec3 getOffset() const;

    mat3 getBasis() const;
    void setBasis(const mat3& basis);

    mat4 getModelMatrix() const;
    void setModelMatrix(const mat4& mat);

    mat4 getWorldMatrix() const;
    void setWorldMatrix(const mat4& mat);

    virtual const StructuredCameraCoordinateTransformer<3>& getCoordinateTransformer(
        const Camera& camera) const override;
    using StructuredGridEntity<3>::getCoordinateTransformer;

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
    DataMapper dataMap_;

    static uvec3 colorCode;
    static const std::string classIdentifier;
    static const std::string dataName;

    template <typename Kind>
    const typename representation_traits<Volume, Kind>::type* getRep() const;

protected:
    virtual std::shared_ptr<VolumeRepresentation> createDefaultRepresentation() const override;
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

#endif  // IVW_VOLUME_H
