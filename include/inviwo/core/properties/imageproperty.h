/*********************************************************************************
*
* Inviwo - Interactive Visualization Workshop
*
* Copyright (c) 2013-2018 Inviwo Foundation
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

#ifndef IVW_IMAGEPROPERTY_H
#define IVW_IMAGEPROPERTY_H

#include <inviwo/core/properties/property.h>

namespace inviwo {
    class IVW_CORE_API ImageProperty : public Property {

    private:
        int imageIdx_;
        const unsigned char* imgData_;
        size2_t imgSize_;

    public:
        InviwoPropertyInfo();
        ImageProperty(std::string identifier,
            std::string displayName,
            int imageIdx = -1,
            const unsigned char* imgData = nullptr,
            size2_t imgSize = size2_t{0,0},
            InvalidationLevel invalidationLevel = InvalidationLevel::InvalidOutput,
            PropertySemantics semantics = PropertySemantics::Default);

        ImageProperty(const ImageProperty& rhs);
        ImageProperty& operator=(const ImageProperty& that);

        virtual ImageProperty* clone() const override;
        virtual ~ImageProperty();

        void setImageIdx(int idx);
        int getImageIdx(void) const;

        const unsigned char* getImgData() const;
        size2_t getImgSize() const;

        // override for custom onChange behavior
        //virtual void propertyModified() override;

        // Override Property::resetToDefaultState, to avoid calling propertyModified on reset.
        //virtual void resetToDefaultState() override;
    };

} //namespace

#endif //IVW_IMAGEPROPERTY_H