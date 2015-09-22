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
#include <modules/opengl/canvasgl.h>
#include <inviwo/qt/widgets/eventconverterqt.h>
#include <inviwo/core/network/processornetworkevaluator.h>
#include <inviwo/core/common/inviwo.h>

#define QT_NO_OPENGL_ES_2
#define GLEXT_64_TYPES_DEFINED
#include <QInputEvent>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QEvent>

#ifndef QT_NO_GESTURES
#include <QGestureEvent>
#include <QPanGesture>
#include <QPinchGesture>
#endif

//#if (QT_VERSION >= QT_VERSION_CHECK(5, 4, 0))
//#define USE_NEW_OPENGLWIDGET
//#endif

#ifdef USE_NEW_OPENGLWIDGET
#include <QOpenGLWidget>
#define QGLWindow QOpenGLWidget
#define QGLParent QWidget
#define QGLContextFormat QSurfaceFormat
#elif USE_QWINDOW
#include <QtGui/QWindow>
#include <QtGui/QSurfaceFormat>
class QOpenGLContext;
#define QGLWindow QWindow
#define QGLParent QWindow
#define QGLContextFormat QSurfaceFormat
#else
#include <QGLWidget>
#define QGLWindow QGLWidget
#define QGLParent QWidget
#define QGLContextFormat QGLFormat
#endif


namespace inviwo {

class IVW_MODULE_OPENGLQT_API CanvasQt : public QGLWindow, public CanvasGL {
    #include <warn/push>
    #include <warn/ignore/all>
    Q_OBJECT
    #include <warn/pop>

    friend class CanvasProcessorWidgetQt;

public:
    explicit CanvasQt(QGLParent* parent = nullptr, uvec2 dim = uvec2(256,256));
    ~CanvasQt();

    static void defineDefaultContextFormat();

    virtual void initialize() override;
    virtual void initializeSquare() override;
    virtual void deinitialize() override;
    virtual void activate() override;
    virtual void glSwapBuffers() override;
    virtual void update() override;
    void repaint();

    virtual void resize(uvec2 size) override;

    virtual std::unique_ptr<Canvas> create() override;

protected:
    void initializeGL() override;
    void paintGL() override;

    virtual bool event(QEvent *e) override;
    virtual void mousePressEvent(QMouseEvent* e) override;
    virtual void mouseReleaseEvent(QMouseEvent* e) override;
    virtual void mouseMoveEvent(QMouseEvent* e) override;
    virtual void wheelEvent (QWheelEvent* e) override;
    virtual void keyPressEvent(QKeyEvent* keyEvent) override;
    virtual void keyReleaseEvent(QKeyEvent* keyEvent) override;

    virtual void resizeEvent(QResizeEvent* event) override;

    static CanvasQt* getSharedCanvas();

#ifdef USE_QWINDOW
    void exposeEvent(QExposeEvent *event);

private:
    QOpenGLContext* thisGLContext_;

    static QOpenGLContext* sharedGLContext_; //For rendering-context sharing
#else
private:
    static QGLWindow* sharedGLContext_; //For rendering-context sharing

#endif
    static CanvasQt* sharedCanvas_;
    static QGLContextFormat sharedFormat_;
    bool swapBuffersAllowed_;

#ifndef QT_NO_GESTURES
    void touchEvent(QTouchEvent*);
    bool gestureEvent(QGestureEvent*);
    void panTriggered(QPanGesture*);
    void pinchTriggered(QPinchGesture*);
    bool gestureMode_;
    Qt::GestureType lastType_;
    int lastNumFingers_;
    std::vector<int> lastTouchIds_;
    vec2 screenPositionNormalized_;
#endif
};

} // namespace

#endif // IVW_CANVASQT_H