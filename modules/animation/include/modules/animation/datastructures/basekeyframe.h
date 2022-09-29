/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2022 Inviwo Foundation
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

#include <modules/animation/animationmoduledefine.h>         // for IVW_MODULE_ANIMATION_API

#include <modules/animation/datastructures/animationtime.h>  // for Seconds
#include <modules/animation/datastructures/keyframe.h>       // for Keyframe

namespace inviwo {
class Deserializer;
class Serializer;

namespace animation {

class IVW_MODULE_ANIMATION_API BaseKeyframe : public Keyframe {
public:
    using value_type = void;
    BaseKeyframe() = default;
    BaseKeyframe(Seconds time);

    virtual ~BaseKeyframe() = default;

    virtual Seconds getTime() const override;
    virtual void setTime(Seconds time) override;

    virtual bool isSelected() const override;
    virtual void setSelected(bool selected) override;

    virtual void serialize(Serializer& s) const override;
    virtual void deserialize(Deserializer& d) override;

protected:
    BaseKeyframe(const BaseKeyframe& rhs) = default;
    BaseKeyframe(BaseKeyframe&& rhs) = default;
    BaseKeyframe& operator=(const BaseKeyframe& that);
    BaseKeyframe& operator=(BaseKeyframe&& that) = default;

private:
    bool isSelected_{false};
    Seconds time_{0.0};
};

}  // namespace animation

}  // namespace inviwo
