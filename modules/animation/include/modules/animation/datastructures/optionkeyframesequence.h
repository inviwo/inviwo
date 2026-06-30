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
#include <modules/animation/datastructures/basekeyframesequence.h>
#include <modules/animation/datastructures/optionkeyframe.h>

#include <cstddef>
#include <memory>
#include <vector>

namespace inviwo {

namespace animation {

/**
 * KeyframeSequence for a BaseOptionProperty. The selected index is held constant between keyframes,
 * i.e. the index of the closest preceding keyframe is used. The index is never interpolated since
 * option properties only have discrete states.
 * @see OptionTrack
 * @see KeyframeSequence
 */
class IVW_MODULE_ANIMATION_API OptionKeyframeSequence
    : public BaseKeyframeSequence<OptionKeyframe> {
public:
    using key_type = typename BaseKeyframeSequence<OptionKeyframe>::key_type;

    OptionKeyframeSequence() = default;
    OptionKeyframeSequence(std::vector<std::unique_ptr<OptionKeyframe>> keyframes);
    OptionKeyframeSequence(const OptionKeyframeSequence& rhs) = default;
    OptionKeyframeSequence& operator=(const OptionKeyframeSequence& that) = default;
    virtual ~OptionKeyframeSequence() = default;

    virtual OptionKeyframeSequence* clone() const override;

    /*
     * Set index to that of the closest keyframe at or before 'to' (constant interpolation).
     */
    void operator()(Seconds from, Seconds to, size_t& index) const;
};

}  // namespace animation

}  // namespace inviwo
