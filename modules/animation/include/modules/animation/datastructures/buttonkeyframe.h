/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2020 Inviwo Foundation
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
#include <modules/animation/datastructures/basekeyframe.h>
#include <modules/animation/datastructures/animationstate.h>

#include <inviwo/core/properties/buttonproperty.h>

namespace inviwo {

namespace animation {

/** \class ButtonKeyframe
 * Keyframe for buttons. Button will be pressed when passing over the keyframe.
 * @see Keyframe
 */
class IVW_MODULE_ANIMATION_API ButtonKeyframe : public BaseKeyframe {
public:
    //using value_type = ButtonProperty;
    ButtonKeyframe() = default;
    ButtonKeyframe(Seconds time, ButtonProperty* prop = nullptr);
    ButtonKeyframe(const ButtonKeyframe& rhs) = default;
    ButtonKeyframe& operator=(const ButtonKeyframe& that) = default;
    virtual ~ButtonKeyframe() = default;

    void setProperty(ButtonProperty* prop);

    const ButtonProperty& getValue() const { return *prop_; }
    ButtonProperty& getValue() { return *prop_; }

    virtual ButtonKeyframe* clone() const override;

    AnimationTimeState operator()(Seconds from, Seconds to, AnimationState state) const;

private:
    ButtonProperty* prop_ = nullptr;
};

}  // namespace animation

}  // namespace inviwo
