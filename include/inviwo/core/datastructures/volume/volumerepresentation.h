/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2018 Inviwo Foundation
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

#ifndef IVW_VOLUMEREPRESENTATION_H
#define IVW_VOLUMEREPRESENTATION_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/datastructures/datarepresentation.h>
#include <inviwo/core/datastructures/representationtraits.h>
#include <inviwo/core/common/inviwo.h>

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

protected:
    VolumeRepresentation() = default;
    VolumeRepresentation(const DataFormatBase* format);
    VolumeRepresentation(const VolumeRepresentation& rhs) = default;
    VolumeRepresentation& operator=(const VolumeRepresentation& that) = default;
};

template <>
struct representation_traits<Volume, kind::Base> {
    using type = VolumeRepresentation;
};

}  // namespace inviwo

#endif  // IVW_VOLUMEREPRESENTATION_H
