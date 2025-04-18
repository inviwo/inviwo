/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2025 Inviwo Foundation
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
#include <inviwo/core/datastructures/diskrepresentation.h>
#include <inviwo/core/datastructures/volume/volumerepresentation.h>
#include <inviwo/core/datastructures/volume/volume.h>

#include <filesystem>

namespace inviwo {

/**
 * \ingroup datastructures
 */
class IVW_CORE_API VolumeDisk : public VolumeRepresentation,
                                public DiskRepresentation<VolumeRepresentation, VolumeDisk> {
public:
    VolumeDisk(size3_t dimensions = VolumeConfig::defaultDimensions,
               const DataFormatBase* format = VolumeConfig::defaultFormat,
               const SwizzleMask& swizzleMask = VolumeConfig::defaultSwizzleMask,
               InterpolationType interpolation = VolumeConfig::defaultInterpolation,
               const Wrapping3D& wrapping = VolumeConfig::defaultWrapping);
    VolumeDisk(const std::filesystem::path& path,
               size3_t dimensions = VolumeConfig::defaultDimensions,
               const DataFormatBase* format = VolumeConfig::defaultFormat,
               const SwizzleMask& swizzleMask = VolumeConfig::defaultSwizzleMask,
               InterpolationType interpolation = VolumeConfig::defaultInterpolation,
               const Wrapping3D& wrapping = VolumeConfig::defaultWrapping);
    explicit VolumeDisk(const VolumeReprConfig& config, const std::filesystem::path& path = {});

    VolumeDisk(const VolumeDisk& rhs) = default;
    VolumeDisk& operator=(const VolumeDisk& that) = default;
    virtual VolumeDisk* clone() const override;
    virtual ~VolumeDisk() = default;

    virtual std::type_index getTypeIndex() const override final;

    virtual const DataFormatBase* getDataFormat() const override;

    virtual void setDimensions(size3_t dimensions) override;
    virtual const size3_t& getDimensions() const override;

    /**
     * \brief update the swizzle mask of the color channels when sampling the volume
     *
     * @param mask new swizzle mask
     */
    virtual void setSwizzleMask(const SwizzleMask& mask) override;
    virtual SwizzleMask getSwizzleMask() const override;

    virtual void setInterpolation(InterpolationType interpolation) override;
    virtual InterpolationType getInterpolation() const override;

    virtual void setWrapping(const Wrapping3D& wrapping) override;
    virtual Wrapping3D getWrapping() const override;

private:
    const DataFormatBase* dataFormatBase_;
    size3_t dimensions_;
    SwizzleMask swizzleMask_;
    InterpolationType interpolation_;
    Wrapping3D wrapping_;
};

template <>
struct representation_traits<Volume, kind::Disk> {
    using type = VolumeDisk;
};

}  // namespace inviwo
