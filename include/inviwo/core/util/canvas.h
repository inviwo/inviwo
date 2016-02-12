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

#ifndef IVW_CANVAS_H
#define IVW_CANVAS_H

#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/datastructures/image/imagetypes.h>
#include <inviwo/core/interaction/pickingcontainer.h>

namespace inviwo {

class ProcessorNetworkEvaluator;
template <class Layer>
class DataWriterType;
class Image;
class EventPropagator;
class ProcessorWidget;
class MouseEvent;
class KeyboardEvent;
class TouchEvent;
class GestureEvent;

class IVW_CORE_API Canvas {
public:
    Canvas(uvec2 dimensions);
    virtual ~Canvas();

    virtual void render(std::shared_ptr<const Image>, LayerType layerType = LayerType::Color,
                        size_t idx = 0) = 0;
    virtual void resize(uvec2 canvasSize);

    uvec2 getScreenDimensions() const;

    virtual void update() = 0;
    virtual void activate() = 0;

    void setEventPropagator(EventPropagator* propagator);
    virtual ProcessorWidget* getProcessorWidgetOwner() const;
    virtual void setProcessorWidgetOwner(ProcessorWidget*);

    // used to create hidden canvases used for context in background threads.
    virtual std::unique_ptr<Canvas> createHiddenCanvas() = 0;

protected:
    void activateDefaultRenderContext();

    void interactionEvent(Event* e);

    void mousePressEvent(MouseEvent* e);
    void mouseDoubleClickEvent(MouseEvent *e);
    void mouseReleaseEvent(MouseEvent* e);
    void mouseMoveEvent(MouseEvent* e);

    void mouseButtonEvent(MouseEvent* e);
    void mouseWheelEvent(MouseEvent* e);

    void keyPressEvent(KeyboardEvent* e);
    void keyReleaseEvent(KeyboardEvent* e);

    void gestureEvent(GestureEvent* e);
    void touchEvent(TouchEvent* e);
    
    bool touchEnabled();

    uvec2 screenDimensions_;
    EventPropagator* propagator_;  //< non-owning reference
    PickingContainer pickingContainer_;
    ProcessorWidget* ownerWidget_;  //< non-owning reference
};

}  // namespace

#endif  // IVW_CANVAS_H
