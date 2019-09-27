/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2019 Inviwo Foundation
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
#include <inviwo/core/interaction/events/wheelevent.h>
#include <inviwo/core/interaction/events/touchevent.h>
#include <inviwo/core/network/networklock.h>
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/util/settings/systemsettings.h>

namespace inviwo {

Canvas::Canvas(size2_t dimensions)
    : screenDimensions_(dimensions)
    , propagator_(nullptr)
    , pickingController_()
    , ownerWidget_(nullptr) {}

void Canvas::resize(size2_t canvasSize) {
    auto previousScreenDimensions = screenDimensions_;
    screenDimensions_ = canvasSize;

    if (propagator_) {
        NetworkLock lock;
        RenderContext::getPtr()->activateDefaultRenderContext();
        ResizeEvent resizeEvent(screenDimensions_, previousScreenDimensions);
        propagator_->propagateEvent(&resizeEvent, nullptr);
    }
}

size2_t Canvas::getCanvasDimensions() const { return screenDimensions_; }

void Canvas::propagateEvent(Event* event) {
    NetworkLock lock;
    if (!propagator_) return;

    pickingController_.propagateEvent(event, propagator_);
    if (event->hasBeenUsed()) return;
    propagator_->propagateEvent(event, nullptr);
}

void Canvas::setEventPropagator(EventPropagator* propagator) { propagator_ = propagator; }

ProcessorWidget* Canvas::getProcessorWidgetOwner() const { return ownerWidget_; }

void Canvas::setProcessorWidgetOwner(ProcessorWidget* ownerWidget) { ownerWidget_ = ownerWidget; }

bool Canvas::isFullScreen() const { return isFullScreen_; }

void Canvas::setFullScreen(bool fullscreen) {
    isFullScreen_ = fullscreen;
    setFullScreenInternal(fullscreen);
}

}  // namespace inviwo
