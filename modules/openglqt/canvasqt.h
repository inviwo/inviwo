/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2017 Inviwo Foundation
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
#include <inviwo/core/util/rendercontext.h>
#include <modules/openglqt/hiddencanvasqt.h>
#include <inviwo/core/interaction/events/mouseevent.h>
#include <inviwo/core/interaction/events/wheelevent.h>
#include <inviwo/core/interaction/events/keyboardevent.h>
#include <inviwo/core/interaction/events/touchevent.h>
#include <inviwo/core/interaction/events/gestureevent.h>
#include <inviwo/core/processors/canvasprocessor.h>
#include <inviwo/core/metadata/processormetadata.h>
#include <modules/qtwidgets/eventconverterqt.h>
#include <modules/qtwidgets/inviwoqtutils.h>

#include <modules/opengl/canvasgl.h>
#include <modules/opengl/debugmessages.h>
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
#include <QApplication>
#include <QClipboard>
#include <QMenu>
#include <QPixmap>
#include <QAction>
#include <QWidget>
#include <warn/pop>

namespace inviwo {

template <typename T>
class CanvasQtBase : public T {
    friend class CanvasProcessorWidgetQt;

public:
    using QtBase = typename T::QtBase;

    explicit CanvasQtBase(size2_t dim = size2_t(256,256), const std::string& name = "Canvas");
    virtual ~CanvasQtBase();

    virtual void render(std::shared_ptr<const Image> image, LayerType layerType = LayerType::Color,
                        size_t idx = 0) override;

    virtual std::unique_ptr<Canvas> createHiddenCanvas() override;

    static CanvasQtBase<T>* getSharedCanvas() {
        return static_cast<CanvasQtBase<T>*>(T::sharedCanvas_);
    }

    virtual bool isFullScreen() const override;
    virtual void setFullScreen(bool fullscreen) override;

    void contextMenuEvent(QContextMenuEvent* event) override;

protected:
    virtual bool event(QEvent *e) override;

private:
    dvec2 normalPos(dvec2 pos) const;

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
        
    Qt::GestureType lastType_;
    int lastNumFingers_;
    vec2 screenPositionNormalized_;
    bool blockContextMenu_;
};

using CanvasQt = CanvasQtBase<CanvasQGLWidget>;
//using CanvasQt = CanvasQtBase<CanvasQWindow>;
//using CanvasQt = CanvasQtBase<CanvasQOpenGLWidget>;


template <typename T>
CanvasQtBase<T>::CanvasQtBase(size2_t dim, const std::string& name) : T(nullptr, dim) , blockContextMenu_(false){
    QtBase::makeCurrent();
    RenderContext::getPtr()->registerContext(this, name);
    utilgl::handleOpenGLDebugMode(this->activeContext());
}

template <typename T>
CanvasQtBase<T>::~CanvasQtBase() {
    RenderContext::getPtr()->unRegisterContext(this);
}

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

    auto newContext = res.get();
    RenderContext::getPtr()->setContextThreadId(newContext->contextId(),
                                                std::this_thread::get_id());
    return std::move(newContext);
}

template <typename T>
void CanvasQtBase<T>::render(std::shared_ptr<const Image> image, LayerType layerType, size_t idx) {
    if (this->isVisible() && this->isValid()) {
        CanvasGL::render(image, layerType, idx);
    }
}

template <typename T>
void inviwo::CanvasQtBase<T>::setFullScreen(bool fullscreen) {
    if (fullscreen) {
        this->parentWidget()->showFullScreen();
    } else {
        this->parentWidget()->showNormal();
    }
}

template <typename T>
bool inviwo::CanvasQtBase<T>::isFullScreen() const {
    return this->parentWidget()->windowState() == Qt::WindowFullScreen;
}

template <typename T>
void CanvasQtBase<T>::contextMenuEvent(QContextMenuEvent* event) {
    if (auto canvasProcessor = dynamic_cast<CanvasProcessor*>(this->ownerWidget_->getProcessor())) {

        if (!canvasProcessor->isContextMenuAllowed()) {
            return;
        }

        if (event->reason() == QContextMenuEvent::Mouse && blockContextMenu_) {
            return;
        }

        QMenu menu(this);

        auto visibleLayer = canvasProcessor->getVisibleLayer();
        auto img = canvasProcessor->getImage();

        this->connect(menu.addAction("Select processor"), &QAction::triggered, [&]() {
            canvasProcessor->getMetaData<ProcessorMetaData>(ProcessorMetaData::CLASS_IDENTIFIER)
                ->setSelected(true);
        });

        for (size_t i = 0; i < img->getNumberOfColorLayers(); i++) {
            std::ostringstream oss;
            oss << "color layer " << i << " to clipboard";
            auto layer = img->getColorLayer(i);
            if (visibleLayer == layer) {
                oss << " (visible)";
            }
            auto copyAction = menu.addAction(("Copy " + oss.str()).c_str());
            this->connect(copyAction, &QAction::triggered, [&]() {
                QApplication::clipboard()->setPixmap(
                    QPixmap::fromImage(utilqt::layerToQImage(*layer)));
            });
        }

        {
            std::ostringstream oss;
            oss << "Copy picking layer to clipboard";
            auto layer = img->getPickingLayer();
            if (visibleLayer == layer) {
                oss << " (visible)";
            }
            auto pickingAction = menu.addAction(oss.str().c_str());
            this->connect(pickingAction, &QAction::triggered, [&]() {
                auto qimg = utilqt::layerToQImage(*layer);
                QApplication::clipboard()->setPixmap(QPixmap::fromImage(qimg));
            });
        }

        {
            std::ostringstream oss;
            oss << "Copy depth layer to clipboard";
            auto layer = img->getDepthLayer();
            if (visibleLayer == layer) {
                oss << " (visible)";
            }
            auto depthAction = menu.addAction(oss.str().c_str());
            this->connect(depthAction, &QAction::triggered, [&]() {
                QApplication::clipboard()->setPixmap(
                    QPixmap::fromImage(utilqt::layerToQImage(*layer)));
            });
        }

        menu.exec(event->globalPos());
    }
}

#include <warn/push>
#include <warn/ignore/switch-enum>

template <typename T>
bool CanvasQtBase<T>::event(QEvent* e) {
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
        default:
            return QtBase::event(e);
    }
}

template <typename T>
dvec2 inviwo::CanvasQtBase<T>::normalPos(dvec2 pos) const {
    return util::invertY(pos, this->getCanvasDimensions()) /
        dvec2(this->getCanvasDimensions() - size2_t(1));
}

template <typename T>
bool CanvasQtBase<T>::mapMousePressEvent(QMouseEvent* e) {
    if (e->source() != Qt::MouseEventNotSynthesized) return true;

    const auto pos{normalPos(utilqt::toGLM(e->localPos()))};

    MouseEvent mouseEvent(utilqt::getMouseButtonCausingEvent(e), MouseState::Press,
                          utilqt::getMouseButtons(e), utilqt::getModifiers(e),
                          pos, this->getImageDimensions(),
                          this->getDepthValueAtNormalizedCoord(pos));

    e->accept();
    Canvas::propagateEvent(&mouseEvent);

    blockContextMenu_ = e->button() == Qt::RightButton && mouseEvent.hasBeenUsed();

    return true;
}

template <typename T>
bool CanvasQtBase<T>::mapMouseDoubleClickEvent(QMouseEvent* e) {
    if (e->source() != Qt::MouseEventNotSynthesized) return true;

    const auto pos{normalPos(utilqt::toGLM(e->localPos()))};
    MouseEvent mouseEvent(utilqt::getMouseButtonCausingEvent(e), MouseState::DoubleClick,
                          utilqt::getMouseButtons(e), utilqt::getModifiers(e),
                          pos, this->getImageDimensions(),
                          this->getDepthValueAtNormalizedCoord(pos));

    e->accept();
    Canvas::propagateEvent(&mouseEvent);
    blockContextMenu_ |= e->button() == Qt::RightButton && mouseEvent.hasBeenUsed();
    return true;
}

template <typename T>
bool CanvasQtBase<T>::mapMouseReleaseEvent(QMouseEvent* e) {
    if (e->source() != Qt::MouseEventNotSynthesized) return true;

    const auto pos{normalPos(utilqt::toGLM(e->localPos()))};

    MouseEvent mouseEvent(utilqt::getMouseButtonCausingEvent(e), MouseState::Release,
                          utilqt::getMouseButtons(e), utilqt::getModifiers(e),
                          pos, this->getImageDimensions(),
                          this->getDepthValueAtNormalizedCoord(pos));
    e->accept();
    Canvas::propagateEvent(&mouseEvent);
    blockContextMenu_ |= e->button() == Qt::RightButton && mouseEvent.hasBeenUsed();
    return true;
}

template <typename T>
bool CanvasQtBase<T>::mapMouseMoveEvent(QMouseEvent* e) {
    if (e->source() != Qt::MouseEventNotSynthesized) return true;

    const auto pos{normalPos(utilqt::toGLM(e->localPos()))};

    MouseEvent mouseEvent(MouseButton::None, MouseState::Move, utilqt::getMouseButtons(e),
                          utilqt::getModifiers(e), pos, this->getImageDimensions(),
                          this->getDepthValueAtNormalizedCoord(pos));
    e->accept();
    Canvas::propagateEvent(&mouseEvent);
    return true;
}

template <typename T>
bool CanvasQtBase<T>::mapWheelEvent(QWheelEvent* e) {
    QPoint numPixels = e->pixelDelta();
    QPoint numDegrees = e->angleDelta() / 8 / 15;

    dvec2 numSteps{0.0};
    if (!numPixels.isNull()) {
        numSteps = utilqt::toGLM(numPixels) / 5;
    } else if (!numDegrees.isNull()) {
        numSteps = utilqt::toGLM(numDegrees);
    }

    const auto pos{normalPos(utilqt::toGLM(QPointF(e->pos())))};

    WheelEvent wheelEvent(utilqt::getMouseWheelButtons(e), utilqt::getModifiers(e), numSteps,
                          pos, this->getImageDimensions(),
                          this->getDepthValueAtNormalizedCoord(pos));
    e->accept();
    Canvas::propagateEvent(&wheelEvent);
    return true;
}

template <typename T>
bool CanvasQtBase<T>::mapKeyPressEvent(QKeyEvent* keyEvent) {
    KeyboardEvent pressKeyEvent(utilqt::getKeyButton(keyEvent), KeyState::Press,
                                utilqt::getModifiers(keyEvent));

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

    Canvas::propagateEvent(&pressKeyEvent);
    if (pressKeyEvent.hasBeenUsed()) {
        keyEvent->accept();
    } else {
        QtBase::keyPressEvent(keyEvent);
    }
    return true;
}

template <typename T>
bool CanvasQtBase<T>::mapKeyReleaseEvent(QKeyEvent* keyEvent) {
    KeyboardEvent releaseKeyEvent(utilqt::getKeyButton(keyEvent), KeyState::Release,
                                  utilqt::getModifiers(keyEvent));

    Canvas::propagateEvent(&releaseKeyEvent);
    if (releaseKeyEvent.hasBeenUsed()) {
        keyEvent->accept();
    } else {
        QtBase::keyReleaseEvent(keyEvent);
    }
    return true;
}

template <typename T>
bool CanvasQtBase<T>::mapTouchEvent(QTouchEvent* touch) {

    // Copy touch points
    std::vector<TouchPoint> touchPoints;
    touchPoints.reserve(touch->touchPoints().size());
    uvec2 screenSize(this->getCanvasDimensions());

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
        touchPoints.emplace_back(touchPoint.id(), touchState, pos, prevPos,pressedPos, screenSize,
                                 pressure, this->getDepthValueAtNormalizedCoord(pos));
    }
    // Ensure that the order to the touch points are the same as last touch event.
    // Note that the ID of a touch point is always the same but the order in which
    // they are given can vary.
    std::sort(touchPoints.begin(), touchPoints.end(),
              [](const auto& a, const auto& b) { return a.id() < b.id(); });

    TouchEvent touchEvent(touchPoints);
    touch->accept();

    lastNumFingers_ = static_cast<int>(touch->touchPoints().size());
    screenPositionNormalized_ = touchEvent.centerPointNormalized();

    Canvas::propagateEvent(&touchEvent);
    return true;
}

template <typename T>
bool CanvasQtBase<T>::mapGestureEvent(QGestureEvent* ge) {
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

template <typename T>
bool CanvasQtBase<T>::mapPanTriggered(QPanGesture* gesture) {
    auto deltaPos =
        dvec2((gesture->lastOffset().x() - gesture->offset().x()) / this->getCanvasDimensions().x,
             (gesture->offset().y() - gesture->lastOffset().y()) /  this->getCanvasDimensions().y);

    if (deltaPos == dvec2(0.f)) return true;

    auto depth = this->getDepthValueAtNormalizedCoord(screenPositionNormalized_);
    GestureEvent ge(deltaPos, 0.0, GestureType::Pan, utilqt::getGestureState(gesture),
                    lastNumFingers_, screenPositionNormalized_,  this->getCanvasDimensions(), depth);

    Canvas::propagateEvent(&ge);
    return true;
}

template <typename T>
bool CanvasQtBase<T>::mapPinchTriggered(QPinchGesture* gesture) {
    auto depth = this->getDepthValueAtNormalizedCoord(screenPositionNormalized_);
    GestureEvent ge(dvec2(gesture->centerPoint().x(), gesture->centerPoint().y()),
                    static_cast<double>(gesture->scaleFactor()) - 1.0, GestureType::Pinch,
                    utilqt::getGestureState(gesture), lastNumFingers_,
                    screenPositionNormalized_, this->getCanvasDimensions(), depth);

    Canvas::propagateEvent(&ge);
    return true;
}

#include <warn/pop>

} // namespace

#endif // IVW_CANVASQT_H
