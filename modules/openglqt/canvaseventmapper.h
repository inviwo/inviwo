/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015 Inviwo Foundation
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

#ifndef IVW_CANVASEVENTMAPPER_H
#define IVW_CANVASEVENTMAPPER_H

#include <modules/openglqt/openglqtmoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <modules/opengl/canvasgl.h>

#include <inviwo/qt/widgets/inviwoqtutils.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QInputEvent>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QEvent>
#include <QGestureEvent>
#include <QPanGesture>
#include <QPinchGesture>
#include <warn/pop>


class MouseEvent;
class KeyboardEvent;

namespace inviwo {

/**
 * \class CanvasEventMapper
 * \brief Maps qt events into Inviwo events.
 */
class IVW_MODULE_OPENGLQT_API CanvasEventMapper { 
public:
    CanvasEventMapper();
    virtual ~CanvasEventMapper() = default;

    int getLastNumFingers() const;

    template <typename F>
    bool mapMousePressEvent(QMouseEvent* e, CanvasGL* canvas, F fun);
    
    template <typename F>
    bool mapMouseReleaseEvent(QMouseEvent* e, CanvasGL* canvas, F fun);
    
    template <typename F>
    bool mapMouseMoveEvent(QMouseEvent* e, CanvasGL* canvas, F fun);
    
    template <typename F>
    bool mapWheelEvent(QWheelEvent* e, CanvasGL* canvas, F fun);
    
    template <typename F>
    bool mapKeyPressEvent(QKeyEvent* keyEvent, F fun);
    template <typename F>
    bool mapKeyReleaseEvent(QKeyEvent* keyEvent, F fun);
    
    template <typename F>
    bool mapTouchEvent(QTouchEvent*, CanvasGL* canavs, F fun);
    
    template <typename F>
    bool mapGestureEvent(QGestureEvent*, CanvasGL* canavs, F fun);
private:

    template <typename F>
    bool panTriggered(QPanGesture*, CanvasGL* canavs, F fun);
    template <typename F>
    bool pinchTriggered(QPinchGesture*, CanvasGL* canavs, F fun);

    bool gestureMode_;
    Qt::GestureType lastType_;
    int lastNumFingers_;
    std::vector<int> lastTouchIds_;
    vec2 screenPositionNormalized_;
};

template <typename F>
bool CanvasEventMapper::mapMousePressEvent(QMouseEvent* e, CanvasGL* canvas, F fun) {
    if (gestureMode_) return true;

    const ivec2 screenPos{utilqt::toGLM(e->pos())};
    const ivec2 screenPosInvY{util::invertY(screenPos, canvas->getScreenDimensions())};

    MouseEvent mouseEvent(screenPos, EventConverterQt::getMouseButton(e),
                          MouseEvent::MOUSE_STATE_PRESS, EventConverterQt::getModifier(e),
                          canvas->getScreenDimensions(),
                          canvas->getDepthValueAtCoord(screenPosInvY));
    e->accept();
    return fun(&mouseEvent);
}

template <typename F>
bool CanvasEventMapper::mapMouseReleaseEvent(QMouseEvent* e, CanvasGL* canvas, F fun) {
    if (gestureMode_) {
        gestureMode_ = false;
        return true;
    }

    const ivec2 screenPos{ utilqt::toGLM(e->pos()) };
    const ivec2 screenPosInvY{ util::invertY(screenPos, canvas->getScreenDimensions()) };
    
    MouseEvent mouseEvent(screenPos, EventConverterQt::getMouseButtonCausingEvent(e),
                          MouseEvent::MOUSE_STATE_RELEASE, EventConverterQt::getModifier(e),
                          canvas->getScreenDimensions(), canvas->getDepthValueAtCoord(screenPosInvY));
    e->accept();
    return fun(&mouseEvent);
}

template <typename F>
bool CanvasEventMapper::mapMouseMoveEvent(QMouseEvent* e, CanvasGL* canvas, F fun) {
    if (gestureMode_) return true;

    const ivec2 screenPos{ utilqt::toGLM(e->pos()) };
    const ivec2 screenPosInvY{ util::invertY(screenPos, canvas->getScreenDimensions()) };

    // Optimization, do not sample depth value when hovering, 
    // i.e. move without holding a mouse button
    int button = EventConverterQt::getMouseButton(e);
    double depth = 1.0;
    if (button != MouseEvent::MOUSE_BUTTON_NONE) {
        depth = canvas->getDepthValueAtCoord(screenPosInvY);
    }

    MouseEvent mouseEvent(screenPos, button, MouseEvent::MOUSE_STATE_MOVE,
                          EventConverterQt::getModifier(e), canvas->getScreenDimensions(), depth);
    e->accept();
    return fun(&mouseEvent);
}

template <typename F>
bool CanvasEventMapper::mapWheelEvent(QWheelEvent* e, CanvasGL* canvas, F fun) {
    MouseEvent::MouseWheelOrientation orientation;
    if (e->orientation() == Qt::Horizontal) {
        orientation = MouseEvent::MOUSE_WHEEL_HORIZONTAL;
    } else {
        orientation = MouseEvent::MOUSE_WHEEL_VERTICAL;
    }

    QPoint numPixels = e->pixelDelta();
    QPoint numDegrees = e->angleDelta() / 8 / 15;

    int numSteps = 0;
    if (!numPixels.isNull()) {
        numSteps =
            (orientation == MouseEvent::MOUSE_WHEEL_HORIZONTAL ? numPixels.x() : numPixels.y()) / 5;
    } else if (!numDegrees.isNull()) {
        numSteps =
            (orientation == MouseEvent::MOUSE_WHEEL_HORIZONTAL ? numDegrees.x() : numDegrees.y());
    }
    
    const ivec2 screenPos{ utilqt::toGLM(e->pos()) };
    const ivec2 screenPosInvY{ util::invertY(screenPos, canvas->getScreenDimensions()) };

    MouseEvent mouseEvent(screenPos, numSteps, EventConverterQt::getMouseWheelButton(e),
                          MouseEvent::MOUSE_STATE_WHEEL, orientation,
                          EventConverterQt::getModifier(e), canvas->getScreenDimensions(),
                          canvas->getDepthValueAtCoord(screenPosInvY));
    e->accept();
    return fun(&mouseEvent);
}

template <typename F>
bool CanvasEventMapper::mapKeyPressEvent(QKeyEvent* keyEvent, F fun) {
    KeyboardEvent pressKeyEvent(EventConverterQt::getKeyButton(keyEvent),
                                EventConverterQt::getModifier(keyEvent),
                                KeyboardEvent::KEY_STATE_PRESS);
    return fun(&pressKeyEvent);
}

template <typename F>
bool CanvasEventMapper::mapKeyReleaseEvent(QKeyEvent* keyEvent, F fun) {
    KeyboardEvent releaseKeyEvent(EventConverterQt::getKeyButton(keyEvent),
                                  EventConverterQt::getModifier(keyEvent),
                                  KeyboardEvent::KEY_STATE_RELEASE);
    return fun(&releaseKeyEvent);
}



template <typename F>
bool CanvasEventMapper::mapTouchEvent(QTouchEvent* touch, CanvasGL* canvas, F fun) {
    size_t nTouchPoints = touch->touchPoints().size();
    if (nTouchPoints < 1) return true;

    QTouchEvent::TouchPoint firstPoint = touch->touchPoints()[0];
    switch (firstPoint.state()) {
        case Qt::TouchPointPressed:
            gestureMode_ = nTouchPoints > 1;  // Treat single touch point as mouse event
            break;
        case Qt::TouchPointMoved:
            gestureMode_ = nTouchPoints > 1;  // Treat single touch point as mouse event
            break;
        case Qt::TouchPointStationary:
            gestureMode_ = nTouchPoints > 1;  // Treat single touch point as mouse event
            break;
        case Qt::TouchPointReleased:
            gestureMode_ = false;
            break;
        default:
            gestureMode_ = false;
    }
    // Copy touch points
    std::vector<TouchPoint> touchPoints;
    touchPoints.reserve(touch->touchPoints().size());
    // Fetch layer before loop (optimization)
    const LayerRAM* depthLayerRAM = canvas->getDepthLayerRAM();
    vec2 screenSize(canvas->getScreenDimensions());

    std::vector<int> endedTouchIds;

    for (auto& touchPoint : touch->touchPoints()) {
        const vec2 screenTouchPos{utilqt::toGLM(touchPoint.pos())};
        const vec2 prevScreenTouchPos{utilqt::toGLM(touchPoint.lastPos())};
        const vec2 pixelCoord{util::invertY(screenTouchPos, screenSize)};

        TouchPoint::TouchState touchState;
        switch (touchPoint.state()) {
            case Qt::TouchPointPressed:
                touchState = TouchPoint::TOUCH_STATE_STARTED;
                break;
            case Qt::TouchPointMoved:
                touchState = TouchPoint::TOUCH_STATE_UPDATED;
                break;
            case Qt::TouchPointStationary:
                touchState = TouchPoint::TOUCH_STATE_STATIONARY;
                break;
            case Qt::TouchPointReleased:
                touchState = TouchPoint::TOUCH_STATE_ENDED;
                break;
            default:
                touchState = TouchPoint::TOUCH_STATE_NONE;
        }

        // Note that screenTouchPos/prevScreenTouchPos are in [0 screenDim] and does not need to be
        // adjusted to become centered in the pixel (+0.5)

        // Saving id order to preserve order of touch points at next touch event

        const auto lastIdIdx = util::find(lastTouchIds_, touchPoint.id());

        if (lastIdIdx != lastTouchIds_.end()) {
            if (touchState == TouchPoint::TOUCH_STATE_ENDED) {
                endedTouchIds.push_back(touchPoint.id());
            }
        } else {
            lastTouchIds_.push_back(touchPoint.id());
        }
        touchPoints.emplace_back(touchPoint.id(), screenTouchPos, screenTouchPos / screenSize,
                                 prevScreenTouchPos, prevScreenTouchPos / screenSize, touchState,
                                 canvas->getDepthValueAtCoord(pixelCoord, depthLayerRAM));
    }
    // Ensure that the order to the touch points are the same as last touch event.
    // Note that the ID of a touch point is always the same but the order in which
    // they are given can vary.
    // Example
    // lastTouchIds_    touchPoints
    //     0                 0
    //     3                 1 
    //     2                 2
    //     4
    // Will result in:
    //                  touchPoints
    //                       0 (no swap)
    //                       2 (2 will swap with 1)
    //                       1

    auto touchIndex = 0;  // Index to first unsorted element in touchPoints array
    for (const auto& lastTouchPointId : lastTouchIds_) {
        const auto touchPointIt = util::find_if(
            touchPoints,
            [lastTouchPointId](const TouchPoint& p) { return p.getId() == lastTouchPointId; });
        // Swap current location in the container with the location it was in last touch event.
        if (touchPointIt != touchPoints.end() &&
            std::distance(touchPoints.begin(), touchPointIt) != touchIndex) {
            std::swap(*(touchPoints.begin() + touchIndex), *touchPointIt);
            ++touchIndex;
        }
    }

    util::erase_remove_if(lastTouchIds_, [&](int e) {
        return util::contains(endedTouchIds, e);
    });

    // We need to send out touch event all the time to support one -> two finger touch switch
    TouchEvent touchEvent(touchPoints, canvas->getScreenDimensions());
    touch->accept();

    lastNumFingers_ = static_cast<int>(touch->touchPoints().size());
    screenPositionNormalized_ = touchEvent.getCenterPointNormalized();

    return fun(&touchEvent);
}

template <typename F>
bool CanvasEventMapper::mapGestureEvent(QGestureEvent* ge, CanvasGL* canvas, F fun) {
    QGesture* gesture = nullptr;
    QPanGesture* panGesture = nullptr;
    QPinchGesture* pinchGesture = nullptr;

    if ((gesture = ge->gesture(Qt::PanGesture))) {
        panGesture = static_cast<QPanGesture*>(gesture);
    }
    if ((gesture = ge->gesture(Qt::PinchGesture))) {
        pinchGesture = static_cast<QPinchGesture*>(gesture);
    }
    ge->accept();

    if (panGesture && pinchGesture) {
        double absDeltaDist = glm::abs(static_cast<double>(pinchGesture->scaleFactor()) - 1.0);
        if (absDeltaDist > 0.05 || (lastType_ == Qt::PinchGesture || lastType_ != Qt::PanGesture)) {
            lastType_ = Qt::PinchGesture;
            return pinchTriggered(pinchGesture, canvas, fun);
        } else {
            lastType_ = Qt::PanGesture;
            return panTriggered(panGesture, canvas, fun);
        }
    } else if (panGesture) {
        lastType_ = Qt::PanGesture;
        return panTriggered(panGesture, canvas, fun);
    } else if (pinchGesture) {
        double absDeltaDist = glm::abs(static_cast<double>(pinchGesture->scaleFactor()) - 1.0);
        if (absDeltaDist > 0.05) lastType_ = Qt::PinchGesture;
        return pinchTriggered(pinchGesture, canvas, fun);
    }
}

#include <warn/push>
#include <warn/ignore/switch-enum>

template <typename F>
bool CanvasEventMapper::panTriggered(QPanGesture* gesture, CanvasGL* canvas, F fun) {
    // Mouse events will be triggered for touch events by Qt 5.3.1 (even though we specify that the
    // touch event is handled)
    // http://www.qtcentre.org/archive/index.php/t-52367.html
    // Therefore keep track of if we are performing and disallow mouse events while performing a
    // gesture ( see CanvasQt::event(QEvent *e) )
    switch (gesture->state()) {
        case Qt::GestureStarted:
        case Qt::GestureUpdated:
            gestureMode_ = true;
            break;
        default:
            gestureMode_ = false;
    }
    vec2 deltaPos =
        vec2((gesture->lastOffset().x() - gesture->offset().x()) / canvas->getScreenDimensions().x,
             (gesture->offset().y() - gesture->lastOffset().y()) / canvas->getScreenDimensions().y);

    if (deltaPos == vec2(0.f)) return true;

    GestureEvent ge(deltaPos, 0.0, GestureEvent::PAN, EventConverterQt::getGestureState(gesture),
                    lastNumFingers_, screenPositionNormalized_, canvas->getScreenDimensions());

    return fun(&ge);
}

template <typename F>
bool CanvasEventMapper::pinchTriggered(QPinchGesture* gesture, CanvasGL* canvas, F fun) {
    // std::cout << "PINCH: " << gesture->scaleFactor() << std::endl;
    // Mouse events will be triggered for touch events by Qt 5.3.1 (even though we specify that the
    // touch event is handled)
    // http://www.qtcentre.org/archive/index.php/t-52367.html
    // Therefore keep track of if we are performing and disallow mouse events while performing a
    // gesture ( see CanvasQt::event(QEvent *e) )
    switch (gesture->state()) {
        case Qt::GestureStarted:
        case Qt::GestureUpdated:
            gestureMode_ = true;
            break;
        default:
            gestureMode_ = false;
    }
    GestureEvent ge(vec2(gesture->centerPoint().x(), gesture->centerPoint().y()),
                    static_cast<double>(gesture->scaleFactor()) - 1.0, GestureEvent::PINCH,
                    EventConverterQt::getGestureState(gesture), lastNumFingers_,
                    screenPositionNormalized_, canvas->getScreenDimensions());

    return fun(&ge);
}

#include <warn/pop>

} // namespace

#endif // IVW_CANVASEVENTMAPPER_H

