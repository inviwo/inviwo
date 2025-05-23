/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2021-2025 Inviwo Foundation
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

#include <modules/openglqt/openglqtmoduledefine.h>  // for IVW_MODULE_OPENGLQT_API

#include <inviwo/core/interaction/events/touchevent.h>  // for TouchPoint
#include <inviwo/core/interaction/events/interactionevent.h>
#include <inviwo/core/interaction/contextmenuaction.h>
#include <inviwo/core/util/glmvec.h>  // for size2_t, dvec2, vec2

#include <functional>  // for function
#include <string>      // for string
#include <vector>      // for vector

#include <QObject>  // for QObject
#include <Qt>       // for CursorShape, GestureType

class QEvent;
class QGestureEvent;
class QHelpEvent;
class QKeyEvent;
class QMouseEvent;
class QPanGesture;
class QPinchGesture;
class QTouchEvent;
class QWheelEvent;

namespace inviwo {

class EventPropagator;
class MouseInteractionEvent;

/**
 * \brief Map Qt interaction events Mouse, Keyboard, Touch to the corresponing inviwo events
 */
class IVW_MODULE_OPENGLQT_API InteractionEventMapperQt : public QObject {
public:
    using ContextMenuCallback = InteractionEvent::ContextMenuCallback;

    InteractionEventMapperQt(QObject* parent, EventPropagator* propagator,
                             std::function<size2_t()> canvasDimensions,
                             std::function<size2_t()> imageDimensions,
                             std::function<double(dvec2)> depth, ContextMenuCallback contextMenu,
                             std::function<void(Qt::CursorShape)> cursorChange);
    virtual bool eventFilter(QObject* obj, QEvent* ev) override;

    void handleTouch(bool on);
    void handleGestures(bool on);

private:
    bool mapMousePressEvent(QMouseEvent* e);
    bool mapMouseDoubleClickEvent(QMouseEvent* e);
    bool mapMouseReleaseEvent(QMouseEvent* e);
    bool mapMouseMoveEvent(QMouseEvent* e);
    bool mapWheelEvent(QWheelEvent* e);
    bool mapKeyPressEvent(QKeyEvent* keyEvent);
    bool mapKeyReleaseEvent(QKeyEvent* keyEvent);
    bool mapTouchEvent(QTouchEvent* e);
    bool mapGestureEvent(QGestureEvent*);
    bool mapPanTriggered(QPanGesture*);
    bool mapPinchTriggered(QPinchGesture* e);

    bool showToolTip(QHelpEvent* e);

    void addCallbacksTo(MouseInteractionEvent* e);

    EventPropagator* propagator_;
    std::function<size2_t()> canvasDimensions_;
    std::function<size2_t()> imageDimensions_;
    std::function<double(dvec2)> depth_;
    ContextMenuCallback contextMenu_;
    std::function<void(Qt::CursorShape)> cursorChange_;
    bool blockContextMenu_ = false;

    //! Compare with next touch event to prevent duplicates
    std::vector<TouchPoint> prevTouchPoints_;

    std::string toolTipText_;

    // Hacks for gestures
    Qt::GestureType lastType_{};
    int lastNumFingers_{0};
    vec2 screenPositionNormalized_{0};

    bool handleTouch_{true};
    bool handleGestures_{true};
};

}  // namespace inviwo
