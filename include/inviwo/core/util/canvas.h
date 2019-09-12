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

#ifndef IVW_CANVAS_H
#define IVW_CANVAS_H

#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/datastructures/image/imagetypes.h>
#include <inviwo/core/interaction/pickingcontroller.h>

namespace inviwo {

class ProcessorNetworkEvaluator;
template <class Layer>
class DataWriterType;
class Image;
class EventPropagator;
class ProcessorWidget;
class MouseEvent;
class WheelEvent;
class KeyboardEvent;
class TouchEvent;
class GestureEvent;

class IVW_CORE_API Canvas {
public:
    Canvas(size2_t dimensions);
    virtual ~Canvas() = default;

    virtual void render(std::shared_ptr<const Image>, LayerType layerType = LayerType::Color,
                        size_t idx = 0) = 0;
    virtual void resize(size2_t canvasSize);

    size2_t getCanvasDimensions() const;
    virtual size2_t getImageDimensions() const = 0;

    virtual void update() = 0;
    virtual void activate() = 0;

    void setEventPropagator(EventPropagator* propagator);
    virtual ProcessorWidget* getProcessorWidgetOwner() const;
    virtual void setProcessorWidgetOwner(ProcessorWidget*);

    // used to create hidden canvases used for context in background threads.
    virtual std::unique_ptr<Canvas> createHiddenCanvas() = 0;
    using ContextID = const void*;
    virtual ContextID activeContext() const = 0;
    virtual ContextID contextId() const = 0;

    virtual void releaseContext() = 0;

    bool isFullScreen() const;
    /**
     * Sets window state of canvas to cover the entire screen.
     * Will call setFullScreenInternal after setting full screen flag.
     * @see setFullScreenInternal
     */
    void setFullScreen(bool fullscreen);
    void propagateEvent(Event* e);

protected:
    /**
     * Derived classes should override to implement actual window state.
     */
    virtual void setFullScreenInternal(bool fullscreen) = 0;

    size2_t screenDimensions_;
    EventPropagator* propagator_;  //< non-owning reference
    PickingController pickingController_;
    ProcessorWidget* ownerWidget_;  //< non-owning reference
    bool isFullScreen_ = false;
};

}  // namespace inviwo

#endif  // IVW_CANVAS_H
