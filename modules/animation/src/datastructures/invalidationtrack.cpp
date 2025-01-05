/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2022-2024 Inviwo Foundation
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

#include <modules/animation/datastructures/invalidationtrack.h>

#include <inviwo/core/network/processornetwork.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/properties/property.h>
#include <inviwo/core/util/stringconversion.h>

#include <modules/animation/algorithm/animationrange.h>  // for animateRange

namespace inviwo {

namespace animation {

InvalidationKeyframe::InvalidationKeyframe(Seconds time) : BaseKeyframe(time) {}
InvalidationKeyframe::~InvalidationKeyframe() = default;
InvalidationKeyframe::InvalidationKeyframe(const InvalidationKeyframe&) = default;
InvalidationKeyframe& InvalidationKeyframe::operator=(const InvalidationKeyframe&) = default;
InvalidationKeyframe* InvalidationKeyframe::clone() const {
    return new InvalidationKeyframe(*this);
}

InvalidationKeyframeSequence::InvalidationKeyframeSequence(
    std::vector<std::unique_ptr<InvalidationKeyframe>> keyframes)
    : BaseKeyframeSequence<InvalidationKeyframe>(std::move(keyframes)) {}
InvalidationKeyframeSequence* InvalidationKeyframeSequence::clone() const {
    return new InvalidationKeyframeSequence(*this);
}

void InvalidationKeyframeSequence::serialize(Serializer& s) const {
    BaseKeyframeSequence<InvalidationKeyframe>::serialize(s);
    s.serialize("path", path);
}
void InvalidationKeyframeSequence::deserialize(Deserializer& d) {
    BaseKeyframeSequence<InvalidationKeyframe>::deserialize(d);
    d.deserialize("path", path);
}

InvalidationTrack::InvalidationTrack(ProcessorNetwork* network)
    : BaseTrack<InvalidationKeyframeSequence>{"Invalidation Track", 0}, network{network} {}

InvalidationTrack::~InvalidationTrack() = default;

InvalidationTrack* InvalidationTrack::clone() const { return new InvalidationTrack(*this); }

std::string_view InvalidationTrack::classIdentifier() {
    return "org.inviwo.animation.InvalidationTrack";
}
std::string_view InvalidationTrack::getClassIdentifier() const { return classIdentifier(); }

AnimationTimeState InvalidationTrack::operator()(Seconds, Seconds to, AnimationState state) const {
    if (!isEnabled() || empty()) return {to, state};

    for (auto it = begin(); it != end(); ++it) {
        if (it->getFirstTime() <= to && it->getLastTime() >= to) {
            const auto [processorId, propertyPath] = util::splitByFirst(it->path, ".");
            if (auto processor = network->getProcessorByIdentifier(processorId)) {
                if (propertyPath.empty()) {
                    processor->invalidate(InvalidationLevel::InvalidOutput);
                } else if (auto property = processor->getPropertyByPath(propertyPath)) {
                    property->propertyModified();
                } else {
                    log::user::warn("Unable to find property '{}' of processor '{}'", propertyPath,
                                    processorId);
                }
            } else {
                log::user::warn("Unable to find processor '{}'", processorId);
            }
        }
    }

    return {to, state};
}

}  // namespace animation

}  // namespace inviwo
