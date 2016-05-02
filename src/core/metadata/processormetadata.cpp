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

#include <inviwo/core/metadata/processormetadata.h>

namespace inviwo {

ProcessorMetaData::ProcessorMetaData() : position_(0, 0), visibility_(true), selection_(false) {}

ProcessorMetaData* ProcessorMetaData::clone() const { return new ProcessorMetaData(*this); }

void ProcessorMetaData::setPosition(const ivec2& pos) {
    if (pos != position_) {
        position_ = pos;
        forEachObserver(
            [&](ProcessorMetaDataObserver* o) { o->onProcessorMetaDataPositionChange(); });
    }
}

ivec2 ProcessorMetaData::getPosition() const { return position_; }

void ProcessorMetaData::setVisibile(bool visibility) {
    if (visibility != visibility_) {
        visibility_ = visibility;
        forEachObserver(
            [&](ProcessorMetaDataObserver* o) { o->onProcessorMetaDataVisibilityChange(); });
    }
}

bool ProcessorMetaData::isVisible() const { return visibility_; }

void ProcessorMetaData::setSelected(bool selection) {
    if (selection != selection_) {
        selection_ = selection;
        forEachObserver(
            [&](ProcessorMetaDataObserver* o) { o->onProcessorMetaDataSelectionChange(); });
    }
}

bool ProcessorMetaData::isSelected() const { return selection_; }

void ProcessorMetaData::serialize(Serializer& s) const {
    s.serialize("type", getClassIdentifier(), SerializationTarget::Attribute);
    s.serialize("position", position_);
    s.serialize("visibility", visibility_);
    s.serialize("selection", selection_);
}

void ProcessorMetaData::deserialize(Deserializer& d) {
    ivec2 position{0,0};
    d.deserialize("position", position);
    setPosition(position);
    
    bool visibility{true};
    d.deserialize("visibility", visibility);
    setVisibile(visibility);
    
    bool selection{false};
    d.deserialize("selection", selection);
    setSelected(selection);
}

bool ProcessorMetaData::equal(const MetaData& rhs) const {
    if (auto tmp = dynamic_cast<const ProcessorMetaData*>(&rhs)) {
        return tmp->position_ == position_ && tmp->visibility_ == visibility_ &&
               tmp->selection_ == selection_;
    } else {
        return false;
    }
}

const std::string ProcessorMetaData::CLASS_IDENTIFIER = "org.inviwo.ProcessorMetaData";

}  // namespace
