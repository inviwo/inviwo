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
#include <inviwo/core/datastructures/image/image.h>
#include <inviwo/core/datastructures/geometry/mesh.h>
#include <inviwo/core/datastructures/buffer/bufferramprecision.h>
#include <inviwo/core/processors/canvasprocessorwidget.h>
#include <inviwo/core/network/processornetworkevaluator.h>
#include <inviwo/core/io/datawriterfactory.h>
#include <inviwo/core/interaction/events/eventpropagator.h>
#include <inviwo/core/interaction/events/resizeevent.h>
#include <inviwo/core/network/networklock.h>

namespace inviwo {

EventHandler* eventHandler_();

Mesh* Canvas::screenAlignedRect_ = nullptr;
DataWriterType<Layer>* Canvas::generalLayerWriter_ = nullptr;

Canvas::Canvas(uvec2 dimensions)
    : initialized_(false)
    , shared_(true)
    , screenDimensions_(dimensions)
    , propagator_(nullptr)
    , pickingContainer_()
    , ownerWidget_(nullptr) {
    if (!screenAlignedRect_) {
        shared_ = false;

        auto verticesBuffer =
            util::makeBuffer<vec2>({{-1.0f, -1.0f}, {1.0f, -1.0f}, {-1.0f, 1.0f}, {1.0f, 1.0f}});

        auto texCoordsBuffer =
            util::makeBuffer<vec2>({{0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 1.0f}, {1.0f, 1.0f}});

        auto indices_ = util::makeIndexBuffer({0, 1, 2, 3});

        Mesh* screenAlignedRectMesh = new Mesh();
        screenAlignedRectMesh->addBuffer(BufferType::POSITION_ATTRIB, verticesBuffer);
        screenAlignedRectMesh->addBuffer(BufferType::TEXCOORD_ATTRIB, texCoordsBuffer);
        screenAlignedRectMesh->addIndicies(
            Mesh::MeshInfo(DrawType::TRIANGLES, ConnectivityType::STRIP), indices_);

        screenAlignedRect_ = screenAlignedRectMesh;
    }

    if (!generalLayerWriter_) {
        generalLayerWriter_ =
            DataWriterFactory::getPtr()->getWriterForTypeAndExtension<Layer>("png").release();
    }
}

Canvas::~Canvas() {
    if (!shared_) {
        delete screenAlignedRect_;
        screenAlignedRect_ = nullptr;

        delete generalLayerWriter_;
        generalLayerWriter_ = nullptr;
    }

    if (this == RenderContext::getPtr()->getDefaultRenderContext()) {
        RenderContext::getPtr()->setDefaultRenderContext(nullptr);
    }
}

void Canvas::initialize() {
    initialized_ = true;
    propagator_ = nullptr;
}

void Canvas::deinitialize() { propagator_ = nullptr; }

void Canvas::render(const Image* im, LayerType layerType, size_t idx) {}

void Canvas::activate() {}

void Canvas::resize(uvec2 canvasSize) {
    uvec2 previousScreenDimensions_ = screenDimensions_;
    screenDimensions_ = canvasSize;

    if (propagator_) {
        NetworkLock lock;
        RenderContext::getPtr()->activateDefaultRenderContext();
        ResizeEvent* resizeEvent = new ResizeEvent(screenDimensions_);
        resizeEvent->setPreviousSize(previousScreenDimensions_);
        propagator_->propagateResizeEvent(resizeEvent, nullptr);
        delete resizeEvent;
    }
}

uvec2 Canvas::getScreenDimensions() const { return screenDimensions_; }

void Canvas::update() {}

bool Canvas::isInitialized() { return initialized_; }

void Canvas::activateDefaultRenderContext() {
    RenderContext::getPtr()->activateDefaultRenderContext();
}

void Canvas::interactionEvent(Event* event) {
    if (propagator_) {
        NetworkLock lock;
        propagator_->propagateEvent(event);
    }
}

void Canvas::mousePressEvent(MouseEvent* e) { mouseButtonEvent(e); }

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

void Canvas::setEventPropagator(EventPropagator* propagator) { propagator_ = propagator; }

ProcessorWidget* Canvas::getProcessorWidgetOwner() const { return ownerWidget_; }

void Canvas::setProcessorWidgetOwner(ProcessorWidget* ownerWidget) { ownerWidget_ = ownerWidget; }

}  // namespace
