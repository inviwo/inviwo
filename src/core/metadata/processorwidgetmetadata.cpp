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

#include <inviwo/core/metadata/processorwidgetmetadata.h>

namespace inviwo {

const std::string ProcessorWidgetMetaData::CLASS_IDENTIFIER = "org.inviwo.ProcessorWidgetMetaData";

ProcessorWidgetMetaData::ProcessorWidgetMetaData()
    : MetaData()
    , Observable<ProcessorWidgetMetaDataObserver>()
    , position_(0, 0)
    , dimensions_(256, 256)
    , visibility_(true) {}

ProcessorWidgetMetaData* ProcessorWidgetMetaData::clone() const {
    return new ProcessorWidgetMetaData(*this);
}

void ProcessorWidgetMetaData::setPosition(const ivec2& pos) {
    if (pos != position_) {
        position_ = pos;
        forEachObserver(
            [&](ProcessorWidgetMetaDataObserver* o) { o->onProcessorWidgetPositionChange(this); });
    }
}

ivec2 ProcessorWidgetMetaData::getPosition() const { return position_; }

void ProcessorWidgetMetaData::setDimensions(const ivec2& dim) {
    if (dim != dimensions_) {
        dimensions_ = dim;
        forEachObserver(
            [&](ProcessorWidgetMetaDataObserver* o) { o->onProcessorWidgetDimensionChange(this); });
    }
}

ivec2 ProcessorWidgetMetaData::getDimensions() const { return dimensions_; }

void ProcessorWidgetMetaData::setVisibile(bool visibility) {
    if (visibility != visibility_) {
        visibility_ = visibility;
        forEachObserver([&](ProcessorWidgetMetaDataObserver* o) {
            o->onProcessorWidgetVisibilityChange(this);
        });
    }
}

bool ProcessorWidgetMetaData::isVisible() const { return visibility_; }

void ProcessorWidgetMetaData::serialize(Serializer& s) const {
    s.serialize("type", getClassIdentifier(), SerializationTarget::Attribute);
    s.serialize("position", position_);
    s.serialize("dimensions", dimensions_);
    s.serialize("visibility", visibility_);
}

void ProcessorWidgetMetaData::deserialize(Deserializer& d) {
    ivec2 position{0, 0};
    d.deserialize("position", position);
    setPosition(position);

    ivec2 dimensions{0, 0};
    d.deserialize("dimensions", dimensions);
    setDimensions(dimensions);

    bool visibility{true};
    d.deserialize("visibility", visibility);
    setVisibile(visibility);
}

bool ProcessorWidgetMetaData::equal(const MetaData& rhs) const {
    if (auto tmp = dynamic_cast<const ProcessorWidgetMetaData*>(&rhs)) {
        return tmp->position_ == position_ && tmp->visibility_ == visibility_ &&
               tmp->dimensions_ == dimensions_;
    } else {
        return false;
    }
}

}  // namespace inviwo
