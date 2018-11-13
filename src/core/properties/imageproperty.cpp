/*********************************************************************************
*
* Inviwo - Interactive Visualization Workshop
*
* Copyright (c) 2014-2018 Inviwo Foundation
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

#include <inviwo/core/properties/imageproperty.h>

namespace inviwo {

    PropertyClassIdentifier(ImageProperty, "org.inviwo.ImageProperty");

    ImageProperty::ImageProperty(
        std::string identifier,
        std::string displayName,
        int imageIdx,
        const unsigned char* imgData,
        size2_t imgSize,
        InvalidationLevel invalidationLevel,
        PropertySemantics semantics) : Property( identifier, displayName, invalidationLevel, semantics)
        , imageIdx_(imageIdx)
        , imgData_(imgData)
        , imgSize_(imgSize)
    {}

    ImageProperty::ImageProperty(const ImageProperty& rhs) : Property(rhs) {}

    ImageProperty& ImageProperty::operator=(const ImageProperty& that) {
        if (this != &that) {
            Property::operator=(that);
        }
        return *this;
    }

    ImageProperty* ImageProperty::clone() const { return new ImageProperty(*this); }

    ImageProperty::~ImageProperty() {}

    void ImageProperty::setImageIdx(int idx) {
        imageIdx_ = idx;
    }

    int ImageProperty::getImageIdx(void) const {
        return imageIdx_;
    }

    const unsigned char* ImageProperty::getImgData() const {
        return imgData_;
    }

    size2_t ImageProperty::getImgSize() const {
        return imgSize_;
    }

    /*void ImageProperty::propertyModified() {
        Property::propertyModified();
    }

    void ImageProperty::resetToDefaultState() {
    }*/

}  // namespace
