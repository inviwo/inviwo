/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2016 Inviwo Foundation
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

#ifndef IVW_GESTUREEVENT_H
#define IVW_GESTUREEVENT_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/interaction/events/interactionevent.h>
#include <inviwo/core/interaction/events/gesturestate.h>
#include <inviwo/core/util/constexprhash.h>

namespace inviwo {

class IVW_CORE_API GestureEvent : public InteractionEvent {
public:
    GestureEvent(vec2 deltaPos, double deltaDistance, GestureType type,
                 GestureState state, int numFingers, vec2 screenPosNorm, uvec2 canvasSize);

    GestureEvent(const GestureEvent& rhs) = default;
    GestureEvent& operator=(const GestureEvent& that) = default;
    virtual GestureEvent* clone() const override;
    virtual ~GestureEvent() = default;

    vec2 deltaPos() const;
    double deltaDistance() const;
    GestureType type() const;
    GestureState state() const;
    int numFingers() const;
    vec2 screenPosNormalized() const;
    vec2 canvasSize() const;

    void modify(vec2);

    virtual uint64_t hash() const override;
    static constexpr uint64_t chash() {
        return util::constexpr_hash("org.inviwo.GestureEvent");
    }

private:
    GestureType type_;
    GestureState state_;
    int numFingers_;

    vec2 deltaPos_;
    double deltaDistance_;
    vec2 screenPosNorm_;
    uvec2 canvasSize_;
};

}  // namespace

#endif  // IVW_GESTUREEVENT_H