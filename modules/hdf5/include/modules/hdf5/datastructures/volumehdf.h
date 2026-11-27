/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2026 Inviwo Foundation
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

#include <modules/hdf5/hdf5moduledefine.h>
#include <modules/hdf5/datastructures/hdf5handle.h>
#include <modules/hdf5/datastructures/hdf5selection.h>

#include <inviwo/core/datastructures/volume/volumerepresentation.h>
#include <inviwo/core/datastructures/volume/volumeconfig.h>
#include <inviwo/core/util/glmvec.h>

#include <cstddef>
#include <span>
#include <typeindex>
#include <vector>

namespace inviwo {

class DataFormatBase;

namespace hdf5 {

/**
 * @ingroup datastructures
 * @brief A VolumeRepresentation backed by a hyperslab of an HDF5 dataset.
 *
 * The representation holds no voxel data itself, only a Handle to the dataset together with a
 * Selection per dataset dimension. The data is read from the file when the representation is
 * converted into another representation, e.g. VolumeRAM or VolumeGL. The data is converted to the
 * requested data format on the fly by HDF5, which allows uploading data directly to the GPU
 * without an intermediate RAM copy of the full volume.
 *
 * The representation is read-only, the dimensions are given by the selection and can not be
 * changed. All HDF5 access is serialized using Handle::globalMutex().
 *
 * @see hdf5::getVolumeAtPathAsType, VolumeHDF2RAMConverter
 */
class IVW_MODULE_HDF5_API VolumeHDF : public VolumeRepresentation {
public:
    /**
     * @param handle    handle pointing at the dataset to read
     * @param selection one Selection per dataset dimension in column major (Inviwo) order, i.e.
     *                  `[X, Y, Z, extraDims...]`. At most three dimensions may select more than
     *                  one element.
     * @param config    the data format to convert the data to (defaults to the format of the
     *                  dataset) and the sampling settings. Dimensions are derived from the
     *                  selection and ignored.
     * @throws Exception if the selection does not match the rank of the dataset, if it selects
     *                   more than three dimensions, or if the data format can not be determined.
     */
    VolumeHDF(Handle handle, std::vector<Selection> selection, const VolumeReprConfig& config = {});

    VolumeHDF(const VolumeHDF&) = default;
    VolumeHDF& operator=(const VolumeHDF&) = default;
    VolumeHDF(VolumeHDF&&) = default;
    VolumeHDF& operator=(VolumeHDF&&) = default;
    virtual ~VolumeHDF() = default;

    virtual VolumeHDF* clone() const override;
    virtual std::type_index getTypeIndex() const override final;

    virtual const DataFormatBase* getDataFormat() const override;

    /// @throws Exception since the dimensions are given by the selection
    virtual void setDimensions(size3_t dimensions) override;
    virtual const size3_t& getDimensions() const override;

    virtual void setSwizzleMask(const SwizzleMask& mask) override;
    virtual SwizzleMask getSwizzleMask() const override;

    virtual void setInterpolation(InterpolationType interpolation) override;
    virtual InterpolationType getInterpolation() const override;

    virtual void setWrapping(const Wrapping3D& wrapping) override;
    virtual Wrapping3D getWrapping() const override;

    const Handle& getHandle() const;
    const std::vector<Selection>& getSelection() const;

    /**
     * Read the selected data into @p dst, converting it to @p format. @p dst must hold exactly
     * `glm::compMul(getDimensions())` elements of @p format, in column major order.
     * @throws Exception if the sizes do not match or if the data could not be read.
     */
    void readInto(std::span<std::byte> dst, const DataFormatBase* format) const;

    /**
     * Read the selected data into @p dst, converting it to getDataFormat().
     * @see readInto(std::span<std::byte>, const DataFormatBase*)
     */
    void readInto(std::span<std::byte> dst) const;

    /// The size in bytes needed to hold the selected data in @p format
    size_t getSizeInBytes(const DataFormatBase* format) const;

private:
    Handle handle_;
    std::vector<Selection> selection_;
    size3_t dimensions_;
    const DataFormatBase* format_;
    SwizzleMask swizzleMask_;
    InterpolationType interpolation_;
    Wrapping3D wrapping_;
};

}  // namespace hdf5

}  // namespace inviwo
