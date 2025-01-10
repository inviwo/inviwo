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
#include <inviwo/core/datastructures/datarepresentation.h>
#include <inviwo/core/datastructures/representationtraits.h>
#include <inviwo/core/datastructures/image/imagetypes.h>
#include <inviwo/core/datastructures/volume/volumeconfig.h>

namespace inviwo {

class Volume;

/**
 * \ingroup datastructures
 */
class IVW_CORE_API VolumeRepresentation : public DataRepresentation<Volume> {
public:
    virtual VolumeRepresentation* clone() const override = 0;
    virtual ~VolumeRepresentation() = default;

    // Removes old data and reallocate for new dimensions.
    // Needs to be overloaded by child classes.
    virtual void setDimensions(size3_t dimensions) = 0;
    virtual const size3_t& getDimensions() const = 0;

    virtual const DataFormatBase* getDataFormat() const = 0;
    std::string_view getDataFormatString() const { return getDataFormat()->getString(); }
    DataFormatId getDataFormatId() const { return getDataFormat()->getId(); }
    /**
     * \brief update the swizzle mask of the color channels when sampling the volume
     *
     * @param mask new swizzle mask
     */
    virtual void setSwizzleMask(const SwizzleMask& mask) = 0;
    virtual SwizzleMask getSwizzleMask() const = 0;

    virtual void setInterpolation(InterpolationType interpolation) = 0;
    virtual InterpolationType getInterpolation() const = 0;

    virtual void setWrapping(const Wrapping3D& wrapping) = 0;
    virtual Wrapping3D getWrapping() const = 0;

    VolumeReprConfig config() const;

protected:
    VolumeRepresentation() = default;
    VolumeRepresentation(const VolumeRepresentation& rhs) = default;
    VolumeRepresentation& operator=(const VolumeRepresentation& that) = default;
};

template <>
struct representation_traits<Volume, kind::Base> {
    using type = VolumeRepresentation;
};

}  // namespace inviwo
