/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2015 Inviwo Foundation
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

#ifndef IVW_CANVASQT_H
#define IVW_CANVASQT_H

#include <modules/openglqt/openglqtmoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <modules/openglqt/hiddencanvasqt.h>
#include <inviwo/core/interaction/events/mouseevent.h>
#include <inviwo/core/interaction/events/keyboardevent.h>
#include <inviwo/core/interaction/events/touchevent.h>
#include <inviwo/core/interaction/events/gestureevent.h>
#include <inviwo/qt/widgets/eventconverterqt.h>
#include <inviwo/qt/widgets/inviwoqtutils.h>

#include <modules/opengl/canvasgl.h>
#include <modules/openglqt/canvasqglwidget.h>
//#include <modules/openglqt/canvasqwindow.h>
//#include <modules/openglqt/canvasqopenglwidget.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QEvent>
#include <QGestureEvent>
#include <QTouchEvent>
#include <QThread>
#include <warn/pop>

namespace inviwo {

template <typename T>
class CanvasQtBase : public T {
    friend class CanvasProcessorWidgetQt;

public:
    using QtBase = typename T::QtBase;

    explicit CanvasQtBase(uvec2 dim = uvec2(256,256));
    virtual ~CanvasQtBase() = default;

    virtual void render(std::shared_ptr<const Image> image, LayerType layerType = LayerType::Color,
                        size_t idx = 0) override;

    virtual std::unique_ptr<Canvas> createHiddenCanvas() override;

    static CanvasQtBase<T>* getSharedCanvas() {
        return static_cast<CanvasQtBase<T>*>(T::sharedCanvas_);
    }

protected:
    virtual bool event(QEvent *e) override;

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
    bool mapPanTriggered(QPanGesture* );
    bool mapPinchTriggered(QPinchGesture* e);
    
    void touchFallback(QTouchEvent*);
    
    bool gestureMode_ = false;
    Qt::GestureType lastType_;
    int lastNumFingers_;
    std::vector<int> lastTouchIds_;
    vec2 screenPositionNormalized_;
};

using CanvasQt = CanvasQtBase<CanvasQGLWidget>;
//using CanvasQt = CanvasQtBase<CanvasQWindow>;
//using CanvasQt = CanvasQtBase<CanvasQOpenGLWidget>;

template <typename T>
CanvasQtBase<T>::CanvasQtBase(uvec2 dim) : T(nullptr, dim) {}

template <typename T>
std::unique_ptr<Canvas> CanvasQtBase<T>::createHiddenCanvas() {
    auto thread = QThread::currentThread();
    // The context has to be created on the main thread.
    auto res = dispatchFront([&thread]() {
        auto canvas = util::make_unique<HiddenCanvasQt<CanvasQtBase<T>>>();       
        canvas->doneCurrent();
        canvas->context()->moveToThread(thread);
        return canvas;
    });
    return res.get();
}

template <typename T>
void CanvasQtBase<T>::render(std::shared_ptr<const Image> image, LayerType layerType, size_t idx) {
    if (this->isVisible() && this->isValid()) {
        CanvasGL::render(image, layerType, idx);
    }
}

#include <warn/push>
#include <warn/ignore/switch-enum>

template <typename T>
bool CanvasQtBase<T>::event(QEvent* e) {
    switch (e->type()) {
        case QEvent::KeyPress: {
            auto keyEvent = static_cast<QKeyEvent*>(e);
            auto parent = this->parentWidget();
            if (parent && keyEvent->key() == Qt::Key_F &&
                keyEvent->modifiers() == Qt::ShiftModifier) {
                if (parent->windowState() == Qt::WindowFullScreen) {
                    parent->showNormal();
                } else {
                    parent->showFullScreen();
                }
            }
            return mapKeyPressEvent(keyEvent);
        }
        case QEvent::KeyRelease: {
            return mapKeyReleaseEvent(static_cast<QKeyEvent*>(e));
        }
        case QEvent::MouseButtonPress:
            return mapMousePressEvent(static_cast<QMouseEvent*>(e));
        case QEvent::MouseButtonDblClick:
            return mapMouseDoubleClickEvent(static_cast<QMouseEvent*>(e));
        case QEvent::MouseButtonRelease:
            return mapMouseReleaseEvent(static_cast<QMouseEvent*>(e));
        case QEvent::MouseMove:
            return mapMouseMoveEvent(static_cast<QMouseEvent*>(e));

        case QEvent::Wheel:
            return mapWheelEvent(static_cast<QWheelEvent*>(e));

        case QEvent::TouchBegin:
        case QEvent::TouchEnd:
        case QEvent::TouchUpdate: {
            touchFallback(static_cast<QTouchEvent*>(e));
            return mapTouchEvent(static_cast<QTouchEvent*>(e));
        }
        case QEvent::Gesture:
            return mapGestureEvent(static_cast<QGestureEvent*>(e));
        default:
            return QGLWidget::event(e);
    }
}

template <typename T>
bool CanvasQtBase<T>::mapMousePressEvent(QMouseEvent* e) {
    if (gestureMode_) return true;

    const ivec2 screenPos{ utilqt::toGLM(e->pos()) };
    const ivec2 screenPosInvY{ util::invertY(screenPos, this->getScreenDimensions()) };

    MouseEvent mouseEvent(screenPos, EventConverterQt::getMouseButton(e),
        MouseEvent::MOUSE_STATE_PRESS, EventConverterQt::getModifier(e),
        this->getScreenDimensions(),
        this->getDepthValueAtCoord(screenPosInvY));

    e->accept();
    Canvas::mousePressEvent(&mouseEvent);
    return true;
}

template <typename T>
bool CanvasQtBase<T>::mapMouseDoubleClickEvent(QMouseEvent* e) {
    if (gestureMode_) return true;

    const ivec2 screenPos{ utilqt::toGLM(e->pos()) };
    const ivec2 screenPosInvY{ util::invertY(screenPos, this->getScreenDimensions()) };

    MouseEvent mouseEvent(screenPos, EventConverterQt::getMouseButton(e),
        MouseEvent::MOUSE_STATE_DOUBLE_CLICK, EventConverterQt::getModifier(e),
        this->getScreenDimensions(),
        this->getDepthValueAtCoord(screenPosInvY));

    e->accept();
    Canvas::mouseDoubleClickEvent(&mouseEvent);
    return true;
}

template <typename T>
bool CanvasQtBase<T>::mapMouseReleaseEvent(QMouseEvent* e) {
    if (gestureMode_) {
        gestureMode_ = false;
        return true;
    }

    const ivec2 screenPos{utilqt::toGLM(e->pos())};
    const ivec2 screenPosInvY{util::invertY(screenPos, this->getScreenDimensions())};

    MouseEvent mouseEvent(screenPos, EventConverterQt::getMouseButtonCausingEvent(e),
                          MouseEvent::MOUSE_STATE_RELEASE, EventConverterQt::getModifier(e),
                          this->getScreenDimensions(),
                          this->getDepthValueAtCoord(screenPosInvY));
    e->accept();
    Canvas::mouseReleaseEvent(&mouseEvent);
    return true;
}

template <typename T>
bool CanvasQtBase<T>::mapMouseMoveEvent(QMouseEvent* e) {
    if (gestureMode_) return true;

    const ivec2 screenPos{utilqt::toGLM(e->pos())};
    const ivec2 screenPosInvY{util::invertY(screenPos, this->getScreenDimensions())};

    // Optimization, do not sample depth value when hovering,
    // i.e. move without holding a mouse button
    int button = EventConverterQt::getMouseButton(e);
    double depth = 1.0;
    if (button != MouseEvent::MOUSE_BUTTON_NONE) {
        depth = this->getDepthValueAtCoord(screenPosInvY);
    }

    MouseEvent mouseEvent(screenPos, button, MouseEvent::MOUSE_STATE_MOVE,
                          EventConverterQt::getModifier(e), this->getScreenDimensions(), depth);
    e->accept();
    Canvas::mouseMoveEvent(&mouseEvent);
    return true;
}

template <typename T>
bool CanvasQtBase<T>::mapWheelEvent(QWheelEvent* e) {
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

    const ivec2 screenPos{utilqt::toGLM(e->pos())};
    const ivec2 screenPosInvY{util::invertY(screenPos, this->getScreenDimensions())};

    MouseEvent mouseEvent(screenPos, numSteps, EventConverterQt::getMouseWheelButton(e),
                          MouseEvent::MOUSE_STATE_WHEEL, orientation,
                          EventConverterQt::getModifier(e), this->getScreenDimensions(),
                          this->getDepthValueAtCoord(screenPosInvY));
    e->accept();
    Canvas::mouseWheelEvent(&mouseEvent);
    return true;
}

template <typename T>
bool CanvasQtBase<T>::mapKeyPressEvent(QKeyEvent* keyEvent) {
    KeyboardEvent pressKeyEvent(EventConverterQt::getKeyButton(keyEvent),
                                EventConverterQt::getModifier(keyEvent),
                                KeyboardEvent::KEY_STATE_PRESS);

    // set respective modifier if the pressed key is one of those
    int modifier = static_cast<int>(pressKeyEvent.modifiers());
    if (keyEvent->key() == Qt::Key_Alt) {
        modifier |= static_cast<int>(InteractionEvent::MODIFIER_ALT);
    }
    else if (keyEvent->key() == Qt::Key_Control) {
        modifier |= static_cast<int>(InteractionEvent::MODIFIER_CTRL);
    }
    else if (keyEvent->key() == Qt::Key_Shift) {
        modifier |= static_cast<int>(InteractionEvent::MODIFIER_SHIFT);
    }
    pressKeyEvent.setModifiers(modifier);
    
    Canvas::keyPressEvent(&pressKeyEvent);
    if (pressKeyEvent.hasBeenUsed()) {
        keyEvent->accept();
    } else {
        QtBase::keyPressEvent(keyEvent);
    }
    return true;
}

template <typename T>
bool CanvasQtBase<T>::mapKeyReleaseEvent(QKeyEvent* keyEvent) {
    KeyboardEvent releaseKeyEvent(EventConverterQt::getKeyButton(keyEvent),
                                  EventConverterQt::getModifier(keyEvent),
                                  KeyboardEvent::KEY_STATE_RELEASE);
    
    Canvas::keyReleaseEvent(&releaseKeyEvent);
    if (releaseKeyEvent.hasBeenUsed()) {
        keyEvent->accept();
    } else {
        QtBase::keyReleaseEvent(keyEvent);
    }
    return true;
}

template <typename T>
bool CanvasQtBase<T>::mapTouchEvent(QTouchEvent* touch) {
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
    const LayerRAM* depthLayerRAM = this->getDepthLayerRAM();
    vec2 screenSize(this->getScreenDimensions());

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
                                 this->getDepthValueAtCoord(pixelCoord, depthLayerRAM));
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
    TouchEvent touchEvent(touchPoints, this->getScreenDimensions());
    touch->accept();

    lastNumFingers_ = static_cast<int>(touch->touchPoints().size());
    screenPositionNormalized_ = touchEvent.getCenterPointNormalized();

    Canvas::touchEvent(&touchEvent);
    return true;
}

template <typename T>
bool CanvasQtBase<T>::mapGestureEvent(QGestureEvent* ge) {
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
            return mapPinchTriggered(pinchGesture);
        } else {
            lastType_ = Qt::PanGesture;
            return mapPanTriggered(panGesture);
        }
    } else if (panGesture) {
        lastType_ = Qt::PanGesture;
        return mapPanTriggered(panGesture);
    } else if (pinchGesture) {
        double absDeltaDist = glm::abs(static_cast<double>(pinchGesture->scaleFactor()) - 1.0);
        if (absDeltaDist > 0.05) lastType_ = Qt::PinchGesture;
        return mapPinchTriggered(pinchGesture);
    }
    return true;
}

template <typename T>
bool CanvasQtBase<T>::mapPanTriggered(QPanGesture* gesture) {
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
        vec2((gesture->lastOffset().x() - gesture->offset().x()) / this->getScreenDimensions().x,
             (gesture->offset().y() - gesture->lastOffset().y()) /  this->getScreenDimensions().y);

    if (deltaPos == vec2(0.f)) return true;

    GestureEvent ge(deltaPos, 0.0, GestureEvent::PAN, EventConverterQt::getGestureState(gesture),
                    lastNumFingers_, screenPositionNormalized_,  this->getScreenDimensions());

    Canvas::gestureEvent(&ge);
    return true;
}

template <typename T>
bool CanvasQtBase<T>::mapPinchTriggered(QPinchGesture* gesture) {
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
                    screenPositionNormalized_, this->getScreenDimensions());

    Canvas::gestureEvent(&ge);
    return true;
}

template <typename T>
void CanvasQtBase<T>::touchFallback(QTouchEvent* touch) {
// Mouse events will be triggered for touch events by Qt4 and Qt >= 5.3.0
// https://bugreports.qt.io/browse/QTBUG-40038
#if (QT_VERSION < QT_VERSION_CHECK(5, 3, 0))
    if (touch->touchPoints().size() == 1 && eventMapper_.getLastNumFingers() < 2) {
        size_t nTouchPoints = touch->touchPoints().size();
        if (nTouchPoints == 0) return;

        QTouchEvent::TouchPoint firstPoint = touch->touchPoints()[0];

        MouseEvent* mouseEvent = nullptr;
        const ivec2 pos = utilqt::toGLM(firstPoint.pos());
        const ivec2 screenPosInvY{util::invertY(pos, getScreenDimensions())};

        double depth = getDepthValueAtCoord(screenPosInvY);
        switch (touch->touchPoints().front().state()) {
            case TouchPoint::TOUCH_STATE_STARTED:
                mouseEvent = new MouseEvent(
                    pos, MouseEvent::MOUSE_BUTTON_LEFT, MouseEvent::MOUSE_STATE_PRESS,
                    EventConverterQt::getModifier(touch), getScreenDimensions(), depth);
                Canvas::mousePressEvent(mouseEvent);
                break;
            case TouchPoint::TOUCH_STATE_UPDATED:
                mouseEvent = new MouseEvent(
                    pos, MouseEvent::MOUSE_BUTTON_LEFT, MouseEvent::MOUSE_STATE_MOVE,
                    EventConverterQt::getModifier(touch), getScreenDimensions(), depth);
                Canvas::mouseMoveEvent(mouseEvent);
                break;
            case TouchPoint::TOUCH_STATE_STATIONARY:
                break;  // Do not fire event while standing still.
            case TouchPoint::TOUCH_STATE_ENDED:
                mouseEvent = new MouseEvent(
                    pos, MouseEvent::MOUSE_BUTTON_LEFT, MouseEvent::MOUSE_STATE_RELEASE,
                    EventConverterQt::getModifier(touch), getScreenDimensions(), depth);
                Canvas::mouseReleaseEvent(mouseEvent);
                break;
            default:
                break;
        }
        delete mouseEvent;
    }
#endif
}

#include <warn/pop>

} // namespace

#endif // IVW_CANVASQT_H