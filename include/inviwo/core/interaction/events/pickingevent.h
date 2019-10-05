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

#ifndef IVW_PICKINGEVENT_H
#define IVW_PICKINGEVENT_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/interaction/events/event.h>
#include <inviwo/core/interaction/events/interactionevent.h>
#include <inviwo/core/interaction/pickingstate.h>
#include <inviwo/core/util/constexprhash.h>

namespace inviwo {

class PickingAction;
class Camera;

/**
 * \class PickingEvent
 */
class IVW_CORE_API PickingEvent : public Event {
public:
    PickingEvent(const PickingAction* pickingAction, InteractionEvent* event, PickingState state,
                 PickingPressState pressState, PickingPressItem pressItem,
                 PickingHoverState hoverState, PickingPressItems pressedState,
                 size_t pickedGlobalId, size_t currentGlobalId, size_t pressedGlobalId,
                 size_t previousGlobalId, dvec3 pressedNDC, dvec3 previousNDC);
    PickingEvent(const PickingAction* pickingAction, std::unique_ptr<InteractionEvent> event,
                 PickingState state, PickingPressState pressState, PickingPressItem pressItem,
                 PickingHoverState hoverState, PickingPressItems pressedState,
                 size_t pickedGlobalId, size_t currentGlobalId, size_t pressedGlobalId,
                 size_t previousGlobalId, dvec3 pressedNDC, dvec3 previousNDC);

    virtual ~PickingEvent();
    PickingEvent(const PickingEvent&);
    PickingEvent& operator=(const PickingEvent&);

    virtual PickingEvent* clone() const override;

    /**
     * Returns the local picking index of the object currently being picked.
     * this id does not change while an item is being pressed even if the position is no longer
     * on the item.
     */
    size_t getPickedId() const;
    /**
     * Returns the global picking index of the object currently being picked.
     * this id does not change while an item is being pressed even if the position is no longer
     * on the item.
     */
    size_t getGlobalPickingId() const;

    /**
     * Returns the current global picking index of the object at the current position
     */
    size_t getCurrentGlobalPickingId() const;
    /**
     * Returns the current local picking index
     * If the global index belongs to a different picking action {false, 0} is returned.
     */
    std::pair<bool, size_t> getCurrentLocalPickingId() const;
    /**
     * Returns the current normalized position
     */
    dvec2 getPosition() const;
    /**
     * Returns the current normalized depth
     */
    double getDepth() const;

    /**
     * Returns the previous global picking index
     */
    size_t getPreviousGlobalPickingId() const;
    /**
     * Returns the previous local picking index
     * If the global index belongs to a different picking action {false, 0} is returned.
     */
    std::pair<bool, size_t> getPreviousLocalPickingId() const;
    /**
     * Returns the previous normalized position
     */
    dvec2 getPreviousPosition() const;
    /**
     * Returns the previous normalized depth
     */
    double getPreviousDepth() const;

    /**
     * Returns the pressed global picking index
     */
    size_t getPressedGlobalPickingId() const;
    /**
     * Returns the pressed local picking index
     * If the global index belongs to a different picking action {false, 0} is returned.
     */
    std::pair<bool, size_t> getPressedLocalPickingId() const;
    /**
     * Returns the normalized position of the initial press
     */
    dvec2 getPressedPosition() const;
    /**
     * Returns the normalized depth of the initial press
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
     * Return the {curr.x, curr.y. press.z} - {prev.x, prev.y, press.z} transformed into world space
     * using the given camera. This is useful when dragging an object in the screen plane.
     */
    dvec3 getWorldSpaceDeltaAtPressDepth(const Camera& camera) const;

    /**
     *	The size of the canvas where the event occurred.
     */
    uvec2 getCanvasSize() const;

    PickingState getState() const;
    PickingPressState getPressState() const;
    PickingPressItem getPressItem() const;
    PickingHoverState getHoverState() const;
    PickingPressItems getPressItems() const;

    KeyModifiers modifiers() const;

    void invoke(Processor* p);
    const PickingAction* getPickingAction() const;

    virtual uint64_t hash() const override;
    static constexpr uint64_t chash() { return util::constexpr_hash("org.inviwo.PickingEvent"); }

    InteractionEvent* getEvent() const;

    template <typename EventType>
    EventType* getEventAs() const;

    /**
     * Display a tool tip using the optionally set tool tip callback.
     * If no tool tip callback is set, the function does nothing.
     * The supported formation depends on the used back end, but simple html is usually supported.
     * Calling the function with an empty sting will hide any existing tool tip.
     */
    void setToolTip(const std::string& tooltip) const;

    const InteractionEvent::ToolTipCallback& getToolTipCallback() const;

    virtual void print(std::ostream& ss) const override;

private:
    const PickingAction* pickingAction_;

    std::unique_ptr<InteractionEvent> owner_ = nullptr;
    InteractionEvent* event_ = nullptr;

    PickingState state_ = PickingState::None;
    PickingPressState pressState_ = PickingPressState::None;
    PickingPressItem pressItem_ = PickingPressItem::None;
    PickingHoverState hoverState_ = PickingHoverState::None;
    PickingPressItems pressedState_ = PickingPressItem::None;

    size_t pickedGlobalId_ = 0;
    size_t currentGlobalId_ = 0;
    size_t pressedGlobalId_ = 0;
    size_t previousGlobalId_ = 0;

    dvec3 pressedNDC_ = dvec3(0.0);
    dvec3 previousNDC_ = dvec3(0.0);
};

template <typename EventType>
EventType* PickingEvent::getEventAs() const {
    if (event_ && event_->hash() == EventType::chash()) {
        return static_cast<EventType*>(event_);
    }
    return nullptr;
}

}  // namespace inviwo

#endif  // IVW_PICKINGEVENT_H
