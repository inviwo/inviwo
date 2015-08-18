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

#include <modules/openglqt/canvasqt.h>
#include <inviwo/core/datastructures/image/layerram.h>
#include <modules/opengl/openglcapabilities.h>
#if defined(USE_NEW_OPENGLWIDGET)
#include <QOpenGLContext>
#elif defined(USE_QWINDOW)
#include <QtGui/QOpenGLContext>
#endif

#pragma warning(disable: 4061)

#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
#define USING_QT5
#else
#define USING_QT4
#endif

namespace inviwo {

inline QGLContextFormat GetQGLFormat() {
  QGLContextFormat sharedFormat = QGLContextFormat(
#if !defined(USE_QWINDOW) && !defined(USE_NEW_OPENGLWIDGET)
      QGL::Rgba | QGL::DoubleBuffer | QGL::AlphaChannel | QGL::DepthBuffer | QGL::StencilBuffer
#endif
      );
  sharedFormat.setProfile(QGLContextFormat::CoreProfile);
#ifdef USING_QT5
  sharedFormat.setVersion(10, 0);
#endif
  return sharedFormat;
}

QGLContextFormat CanvasQt::sharedFormat_ = GetQGLFormat();
CanvasQt* CanvasQt::sharedCanvas_ = nullptr;

#if defined(USE_NEW_OPENGLWIDGET)
QGLWindow* CanvasQt::sharedGLContext_ = nullptr;

CanvasQt::CanvasQt(QGLParent* parent, uvec2 dim)
    : QGLWindow(parent)
    , CanvasGL(dim)
    , swapBuffersAllowed_(false)
#ifndef QT_NO_GESTURES
    , gestureMode_(false)
    , lastType_(Qt::CustomGesture)
    , lastNumFingers_(0)
    , screenPositionNormalized_(vec2(0.f))
#endif
{
    setFormat(sharedFormat_);

    if (sharedGLContext_) {
        this->context()->setShareContext(sharedGLContext_->context());
    }
    create();

    setFocusPolicy(Qt::StrongFocus);

#ifndef QT_NO_GESTURES
    grabGesture(Qt::PanGesture);
    grabGesture(Qt::PinchGesture);
#endif
    QGLWindow::resizeEvent(&QResizeEvent(QSize(dim.x, dim.y), QSize(width(), height())));
    if (!sharedGLContext_) {
        sharedFormat_ = this->format();
        sharedGLContext_ = this;
        sharedCanvas_ = this;
    }
}
#elif defined(USE_QWINDOW)
QOpenGLContext* CanvasQt::sharedGLContext_ = nullptr;

CanvasQt::CanvasQt(QGLParent* parent, uvec2 dim)
    : QGLWindow(parent)
    , CanvasGL(dim)
    , thisGLContext_(nullptr)
    , swapBuffersAllowed_(false)
#ifndef QT_NO_GESTURES
    , gestureMode_(false)
    , lastNumFingers_(0)
    , screenPositionNormalized_(vec2(0.f))
#endif
{
    setSurfaceType(QWindow::OpenGLSurface);
    setFormat(sharedFormat_);
    create();

    thisGLContext_ = new QOpenGLContext(this);
    thisGLContext_->setFormat(sharedFormat_);

    //This is our default rendering context
    //Initialized once. So "THE" first object of this class will not have any shared context (or widget)
    //But Following objects, will share the context of initial object
    bool contextCreated;
    if (!sharedGLContext_) {
        contextCreated = thisGLContext_->create();
        sharedFormat_ = thisGLContext_->format();
        sharedGLContext_ = thisGLContext_;
        sharedCanvas_ = this;
        activate();
        initializeGL();
    }
    else{
        thisGLContext_->setShareContext(sharedGLContext_);
        contextCreated = thisGLContext_->create();
    }

    if (!contextCreated) {
        std::cout << "OpenGL context was not created successfully!" << std::endl;
        int major = sharedFormat_.majorVersion();
        int minor = sharedFormat_.minorVersion();
        std::cout << "GL Version: " << major << "." << minor << std::endl;
        std::cout << "GL Profile: " << (sharedFormat_.profile() == QSurfaceFormat::CoreProfile ? "Core" : "CompatibilityProfile") << std::endl;
        const GLubyte* vendor = glGetString(GL_VENDOR);
        std::string vendorStr = std::string((vendor!=nullptr ? reinterpret_cast<const char*>(vendor) : "INVALID"));
        std::cout << "GL Vendor: " << vendorStr << std::endl;
    }
}
#else
QGLWindow* CanvasQt::sharedGLContext_ = nullptr;

CanvasQt::CanvasQt(QGLParent* parent, uvec2 dim)
    : QGLWindow(sharedFormat_, parent, sharedGLContext_)
      , CanvasGL(dim)
      , swapBuffersAllowed_(false)
#ifndef QT_NO_GESTURES
      , gestureMode_(false)
      , lastType_(Qt::CustomGesture)
      , lastNumFingers_(0)
      , screenPositionNormalized_(vec2(0.f))
#endif
{
    //This is our default rendering context
    //Initialized once. So "THE" first object of this class will not have any shared context (or widget)
    //But Following objects, will share the context of initial object
    if (!sharedGLContext_) {
        sharedFormat_ = this->format();
        sharedGLContext_ = this;
        sharedCanvas_ = this;
        QGLWindow::glInit();
    }

    setAutoBufferSwap(false);
    setFocusPolicy(Qt::StrongFocus);

#ifndef QT_NO_GESTURES
    grabGesture(Qt::PanGesture);
    grabGesture(Qt::PinchGesture);
#endif
}
#endif

void CanvasQt::defineDefaultContextFormat(){
    if(!sharedGLContext_){
        std::string preferProfile = OpenGLCapabilities::getPreferredProfile();
        if(preferProfile == "core")
            sharedFormat_.setProfile(QGLContextFormat::CoreProfile);
        else if(preferProfile == "compatibility")
            sharedFormat_.setProfile(QGLContextFormat::CompatibilityProfile);
    }
}

CanvasQt::~CanvasQt() {}

void CanvasQt::initialize() {
    activate();
    CanvasGL::initialize();
}

void CanvasQt::initializeSquare() {
    activate();
    CanvasGL::initializeSquare();
}

void CanvasQt::deinitialize() {
    activate();
    CanvasGL::deinitialize();
}

void CanvasQt::activate() {
#ifdef USE_QWINDOW
    thisGLContext_->makeCurrent(this);
#else
    makeCurrent();
#endif
}

void CanvasQt::initializeGL() {
    OpenGLCapabilities::initializeGLEW();
#ifndef USE_QWINDOW
    QGLWindow::initializeGL();
    activate();
#endif
}

void CanvasQt::glSwapBuffers() {
    if (swapBuffersAllowed_) {
        activate();
#if defined(USE_NEW_OPENGLWIDGET)
        this->context()->swapBuffers(this->context()->surface());
#elif defined(USE_QWINDOW)
        thisGLContext_->swapBuffers(this);
#else
        QGLWindow::swapBuffers();
#endif
    }
}

void CanvasQt::update() {
    CanvasGL::update();
}

void CanvasQt::repaint() {
#if !defined(USE_QWINDOW) && !defined(USE_NEW_OPENGLWIDGET)
    QGLWindow::updateGL();
#endif
}

void CanvasQt::paintGL() {
#ifdef USE_QWINDOW
    if (!isExposed())
        return;
#endif

    swapBuffersAllowed_ = true;
    CanvasGL::update();
}

bool CanvasQt::event(QEvent *e) {
    switch (e->type()) {
    case QEvent::KeyPress:
        keyPressEvent(static_cast<QKeyEvent*>(e));
        return true;
    case QEvent::KeyRelease:
        keyReleaseEvent(static_cast<QKeyEvent*>(e));
        return true;
    case QEvent::MouseButtonPress:
        if(!gestureMode_)
            mousePressEvent(static_cast<QMouseEvent*>(e));
        return true;
    case QEvent::MouseButtonRelease:
        if(!gestureMode_)
            mouseReleaseEvent(static_cast<QMouseEvent*>(e));
        return true;
    case QEvent::MouseMove:
        if(!gestureMode_)
            mouseMoveEvent(static_cast<QMouseEvent*>(e));
        return true;
    case QEvent::Wheel:
        wheelEvent(static_cast<QWheelEvent*>(e));
        return true;
#ifndef QT_NO_GESTURES
    case QEvent::TouchBegin:
    case QEvent::TouchEnd:
    case QEvent::TouchUpdate:
        touchEvent(static_cast<QTouchEvent*>(e));
        return true;
    case QEvent::Gesture:
        return gestureEvent(static_cast<QGestureEvent*>(e));
#endif
#ifdef USE_QWINDOW
    case QEvent::UpdateRequest:
        paintGL();
        return true;
#endif
    default:
        return QGLWindow::event(e);
    }
}

void CanvasQt::mousePressEvent(QMouseEvent* e) {

#if !defined(QT_NO_GESTURES) && defined(USING_QT4)
    if (gestureMode_) return;
#endif
    ivec2 screenPos(e->pos().x(), e->pos().y());
    ivec2 screenPosInvY(screenPos.x, static_cast<int>(getScreenDimensions().y) - 1 - screenPos.y);
    MouseEvent mouseEvent(screenPos,
                           EventConverterQt::getMouseButton(e), MouseEvent::MOUSE_STATE_PRESS,
                           EventConverterQt::getModifier(e), getScreenDimensions(), 
                           getDepthValueAtCoord(screenPosInvY));
    e->accept();
    Canvas::mousePressEvent(&mouseEvent);
}

void CanvasQt::mouseReleaseEvent(QMouseEvent* e) {
#if !defined(QT_NO_GESTURES) && defined(USING_QT4)
    if (gestureMode_){ 
        gestureMode_ = false;
        return;
    }
#endif
    uvec2 screenPos(static_cast<unsigned int>(e->pos().x()), static_cast<unsigned int>(e->pos().y()));
    MouseEvent mouseEvent(screenPos,
        EventConverterQt::getMouseButtonCausingEvent(e), MouseEvent::MOUSE_STATE_RELEASE,
        EventConverterQt::getModifier(e), getScreenDimensions(),
        getDepthValueAtCoord(screenPos));
    e->accept();
    Canvas::mouseReleaseEvent(&mouseEvent);
}

void CanvasQt::mouseMoveEvent(QMouseEvent* e) {

#if !defined(QT_NO_GESTURES)
    if (gestureMode_) return;
#endif


    ivec2 screenPos(e->pos().x(), e->pos().y());
    ivec2 screenPosInvY(screenPos.x, static_cast<int>(getScreenDimensions().y) - 1 - screenPos.y);

    // Optimization, do not sample depth value when hovering, i.e. move without holding a mouse button
    int button = EventConverterQt::getMouseButton(e);
    double depth = 1.0;
    if (button != MouseEvent::MOUSE_BUTTON_NONE)
        depth = getDepthValueAtCoord(screenPosInvY);

    MouseEvent mouseEvent(screenPos, button, MouseEvent::MOUSE_STATE_MOVE,
                            EventConverterQt::getModifier(e), getScreenDimensions(),
                            depth);
    e->accept();
    Canvas::mouseMoveEvent(&mouseEvent);
}

void CanvasQt::wheelEvent(QWheelEvent* e){
    MouseEvent::MouseWheelOrientation orientation;
    if (e->orientation() == Qt::Horizontal) {
        orientation = MouseEvent::MOUSE_WHEEL_HORIZONTAL;
    } else {
        orientation = MouseEvent::MOUSE_WHEEL_VERTICAL;
    }

#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
    QPoint numPixels = e->pixelDelta();
    QPoint numDegrees = e->angleDelta() / 8 / 15;
#else
    QPoint numPixels;
    QPoint numDegrees = QPoint(0, e->delta() / 8 / 15);
#endif

    int numSteps = 0;
    if (!numPixels.isNull()) {
        numSteps = (orientation==MouseEvent::MOUSE_WHEEL_HORIZONTAL? numPixels.x() : numPixels.y()) / 5;
    } else if (!numDegrees.isNull()) {
        numSteps = (orientation==MouseEvent::MOUSE_WHEEL_HORIZONTAL? numDegrees.x() : numDegrees.y());
    }
    ivec2 screenPos(e->pos().x(), e->pos().y());
    ivec2 screenPosInvY(screenPos.x, static_cast<int>(getScreenDimensions().y) - 1 - screenPos.y);
    MouseEvent mouseEvent(screenPos, numSteps,
        EventConverterQt::getMouseWheelButton(e), MouseEvent::MOUSE_STATE_WHEEL, orientation,
        EventConverterQt::getModifier(e), getScreenDimensions(),
        getDepthValueAtCoord(screenPosInvY));
    e->accept();
    Canvas::mouseWheelEvent(&mouseEvent);
}

void CanvasQt::keyPressEvent(QKeyEvent* keyEvent) {
#ifdef USE_QWINDOW
    QWindow* parent = this->parent();
#else
    QWidget* parent = this->parentWidget();
#endif
    if (parent && keyEvent->key() == Qt::Key_F && keyEvent->modifiers() == Qt::ShiftModifier){
        if(parent->windowState() == Qt::WindowFullScreen) {
            parent->showNormal();
        } else {
            parent->showFullScreen();
        }
    }
    KeyboardEvent pressKeyEvent(EventConverterQt::getKeyButton(keyEvent),
                                 EventConverterQt::getModifier(keyEvent),
                                 KeyboardEvent::KEY_STATE_PRESS);
    keyEvent->accept();
    Canvas::keyPressEvent(&pressKeyEvent);
}

void CanvasQt::keyReleaseEvent(QKeyEvent* keyEvent) {
    KeyboardEvent releaseKeyEvent(EventConverterQt::getKeyButton(keyEvent),
                                  EventConverterQt::getModifier(keyEvent),
                                  KeyboardEvent::KEY_STATE_RELEASE);
    keyEvent->accept();
    Canvas::keyReleaseEvent(&releaseKeyEvent);
}

CanvasQt* CanvasQt::getSharedCanvas() { 
    return sharedCanvas_; 
}

#ifdef USE_QWINDOW
void CanvasQt::exposeEvent(QExposeEvent *e){
    Q_UNUSED(e);

    if (isExposed())
        paintGL();
}
#endif

#ifndef QT_NO_GESTURES

void CanvasQt::touchEvent(QTouchEvent* touch) {
    size_t nTouchPoints = touch->touchPoints().size();
    if (nTouchPoints < 1) {
        return;
    }

    QTouchEvent::TouchPoint firstPoint = touch->touchPoints()[0];
    
    switch (firstPoint.state())
    {
    case Qt::TouchPointPressed:
        gestureMode_ = nTouchPoints > 1; // Treat single touch point as mouse event
        break;
    case Qt::TouchPointMoved:
        gestureMode_ = nTouchPoints > 1; // Treat single touch point as mouse event
        break;
    case Qt::TouchPointStationary:
        gestureMode_ = nTouchPoints > 1; // Treat single touch point as mouse event
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
    const LayerRAM* depthLayerRAM = getDepthLayerRAM();
    vec2 screenSize(getScreenDimensions());

    std::vector<int> endedTouchIds;

    for (auto& touchPoint : touch->touchPoints()) {
        vec2 screenTouchPos(touchPoint.pos().x(), touchPoint.pos().y());
        vec2 prevScreenTouchPos(touchPoint.lastPos().x(), touchPoint.lastPos().y());
        TouchPoint::TouchState touchState;
        switch (touchPoint.state())
        {
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

        ivec2 pixelCoord = ivec2(static_cast<int>(screenTouchPos.x),
            screenSize.y - 1 - static_cast<int>(screenTouchPos.y));
        
        // Note that screenTouchPos/prevScreenTouchPos are in [0 screenDim] and does not need to be 
        // adjusted to become centered in the pixel (+0.5)
        
        // Saving id order to preserve order of touch points at next touch event
        
        const auto lastIdIdx = std::find(lastTouchIds_.begin(), lastTouchIds_.end(), touchPoint.id());

        if (lastIdIdx != lastTouchIds_.end()) {
            if (touchState == TouchPoint::TOUCH_STATE_ENDED){
                endedTouchIds.push_back(touchPoint.id());
            }
        }
        else{
            lastTouchIds_.push_back(touchPoint.id());


        }
        touchPoints.emplace_back(touchPoint.id(), screenTouchPos,
            (screenTouchPos) / screenSize,
            prevScreenTouchPos,
            (prevScreenTouchPos) / screenSize,
            touchState, getDepthValueAtCoord(pixelCoord, depthLayerRAM));
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

    auto touchIndex = 0; // Index to first unsorted element in touchPoints array
    for (const auto& lastTouchPointId : lastTouchIds_) {
        const auto touchPointIt = std::find_if(touchPoints.begin(), touchPoints.end(), [lastTouchPointId](const TouchPoint& p) { return p.getId() == lastTouchPointId; });
        // Swap current location in the container with the location it was in last touch event.
        if (touchPointIt != touchPoints.end() && std::distance(touchPoints.begin(), touchPointIt) != touchIndex) {
            std::swap(*(touchPoints.begin() + touchIndex), *touchPointIt);
            ++touchIndex;
        }
    }

    for (auto& endedId : endedTouchIds) {
        std::vector<int>::iterator foundIdx = std::find(lastTouchIds_.begin(), lastTouchIds_.end(), endedId);
        if (foundIdx != lastTouchIds_.end())
            lastTouchIds_.erase(foundIdx);
    }

    TouchEvent touchEvent(touchPoints, getScreenDimensions());
    touch->accept();

    // We need to send out touch event all the time to support one -> two finger touch switch
    Canvas::touchEvent(&touchEvent);

    // Mouse events will be triggered for touch events by Qt4 and Qt >= 5.3.0
    // https://bugreports.qt.io/browse/QTBUG-40038
#if defined(USING_QT5) && (QT_VERSION < QT_VERSION_CHECK(5, 3, 0))
    if(touch->touchPoints().size() == 1 && lastNumFingers_ < 2){
        MouseEvent* mouseEvent = nullptr;
        ivec2 pos = ivec2(static_cast<int>(firstPoint.pos().x()), static_cast<int>(firstPoint.pos().y()));
        uvec2 screenPosInvY(static_cast<unsigned int>(pos.x), static_cast<unsigned int>(getScreenDimensions().y-1-pos.y));
        double depth = getDepthValueAtCoord(screenPosInvY);
        switch (touchPoints.front().state())
        {
        case TouchPoint::TOUCH_STATE_STARTED:
            mouseEvent = new MouseEvent(pos, MouseEvent::MOUSE_BUTTON_LEFT, MouseEvent::MOUSE_STATE_PRESS, 
                EventConverterQt::getModifier(touch), getScreenDimensions(), depth);
            Canvas::mousePressEvent(mouseEvent);
            break;
        case TouchPoint::TOUCH_STATE_UPDATED:
            mouseEvent = new MouseEvent(pos, MouseEvent::MOUSE_BUTTON_LEFT, MouseEvent::MOUSE_STATE_MOVE, 
                EventConverterQt::getModifier(touch), getScreenDimensions(), depth);
            Canvas::mouseMoveEvent(mouseEvent);
            break;
        case TouchPoint::TOUCH_STATE_STATIONARY:
            break; // Do not fire event while standing still.
        case TouchPoint::TOUCH_STATE_ENDED:
            mouseEvent = new MouseEvent(pos, MouseEvent::MOUSE_BUTTON_LEFT, MouseEvent::MOUSE_STATE_RELEASE, 
                EventConverterQt::getModifier(touch), getScreenDimensions(), depth);
            Canvas::mouseReleaseEvent(mouseEvent);
            break;
        default:
            break;
        }
        delete mouseEvent;
    }
#endif

    lastNumFingers_ = static_cast<int>(touch->touchPoints().size());
    screenPositionNormalized_ = touchEvent.getCenterPointNormalized();
}

bool CanvasQt::gestureEvent(QGestureEvent* ge) {
    QGesture* gesture = nullptr;
    QPanGesture* panGesture = nullptr;
    QPinchGesture* pinchGesture = nullptr;

    if((gesture = ge->gesture(Qt::PanGesture))){
        panGesture = static_cast<QPanGesture *>(gesture);
    }
    if ((gesture = ge->gesture(Qt::PinchGesture))){
        pinchGesture = static_cast<QPinchGesture *>(gesture);
    }

    if(panGesture && pinchGesture){
        double absDeltaDist = glm::abs(static_cast<double>(pinchGesture->scaleFactor())-1.0);
        if(absDeltaDist > 0.05 || (lastType_ == Qt::PinchGesture || lastType_ != Qt::PanGesture)){
            lastType_ = Qt::PinchGesture;
            pinchTriggered(pinchGesture);
        }
        else{
            lastType_ = Qt::PanGesture;
            panTriggered(panGesture);
        }
    }
    else if(panGesture){
        lastType_ = Qt::PanGesture;
        panTriggered(panGesture);
    }
    else if(pinchGesture){
        double absDeltaDist = glm::abs(static_cast<double>(pinchGesture->scaleFactor())-1.0);
        if(absDeltaDist > 0.05)
            lastType_ = Qt::PinchGesture;
        pinchTriggered(pinchGesture);
    }

    ge->accept();
    return true;
}

void CanvasQt::panTriggered(QPanGesture* gesture) {

#ifndef QT_NO_CURSOR
    switch (gesture->state()) {
         case Qt::GestureStarted:
         case Qt::GestureUpdated:
             setCursor(Qt::SizeAllCursor);
             break;
         default:
             setCursor(Qt::ArrowCursor);
    }
#endif
    // Mouse events will be triggered for touch events by Qt 5.3.1 (even though we specify that the touch event is handled)
    // http://www.qtcentre.org/archive/index.php/t-52367.html
    // Therefore keep track of if we are performing and disallow mouse events while performing a gesture ( see CanvasQt::event(QEvent *e) )
    switch (gesture->state()) {
    case Qt::GestureStarted:
    case Qt::GestureUpdated:
        gestureMode_ = true;
        break;
    default:
        gestureMode_ = false;
    }
    vec2 deltaPos = vec2((gesture->lastOffset().x()-gesture->offset().x())/getScreenDimensions().x, (gesture->offset().y()-gesture->lastOffset().y())/getScreenDimensions().y);

    if(deltaPos == vec2(0.f))
        return;

    //std::cout << "PAN: " << deltaPos.x << ":" << deltaPos.y << std::endl;

    GestureEvent gestureEvent(deltaPos, 0.0, GestureEvent::PAN, EventConverterQt::getGestureState(gesture), lastNumFingers_, screenPositionNormalized_, getScreenDimensions());
    Canvas::gestureEvent(&gestureEvent);
}

void CanvasQt::pinchTriggered(QPinchGesture* gesture) { 
    //std::cout << "PINCH: " << gesture->scaleFactor() << std::endl;
    // Mouse events will be triggered for touch events by Qt 5.3.1 (even though we specify that the touch event is handled)
    // http://www.qtcentre.org/archive/index.php/t-52367.html
    // Therefore keep track of if we are performing and disallow mouse events while performing a gesture ( see CanvasQt::event(QEvent *e) )
    switch (gesture->state()) {
    case Qt::GestureStarted:
    case Qt::GestureUpdated:
        gestureMode_ = true;
        break;
    default:
        gestureMode_ = false;
    }
    GestureEvent gestureEvent(vec2(gesture->centerPoint().x(), gesture->centerPoint().y()), static_cast<double>(gesture->scaleFactor())-1.0,
        GestureEvent::PINCH, EventConverterQt::getGestureState(gesture), lastNumFingers_, screenPositionNormalized_, getScreenDimensions());
    Canvas::gestureEvent(&gestureEvent);
}

void CanvasQt::resize(uvec2 size) {
    QGLWindow::resize(size.x, size.y);
    CanvasGL::resize(size);
}

void CanvasQt::resizeEvent(QResizeEvent* event) {
    if (event->spontaneous()) {
        QGLWindow::resizeEvent(event);
        return;
    }
    CanvasGL::resize(uvec2(event->size().width(), event->size().height()));
    QGLWindow::resizeEvent(event);
}

#endif

} // namespace