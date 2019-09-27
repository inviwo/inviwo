/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2019 Inviwo Foundation
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

#ifndef IVW_VOLUMEDATAREADERDIALOG_H
#define IVW_VOLUMEDATAREADERDIALOG_H

#include <inviwo/core/util/formats.h>
#include <inviwo/core/util/dialog.h>
#include <inviwo/core/datastructures/datamapper.h>

namespace inviwo {

class IVW_CORE_API VolumeDataReaderDialog : public Dialog {
public:
    VolumeDataReaderDialog() = default;
    virtual ~VolumeDataReaderDialog() = default;

    virtual bool show() = 0;
    virtual void setFile(std::string fileName) = 0;

    virtual const DataFormatBase* getFormat() const = 0;
    virtual uvec3 getDimensions() const = 0;
    virtual dvec3 getSpacing() const = 0;
    virtual bool getEndianess() const = 0;
    virtual DataMapper getDataMapper() const = 0;
    virtual size_t getByteOffset() const = 0;

    virtual void setFormat(const DataFormatBase* format) = 0;
    virtual void setDimensions(uvec3 dim) = 0;
    virtual void setSpacing(dvec3 spacing) = 0;
    virtual void setEndianess(bool endian) = 0;
    virtual void setDataMapper(const DataMapper& datamapper) = 0;
    virtual void setByteOffset(size_t offset) = 0;
};

}  // namespace inviwo

#endif  // IVW_VOLUMEDATAREADERDIALOG_H
