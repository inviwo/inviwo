/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2026 Inviwo Foundation
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

#include <modules/animation/datastructures/optiontrack.h>

#include <inviwo/core/io/serialization/deserializer.h>
#include <inviwo/core/io/serialization/serializationexception.h>
#include <inviwo/core/io/serialization/serializer.h>
#include <inviwo/core/network/processornetwork.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/properties/property.h>
#include <inviwo/core/properties/propertyowner.h>
#include <inviwo/core/util/assertion.h>
#include <inviwo/core/util/exception.h>

#include <modules/animation/datastructures/optionkeyframe.h>
#include <modules/animation/datastructures/optionkeyframesequence.h>

#include <algorithm>
#include <iterator>
#include <vector>

namespace inviwo {

namespace animation {

OptionTrack::OptionTrack(ProcessorNetwork* net)
    : BaseTrack<OptionKeyframeSequence>{"", 100}, property_{nullptr}, network_{net} {}

OptionTrack::OptionTrack(BaseOptionProperty* property)
    : BaseTrack<OptionKeyframeSequence>{property->getDisplayName(), 100}
    , property_{property}
    , network_{property->getOwner()->getProcessor()->getNetwork()} {}

OptionTrack::OptionTrack(BaseOptionProperty* property, ProcessorNetwork* net)
    : BaseTrack<OptionKeyframeSequence>{property->getDisplayName(), 100}
    , property_{property}
    , network_{net} {}

OptionTrack::~OptionTrack() = default;

OptionTrack* OptionTrack::clone() const { return new OptionTrack(*this); }

std::string_view OptionTrack::getClassIdentifier() const { return classIdentifier(); }

const BaseOptionProperty* OptionTrack::getProperty() const { return property_; }

BaseOptionProperty* OptionTrack::getProperty() { return property_; }

void OptionTrack::setProperty(Property* property) {
    if (auto prop = dynamic_cast<BaseOptionProperty*>(property)) {
        property_ = prop;
        this->setName(property_->getDisplayName());
    } else {
        throw Exception("Invalid property set to track");
    }
}

Track* OptionTrack::toTrack() { return this; }

std::unique_ptr<OptionKeyframe> OptionTrack::createKeyframe(Seconds time) const {
    return createKeyframe(property_, time);
}

std::unique_ptr<OptionKeyframe> OptionTrack::createKeyframe(const BaseOptionProperty* property,
                                                            Seconds time) const {
    return std::make_unique<OptionKeyframe>(time, property->getSelectedIndex());
}

void OptionTrack::setPropertyFromKeyframe(Property* dstProperty, const Keyframe* keyframe) const {
    auto prop = dynamic_cast<BaseOptionProperty*>(dstProperty);
    IVW_ASSERT(prop, "Invalid Property type");
    prop->setSelectedIndex(static_cast<const OptionKeyframe*>(keyframe)->getIndex());
}

void OptionTrack::setKeyframeFromProperty(const Property* srcProperty, Keyframe* keyframe) {
    auto prop = dynamic_cast<const BaseOptionProperty*>(srcProperty);
    IVW_ASSERT(prop, "Invalid Property type");
    static_cast<OptionKeyframe*>(keyframe)->setIndex(prop->getSelectedIndex());
}

/**
 * Track of sequences
 * ----------X======X====X-----------X=========X-------X=====X--------
 * |- case 1-|-case 2----------------|-case 2----------|-case 2------|
 *           |-case 2a---|-case 2b---|
 */
AnimationTimeState OptionTrack::operator()(Seconds from, Seconds to, AnimationState state) const {
    if (!this->isEnabled() || this->empty()) return {to, state};

    // 'it' will be the first seq. with a first time larger then 'to'.
    auto it = std::upper_bound(this->begin(), this->end(), to,
                               [](const auto& a, const auto& b) { return a < b; });

    if (it == this->begin()) {
        if (from > it->getFirstTime()) {  // case 1
            setPropertyFromKeyframe(property_, &(it->getFirst()));
        }
    } else {  // case 2
        auto& seq1 = *std::prev(it);

        if (to < seq1.getLastTime()) {  // case 2a
            size_t index = property_->getSelectedIndex();
            seq1(from, to, index);
            property_->setSelectedIndex(index);
        } else {  // case 2b
            if (from < seq1.getLastTime()) {
                // We came from before the previous key
                setPropertyFromKeyframe(property_, &(seq1.getLast()));
            } else if (it != this->end() && from > it->getFirstTime()) {
                // We came form after the next key
                setPropertyFromKeyframe(property_, &(it->getFirst()));
            }
            // we moved in an unmarked region, do nothing.
        }
    }
    return {to, state};
}

OptionKeyframe* OptionTrack::addKeyFrameUsingPropertyValue(const Property* property, Seconds time,
                                                           std::unique_ptr<Interpolation>) {
    auto prop = dynamic_cast<const BaseOptionProperty*>(property);
    if (!prop) {
        throw Exception(SourceContext{}, "Cannot add key frame from property type '{}' for '{}'.",
                        (property ? property->getClassIdentifier() : "null"),
                        (property_ ? property_->getClassIdentifier() : "null"));
    }
    if (this->empty()) {
        std::vector<std::unique_ptr<OptionKeyframe>> keys;
        keys.push_back(createKeyframe(prop, time));
        auto sequence = std::make_unique<OptionKeyframeSequence>(std::move(keys));
        if (auto se = this->add(std::move(sequence))) {
            return &se->getFirst();
        }
    } else {
        return static_cast<OptionKeyframe*>(this->addToClosestSequence(createKeyframe(prop, time)));
    }
    return nullptr;
}

OptionKeyframe* OptionTrack::addKeyFrameUsingPropertyValue(
    Seconds time, std::unique_ptr<Interpolation> interpolation) {
    return addKeyFrameUsingPropertyValue(property_, time, std::move(interpolation));
}

OptionKeyframeSequence* OptionTrack::addSequenceUsingPropertyValue(Seconds time,
                                                                   std::unique_ptr<Interpolation>) {
    std::vector<std::unique_ptr<OptionKeyframe>> keys;
    keys.push_back(createKeyframe(property_, time));
    auto sequence = std::make_unique<OptionKeyframeSequence>(std::move(keys));
    return this->add(std::move(sequence));
}

void OptionTrack::serialize(Serializer& s) const {
    if (!property_) {
        throw SerializationException(SourceContext{}, "No property set for the OptionTrack");
    }
    BaseTrack<OptionKeyframeSequence>::serialize(s);
    s.serialize("property", property_->getPath());
}

void OptionTrack::deserialize(Deserializer& d) {
    BaseTrack<OptionKeyframeSequence>::deserialize(d);

    std::string propertyId;
    d.deserialize("property", propertyId);

    IVW_ASSERT(network_, "Option track deserialization requires a ProcessorNetwork");
    property_ = dynamic_cast<BaseOptionProperty*>(network_->getProperty(propertyId));
    if (!property_) {
        throw SerializationException(SourceContext{}, "Could not find property {}", propertyId);
    }
}

}  // namespace animation

}  // namespace inviwo
