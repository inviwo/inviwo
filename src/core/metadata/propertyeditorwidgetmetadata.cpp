/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2015 Inviwo Foundation
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

#include <inviwo/core/metadata/propertyeditorwidgetmetadata.h>

namespace inviwo {

PropertyEditorWidgetDockStatus::PropertyEditorWidgetDockStatus() : dockStatus_("Floating") {}
PropertyEditorWidgetDockStatus::PropertyEditorWidgetDockStatus(const std::string& dockStatus)
    : dockStatus_(dockStatus) {}
PropertyEditorWidgetDockStatus::PropertyEditorWidgetDockStatus(
    const PropertyEditorWidgetDockStatus& rhs)
    : dockStatus_(rhs.dockStatus_){};
PropertyEditorWidgetDockStatus& PropertyEditorWidgetDockStatus::operator=(
    const PropertyEditorWidgetDockStatus& that) {
    if (this != &that) dockStatus_ = that.dockStatus_;

    return *this;
}
bool PropertyEditorWidgetDockStatus::operator==(const PropertyEditorWidgetDockStatus& that) {
    if (this->getString() == that.getString()) return true;

    return false;
}
const std::string& PropertyEditorWidgetDockStatus::getString() const { return dockStatus_; }
const PropertyEditorWidgetDockStatus PropertyEditorWidgetDockStatus::Floating("Floating");
const PropertyEditorWidgetDockStatus PropertyEditorWidgetDockStatus::DockedLeft("DockedLeft");
const PropertyEditorWidgetDockStatus PropertyEditorWidgetDockStatus::DockedRight("DockedRight");

//////////////////////////////////////////////////////////////////////////

PropertyEditorWidgetMetaData::PropertyEditorWidgetMetaData()
    : position_(0,0)
    , dimensions_(256,256)
    , visibility_(false)
    , dockStatus_(PropertyEditorWidgetDockStatus::Floating.getString()) {}

PropertyEditorWidgetMetaData::PropertyEditorWidgetMetaData(const PropertyEditorWidgetMetaData& rhs)
    : position_(rhs.position_)
    , dimensions_(rhs.dimensions_)
    , visibility_(rhs.visibility_)
    , dockStatus_(PropertyEditorWidgetDockStatus::Floating.getString()) {}

PropertyEditorWidgetMetaData& PropertyEditorWidgetMetaData::operator=(
    const PropertyEditorWidgetMetaData& that) {
    if (this != &that) {
        position_ = that.position_;
        dimensions_ = that.dimensions_;
        visibility_ = that.visibility_;
        dockStatus_ = that.dockStatus_;
    }

    return *this;
}

PropertyEditorWidgetMetaData::~PropertyEditorWidgetMetaData() {}

PropertyEditorWidgetMetaData* PropertyEditorWidgetMetaData::clone() const {
    return new PropertyEditorWidgetMetaData(*this);
}

void PropertyEditorWidgetMetaData::setWidgetPosition(const ivec2 &pos) {
    position_ = pos;
}

ivec2 PropertyEditorWidgetMetaData::getWidgetPosition() const {
    return position_;
}

void PropertyEditorWidgetMetaData::setDimensions(const ivec2 &dim) {
    dimensions_ = dim;
}

ivec2 PropertyEditorWidgetMetaData::getDimensions() const {
    return dimensions_;
}

void PropertyEditorWidgetMetaData::setVisibile(bool visibility) {
    visibility_ = visibility;
}

bool PropertyEditorWidgetMetaData::isVisible() const {
    return visibility_;
}

void PropertyEditorWidgetMetaData::setDockStatus(PropertyEditorWidgetDockStatus& dockStatus) {
    dockStatus_ = dockStatus.getString();
}

const PropertyEditorWidgetDockStatus PropertyEditorWidgetMetaData::getDocStatus() const {
    return PropertyEditorWidgetDockStatus(dockStatus_);
}

void PropertyEditorWidgetMetaData::serialize(Serializer& s) const {
    s.serialize("type", getClassIdentifier(), SerializationTarget::Attribute);
    s.serialize("position", position_);
    s.serialize("dimensions", dimensions_);
    s.serialize("visibility", visibility_);
    s.serialize("dockstatus", dockStatus_);
}

void PropertyEditorWidgetMetaData::deserialize(Deserializer& d) {
    d.deserialize("position", position_);
    d.deserialize("dimensions", dimensions_);
    d.deserialize("visibility", visibility_);
    d.deserialize("dockstatus", dockStatus_);
    if (dockStatus_.empty()) dockStatus_="Floating";
}

bool PropertyEditorWidgetMetaData::equal(const MetaData& rhs) const {
    if (auto tmp = dynamic_cast<const PropertyEditorWidgetMetaData*>(&rhs)) {
        return tmp->position_ == position_ && tmp->visibility_ == visibility_ &&
               tmp->visibility_ == visibility_ && tmp->dockStatus_ == dockStatus_;
    } else {
        return false;
    }
}

const std::string PropertyEditorWidgetMetaData::CLASS_IDENTIFIER = "org.inviwo.PropertyEditorWidgetMetaData";

} // namespace
