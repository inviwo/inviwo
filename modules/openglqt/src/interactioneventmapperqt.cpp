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

#include <modules/openglqt/interactioneventmapperqt.h>

#include <inviwo/core/interaction/events/gestureevent.h>
#include <inviwo/core/interaction/events/keyboardevent.h>
#include <inviwo/core/interaction/events/mouseevent.h>
#include <inviwo/core/interaction/events/wheelevent.h>
#include <inviwo/core/interaction/events/touchstate.h>
#include <inviwo/core/interaction/events/eventpropagator.h>

#include <inviwo/core/util/rendercontext.h>
#include <inviwo/core/util/glm.h>

#include <modules/qtwidgets/inviwoqtutils.h>
#include <modules/qtwidgets/eventconverterqt.h>

#include <map>

#include <warn/push>
#include <warn/ignore/all>
#include <QEvent>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QEvent>
#include <QGestureEvent>
#include <QTouchEvent>
#include <QHelpEvent>
#include <QToolTip>
#include <warn/pop>

namespace inviwo {

namespace {

dvec2 normalizePosition(QPointF pos, size2_t dim) {
    return util::invertY(utilqt::toGLM(pos), dim) / dvec2(dim - size2_t(1));
}

template <typename QE>
dvec2 normalizePosition(QE* e, size2_t dim) {
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    return normalizePosition(e->position(), dim);
#else
    return normalizePosition(e->localPos(), dim);
#endif
}

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
TouchState mapTouchState(const QEventPoint& point) {
    switch (point.state()) {
        case QEventPoint::Pressed:
            return TouchState::Started;
        case QEventPoint::Updated:
            return TouchState::Updated;
        case QEventPoint::Stationary:
            return TouchState::Stationary;
        case QEventPoint::Released:
            return TouchState::Finished;
        default:
            return TouchState::None;
    }
}
#else
TouchState mapTouchState(const QTouchEvent::TouchPoint& point) {
    switch (point.state()) {
        case Qt::TouchPointPressed:
            return TouchState::Started;
        case Qt::TouchPointMoved:
            return TouchState::Updated;
        case Qt::TouchPointStationary:
            return TouchState::Stationary;
        case Qt::TouchPointReleased:
            return TouchState::Finished;
        default:
            return TouchState::None;
    }
}
#endif

TouchDevice::DeviceType mapDeviceType(QTouchEvent* touch) {
    switch (touch->device()->type()) {
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
        case QPointingDevice::DeviceType::TouchScreen:
            return TouchDevice::DeviceType::TouchScreen;
        case QPointingDevice::DeviceType::TouchPad:
            return TouchDevice::DeviceType::TouchPad;
        default:
            return TouchDevice::DeviceType::TouchScreen;
#else
        case QTouchDevice::DeviceType::TouchScreen:
            return TouchDevice::DeviceType::TouchScreen;
        case QTouchDevice::DeviceType::TouchPad:
            return TouchDevice::DeviceType::TouchPad;
        default:
            return TouchDevice::DeviceType::TouchScreen;
#endif
    }
}

const TouchDevice* mapTouchDevice(QTouchEvent* touch) {
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    using QInputDevice = QTouchDevice;
#endif

    //! Links QPointingDevice to inviwo::TouchDevice
    static std::map<const QInputDevice*, TouchDevice> devices_;

    auto deviceIt = devices_.find(touch->device());
    if (deviceIt != devices_.end()) {
        return &(deviceIt->second);
    } else {
        // Insert device new device into map
        auto [it, inserted] = devices_.try_emplace(touch->device(), mapDeviceType(touch),
                                                   utilqt::fromQString(touch->device()->name()));
        return &(it->second);
    }
}

}  // namespace

InteractionEventMapperQt::InteractionEventMapperQt(QObject* parent, EventPropagator* propagator,
                                                   std::function<size2_t()> canvasDimensions,
                                                   std::function<size2_t()> imageDimensions,
                                                   std::function<double(dvec2)> depth,
                                                   std::function<void(QMouseEvent*)> contextMenu)
    : QObject(parent)
    , propagator_{propagator}
    , canvasDimensions_{canvasDimensions}
    , imageDimensions_{imageDimensions}
    , depth_{depth}
    , contextMenu_{contextMenu} {}

bool InteractionEventMapperQt::eventFilter(QObject*, QEvent* e) {
    switch (e->type()) {
        case QEvent::KeyPress:
            return mapKeyPressEvent(static_cast<QKeyEvent*>(e));
        case QEvent::KeyRelease:
            return mapKeyReleaseEvent(static_cast<QKeyEvent*>(e));
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
            return mapTouchEvent(static_cast<QTouchEvent*>(e));
        case QEvent::TouchEnd:
            return mapTouchEvent(static_cast<QTouchEvent*>(e));
        case QEvent::TouchUpdate:
            return mapTouchEvent(static_cast<QTouchEvent*>(e));
        case QEvent::Gesture:
            return mapGestureEvent(static_cast<QGestureEvent*>(e));
        case QEvent::ToolTip:
            return showToolTip(static_cast<QHelpEvent*>(e));
        default:
            return false;
    }
}

bool InteractionEventMapperQt::mapMousePressEvent(QMouseEvent* e) {
    if (e->source() != Qt::MouseEventNotSynthesized) return true;
    RenderContext::getPtr()->activateDefaultRenderContext();

    const auto pos = normalizePosition(e, canvasDimensions_());
    MouseEvent mouseEvent(utilqt::getMouseButtonCausingEvent(e), MouseState::Press,
                          utilqt::getMouseButtons(e), utilqt::getModifiers(e), pos,
                          imageDimensions_(), depth_(pos));
    e->accept();

    propagator_->propagateEvent(&mouseEvent, nullptr);

    if (e->button() == Qt::RightButton && mouseEvent.hasBeenUsed()) blockContextMenu_ = true;

    return true;
}

bool InteractionEventMapperQt::mapMouseDoubleClickEvent(QMouseEvent* e) {
    if (e->source() != Qt::MouseEventNotSynthesized) return true;
    RenderContext::getPtr()->activateDefaultRenderContext();

    const auto pos = normalizePosition(e, canvasDimensions_());
    MouseEvent mouseEvent(utilqt::getMouseButtonCausingEvent(e), MouseState::DoubleClick,
                          utilqt::getMouseButtons(e), utilqt::getModifiers(e), pos,
                          imageDimensions_(), depth_(pos));

    e->accept();
    propagator_->propagateEvent(&mouseEvent, nullptr);
    if (e->button() == Qt::RightButton) blockContextMenu_ = true;

    return true;
}

bool InteractionEventMapperQt::mapMouseReleaseEvent(QMouseEvent* e) {
    if (e->source() != Qt::MouseEventNotSynthesized) return true;
    RenderContext::getPtr()->activateDefaultRenderContext();

    const auto pos = normalizePosition(e, canvasDimensions_());
    MouseEvent mouseEvent(utilqt::getMouseButtonCausingEvent(e), MouseState::Release,
                          utilqt::getMouseButtons(e), utilqt::getModifiers(e), pos,
                          imageDimensions_(), depth_(pos));
    e->accept();
    propagator_->propagateEvent(&mouseEvent, nullptr);

    // Only show context menu when we have not used the event and the mouse have not been dragged.
    if (e->button() == Qt::RightButton && !mouseEvent.hasBeenUsed() && !blockContextMenu_) {
        contextMenu_(e);
    }
    blockContextMenu_ = false;

    return true;
}

bool InteractionEventMapperQt::mapMouseMoveEvent(QMouseEvent* e) {
    if (e->source() != Qt::MouseEventNotSynthesized) return true;
    RenderContext::getPtr()->activateDefaultRenderContext();
    const auto pos = normalizePosition(e, canvasDimensions_());

    MouseEvent mouseEvent(MouseButton::None, MouseState::Move, utilqt::getMouseButtons(e),
                          utilqt::getModifiers(e), pos, imageDimensions_(), depth_(pos));
    e->accept();
    propagator_->propagateEvent(&mouseEvent, nullptr);
    if (e->button() == Qt::RightButton) blockContextMenu_ = true;

    return true;
}

bool InteractionEventMapperQt::mapWheelEvent(QWheelEvent* e) {
    if (e->source() != Qt::MouseEventNotSynthesized) return true;
    RenderContext::getPtr()->activateDefaultRenderContext();
    QPoint numPixels = e->pixelDelta();
    QPoint numDegrees = e->angleDelta() / 8 / 15;

    dvec2 numSteps{0.0};
    if (!numPixels.isNull()) {
        numSteps = utilqt::toGLM(numPixels) / 5;
    } else if (!numDegrees.isNull()) {
        numSteps = utilqt::toGLM(numDegrees);
    }

#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
    const auto pos = normalizePosition(e->position(), canvasDimensions_());
#else
    const auto pos = normalizePosition(e->pos(), canvasDimensions_());
#endif

    WheelEvent wheelEvent(utilqt::getMouseWheelButtons(e), utilqt::getModifiers(e), numSteps, pos,
                          imageDimensions_(), depth_(pos));
    e->accept();
    propagator_->propagateEvent(&wheelEvent, nullptr);

    return true;
}

bool InteractionEventMapperQt::mapKeyPressEvent(QKeyEvent* keyEvent) {
    RenderContext::getPtr()->activateDefaultRenderContext();
    KeyboardEvent event(utilqt::getKeyButton(keyEvent), KeyState::Press,
                        utilqt::getModifiers(keyEvent), keyEvent->nativeVirtualKey(),
                        utilqt::fromQString(keyEvent->text()));
    // set respective modifier if the pressed key is one of those
    auto modifiers = event.modifiers();
    if (keyEvent->key() == Qt::Key_Alt) {
        modifiers |= KeyModifier::Alt;
    } else if (keyEvent->key() == Qt::Key_Control) {
        modifiers |= KeyModifier::Control;
    } else if (keyEvent->key() == Qt::Key_Shift) {
        modifiers |= KeyModifier::Shift;
    }
    event.setModifiers(modifiers);

    propagator_->propagateEvent(&event, nullptr);
    if (event.hasBeenUsed()) {
        keyEvent->accept();
        return true;
    } else {
        return false;
    }
}

bool InteractionEventMapperQt::mapKeyReleaseEvent(QKeyEvent* keyEvent) {
    RenderContext::getPtr()->activateDefaultRenderContext();
    KeyboardEvent event(utilqt::getKeyButton(keyEvent), KeyState::Release,
                        utilqt::getModifiers(keyEvent), keyEvent->nativeVirtualKey(),
                        utilqt::fromQString(keyEvent->text()));

    propagator_->propagateEvent(&event, nullptr);
    if (event.hasBeenUsed()) {
        keyEvent->accept();
        return true;
    } else {
        return false;
    }
}

bool InteractionEventMapperQt::mapTouchEvent(QTouchEvent* touch) {
    if (!handleTouch_) return false;
    RenderContext::getPtr()->activateDefaultRenderContext();

    // Copy touch points
    std::vector<TouchPoint> touchPoints;
    touchPoints.reserve(touch->touchPoints().size());
    const uvec2 imageSize = imageDimensions_();

    for (const auto& touchPoint : touch->touchPoints()) {
        const auto pos = normalizePosition(touchPoint.pos(), canvasDimensions_());
        const auto prevPos = normalizePosition(touchPoint.lastPos(), canvasDimensions_());
        const auto pressedPos = normalizePosition(touchPoint.startPos(), canvasDimensions_());
        const auto pressure = touchPoint.pressure();
        const auto touchState = mapTouchState(touchPoint);
        touchPoints.emplace_back(touchPoint.id(), touchState, pos, prevPos, pressedPos, imageSize,
                                 pressure, depth_(pos));
    }
    // Ensure that the order to the touch points are the same as last touch event.
    // Note that the ID of a touch point is always the same but the order in which
    // they are given can vary.
    std::sort(touchPoints.begin(), touchPoints.end(),
              [](const auto& a, const auto& b) { return a.id() < b.id(); });

    // Touchpad on Mac sends one event per touch point. Prevent duplicate events from being sent.
    // We consider an event to be a duplicate if all points in the event are the same as the last
    // event.
    if (touchPoints == prevTouchPoints_) {
        prevTouchPoints_ = touchPoints;
        touch->accept();
        return true;
    }
    // Store previous touch points
    prevTouchPoints_ = touchPoints;

    // Get the device generating the event (touch screen or touch pad)
    const auto device = mapTouchDevice(touch);
    TouchEvent touchEvent(touchPoints, device, utilqt::getModifiers(touch));
    touch->accept();

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    lastNumFingers_ = static_cast<int>(touch->touchPoints().size());
#else
    lastNumFingers_ = static_cast<int>(touch->points().size());
#endif
    screenPositionNormalized_ = touchEvent.centerPointNormalized();

    propagator_->propagateEvent(&touchEvent, nullptr);
    return true;
}

bool InteractionEventMapperQt::mapGestureEvent(QGestureEvent* ge) {
    if (!handleGestures_) return false;

    QPanGesture* panGesture = nullptr;
    QPinchGesture* pinchGesture = nullptr;

    if (auto gesture = ge->gesture(Qt::PanGesture)) {
        panGesture = static_cast<QPanGesture*>(gesture);
    }
    if (auto gesture = ge->gesture(Qt::PinchGesture)) {
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

bool InteractionEventMapperQt::mapPanTriggered(QPanGesture* gesture) {
    RenderContext::getPtr()->activateDefaultRenderContext();

    // determine delta position, use canvas dimensions here for normalization
    auto deltaPos =
        dvec2((gesture->lastOffset().x() - gesture->offset().x()) / (canvasDimensions_().x - 1),
              (gesture->offset().y() - gesture->lastOffset().y()) / (canvasDimensions_().y - 1));

    if (deltaPos == dvec2(0.f)) return true;

    GestureEvent ge(deltaPos, 0.0, GestureType::Pan, utilqt::getGestureState(gesture),
                    lastNumFingers_, screenPositionNormalized_, imageDimensions_(),
                    depth_(screenPositionNormalized_));

    propagator_->propagateEvent(&ge, nullptr);
    return true;
}

bool InteractionEventMapperQt::mapPinchTriggered(QPinchGesture* gesture) {
    RenderContext::getPtr()->activateDefaultRenderContext();

    GestureEvent ge(dvec2(gesture->centerPoint().x(), gesture->centerPoint().y()),
                    static_cast<double>(gesture->scaleFactor()) - 1.0, GestureType::Pinch,
                    utilqt::getGestureState(gesture), lastNumFingers_, screenPositionNormalized_,
                    imageDimensions_(), depth_(screenPositionNormalized_));

    propagator_->propagateEvent(&ge, nullptr);
    return true;
}

void InteractionEventMapperQt::setToolTipCallback(MouseInteractionEvent* e) {
    // Save tooltip text to be displayed when Qt raises a QHelpEvent (mouse is still for a while)
    e->setToolTipCallback([this](const std::string& tooltip) -> void { toolTipText_ = tooltip; });
}

bool InteractionEventMapperQt::showToolTip(QHelpEvent* e) {
    // Raised when mouse is still for a while
    // Display the saved text from tooltip callback (setToolTipCallback)
    QToolTip::showText(e->globalPos(), utilqt::toLocalQString(toolTipText_));
    if (toolTipText_.empty()) {
        e->ignore();
    } else {
        e->accept();
    }
    return true;
}

void InteractionEventMapperQt::handleTouch(bool on) { handleTouch_ = on; }
void InteractionEventMapperQt::handleGestures(bool on) { handleGestures_ = on; }

}  // namespace inviwo
