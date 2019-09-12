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

#ifndef IVW_PICKINGCONTROLLER_H
#define IVW_PICKINGCONTROLLER_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/interaction/pickingmanager.h>
#include <inviwo/core/interaction/pickingcontrollermousestate.h>

namespace inviwo {

class Image;
class EventPropagator;
class Event;
class PickingAction;
class MouseInteractionEvent;
class GestureEvent;
class TouchEvent;
class WheelEvent;

/**
 * \class PickingController
 * Handle mapping of interaction events into picking events and propagation of them
 * if the index of the color in the picking buffer of the src image if found by the PickingManager
 */
class IVW_CORE_API PickingController {
public:
    PickingController();
    ~PickingController();

    void propagateEvent(Event*, EventPropagator*);
    void setPickingSource(const std::shared_ptr<const Image>& src);
    bool pickingEnabled() const;

private:
    void propagateEvent(TouchEvent*, EventPropagator*);
    void propagateEvent(GestureEvent*, EventPropagator*);

    struct PCTouchState {
        std::unordered_map<int, PickingManager::Result> pointIdToPickingId;
        std::unordered_map<size_t, dvec3> pickingIdToPressNDC;
        std::unordered_map<size_t, dvec3> pickingIdToPreviousNDC;
    };

    PickingManager::Result findPickingAction(const uvec2& coord);
    std::shared_ptr<const Image> src_;

    PickingControllerMouseState mouseState_;

    PCTouchState tstate_;
};

}  // namespace inviwo

#endif  // IVW_PICKINGCONTROLLER_H
