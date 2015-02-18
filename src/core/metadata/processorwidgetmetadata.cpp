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

#include <inviwo/core/metadata/processorwidgetmetadata.h>

namespace inviwo {

const std::string ProcessorWidgetMetaData::CLASS_IDENTIFIER = "org.inviwo.ProcessorWidgetMetaData";

ProcessorWidgetMetaData::ProcessorWidgetMetaData()
    : MetaData()
    , position_(0,0)
    , dimensions_(256,256)
    , visibility_(true) {
}

ProcessorWidgetMetaData::ProcessorWidgetMetaData(const ProcessorWidgetMetaData& rhs)
    : MetaData()
    , position_(rhs.position_)
    , dimensions_(rhs.dimensions_)
    , visibility_(rhs.visibility_) {
}

ProcessorWidgetMetaData& ProcessorWidgetMetaData::operator=(const ProcessorWidgetMetaData& that) {
    if (this != &that) {
        position_ = that.position_;
        dimensions_ = that.dimensions_;
        visibility_ = that.visibility_;
    }

    return *this;
}

ProcessorWidgetMetaData::~ProcessorWidgetMetaData() {}

ProcessorWidgetMetaData* ProcessorWidgetMetaData::clone() const {
    return new ProcessorWidgetMetaData(*this);
}

void ProcessorWidgetMetaData::setPosition(const ivec2 &pos) {
    position_ = pos;
}

ivec2 ProcessorWidgetMetaData::getPosition() const {
    return position_;
}

void ProcessorWidgetMetaData::setDimensions(const ivec2 &dim) {
    dimensions_ = dim;
}

ivec2 ProcessorWidgetMetaData::getDimensions() const {
    return dimensions_;
}

void ProcessorWidgetMetaData::setVisibile(bool visibility) {
    visibility_  = visibility;
}

bool ProcessorWidgetMetaData::isVisible() const {
    return visibility_;
}

void ProcessorWidgetMetaData::serialize(IvwSerializer& s) const {
    s.serialize("type", getClassIdentifier(), true);
    s.serialize("position", position_);
    s.serialize("dimensions", dimensions_);
    s.serialize("visibility", visibility_);
}

void ProcessorWidgetMetaData::deserialize(IvwDeserializer& d) {
    std::string className;
    d.deserialize("type", className, true);
    d.deserialize("position", position_);
    d.deserialize("dimensions", dimensions_);
    d.deserialize("visibility", visibility_);
}

bool ProcessorWidgetMetaData::equal(const MetaData& rhs) const {
    const ProcessorWidgetMetaData* tmp = dynamic_cast<const ProcessorWidgetMetaData*>(&rhs);
    if (tmp) {
        return tmp->position_ == position_
            && tmp->visibility_ == visibility_
            && tmp->dimensions_ == dimensions_;
    } else {
        return false;
    }
}





} // namespace
