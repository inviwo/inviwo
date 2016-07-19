/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2016 Inviwo Foundation
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
#include <inviwo/core/interaction/events/event.h>
#include <inviwo/core/interaction/pickingstate.h>

namespace inviwo {

/** \class PickingObject
 */
class IVW_CORE_API PickingObject {
public:
    friend class PickingManager;

    using Action = std::function<void(const PickingObject*)>;

    /**
    * Create a picking object for "size" objects starting a "start"
    * PickingObjects are usually created by the Picking manager
    */
    PickingObject(size_t start, size_t size = 1);

    virtual ~PickingObject();

    /**
    * Returns the global picking index, the global index can be used with the 
    * PickingManager::indexToColor(size_t index) function to get a picking color.
    * \param id the local picking index 
    */
    size_t getPickingId(size_t id = 0) const;

    /**
     *	The picking color to use for the object with local index id.
     *  This is eqvivalent to PickingManager::indexToColor(getPickingId(id))/255.0
     * \param id the local picking index 
     */
    vec3 getColor(size_t id = 0) const;
    /**
     *	The number of local picking indices in this picking object.
     */
    size_t getSize() const;

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
    dvec2 getPressPosition() const;
    /**
    * Returns the normalized depth of the most resent press
    */
    double getPressDepth() const;

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
    dvec2 getDeltaPressPosition() const;
    
    /**
    * Returns the delta of the press depth and current depth;
    */
    double getDeltaPressDepth() const;

    /**
     * Returns the normalized device coordinates. Position and depth normalized to the range of
     * (-1,1) In in a left handed coordinate system.  The lower left near will be (-1,-1,-1)
     * And the upper right far (1,1,1)
     */
    dvec3 getNDC() const;
    dvec3 getPreviousNDC() const;
    dvec3 getPressNDC() const;

    PickingState getState() const;

    void picked(Event* event, PickingState state);

    Event* getEvent() const;

    template <typename EventType>
    EventType* getEventAs() const;

    void setAction(Action action);
    Action getAction() const;

    bool isEnabled() const;
    void setEnabled(bool enabled);

private:
    size_t getCapacity() const;
    void setSize(size_t size);
    void setPickedId(size_t id);

    size_t start_;
    size_t size_;
    size_t capacity_;

    size_t pickedId_ = 0;
    PickingState state_ = PickingState::None;

    Event* event_ = nullptr;
    
    dvec3 pressNDC_ = dvec3(0.0);
    dvec2 pressPosition_ = vec2(0.0);

    dvec3 previousNDC_ = dvec3(0.0);
    dvec2 previousPosition_ = dvec2(0.0);

    Action action_;
    bool enabled_ = true;
};

template <typename EventType>
EventType* PickingObject::getEventAs() const {
    if (event_ && event_->hash() == EventType::chash()) {
        return static_cast<EventType*>(event_);
    }
    return nullptr;
}

} // namespace

#endif // IVW_PICKINGOBJECT_H