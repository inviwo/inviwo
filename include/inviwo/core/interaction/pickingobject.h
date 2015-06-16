/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2015 Inviwo Foundation
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

#ifndef IVW_PICKINGOBJECT_H
#define IVW_PICKINGOBJECT_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/interaction/pickingcallback.h>
#include <inviwo/core/interaction/events/mouseevent.h>
#include <inviwo/core/interaction/events/touchevent.h>

namespace inviwo {

/** \class PickingObject
 */
class IVW_CORE_API PickingObject {
public:
    enum InteractionEventType {
        NONE_SUPPORTED = 0,
        MOUSE_INTERACTION_EVENT = 1,
        TOUCH_INTERACTION_EVENT = 2
    };

    PickingObject(size_t, DataVec3UINT8::type);

    virtual ~PickingObject();

    const size_t& getPickingId() const;
    const vec3& getPickingColor() const;
    const DataVec3UINT8::type& getPickingColorAsUINT8() const;
    const vec2& getPickingPosition() const;
    const vec2& getPickingMove() const;

    const double& getPickingDepth() const;

    void picked() const;

    InteractionEventType getPickingInteractionType() const;

    void setPickingMouseEvent(MouseEvent);
    const MouseEvent& getPickingMouseEvent() const;

    void setPickingTouchEvent(TouchEvent);
    const TouchEvent& getPickingTouchEvent() const;

    void setPickingMove(vec2);
    void setPickingPosition(vec2);
    void setPickingDepth(double);

    PickingCallback* getCallbackContainer();

private:
    size_t id_;
    DataVec3UINT8::type colorUINT8_;
    vec3 color_;

    MouseEvent mouseEvent_;
    TouchEvent touchEvent_;
    InteractionEventType interactionEventType_;

    vec2 pos_;
    double depth_;
    vec2 move_;

    PickingCallback* onPickedCallback_;
};

} // namespace

#endif // IVW_PICKINGOBJECT_H