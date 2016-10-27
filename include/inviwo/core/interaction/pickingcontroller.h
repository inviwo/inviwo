/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016 Inviwo Foundation
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

namespace inviwo {

class Image;
class EventPropagator;
class Event;
class PickingAction;
class MouseInteractionEvent;
class TouchEvent;
class GestureEvent;

/**
 * \class PickingController
 */
class IVW_CORE_API PickingController { 
public:
    PickingController();
    virtual ~PickingController() = default;
    
    void handlePickingEvent(EventPropagator*, Event*);
   
    void setPickingSource(const std::shared_ptr<const Image>& src);
    bool pickingEnabled() const;
private:
    void handlePickingEvent(EventPropagator*, MouseInteractionEvent*);
    void handlePickingEvent(EventPropagator*, TouchEvent*);
    void handlePickingEvent(EventPropagator*, GestureEvent*);

    struct State {
        PickingManager::Result update(PickingController& pc, MouseInteractionEvent* e);

        PickingManager::Result previousPickingAction = {0, nullptr};
        dvec3 previousNDC = dvec3(0.0);
        size_t previousPickingId = 0;

        bool mousePressed = false;
        PickingManager::Result pressedPickingAction = {0, nullptr};
        dvec3 pressNDC = dvec3(0.0);
    };


    PickingManager::Result findPickingAction(const uvec2& coord);
    std::shared_ptr<const Image> src_;

    State state_;
};

} // namespace

#endif // IVW_PICKINGCONTROLLER_H

