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

#ifndef IVW_TRACK_H
#define IVW_TRACK_H

#include <modules/animation/animationmoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/util/exception.h>
#include <inviwo/core/io/serialization/serializable.h>

#include <modules/animation/datastructures/animationtime.h>
#include <modules/animation/datastructures/keyframe.h>
#include <modules/animation/datastructures/keyframesequence.h>
#include <modules/animation/datastructures/trackobserver.h>
#include <modules/animation/datastructures/animationstate.h>

namespace inviwo {

namespace animation {

/**
 * A Animation base Track interface, provides access to a list of KeyframeSequence,
 * and track metadata: Identifier, Name, Enabled, Priority. 
 */
class IVW_MODULE_ANIMATION_API Track : public Serializable,
                                       public TrackObservable,
                                       public KeyframeSequenceObserver {
public:
    Track() = default;
    virtual ~Track() = default;
    Track(const Track&) = delete;
    Track& operator=(const Track&) = delete;

    virtual std::string getClassIdentifier() const = 0;

    virtual void setEnabled(bool enabled) = 0;
    virtual bool isEnabled() const = 0;

    virtual void setIdentifier(const std::string& identifier) = 0;
    virtual const std::string& getIdentifier() const = 0;

    virtual void setName(const std::string& name) = 0;
    virtual const std::string& getName() const = 0;

    virtual void setPriority(size_t priority) = 0;
    virtual size_t getPriority() const = 0;

    virtual Seconds firstTime() const = 0;
    virtual Seconds lastTime() const = 0;

    virtual size_t size() const = 0;

    virtual AniamtionTimeState operator()(Seconds from, Seconds to, AnimationState state) const = 0;

    virtual KeyframeSequence& operator[](size_t i) = 0;
    virtual const KeyframeSequence& operator[](size_t i) const = 0;

    virtual void add(const KeyframeSequence& sequence) = 0;
    virtual void remove(size_t i) = 0;

    virtual void serialize(Serializer& s) const override = 0;
    virtual void deserialize(Deserializer& d) override = 0;
};

template <typename Key>
class TrackTyped : public Track {
public:
    TrackTyped() = default;
    virtual ~TrackTyped() = default;

    virtual KeyframeSequenceTyped<Key>& operator[](size_t i) override = 0;
    virtual const KeyframeSequenceTyped<Key>& operator[](size_t i) const override = 0;

    virtual void addTyped(const KeyframeSequenceTyped<Key>& sequence) = 0;
};



} // namespace

} // namespace

#endif // IVW_TRACK_H

