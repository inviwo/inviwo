/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2024 Inviwo Foundation
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

#include <modules/animation/animationmoduledefine.h>  // for IVW_MODULE_ANIMATION...

#include <inviwo/core/io/serialization/serializable.h>               // for Serializable
#include <modules/animation/datastructures/animationstate.h>         // for AnimationState, Anim...
#include <modules/animation/datastructures/animationtime.h>          // for Seconds
#include <modules/animation/datastructures/trackobserver.h>          // for TrackObservable
#include <modules/animation/datastructures/valuekeyframesequence.h>  // for KeyframeSequenceTyped

#include <cstddef>  // for size_t
#include <memory>   // for unique_ptr
#include <string>   // for string
#include <vector>   // for vector

namespace inviwo {
class Deserializer;
class Serializer;

namespace animation {
class Keyframe;
class KeyframeSequence;

/** \class Track
 * Interface for tracks in an animation.
 * A track usually represent a value to be animated over time and
 * contains a list of KeyFrameSequence.
 * The value will only be animated when the time in the animation
 * is within a KeyFrameSequence in the Track.
 *
 * A Track has additional metadata information,
 * such as (display) Name, Enabled, Priority.
 * Tracks are listed in order based on their priority.
 * @see KeyFrameSequence
 */
class IVW_MODULE_ANIMATION_API Track : public Serializable, public TrackObservable {
public:
    Track() = default;

    /**
     * Remove all keyframe sequences and call TrackObserver::notifyKeyframeSequenceRemoved
     */
    virtual ~Track() = default;
    /**
     * Create a deep copy of the Track including references to its possibly underlying Property.
     */
    virtual Track* clone() const = 0;

    virtual std::string getClassIdentifier() const = 0;

    virtual bool isEnabled() const = 0;
    virtual void setEnabled(bool enabled) = 0;

    /**
     * Set Track name. Used when displaying the track.
     */
    virtual void setName(const std::string& name) = 0;
    virtual const std::string& getName() const = 0;

    /**
     * Return Track priority (0 is highest)
     */
    virtual size_t getPriority() const = 0;

    /**
     * Set priority (0 is highest).
     * The Track with highest priority is evaluated first
     * at a given time by Animation.
     */
    virtual void setPriority(size_t priority) = 0;

    virtual Seconds getFirstTime() const = 0;
    virtual Seconds getLastTime() const = 0;
    /*
     * Return time of previous keyframe if found.
     */
    virtual std::optional<Seconds> getPrevTime(Seconds at) const = 0;
    /*
     * Return time of next keyframe if found.
     */
    virtual std::optional<Seconds> getNextTime(Seconds at) const = 0;
    virtual std::vector<Seconds> getAllTimes() const = 0;

    /**
     * Return the number of KeyframeSequences in the track.
     */
    virtual size_t size() const = 0;
    virtual bool empty() const = 0;

    /**
     * Animate track between times from and to
     * @return the playback state after performing animation.
     */
    virtual AnimationTimeState operator()(Seconds from, Seconds to, AnimationState state) const = 0;

    virtual KeyframeSequence& operator[](size_t i) = 0;
    virtual const KeyframeSequence& operator[](size_t i) const = 0;

    virtual const KeyframeSequence& getFirst() const = 0;
    virtual KeyframeSequence& getFirst() = 0;
    virtual const KeyframeSequence& getLast() const = 0;
    virtual KeyframeSequence& getLast() = 0;

    /**
     * Add a Keyframe at time and return the added keyframe.
     * The Keyframe is added to a new KeyframeSequence if asNewSequence is true,
     * otherwise it should be added to the closest KeyframeSequence at specified time.
     * A KeyframeSequence is added to the Track if none exists.
     * @see BaseTrack::add(Seconds time, bool asNewSequence)
     */
    virtual Keyframe* add(Seconds time, bool asNewSequence) = 0;

    /**
     * Add KeyframeSequence and call TrackObserver::notifyKeyframeSequenceAdded
     */
    virtual KeyframeSequence* add(std::unique_ptr<KeyframeSequence> sequence) = 0;

    /**
     * Remove KeyframeSequence at index i and call TrackObserver::notifyKeyframeSequenceRemoved
     */
    virtual std::unique_ptr<KeyframeSequence> remove(size_t i) = 0;
    virtual std::unique_ptr<KeyframeSequence> remove(KeyframeSequence* seq) = 0;
    virtual std::unique_ptr<Keyframe> remove(Keyframe* key) = 0;

    virtual void serialize(Serializer& s) const override = 0;
    virtual void deserialize(Deserializer& d) override = 0;

    friend bool operator<(const Track& a, const Track& b) {
        return a.getFirstTime() < b.getFirstTime();
    }
    friend bool operator<=(const Track& a, const Track& b) {
        return a.getFirstTime() <= b.getFirstTime();
    }
    friend bool operator>(const Track& a, const Track& b) {
        return a.getFirstTime() > b.getFirstTime();
    }
    friend bool operator>=(const Track& a, const Track& b) {
        return a.getFirstTime() >= b.getFirstTime();
    }

    friend bool operator<(const Track& a, const Seconds& b) { return a.getFirstTime() < b; }
    friend bool operator<=(const Track& a, const Seconds& b) { return a.getFirstTime() <= b; }
    friend bool operator>(const Track& a, const Seconds& b) { return a.getFirstTime() > b; }
    friend bool operator>=(const Track& a, const Seconds& b) { return a.getFirstTime() >= b; }

    friend bool operator<(const Seconds& a, const Track& b) { return a < b.getFirstTime(); }
    friend bool operator<=(const Seconds& a, const Track& b) { return a <= b.getFirstTime(); }
    friend bool operator>(const Seconds& a, const Track& b) { return a > b.getFirstTime(); }
    friend bool operator>=(const Seconds& a, const Track& b) { return a >= b.getFirstTime(); }
};

/** \class TrackTyped
 * Track containing KeyFrameSequence of a given KeyFrame type.
 * @see Track
 * @see KeyframeSequenceTyped
 */
template <typename Key>
class TrackTyped : public Track {
public:
    TrackTyped() = default;
    virtual ~TrackTyped() = default;

    virtual KeyframeSequenceTyped<Key>& operator[](size_t i) override = 0;
    virtual const KeyframeSequenceTyped<Key>& operator[](size_t i) const override = 0;

    virtual void addTyped(const KeyframeSequenceTyped<Key>& sequence) = 0;
};

}  // namespace animation

}  // namespace inviwo
