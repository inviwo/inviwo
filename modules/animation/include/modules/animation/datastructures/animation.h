/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2021 Inviwo Foundation
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
#include <inviwo/core/io/serialization/serialization.h>
#include <inviwo/core/util/indirectiterator.h>
#include <inviwo/core/properties/property.h>
#include <inviwo/core/properties/propertyownerobserver.h>

#include <modules/animation/datastructures/animationtime.h>
#include <modules/animation/datastructures/track.h>
#include <modules/animation/datastructures/trackobserver.h>
#include <modules/animation/datastructures/animationobserver.h>
#include <modules/animation/datastructures/animationstate.h>

#include <memory>
#include <vector>

namespace inviwo {

namespace animation {

class AnimationManager;
class BasePropertyTrack;

/** \class Animation
 * Animation data structure, owns a list of tracks.
 * Each Track usually represents a value to be animated.
 */
class IVW_MODULE_ANIMATION_API Animation : public AnimationObservable,
                                           public Serializable,
                                           public TrackObserver,
                                           public PropertyOwnerObserver {
public:
    using iterator = util::IndirectIterator<typename std::vector<std::unique_ptr<Track>>::iterator>;
    using const_iterator =
        util::IndirectIterator<typename std::vector<std::unique_ptr<Track>>::const_iterator>;
    /**
     * Creates an empty Animation.
     * AnimationManager is required only for the Property-based adding convenience functions:
     * add(Property* property)
     * addKeyframe(Property* property, Seconds time)
     * addKeyframeSequence(Property* property, Seconds time)
     * Trying to use these functions with an invalid AnimationManager will throw Exceptions.
     *
     * @note nullptr to AnimationManager should mainly be used for unit testing and the likes.
     * @param animationManager used for creating PropertyTrack/KeyframeSequence/Keyframe.
     * @param name default Animation
     */
    Animation(AnimationManager* animationManager = nullptr, std::string_view name = "Animation");
    Animation(const Animation&);
    Animation(Animation&&) = default;
    Animation& operator=(const Animation& that);
    Animation& operator=(Animation&& that) = default;

    AnimationTimeState operator()(Seconds from, Seconds to, AnimationState state) const;

    bool empty() const;
    size_t size() const;
    Track& operator[](size_t i);
    const Track& operator[](size_t i) const;

    iterator begin();
    const_iterator begin() const;
    iterator end();
    const_iterator end() const;

    void add(std::unique_ptr<Track> track);

    /**
     * Adds a PropertyTrack, or returns an existing it has one for the property.
     * The added PropertyTrack will be removed if the property is removed.
     * @return BasePropertyTrack if added, nullptr otherwise.
     * @see BasePropertyTrack
     * @throws Exception if no AnimationManager has been supplied.
     */
    BasePropertyTrack* add(Property* property);
    /**
     * Add keyframe at specified time.
     * Creates a new track if no track with the supplied property exists.
     * The PropertyTrack will be removed if the property is removed.
     * @return Keyframe if successsfully added, nullptr otherwise.
     * @throws Exception if no AnimationManager has been supplied.
     */
    Keyframe* addKeyframe(Property* property, Seconds time);
    /**
     * Add sequence at specified time.
     * Creates a new track if no track with the supplied property exists.
     * The PropertyTrack will be removed if the property is removed.
     * @return KeyframeSequence if successsfully added, nullptr otherwise.
     * @throws Exception if no AnimationManager has been supplied.
     */
    KeyframeSequence* addKeyframeSequence(Property* property, Seconds time);

    /**
     * Remove tracks at index i, indicating the order in which the track was added,
     * not the order in which they are sorted by Track priority.
     * No range check is done.
     * Calls TrackObserver::notifyTrackRemoved after removing track.
     */
    std::unique_ptr<Track> remove(size_t i);

    /**
     * Remove Track if matching any of the tracks.
     * Calls AnimationObservable::notifyTrackRemoved after removing Track.
     * @note Track will be deleted if removed so do not use pointer after calling this function.
     */
    std::unique_ptr<Track> remove(Track* track);

    /**
     * Remove Keyframe if matching any of the Keyframes in the tracks.
     * Calls TrackObserver::notifyKeyframeRemoved after removing Keyframe.
     * Removes the KeyFrameSequence owning the Keyframe if it does not contain any Keyframe
     * after removal, thereby calling KeyFrameSequenceObserver::notifyKeyframeSequenceRemoved
     * Does nothing if no match was found.
     * @note Keyframe will be deleted if removed so do not use pointer after calling this
     * function.
     */
    std::unique_ptr<Keyframe> remove(Keyframe* key);

    /**
     * Remove KeyframeSequence if matching any of the Sequences in the tracks.
     * Calls KeyFrameSequenceObserver::notifyKeyframeSequenceRemoved for the sequence removed
     * and TrackObserver::notifyKeyframeRemoved for every removed keyframe.
     * Does nothing if no match was found.
     * @note KeyframeSequences and its Keyframes will be deleted if removed so
     * do not use pointer after calling this function.
     */
    std::unique_ptr<KeyframeSequence> remove(KeyframeSequence* seq);

    /**
     * Find PropertyTrack containing the property.
     * @return iterator to the first PropertyTrack containing the specified property,
     * or end() if not found.
     * @see BasePropertyTrack
     */
    iterator findTrack(Property* withMe);

    /**
     * Remove all tracks. Calls TrackObserver::notifyTrackRemoved for each removed track.
     */
    void clear();

    /**
     * Return a sorted list, in ascending order, of all Keyframe times existing in the
     * animation.
     */
    std::vector<Seconds> getAllTimes() const;

    /**
     * Return time of first Keyframe in all tracks, or 0 if no track exist.
     */
    Seconds getFirstTime() const;

    /**
     * Return time of last Keyframe in all tracks, or 0 if no track exist.
     */
    Seconds getLastTime() const;

    /**
     * Return the name of the Animation. Used for display in the GUI.
     */
    const std::string& getName() const;
    /**
     * Set new name and notify observers if different from old name.
     */
    void setName(std::string_view name);

    virtual void serialize(Serializer& s) const override;
    virtual void deserialize(Deserializer& d) override;

    // PropertyOwnerObserver overload. Removes its corresponding Track
    virtual void onWillRemoveProperty(Property* property, size_t index) override;

private:
    /**
     * Call when a track has been pushed to tracks_.
     * Adds specified Track to priorityTracks_ and notifies observers.
     */
    void trackAddedInternal(Track* t);
    /**
     * Call when a track has been removed from tracks_.
     * Removes specified Track from priorityTracks_ and notifies observers.
     */
    void trackRemovedInternal(Track* t);
    AnimationManager* getManager();
    virtual void onPriorityChanged(Track* t) override;
    void doPrioritySort();

    virtual void onFirstMoved(Track* t) override;
    virtual void onLastMoved(Track* t) override;

    std::vector<std::unique_ptr<Track>> tracks_;
    std::vector<Track*> priorityTracks_;
    std::string name_;
    AnimationManager* am_;
};

}  // namespace animation

}  // namespace inviwo
