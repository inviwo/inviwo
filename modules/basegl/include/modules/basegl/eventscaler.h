/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2021 Inviwo Foundation
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

#include <modules/basegl/baseglmoduledefine.h>
#include <inviwo/core/interaction/events/event.h>

namespace inviwo {

class MouseEvent;
class GestureEvent;
class TouchEvent;
class PickingEvent;
class WheelEvent;

/**
 * \brief Scales events going between ImageInport/ImageOutport with different image dimensions.
 *
 * Use case:
 * The ImageInport is scaled depending on the ImageOutport size causing a mismatch between the event
 * canvasSize and the Image size.
 *
 * In practice, a copy of the input event is created with a canvasSize corresponding to the
 * specified size. Coordinates are not scaled because they are already provided as normalized
 * coordinates.
 *
 * @see ImageScaling for a usage example.
 * @see ViewportManager if you want to divide the viewport instead of resizing.
 */
class IVW_MODULE_BASEGL_API EventScaler {
public:
    using Propagator = std::function<void(Event*)>;
    EventScaler(uvec2 inportImageSize = uvec2(0, 0));
    virtual ~EventScaler() = default;

    /**
     * \brief Maps event and propagates it using the supplied propagator.
     * return whether the event was propagated
     */
    bool propagateEvent(Event* event, Propagator propagator);

    void setSize(uvec2 inportImageSize) { size_ = inportImageSize; }

private:
    bool propagatePickingEvent(PickingEvent* pe, Propagator propagator);
    bool propagateMouseEvent(MouseEvent* me, Propagator propagator);
    bool propagateWheelEvent(WheelEvent* we, Propagator propagator);
    bool propagateGestureEvent(GestureEvent* ge, Propagator propagator);
    bool propagateTouchEvent(TouchEvent* te, Propagator propagator);

    uvec2 size_;
};

}  // namespace inviwo
