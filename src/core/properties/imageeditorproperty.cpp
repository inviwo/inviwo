/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2015 Inviwo Foundation
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

#include <inviwo/core/properties/imageeditorproperty.h>

namespace inviwo {

ImageLabel::ImageLabel() : name_(""), startPoint_(0), rectSize_(0) {
}

ImageLabel::ImageLabel(vec2 startPoint, vec2 rectSize, std::string name)
    : name_(name)
    , startPoint_(startPoint)
    , rectSize_(rectSize) {
}

ImageEditorProperty::ImageEditorProperty(const ImageEditorProperty& rhs)
    : FileProperty(rhs)
    , labels_(rhs.labels_)
    , dimensions_(rhs.dimensions_) {
}

ImageEditorProperty& ImageEditorProperty::operator=(const ImageEditorProperty& that) {
    if (this != &that) {
        FileProperty::operator=(that);
        labels_ = that.labels_;
        dimensions_ = that.dimensions_;
    }
    return *this;
}

ImageEditorProperty* ImageEditorProperty::clone() const {
    return new ImageEditorProperty(*this);
}

ImageEditorProperty::~ImageEditorProperty() {}

void ImageLabel::serialize(Serializer& s) const {
    s.serialize("labelName", name_, SerializationTarget::Attribute);
    s.serialize("topLeft", startPoint_);
    s.serialize("size", rectSize_);
}

void ImageLabel::deserialize(Deserializer& d) {
    d.deserialize("labelName", name_, SerializationTarget::Attribute);
    d.deserialize("topLeft", startPoint_);
    d.deserialize("size", rectSize_);
}

PropertyClassIdentifier(ImageEditorProperty, "org.inviwo.ImageEditorProperty");

ImageEditorProperty::ImageEditorProperty(std::string identifier, std::string displayName,std::string value,
        InvalidationLevel invalidationLevel,
        PropertySemantics semantics)
    : FileProperty(identifier, displayName, value , "image" , invalidationLevel, semantics)
    , labels_()
{}

void ImageEditorProperty::setDimensions(ivec2 imgSize) {
    dimensions_ = imgSize;
}

void ImageEditorProperty::addLabel(vec2 startPoint, vec2 rectSize, std::string name) {
    labels_.push_back(ImageLabel(startPoint, rectSize, name));
}

const std::vector<ImageLabel>& ImageEditorProperty::getLabels() const {
    return labels_;
}

void ImageEditorProperty::clearLabels() {
    labels_.clear();
}

void ImageEditorProperty::serialize(Serializer& s) const {
    FileProperty::serialize(s);
    s.serialize("Dimension", dimensions_);
    s.serialize("Labels", labels_, "Label");
}

void ImageEditorProperty::deserialize(Deserializer& d) {
    FileProperty::deserialize(d);
    d.deserialize("Dimension", dimensions_);
    d.deserialize("Labels", labels_, "Label");
}


} // namespace
