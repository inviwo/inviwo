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

#ifndef IVW_CONTROLTRACK_H
#define IVW_CONTROLTRACK_H

#include <modules/animation/animationmoduledefine.h>
#include <inviwo/core/common/inviwo.h>

#include <modules/animation/datastructures/track.h>
#include <modules/animation/datastructures/animationtime.h>
#include <modules/animation/datastructures/animationstate.h>
#include <modules/animation/datastructures/controlkeyframesequence.h>

namespace inviwo {

namespace animation {

/** \class ControlTrack
 * A special track for manipulating the playback.
 * Exposes functions for adding a KeyFrame and KeyFrameSequence
 * using the current values of the Property.
 * @see Track
 */
class ControlTrack : public Track {
public:

	ControlTrack();

    /**
     * Remove all keyframe sequences and call TrackObserver::notifyKeyframeSequenceRemoved
     */
    virtual ~ControlTrack();

    static std::string classIdentifier();

    virtual std::string getClassIdentifier() const override;

    virtual void setEnabled(bool enabled) override;
    virtual bool isEnabled() const override;

    virtual void setIdentifier(const std::string& identifier) override;

    virtual const std::string& getIdentifier() const override;

    virtual void setName(const std::string& name) override;
    virtual const std::string& getName() const override;

    virtual void setPriority(size_t priority) override;

    virtual size_t getPriority() const override;

    virtual Seconds firstTime() const override;

    virtual Seconds lastTime() const override;

    virtual AnimationTimeState operator()(Seconds from, Seconds to,
                                          AnimationState state) const override;

    virtual void addTyped(const ControlKeyframeSequence& sequence);

    virtual void add(const KeyframeSequence& sequence) override;

    virtual size_t size() const override;

    virtual ControlKeyframeSequence& operator[](size_t i) override;

    virtual const ControlKeyframeSequence& operator[](size_t i) const override;

    virtual void remove(size_t i) override;

    virtual void serialize(Serializer& s) const override;
    virtual void deserialize(Deserializer& d) override;

private:
    virtual void onKeyframeSequenceMoved(KeyframeSequence* seq) override;

    bool enabled_{true};
    std::string identifier_;
    std::string name_;
    size_t priority_{0};

    // Sorted list of non-overlapping sequences of key frames
    std::vector<std::unique_ptr<ControlKeyframeSequence>> sequences_;
};

} // namespace

} // namespace

#endif // IVW_CONTROLTRACK_H

