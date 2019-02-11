/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2019 Inviwo Foundation
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

#ifndef IVW_KEYFRAME_H
#define IVW_KEYFRAME_H

#include <modules/animation/animationmoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/io/serialization/serializable.h>
#include <modules/animation/datastructures/keyframeobserver.h>
#include <modules/animation/datastructures/animationtime.h>

namespace inviwo {

namespace animation {

/** \class Keyframe
 * Interface for keyframes in an animation Track.
 * A keyframe usually contain a snapshot of a value at a given time,
 * which will used for interpolation in a KeyFrameSequence.
 * Note that a keyframe could also be a script to be executed.
 * @see KeyFrameSequence
 * @see Track
 */
class IVW_MODULE_ANIMATION_API Keyframe : public Serializable, public KeyframeObservable {
public:
    virtual Keyframe* clone() const = 0;

    virtual void setTime(Seconds time) = 0;
    virtual Seconds getTime() const = 0;

    virtual bool isSelected() const = 0;
    virtual void setSelected(bool selected) = 0;

    virtual void serialize(Serializer& s) const override = 0;
    virtual void deserialize(Deserializer& d) override = 0;
};

IVW_MODULE_ANIMATION_API bool operator<(const Keyframe& a, const Keyframe& b);
IVW_MODULE_ANIMATION_API bool operator<=(const Keyframe& a, const Keyframe& b);
IVW_MODULE_ANIMATION_API bool operator>(const Keyframe& a, const Keyframe& b);
IVW_MODULE_ANIMATION_API bool operator>=(const Keyframe& a, const Keyframe& b);

IVW_MODULE_ANIMATION_API bool operator<(const Keyframe& a, const Seconds& b);
IVW_MODULE_ANIMATION_API bool operator<=(const Keyframe& a, const Seconds& b);
IVW_MODULE_ANIMATION_API bool operator>(const Keyframe& a, const Seconds& b);
IVW_MODULE_ANIMATION_API bool operator>=(const Keyframe& a, const Seconds& b);

IVW_MODULE_ANIMATION_API bool operator<(const Seconds& a, const Keyframe& b);
IVW_MODULE_ANIMATION_API bool operator<=(const Seconds& a, const Keyframe& b);
IVW_MODULE_ANIMATION_API bool operator>(const Seconds& a, const Keyframe& b);
IVW_MODULE_ANIMATION_API bool operator>=(const Seconds& a, const Keyframe& b);

}  // namespace animation

}  // namespace inviwo

#endif  // IVW_KEYFRAME_H
