/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2020-2021 Inviwo Foundation
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

#include <modules/openglqt/canvasqt.h>

#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/interaction/events/gestureevent.h>
#include <inviwo/core/interaction/events/keyboardevent.h>
#include <inviwo/core/interaction/events/mouseevent.h>
#include <inviwo/core/interaction/events/viewevent.h>
#include <inviwo/core/interaction/events/wheelevent.h>
#include <inviwo/core/interaction/events/touchstate.h>
#include <inviwo/core/metadata/processormetadata.h>
#include <inviwo/core/processors/canvasprocessor.h>
#include <inviwo/core/processors/processorwidget.h>
#include <inviwo/core/util/rendercontext.h>
#include <modules/openglqt/hiddencanvasqt.h>
#include <modules/qtwidgets/eventconverterqt.h>
#include <modules/qtwidgets/inviwoqtutils.h>
#include <modules/qtwidgets/mousecursorutils.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QThread>
#include <QMenu>
#include <QAction>
#include <QWidget>
#include <QApplication>
#include <QToolTip>
#include <warn/pop>

namespace inviwo {

CanvasQt::CanvasQt(QWidget* parent, size2_t dim, std::string_view name)
    : CanvasQOpenGLWidget(parent, dim, name), blockContextMenu_(false) {

    setContextMenuPolicy(Qt::PreventContextMenu);
}

CanvasQt::~CanvasQt() = default;

std::unique_ptr<Canvas> CanvasQt::createHiddenCanvas() {
    return HiddenCanvasQt::createHiddenQtCanvas();
}

void CanvasQt::render(std::shared_ptr<const Image> image, LayerType layerType, size_t idx) {
    if (isVisible() && isValid()) {
        CanvasGL::render(image, layerType, idx);
    }
}

void CanvasQt::setFullScreenInternal(bool fullscreen) {
    if (fullscreen) {
        // Prevent Qt resize event with incorrect size when going full screen.
        // Reproduce error by loading a workspace with a full screen canvas.
        // This is equivalent to suggested solution using QTimer
        // https://stackoverflow.com/questions/19817881/qt-fullscreen-on-startup
        // No need to process user events, i.e. mouse/keyboard etc.
        QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
        parentWidget()->setWindowState(parentWidget()->windowState() | Qt::WindowFullScreen);
    } else {
        parentWidget()->setWindowState(parentWidget()->windowState() & ~Qt::WindowFullScreen);
    }
}

void CanvasQt::doContextMenu(QMouseEvent* event) {
    if (auto canvasProcessor = dynamic_cast<CanvasProcessor*>(ownerWidget_->getProcessor())) {

        if (!canvasProcessor->isContextMenuAllowed()) return;

        QMenu menu(this);

        connect(menu.addAction(QIcon(":svgicons/edit-selectall.svg"), "&Select Processor"),
                &QAction::triggered, this, [&]() {
                    canvasProcessor
                        ->getMetaData<ProcessorMetaData>(ProcessorMetaData::CLASS_IDENTIFIER)
                        ->setSelected(true);
                });
        connect(menu.addAction(QIcon(":svgicons/canvas-hide.svg"), "&Hide Canvas"),
                &QAction::triggered, this, [&]() { ownerWidget_->setVisible(false); });

        connect(menu.addAction(QIcon(":svgicons/fullscreen.svg"), "&Toggle Full Screen"),
                &QAction::triggered, this, [&]() { setFullScreen(!Canvas::isFullScreen()); });

        if (auto image = image_.lock()) {
            menu.addSeparator();
            utilqt::addImageActions(menu, *image, layerType_, layerIdx_);
        }

        {
            menu.addSeparator();
            auto prop = [this](auto action) {
                return [this, action]() {
                    ViewEvent e{action};
                    propagateEvent(&e);
                };
            };
            connect(menu.addAction(QIcon(":svgicons/view-fit-to-data.svg"), "Fit to data"),
                    &QAction::triggered, this, prop(ViewEvent::FitData{}));
            connect(menu.addAction(QIcon(":svgicons/view-x-p.svg"), "View from X+"),
                    &QAction::triggered, this, prop(camerautil::Side::XPositive));
            connect(menu.addAction(QIcon(":svgicons/view-x-m.svg"), "View from X-"),
                    &QAction::triggered, this, prop(camerautil::Side::XNegative));
            connect(menu.addAction(QIcon(":svgicons/view-y-p.svg"), "View from Y+"),
                    &QAction::triggered, this, prop(camerautil::Side::YPositive));
            connect(menu.addAction(QIcon(":svgicons/view-y-m.svg"), "View from Y-"),
                    &QAction::triggered, this, prop(camerautil::Side::YNegative));
            connect(menu.addAction(QIcon(":svgicons/view-z-p.svg"), "View from Z+"),
                    &QAction::triggered, this, prop(camerautil::Side::ZPositive));
            connect(menu.addAction(QIcon(":svgicons/view-z-m.svg"), "View from Z-"),
                    &QAction::triggered, this, prop(camerautil::Side::ZNegative));
            connect(menu.addAction(QIcon(":svgicons/view-flip.svg"), "Flip Up Vector"),
                    &QAction::triggered, this, prop(ViewEvent::FlipUp{}));
        }

        menu.exec(event->globalPos());
    }
}

#include <warn/push>
#include <warn/ignore/switch-enum>

bool CanvasQt::event(QEvent* e) {
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
            return CanvasQOpenGLWidget::event(e);
    }
}

dvec2 CanvasQt::normalPos(dvec2 pos) const {
    return util::invertY(pos, getCanvasDimensions()) / dvec2(getCanvasDimensions() - size2_t(1));
}

bool CanvasQt::mapMousePressEvent(QMouseEvent* e) {
    if (e->source() != Qt::MouseEventNotSynthesized) return true;
    RenderContext::getPtr()->activateDefaultRenderContext();

    const auto pos{normalPos(utilqt::toGLM(e->localPos()))};

    MouseEvent mouseEvent(utilqt::getMouseButtonCausingEvent(e), MouseState::Press,
                          utilqt::getMouseButtons(e), utilqt::getModifiers(e), pos,
                          getImageDimensions(), getDepthValueAtNormalizedCoord(pos));

    e->accept();
    propagateEvent(&mouseEvent);

    if (e->button() == Qt::RightButton && mouseEvent.hasBeenUsed()) {
        blockContextMenu_ = true;
    }

    return true;
}

bool CanvasQt::mapMouseDoubleClickEvent(QMouseEvent* e) {
    if (e->source() != Qt::MouseEventNotSynthesized) return true;
    RenderContext::getPtr()->activateDefaultRenderContext();

    const auto pos{normalPos(utilqt::toGLM(e->localPos()))};
    MouseEvent mouseEvent(utilqt::getMouseButtonCausingEvent(e), MouseState::DoubleClick,
                          utilqt::getMouseButtons(e), utilqt::getModifiers(e), pos,
                          getImageDimensions(), getDepthValueAtNormalizedCoord(pos));

    e->accept();
    propagateEvent(&mouseEvent);
    if (e->button() == Qt::RightButton) {
        blockContextMenu_ = true;
    }
    return true;
}

bool CanvasQt::mapMouseReleaseEvent(QMouseEvent* e) {
    if (e->source() != Qt::MouseEventNotSynthesized) return true;
    RenderContext::getPtr()->activateDefaultRenderContext();
    const auto pos{normalPos(utilqt::toGLM(e->localPos()))};

    MouseEvent mouseEvent(utilqt::getMouseButtonCausingEvent(e), MouseState::Release,
                          utilqt::getMouseButtons(e), utilqt::getModifiers(e), pos,
                          getImageDimensions(), getDepthValueAtNormalizedCoord(pos));
    e->accept();
    propagateEvent(&mouseEvent);

    // Only show context menu when we have not used the event and the mouse have not been dragged.
    if (e->button() == Qt::RightButton && !mouseEvent.hasBeenUsed() && !blockContextMenu_) {
        doContextMenu(e);
    }
    blockContextMenu_ = false;

    return true;
}

bool CanvasQt::mapMouseMoveEvent(QMouseEvent* e) {
    if (e->source() != Qt::MouseEventNotSynthesized) return true;
    RenderContext::getPtr()->activateDefaultRenderContext();
    const auto pos{normalPos(utilqt::toGLM(e->localPos()))};

    MouseEvent mouseEvent(MouseButton::None, MouseState::Move, utilqt::getMouseButtons(e),
                          utilqt::getModifiers(e), pos, getImageDimensions(),
                          getDepthValueAtNormalizedCoord(pos));
    e->accept();
    propagateEvent(&mouseEvent);
    if (e->button() == Qt::RightButton) {
        blockContextMenu_ = true;
    }
    return true;
}

bool CanvasQt::mapWheelEvent(QWheelEvent* e) {
    RenderContext::getPtr()->activateDefaultRenderContext();
    QPoint numPixels = e->pixelDelta();
    QPoint numDegrees = e->angleDelta() / 8 / 15;

    dvec2 numSteps{0.0};
    if (!numPixels.isNull()) {
        numSteps = utilqt::toGLM(numPixels) / 5;
    } else if (!numDegrees.isNull()) {
        numSteps = utilqt::toGLM(numDegrees);
    }
#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
    const auto pos{normalPos(utilqt::toGLM(QPointF(e->position())))};
#else
    const auto pos{normalPos(utilqt::toGLM(QPointF(e->pos())))};
#endif
    WheelEvent wheelEvent(utilqt::getMouseWheelButtons(e), utilqt::getModifiers(e), numSteps, pos,
                          getImageDimensions(), getDepthValueAtNormalizedCoord(pos));
    e->accept();
    propagateEvent(&wheelEvent);
    return true;
}

bool CanvasQt::mapKeyPressEvent(QKeyEvent* keyEvent) {
    RenderContext::getPtr()->activateDefaultRenderContext();
    KeyboardEvent pressKeyEvent(utilqt::getKeyButton(keyEvent), KeyState::Press,
                                utilqt::getModifiers(keyEvent), keyEvent->nativeVirtualKey(),
                                utilqt::fromQString(keyEvent->text()));
    // set respective modifier if the pressed key is one of those
    auto modifiers = pressKeyEvent.modifiers();
    if (keyEvent->key() == Qt::Key_Alt) {
        modifiers |= KeyModifier::Alt;
    } else if (keyEvent->key() == Qt::Key_Control) {
        modifiers |= KeyModifier::Control;
    } else if (keyEvent->key() == Qt::Key_Shift) {
        modifiers |= KeyModifier::Shift;
    }
    pressKeyEvent.setModifiers(modifiers);

    propagateEvent(&pressKeyEvent);
    if (pressKeyEvent.hasBeenUsed()) {
        keyEvent->accept();
    } else {
        CanvasQOpenGLWidget::keyPressEvent(keyEvent);
    }
    return true;
}

bool CanvasQt::mapKeyReleaseEvent(QKeyEvent* keyEvent) {
    RenderContext::getPtr()->activateDefaultRenderContext();
    KeyboardEvent releaseKeyEvent(utilqt::getKeyButton(keyEvent), KeyState::Release,
                                  utilqt::getModifiers(keyEvent), keyEvent->nativeVirtualKey(),
                                  utilqt::fromQString(keyEvent->text()));
    propagateEvent(&releaseKeyEvent);
    if (releaseKeyEvent.hasBeenUsed()) {
        keyEvent->accept();
    } else {
        CanvasQOpenGLWidget::keyReleaseEvent(keyEvent);
    }
    return true;
}

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
bool CanvasQt::mapTouchEvent(QTouchEvent* touch) {
    RenderContext::getPtr()->activateDefaultRenderContext();

    // Copy touch points
    std::vector<TouchPoint> touchPoints;
    touchPoints.reserve(touch->points().size());
    const uvec2 imageSize(getImageDimensions());

    for (const auto& touchPoint : touch->points()) {
        const auto pos = normalPos(utilqt::toGLM(touchPoint.position()));
        const auto prevPos = normalPos(utilqt::toGLM(touchPoint.lastPosition()));
        const auto pressedPos = normalPos(utilqt::toGLM(touchPoint.pressPosition()));
        const auto pressure = touchPoint.pressure();

        TouchState touchState;
        switch (touchPoint.state()) {
            case QEventPoint::Pressed:
                touchState = TouchState::Started;
                break;
            case QEventPoint::Updated:
                touchState = TouchState::Updated;
                break;
            case QEventPoint::Stationary:
                touchState = TouchState::Stationary;
                break;
            case QEventPoint::Released:
                touchState = TouchState::Finished;
                break;
            default:
                touchState = TouchState::None;
        }
        touchPoints.emplace_back(touchPoint.id(), touchState, pos, prevPos, pressedPos, imageSize,
                                 pressure, getDepthValueAtNormalizedCoord(pos));
    }
    // Ensure that the order to the touch points are the same as last touch event.
    // Note that the ID of a touch point is always the same but the order in which
    // they are given can vary.
    std::sort(touchPoints.begin(), touchPoints.end(),
              [](const auto& a, const auto& b) { return a.id() < b.id(); });

    // Touchpad on Mac sends one event per touch point. Prevent duplicate events from being sent.
    // We consider an event to be a duplicate if all points in the event are the same as the last
    // event.
    if (std::equal(touchPoints.begin(), touchPoints.end(), prevTouchPoints_.begin(),
                   prevTouchPoints_.end())) {
        prevTouchPoints_ = touchPoints;
        touch->accept();
        return true;
    }
    // Store previous touch points
    prevTouchPoints_ = touchPoints;

    // Get the device generating the event (touch screen or touch pad)
    auto deviceIt = devices_.find(touch->pointingDevice());
    TouchDevice* device = nullptr;
    if (deviceIt != devices_.end()) {
        device = &(deviceIt->second);
    } else {
        // Insert device new device into map
        TouchDevice::DeviceType deviceType;
        switch (touch->device()->type()) {
            case QPointingDevice::DeviceType::TouchScreen:
                deviceType = TouchDevice::DeviceType::TouchScreen;
                break;
            case QPointingDevice::DeviceType::TouchPad:
                deviceType = TouchDevice::DeviceType::TouchPad;
                break;
            default:
                deviceType = TouchDevice::DeviceType::TouchScreen;
        }
        device = &(devices_[touch->pointingDevice()] =
                       TouchDevice(deviceType, (touch->device()->name().toStdString())));
    }
    TouchEvent touchEvent(touchPoints, device, utilqt::getModifiers(touch));
    touch->accept();

    lastNumFingers_ = static_cast<int>(touch->points().size());
    screenPositionNormalized_ = touchEvent.centerPointNormalized();

    propagateEvent(&touchEvent);
    return true;
}
#else
bool CanvasQt::mapTouchEvent(QTouchEvent* touch) {
    RenderContext::getPtr()->activateDefaultRenderContext();

    // Copy touch points
    std::vector<TouchPoint> touchPoints;
    touchPoints.reserve(touch->touchPoints().size());
    const uvec2 imageSize(getImageDimensions());

    for (const auto& touchPoint : touch->touchPoints()) {
        const auto pos = normalPos(utilqt::toGLM(touchPoint.pos()));
        const auto prevPos = normalPos(utilqt::toGLM(touchPoint.lastPos()));
        const auto pressedPos = normalPos(utilqt::toGLM(touchPoint.startPos()));
        const auto pressure = touchPoint.pressure();

        TouchState touchState;
        switch (touchPoint.state()) {
            case Qt::TouchPointPressed:
                touchState = TouchState::Started;
                break;
            case Qt::TouchPointMoved:
                touchState = TouchState::Updated;
                break;
            case Qt::TouchPointStationary:
                touchState = TouchState::Stationary;
                break;
            case Qt::TouchPointReleased:
                touchState = TouchState::Finished;
                break;
            default:
                touchState = TouchState::None;
        }
        touchPoints.emplace_back(touchPoint.id(), touchState, pos, prevPos, pressedPos, imageSize,
                                 pressure, getDepthValueAtNormalizedCoord(pos));
    }
    // Ensure that the order to the touch points are the same as last touch event.
    // Note that the ID of a touch point is always the same but the order in which
    // they are given can vary.
    std::sort(touchPoints.begin(), touchPoints.end(),
              [](const auto& a, const auto& b) { return a.id() < b.id(); });

    // Touchpad on Mac sends one event per touch point. Prevent duplicate events from being sent.
    // We consider an event to be a duplicate if all points in the event are the same as the last
    // event.
    if (std::equal(touchPoints.begin(), touchPoints.end(), prevTouchPoints_.begin(),
                   prevTouchPoints_.end())) {
        prevTouchPoints_ = touchPoints;
        touch->accept();
        return true;
    }
    // Store previous touch points
    prevTouchPoints_ = touchPoints;

    // Get the device generating the event (touch screen or touch pad)
    auto deviceIt = touchDevices_.find(touch->device());
    TouchDevice* device = nullptr;
    if (deviceIt != touchDevices_.end()) {
        device = &(deviceIt->second);
    } else {
        // Insert device new device into map
        TouchDevice::DeviceType deviceType;
        switch (touch->device()->type()) {
            case QTouchDevice::DeviceType::TouchScreen:
                deviceType = TouchDevice::DeviceType::TouchScreen;
                break;
            case QTouchDevice::DeviceType::TouchPad:
                deviceType = TouchDevice::DeviceType::TouchPad;
                break;
            default:
                deviceType = TouchDevice::DeviceType::TouchScreen;
        }
        device = &(touchDevices_[touch->device()] =
                       TouchDevice(deviceType, (touch->device()->name().toStdString())));
    }
    TouchEvent touchEvent(touchPoints, device, utilqt::getModifiers(touch));
    touch->accept();

    lastNumFingers_ = static_cast<int>(touch->touchPoints().size());
    screenPositionNormalized_ = touchEvent.centerPointNormalized();

    propagateEvent(&touchEvent);
    return true;
}

#endif

bool CanvasQt::mapGestureEvent(QGestureEvent* ge) {
    RenderContext::getPtr()->activateDefaultRenderContext();

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

bool CanvasQt::mapPanTriggered(QPanGesture* gesture) {
    RenderContext::getPtr()->activateDefaultRenderContext();

    // determine delta position, use canvas dimensions here for normalization
    auto deltaPos =
        dvec2((gesture->lastOffset().x() - gesture->offset().x()) / (getCanvasDimensions().x - 1),
              (gesture->offset().y() - gesture->lastOffset().y()) / (getCanvasDimensions().y - 1));

    if (deltaPos == dvec2(0.f)) return true;

    auto depth = getDepthValueAtNormalizedCoord(screenPositionNormalized_);
    GestureEvent ge(deltaPos, 0.0, GestureType::Pan, utilqt::getGestureState(gesture),
                    lastNumFingers_, screenPositionNormalized_, getImageDimensions(), depth);

    propagateEvent(&ge);
    return true;
}

bool CanvasQt::mapPinchTriggered(QPinchGesture* gesture) {
    RenderContext::getPtr()->activateDefaultRenderContext();

    auto depth = getDepthValueAtNormalizedCoord(screenPositionNormalized_);
    GestureEvent ge(dvec2(gesture->centerPoint().x(), gesture->centerPoint().y()),
                    static_cast<double>(gesture->scaleFactor()) - 1.0, GestureType::Pinch,
                    utilqt::getGestureState(gesture), lastNumFingers_, screenPositionNormalized_,
                    getImageDimensions(), depth);

    propagateEvent(&ge);
    return true;
}
#include <warn/pop>

void CanvasQt::propagateEvent(Event* e) { CanvasQOpenGLWidget::propagateEvent(e); }

void CanvasQt::propagateEvent(MouseInteractionEvent* e) {
    e->setToolTipCallback([this](const std::string& tooltip) -> void {
        // Save tooltip text to be displayed when Qt raises a QHelpEvent (mouse is still for a
        // while)
        toolTipText_ = tooltip;
    });
    e->setMouseCursorCallback([this](MouseCursor c) -> void { setCursor(util::toCursorShape(c)); });

    CanvasQOpenGLWidget::propagateEvent(e);
}

bool CanvasQt::showToolTip(QHelpEvent* e) {
    // Raised when mouse is still for a while
    // Display the saved text from tooltip callback (setToolTipCallback)
    QToolTip::showText(e->globalPos(), utilqt::toLocalQString(toolTipText_), this);
    e->accept();
    return true;
}

}  // namespace inviwo
