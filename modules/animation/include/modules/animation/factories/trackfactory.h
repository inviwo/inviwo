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
#pragma once

#include <modules/animation/animationmoduledefine.h>

#include <inviwo/core/util/factory.h>
#include <modules/animation/datastructures/track.h>
#include <modules/animation/datastructures/keyframe.h>
#include <modules/animation/datastructures/keyframesequence.h>
#include <modules/animation/factories/trackfactoryobject.h>

#include <functional>
#include <map>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace inviwo {

class ProcessorNetwork;
class Property;

namespace animation {

class TrackFactory;

class IVW_MODULE_ANIMATION_API KeyframeFactory : public Factory<Keyframe> {
public:
    explicit KeyframeFactory(TrackFactory& tf);
    virtual std::unique_ptr<Keyframe> create(std::string_view key) const override;
    virtual bool hasKey(std::string_view key) const override;

private:
    TrackFactory* trackFactory_;
};

class IVW_MODULE_ANIMATION_API KeyframeSequenceFactory : public Factory<KeyframeSequence> {
public:
    explicit KeyframeSequenceFactory(TrackFactory& tf);
    virtual std::unique_ptr<KeyframeSequence> create(std::string_view key) const override;
    virtual bool hasKey(std::string_view key) const override;

private:
    TrackFactory* trackFactory_;
};

class IVW_MODULE_ANIMATION_API TrackFactory
    : public Factory<Track>,
      public StandardFactory<Track, TrackFactoryObject, std::string_view, ProcessorNetwork*> {

    using Parent = StandardFactory<Track, TrackFactoryObject, std::string_view, ProcessorNetwork*>;

public:
    TrackFactory(ProcessorNetwork* network);

    using Parent::create;

    virtual bool hasKey(std::string_view key) const override;
    virtual std::unique_ptr<Track> create(std::string_view key) const override;
    std::unique_ptr<Track> create(Property* property) const;
    std::unique_ptr<KeyframeSequence> createSequence(std::string_view key) const;
    std::unique_ptr<Keyframe> createKeyframe(std::string_view key) const;

    /**
     * Register connection between a property and a track.
     * Used to create typed tracks for a property.
     * @param propertyClassID Property::getClassIdentifier
     * @param trackClassID PropertyTrack::getIdentifier()
     */
    void registerPropertyTrackConnection(std::string_view propertyClassID,
                                         std::string_view trackClassID);

    ProcessorNetwork* network_;

    KeyframeFactory keyframeFactory;
    KeyframeSequenceFactory keyframeSequenceFactory;

protected:
    std::map<std::string, std::string, std::less<>> propertyToTrackMap_;
};

}  // namespace animation

}  // namespace inviwo
