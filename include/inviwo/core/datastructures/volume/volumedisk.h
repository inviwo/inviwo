/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2015 Inviwo Foundation
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

#ifndef IVW_VOLUMEDISK_H
#define IVW_VOLUMEDISK_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/datastructures/diskrepresentation.h>
#include <inviwo/core/datastructures/volume/volumerepresentation.h>
#include <inviwo/core/datastructures/volume/volume.h>

namespace inviwo {

class IVW_CORE_API VolumeDisk : public VolumeRepresentation, public DiskRepresentation {
public:
    VolumeDisk(size3_t dimensions = size3_t(128, 128, 128),
               const DataFormatBase* format = DataUINT8::get());
    VolumeDisk(std::string url, size3_t dimensions = size3_t(128, 128, 128),
               const DataFormatBase* format = DataUINT8::get());
    VolumeDisk(const VolumeDisk& rhs);
    VolumeDisk& operator=(const VolumeDisk& that);
    virtual VolumeDisk* clone() const override;
    virtual ~VolumeDisk();

    virtual std::type_index getTypeIndex() const override final;

    virtual void setDimensions(size3_t dimensions) override;
    virtual const size3_t& getDimensions() const override;
private:
    size3_t dimensions_;
};

template <>
struct representation_traits<Volume, kind::Disk> {
    using type = VolumeDisk;
};

}  // namespace

#endif  // IVW_VOLUMEDISK_H
