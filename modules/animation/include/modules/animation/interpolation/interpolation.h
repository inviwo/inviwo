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

#ifndef IVW_KEYFRAME_INTERPOLATION_H
#define IVW_KEYFRAME_INTERPOLATION_H

#include <modules/animation/animationmoduledefine.h>
#include <inviwo/core/common/inviwo.h>

#include <modules/animation/datastructures/keyframe.h>
#include <modules/animation/datastructures/easing.h>
#include <inviwo/core/io/serialization/serializable.h>

#include <algorithm>

namespace inviwo {

namespace animation {

/** \class Interpolation
 *	Interface for keyframe interpolations.
 */
class IVW_MODULE_ANIMATION_API Interpolation : public Serializable {
public:
    Interpolation() = default;
    virtual ~Interpolation() = default;

    virtual Interpolation* clone() const = 0;

    virtual std::string getName() const = 0;

    virtual std::string getClassIdentifier() const = 0;
    virtual bool equal(const Interpolation& other) const = 0;

    virtual void serialize(Serializer& s) const override = 0;
    virtual void deserialize(Deserializer& d) override = 0;
};

IVW_MODULE_ANIMATION_API bool operator==(const Interpolation& a, const Interpolation& b);
IVW_MODULE_ANIMATION_API bool operator!=(const Interpolation& a, const Interpolation& b);

/** \class InterpolationTyped
 *	Base class for interpolation between key frames.
 *  Interpolation will always be performed between at least two key frames.
 *
 *  @see KeyFrame
 *  @see KeyFrameSequence
 */
template <typename Key>
class InterpolationTyped : public Interpolation {
public:
    InterpolationTyped() = default;
    virtual ~InterpolationTyped() = default;

    virtual InterpolationTyped* clone() const override = 0;

    virtual std::string getClassIdentifier() const override = 0;
    virtual void serialize(Serializer& s) const override = 0;
    virtual void deserialize(Deserializer& d) override = 0;

    // Override this function to interpolate between key frames
    virtual auto operator()(const std::vector<std::unique_ptr<Key>>& keys, Seconds from, Seconds to,
                            easing::EasingType easing) const -> typename Key::value_type = 0;
};

}  // namespace animation

}  // namespace inviwo

#endif  // IVW_KEYFRAME_INTERPOLATION_H
