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

#ifndef IVW_CONTROLKEYFRAMESEQUENCE_H
#define IVW_CONTROLKEYFRAMESEQUENCE_H

#include <modules/animation/animationmoduledefine.h>

#include <modules/animation/datastructures/basekeyframesequence.h>
#include <modules/animation/datastructures/controlkeyframe.h>

namespace inviwo {

namespace animation {

/** \class ControlKeyframeSequence
 * KeyframeSequence for Control Keyframes.
 * @see KeyframeSequence
 */
class IVW_MODULE_ANIMATION_API ControlKeyframeSequence
    : public BaseKeyframeSequence<ControlKeyframe> {
public:
    ControlKeyframeSequence();
    ControlKeyframeSequence(std::vector<std::unique_ptr<ControlKeyframe>> keyframes);
    ControlKeyframeSequence(const ControlKeyframeSequence& rhs);
    ControlKeyframeSequence& operator=(const ControlKeyframeSequence& that);
    virtual ~ControlKeyframeSequence();

    virtual ControlKeyframeSequence* clone() const override;

    virtual AnimationTimeState operator()(Seconds from, Seconds to, AnimationState state) const;
};

}  // namespace animation

}  // namespace inviwo

#endif  // IVW_CONTROLKEYFRAMESEQUENCE_H
