/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2019 Inviwo Foundation
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

#ifndef IVW_ISOVALUECOLLECTION_H
#define IVW_ISOVALUECOLLECTION_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/inviwo.h>

#include <inviwo/core/datastructures/tfprimitiveset.h>
#include <inviwo/core/util/fileextension.h>

namespace inviwo {

/**
 * \ingroup datastructures
 * \class IsoValueCollection
 * \brief data structure representing isovalues
 *
 * \see IsoValue
 */
class IVW_CORE_API IsoValueCollection : public TFPrimitiveSet {
public:
    IsoValueCollection(const std::vector<TFPrimitiveData>& values = {},
                       TFPrimitiveSetType type = TFPrimitiveSetType::Relative);
    IsoValueCollection(const IsoValueCollection& rhs) = default;
    IsoValueCollection(IsoValueCollection&& rhs) = default;
    IsoValueCollection& operator=(const IsoValueCollection& rhs) = default;
    virtual ~IsoValueCollection() = default;

    virtual std::vector<FileExtension> getSupportedExtensions() const override;

    virtual void save(const std::string& filename,
                      const FileExtension& ext = FileExtension()) const override;
    virtual void load(const std::string& filename,
                      const FileExtension& ext = FileExtension()) override;

    virtual std::string getTitle() const override;

protected:
    virtual std::string serializationKey() const override;
    virtual std::string serializationItemKey() const override;
};

}  // namespace inviwo

#endif  // IVW_ISOVALUECOLLECTION_H
