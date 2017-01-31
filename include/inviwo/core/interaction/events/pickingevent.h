/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2017 Inviwo Foundation
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

#ifndef IVW_PICKINGEVENT_H
#define IVW_PICKINGEVENT_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/interaction/events/event.h>
#include <inviwo/core/interaction/pickingstate.h>
#include <inviwo/core/util/constexprhash.h>

namespace inviwo {

class PickingAction;
class InteractionEvent;

/**
 * \class PickingEvent
 */
class IVW_CORE_API PickingEvent : public Event {
public:
    PickingEvent(const PickingAction* pickingAction, PickingState state, Event* event,
                 dvec3 pressNDC, dvec3 previousNDC, size_t pickedId);
    PickingEvent(const PickingAction* pickingAction, PickingState state,
                 std::unique_ptr<Event> event, dvec3 pressNDC, dvec3 previousNDC, size_t pickedId);

    virtual ~PickingEvent();
    PickingEvent(const PickingEvent&);
    PickingEvent& operator=(const PickingEvent&);


    virtual PickingEvent* clone() const override;
    
    /**
     *	Returns the local picking index of the object currently being picked. 
    */
    size_t getPickedId() const;

    /**
    * Returns the current normalized position
    */
    dvec2 getPosition() const;
    /**
    * Returns the current normalized depth
    */
    double getDepth() const;

    /**
    * Returns the previous normalized position
    */
    dvec2 getPreviousPosition() const;
    /**
    * Returns the previous normalized depth
    */
    double getPreviousDepth() const;

    /**
    * Returns the normalized position of the most resent press
    */
    dvec2 getPressedPosition() const;
    /**
    * Returns the normalized depth of the most resent press
    */
    double getPressedDepth() const;

    /**
    * Returns the delta of the previous and current position;
    */
    dvec2 getDeltaPosition() const;
    /**
    * Returns the delta of the previous and current depth;
    */
    double getDeltaDepth() const;

    /**
    * Returns the delta of the press position and current position;
    */
    dvec2 getDeltaPressedPosition() const;
    
    /**
    * Returns the delta of the press depth and current depth;
    */
    double getDeltaPressedDepth() const;

    /**
     * Returns the normalized device coordinates. Position and depth normalized to the range of
     * (-1,1) In in a left handed coordinate system.  The lower left near will be (-1,-1,-1)
     * And the upper right far (1,1,1)
     */
    dvec3 getNDC() const;
    dvec3 getPreviousNDC() const;
    dvec3 getPressedNDC() const;

    /**
     *	The size of the canvas where the event occurred.
     */
    uvec2 getCanvasSize() const;

    PickingState getState() const;



    void invoke(Processor* p);
    const PickingAction* getPickingAction() const;

    virtual uint64_t hash() const override;
    static constexpr uint64_t chash() {
        return util::constexpr_hash("org.inviwo.PickingEvent");
    }

    Event* getEvent() const;

    template <typename EventType>
    EventType* getEventAs() const;

private:
    using EventPtr = std::unique_ptr<Event,std::function<void(Event*)>>;

    const PickingAction* pickingAction_;
    PickingState state_ = PickingState::None;
    EventPtr event_;
    bool ownsEvent_ = false;
    
    dvec3 pressedNDC_ = dvec3(0.0);
    dvec3 previousNDC_ = dvec3(0.0);
    size_t pickedId_ = 0;
};

template <typename EventType>
EventType* PickingEvent::getEventAs() const {
    if (event_ && event_->hash() == EventType::chash()) {
        return static_cast<EventType*>(event_.get());
    }
    return nullptr;
}

} // namespace

#endif // IVW_PICKINGEVENT_H

