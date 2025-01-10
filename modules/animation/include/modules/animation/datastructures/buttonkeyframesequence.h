/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2020-2025 Inviwo Foundation
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

#include <modules/animation/animationmoduledefine.h>  // for IVW_MODULE_ANIMATION_API

#include <modules/animation/datastructures/animationtime.h>         // for Seconds
#include <modules/animation/datastructures/basekeyframesequence.h>  // for BaseKeyframeSequence
#include <modules/animation/datastructures/buttonkeyframe.h>        // for ButtonKeyframe
#include <modules/animation/datastructures/keyframe.h>              // for operator<

#include <functional>  // for __base
#include <memory>      // for unique_ptr
#include <string>      // for basic_string
#include <vector>      // for vector

namespace inviwo {

namespace animation {

/** \class ButtonKeyframeSequence
 * KeyframeSequence for ButtonProperty. Button will be pressed when passing over keyframe.
 * @see KeyframeSequence
 */
class IVW_MODULE_ANIMATION_API ButtonKeyframeSequence
    : public BaseKeyframeSequence<ButtonKeyframe> {
public:
    using key_type = typename BaseKeyframeSequence<ButtonKeyframe>::key_type;
    ButtonKeyframeSequence() = default;
    ButtonKeyframeSequence(std::vector<std::unique_ptr<ButtonKeyframe>> keyframes);
    ButtonKeyframeSequence(const ButtonKeyframeSequence& rhs) = default;
    ButtonKeyframeSequence& operator=(const ButtonKeyframeSequence& that) = default;
    virtual ~ButtonKeyframeSequence() = default;

    virtual ButtonKeyframeSequence* clone() const override;
    /*
     * Pressed will be true if any keyframe is between from and to.
     * Calls ButtonKeyframe::operator()(Seconds from, Seconds to, bool& pressed) for the
     * corresponding keyframe.
     */
    void operator()(Seconds from, Seconds to, bool& pressed) const;
};

}  // namespace animation

}  // namespace inviwo
