/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2017 Inviwo Foundation
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

#ifndef IVW_ANIMATION_H
#define IVW_ANIMATION_H

#include <modules/animation/animationmoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/io/serialization/serializable.h>

#include <modules/animation/datastructures/animationtime.h>
#include <modules/animation/datastructures/track.h>
#include <modules/animation/datastructures/trackobserver.h>
#include <modules/animation/datastructures/animationobserver.h>
#include <modules/animation/datastructures/animationstate.h>

namespace inviwo {

namespace animation {

/**
 *	Animation data structure, owns a list of tracks.
 */
class IVW_MODULE_ANIMATION_API Animation : public AnimationObservable,
                                           public Serializable,
                                           public TrackObserver {
public:
    Animation();
    Animation(const Animation&) = delete;
    Animation& operator=(const Animation& that) = delete;

    AniamtionTimeState operator()(Seconds from, Seconds to, AnimationState state) const;

    bool empty() const;
    size_t size() const;
    Track& operator[](size_t i);
    const Track& operator[](size_t i) const;

    void add(std::unique_ptr<Track> track);
    void removeTrack(size_t i);
    void removeTrack(const std::string& id);

    void removeKeyframe(Keyframe* key);

    void clear();

    std::vector<Seconds> getAllTimes() const;

    Seconds firstTime() const;
    Seconds lastTime() const;

    virtual void serialize(Serializer& s) const override;
    virtual void deserialize(Deserializer& d) override;

private:
    virtual void onPriorityChanged(Track* t) override;
    void doPrioritySort();

    virtual void onFirstMoved(Track* t) override;
    virtual void onLastMoved(Track* t) override;

    std::vector<std::unique_ptr<Track>> tracks_;
    std::vector<Track*> priorityTracks_;
};

}  // namespace

}  // namespace

#endif // IVW_ANIMATION_H

