/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2022 Inviwo Foundation
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

#include <inviwo/core/io/serialization/deserializer.h>  // for ContainerWrapper<>::Item
#include <inviwo/core/util/exception.h>                 // for Exception

#include <modules/animation/datastructures/animationstate.h>  // for AnimationState, AnimationTi...
#include <modules/animation/datastructures/animationtime.h>   // for Seconds
#include <modules/animation/datastructures/basekeyframe.h>    // for BaseKeyframe
#include <modules/animation/datastructures/basekeyframesequence.h>  // for BaseKeyframeSequence
#include <modules/animation/datastructures/keyframe.h>              // for operator<
#include <modules/animation/datastructures/basetrack.h>             // for BaseTrack<>::key_type

#include <functional>  // for __base
#include <memory>      // for unique_ptr
#include <string>      // for basic_string
#include <vector>      // for vector

namespace inviwo {

class ProcessorNetwork;

namespace animation {

class IVW_MODULE_ANIMATION_API InvalidationKeyframe : public BaseKeyframe {
public:
    using value_type = void;
    InvalidationKeyframe() = default;
    InvalidationKeyframe(Seconds time);
    InvalidationKeyframe(const InvalidationKeyframe&);
    InvalidationKeyframe& operator=(const InvalidationKeyframe&);
    virtual ~InvalidationKeyframe();
    virtual InvalidationKeyframe* clone() const override;

private:
};

class IVW_MODULE_ANIMATION_API InvalidationKeyframeSequence
    : public BaseKeyframeSequence<InvalidationKeyframe> {
public:
    InvalidationKeyframeSequence() = default;
    InvalidationKeyframeSequence(std::vector<std::unique_ptr<InvalidationKeyframe>> keyframes);
    InvalidationKeyframeSequence(const InvalidationKeyframeSequence& rhs) = default;
    InvalidationKeyframeSequence& operator=(const InvalidationKeyframeSequence& that) = default;
    InvalidationKeyframeSequence& operator=(InvalidationKeyframeSequence&& that) = default;
    virtual ~InvalidationKeyframeSequence() = default;

    virtual InvalidationKeyframeSequence* clone() const override;

    virtual void serialize(Serializer& s) const override;
    virtual void deserialize(Deserializer& d) override;
    
    std::string processorId;
};

class IVW_MODULE_ANIMATION_API InvalidationTrack : public BaseTrack<InvalidationKeyframeSequence> {
public:
    InvalidationTrack(ProcessorNetwork* network);
    virtual ~InvalidationTrack();

    virtual InvalidationTrack* clone() const override;

    static std::string classIdentifier();
    virtual std::string getClassIdentifier() const override;

    virtual AnimationTimeState operator()(Seconds from, Seconds to,
                                          AnimationState state) const override;
                                          
    ProcessorNetwork* network;
};

}  // namespace animation

}  // namespace inviwo
