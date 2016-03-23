/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2015 Inviwo Foundation
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

#include <inviwo/core/util/canvas.h>
#include <inviwo/core/util/rendercontext.h>
#include <inviwo/core/processors/canvasprocessorwidget.h>
#include <inviwo/core/interaction/events/eventpropagator.h>
#include <inviwo/core/interaction/events/resizeevent.h>
#include <inviwo/core/interaction/events/gestureevent.h>
#include <inviwo/core/interaction/events/keyboardevent.h>
#include <inviwo/core/interaction/events/mouseevent.h>
#include <inviwo/core/interaction/events/touchevent.h>
#include <inviwo/core/network/networklock.h>
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/util/settings/systemsettings.h>

namespace inviwo {

Canvas::Canvas(uvec2 dimensions)
    : screenDimensions_(dimensions)
    , propagator_(nullptr)
    , pickingContainer_()
    , ownerWidget_(nullptr) {}

void Canvas::resize(uvec2 canvasSize) {
    uvec2 previousScreenDimensions_ = screenDimensions_;
    screenDimensions_ = canvasSize;

    if (propagator_) {
        NetworkLock lock;
        RenderContext::getPtr()->activateDefaultRenderContext();
        ResizeEvent resizeEvent(screenDimensions_);
        resizeEvent.setPreviousSize(previousScreenDimensions_);
        propagator_->propagateResizeEvent(&resizeEvent, nullptr);
    }
}

uvec2 Canvas::getScreenDimensions() const { return screenDimensions_; }

void Canvas::interactionEvent(Event* event) {
    if (propagator_) {
        NetworkLock lock;
        propagator_->propagateEvent(event, nullptr);
    }
}

void Canvas::mousePressEvent(MouseEvent* e) { mouseButtonEvent(e); }

void Canvas::mouseDoubleClickEvent(MouseEvent* e) { mouseButtonEvent(e); }

void Canvas::mouseReleaseEvent(MouseEvent* e) { mouseButtonEvent(e); }

void Canvas::mouseMoveEvent(MouseEvent* e) { mouseButtonEvent(e); }

void Canvas::mouseButtonEvent(MouseEvent* e) {
    NetworkLock lock;
    if (!pickingContainer_.performMousePick(e)) interactionEvent(e);
}

void Canvas::mouseWheelEvent(MouseEvent* e) { interactionEvent(e); }

void Canvas::keyPressEvent(KeyboardEvent* e) { interactionEvent(e); }

void Canvas::keyReleaseEvent(KeyboardEvent* e) { interactionEvent(e); }

void Canvas::gestureEvent(GestureEvent* e) { interactionEvent(e); }

void Canvas::touchEvent(TouchEvent* e) {
    if (!touchEnabled()) return;

    NetworkLock lock;

    if (!pickingContainer_.performTouchPick(e)) {
        // One single touch point is already sent out as mouse event
        if (e->getTouchPoints().size() > 1) {
            interactionEvent(e);
        }
    } else if (e->hasTouchPoints()) {
        // As one touch point is handle as mouse event
        // Send out a mouse event if only one touch point remains
        const std::vector<TouchPoint>& touchPoints = e->getTouchPoints();
        if (touchPoints.size() == 1) {
            MouseEvent mouseEvent(touchPoints[0].getPos(), MouseEvent::MOUSE_BUTTON_LEFT,
                                  MouseEvent::MOUSE_STATE_MOVE, InteractionEvent::MODIFIER_NONE,
                                  e->canvasSize(), touchPoints[0].getDepth());
            interactionEvent(&mouseEvent);
        } else {
            interactionEvent(e);
        }
    }
}

bool Canvas::touchEnabled() {
    auto touchEnabledProperty =
        InviwoApplication::getPtr()->getSettingsByType<SystemSettings>()->enableTouchProperty_;
    return (touchEnabledProperty.get());
}

void Canvas::setEventPropagator(EventPropagator* propagator) { propagator_ = propagator; }

ProcessorWidget* Canvas::getProcessorWidgetOwner() const { return ownerWidget_; }

void Canvas::setProcessorWidgetOwner(ProcessorWidget* ownerWidget) { ownerWidget_ = ownerWidget; }

}  // namespace
