/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2026 Inviwo Foundation
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

#include <modules/animation/datastructures/animationtime.h>
#include <modules/animation/datastructures/basekeyframe.h>

#include <cstddef>

namespace inviwo {

class Deserializer;
class Serializer;

namespace animation {

/**
 * Keyframe for a BaseOptionProperty. Stores the selected index of the option property instead of
 * the selected value. This makes the keyframe independent of the value type of the option property
 * and allows it to be used with any OptionProperty<T>.
 * @see OptionTrack
 * @see OptionKeyframeSequence
 */
class IVW_MODULE_ANIMATION_API OptionKeyframe : public BaseKeyframe {
public:
    using value_type = size_t;  ///< make it possible to use with BaseKeyframeSequence

    OptionKeyframe() = default;
    OptionKeyframe(Seconds time);
    OptionKeyframe(Seconds time, size_t index);
    OptionKeyframe(const OptionKeyframe& rhs) = default;
    OptionKeyframe& operator=(const OptionKeyframe& that);
    virtual ~OptionKeyframe() = default;

    virtual OptionKeyframe* clone() const override;

    size_t getIndex() const;
    void setIndex(size_t index);

    virtual void serialize(Serializer& s) const override;
    virtual void deserialize(Deserializer& d) override;

private:
    size_t index_{0};
};

IVW_MODULE_ANIMATION_API bool operator==(const OptionKeyframe& a, const OptionKeyframe& b);
IVW_MODULE_ANIMATION_API bool operator!=(const OptionKeyframe& a, const OptionKeyframe& b);

}  // namespace animation

}  // namespace inviwo
