/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2026 Inviwo Foundation
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

#include <modules/animation/factories/trackfactory.h>

#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/properties/property.h>
#include <inviwo/core/util/exception.h>
#include <inviwo/core/util/logcentral.h>
#include <modules/animation/datastructures/optiontrack.h>
#include <modules/animation/datastructures/propertytrack.h>
#include <modules/animation/factories/trackfactoryobject.h>

#include <ostream>
#include <string_view>
#include <utility>

namespace inviwo {
namespace animation {

TrackFactory::TrackFactory(ProcessorNetwork* network)
    : network_{network}, keyframeFactory{*this}, keyframeSequenceFactory{*this} {}

bool TrackFactory::hasKey(std::string_view key) const { return Parent::hasKey(key); }

std::unique_ptr<Track> TrackFactory::create(std::string_view key) const {
    return Parent::create(key, network_);
}

std::unique_ptr<Track> TrackFactory::create(Property* property) const {
    const auto createTrack = [&](std::string_view trackId) -> std::unique_ptr<Track> {
        if (auto track = create(trackId)) {
            if (auto basePropertyTrack = dynamic_cast<BasePropertyTrack*>(track.get())) {
                try {
                    basePropertyTrack->setProperty(property);
                } catch (const Exception& e) {
                    log::warn("{} Invalid property class identified?", e.getMessage());
                    return nullptr;
                }
                return track;
            }
        }
        return nullptr;
    };

    if (auto it = propertyToTrackMap_.find(property->getClassIdentifier());
        it != propertyToTrackMap_.end()) {
        return createTrack(it->second);
    }

    // Any OptionProperty<T> can be animated by the generic OptionTrack using the selected index
    // through the BaseOptionProperty interface. This avoids having to register a separate track for
    // each option value type and also supports option properties with custom or unregistered enum
    // value types.
    if (dynamic_cast<BaseOptionProperty*>(property)) {
        return createTrack(OptionTrack::classIdentifier());
    }

    return nullptr;
}

std::unique_ptr<KeyframeSequence> TrackFactory::createSequence(std::string_view key) const {
    auto it = this->map_.find(key);
    if (it != end(this->map_)) {
        return it->second->createSequence();
    } else {
        return nullptr;
    }
}
std::unique_ptr<Keyframe> TrackFactory::createKeyframe(std::string_view key) const {
    auto it = this->map_.find(key);
    if (it != end(this->map_)) {
        return it->second->createKeyframe();
    } else {
        return nullptr;
    }
}

void TrackFactory::registerPropertyTrackConnection(std::string_view propertyClassID,
                                                   std::string_view trackClassID) {
    propertyToTrackMap_.emplace(propertyClassID, trackClassID);
}

KeyframeSequenceFactory::KeyframeSequenceFactory(TrackFactory& tf) : trackFactory_{&tf} {}
std::unique_ptr<KeyframeSequence> KeyframeSequenceFactory::create(std::string_view key) const {
    return trackFactory_->createSequence(key);
}
bool KeyframeSequenceFactory::hasKey(std::string_view key) const {
    return trackFactory_->hasKey(key);
}

KeyframeFactory::KeyframeFactory(TrackFactory& tf) : trackFactory_{&tf} {}
std::unique_ptr<Keyframe> KeyframeFactory::create(std::string_view key) const {
    return trackFactory_->createKeyframe(key);
}
bool KeyframeFactory::hasKey(std::string_view key) const { return trackFactory_->hasKey(key); }

}  // namespace animation
}  // namespace inviwo
