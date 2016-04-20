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
#include <inviwo/core/interaction/events/mouseevent.h>
#include <inviwo/core/interaction/events/touchevent.h>

namespace inviwo {

/** \class PickingObject
 */
class IVW_CORE_API PickingObject {
public:
    friend class PickingManager;

    enum class InteractionEventType {
        NoneSupported = 0,
        MouseInteraction = 1,
        TouchInteraction = 2
    };

    PickingObject(size_t start, size_t size = 1);

    virtual ~PickingObject();

    size_t getPickingId(size_t id = 0) const;
    size_t getPickedId() const;
    size_t getSize() const;

    vec3 getPickingColor(size_t id = 0) const;
    /**
    * \brief returns initial position of picking. The current position (mouse move) 
    *        is encoded in the MouseEvent.
    *
    * @return initial picking position
    *
    * \see getPickingMouseEvent
    */
    const vec2& getPickingPosition() const;
    /**
    * \brief returns the delta of the last mouse movement
    *
    * @return last mouse movement delta (normalized with respect to screen coordinates)
    */
    const vec2& getPickingMove() const;
    const double& getPickingDepth() const;

    /** 
     * \brief returns delta between current mouse position and initial picking position
     * 
     * @return delta of current mouse position and picking start (normalized with respect to screen coordinates)
     *
     * \see getPickingPosition
     */
    vec2 getPickingTotalDelta() const;

    void picked() const;

    InteractionEventType getPickingInteractionType() const;

    void setPickingMouseEvent(MouseEvent);
    const MouseEvent& getPickingMouseEvent() const;

    void setPickingTouchEvent(TouchEvent);
    const TouchEvent& getPickingTouchEvent() const;

    void setPickingMove(vec2);
    void setPickingPosition(vec2);
    void setPickingDepth(double);

private:
    size_t getCapacity() const;
    void setSize(size_t size);
    void setPickedId(size_t id);
    void setCallback(std::function<void(const PickingObject*)> func);

    size_t start_;
    size_t size_;
    size_t capacity_;
    size_t pickedId_ = 0;

    MouseEvent mouseEvent_;
    TouchEvent touchEvent_;
    InteractionEventType interactionEventType_;

    vec2 pos_;
    double depth_;
    vec2 move_;

    std::function<void(const PickingObject*)> callback_;
};

} // namespace

#endif // IVW_PICKINGOBJECT_H