/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 * Version 0.9
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
#include <modules/opengl/openglcapabilities.h>
#if defined(USE_NEW_OPENGLWIDGET)
#include <QOpenGLContext>
#elif defined(USE_QWINDOW)
#include <QtGui/QOpenGLContext>
#endif

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
CanvasQt* CanvasQt::sharedCanvas_ = NULL;

#if defined(USE_NEW_OPENGLWIDGET)
QGLWindow* CanvasQt::sharedGLContext_ = NULL;

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
QOpenGLContext* CanvasQt::sharedGLContext_ = NULL;

CanvasQt::CanvasQt(QGLParent* parent, uvec2 dim)
    : QGLWindow(parent)
    , CanvasGL(dim)
    , thisGLContext_(NULL)
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
        std::string vendorStr = std::string((vendor!=NULL ? reinterpret_cast<const char*>(vendor) : "INVALID"));
        std::cout << "GL Vendor: " << vendorStr << std::endl;
    }
}
#else
QGLWindow* CanvasQt::sharedGLContext_ = NULL;

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

    MouseEvent mouseEvent(ivec2(e->pos().x(), e->pos().y()),
                           EventConverterQt::getMouseButton(e), MouseEvent::MOUSE_STATE_PRESS,
                           EventConverterQt::getModifier(e), getScreenDimensions());
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

    MouseEvent mouseEvent(ivec2(e->pos().x(), e->pos().y()),
                          EventConverterQt::getMouseButton(e),MouseEvent::MOUSE_STATE_RELEASE,
                          EventConverterQt::getModifier(e), getScreenDimensions());
    e->accept();
    Canvas::mouseReleaseEvent(&mouseEvent);
}

void CanvasQt::mouseMoveEvent(QMouseEvent* e) {

#if !defined(QT_NO_GESTURES)
    if (gestureMode_) return;
#endif


    if (e->buttons() == Qt::LeftButton || e->buttons() == Qt::RightButton || e->buttons() == Qt::MiddleButton) {
        MouseEvent mouseEvent(ivec2(e->pos().x(), e->pos().y()),
                              EventConverterQt::getMouseButton(e), MouseEvent::MOUSE_STATE_MOVE,
                              EventConverterQt::getModifier(e), getScreenDimensions());
        e->accept();
        Canvas::mouseMoveEvent(&mouseEvent);
    }
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

    MouseEvent mouseEvent(ivec2(e->pos().x(), e->pos().y()), numSteps,
        EventConverterQt::getMouseWheelButton(e), MouseEvent::MOUSE_STATE_WHEEL, orientation,
        EventConverterQt::getModifier(e), getScreenDimensions());
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
    QTouchEvent::TouchPoint firstPoint = touch->touchPoints()[0];
    ivec2 pos = ivec2(static_cast<int>(glm::floor(firstPoint.pos().x())), static_cast<int>(glm::floor(firstPoint.pos().y())));
    TouchEvent::TouchState touchState;

    switch (firstPoint.state())
    {
    case Qt::TouchPointPressed:
        touchState = TouchEvent::TOUCH_STATE_STARTED;
        break;
    case Qt::TouchPointMoved:
        touchState = TouchEvent::TOUCH_STATE_UPDATED;
        break;
    case Qt::TouchPointReleased:
        touchState = TouchEvent::TOUCH_STATE_ENDED;
        break;
    default:
        touchState = TouchEvent::TOUCH_STATE_NONE;
    }

    TouchEvent touchEvent(pos, touchState);
    touch->accept();
    Canvas::touchEvent(&touchEvent);

    // Mouse events will be triggered for touch events by Qt4 and Qt >= 5.3.0
    // https://bugreports.qt.io/browse/QTBUG-40038
#if defined(USING_QT5) && (QT_VERSION < QT_VERSION_CHECK(5, 3, 0))
    if(touch->touchPoints().size() == 1 && lastNumFingers_ < 2){
        MouseEvent* mouseEvent = NULL;
        switch (touchState)
        {
        case TouchEvent::TOUCH_STATE_STARTED:
            mouseEvent = new MouseEvent(pos, MouseEvent::MOUSE_BUTTON_LEFT, MouseEvent::MOUSE_STATE_PRESS, 
                EventConverterQt::getModifier(touch), getScreenDimensions());
            Canvas::mousePressEvent(mouseEvent);
            break;
        case TouchEvent::TOUCH_STATE_UPDATED:
            mouseEvent = new MouseEvent(pos, MouseEvent::MOUSE_BUTTON_LEFT, MouseEvent::MOUSE_STATE_MOVE, 
                EventConverterQt::getModifier(touch), getScreenDimensions());
            Canvas::mouseMoveEvent(mouseEvent);
            break;
        case TouchEvent::TOUCH_STATE_ENDED:
            mouseEvent = new MouseEvent(pos, MouseEvent::MOUSE_BUTTON_LEFT, MouseEvent::MOUSE_STATE_RELEASE, 
                EventConverterQt::getModifier(touch), getScreenDimensions());
            Canvas::mouseReleaseEvent(mouseEvent);
            break;
        default:
            break;
        }
        delete mouseEvent;
    }
#endif

    lastNumFingers_ = static_cast<int>(touch->touchPoints().size());

    if(lastNumFingers_ > 1){
        vec2 fpos = vec2(0.f);
        for(int i=0; i<touch->touchPoints().size(); i++)
            fpos += vec2(touch->touchPoints().at(i).pos().x(), glm::floor(touch->touchPoints().at(i).pos().y()));

        fpos /= static_cast<float>(touch->touchPoints().size());

        screenPositionNormalized_ = vec2(glm::floor(fpos.x)/getScreenDimensions().x, glm::floor(fpos.y)/getScreenDimensions().y);
    }
    else{
        screenPositionNormalized_ = vec2(static_cast<float>(pos.x)/getScreenDimensions().x, static_cast<float>(pos.y)/getScreenDimensions().y);
    }
}

bool CanvasQt::gestureEvent(QGestureEvent* ge) {
    QGesture* gesture = NULL;
    QPanGesture* panGesture = NULL;
    QPinchGesture* pinchGesture = NULL;

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