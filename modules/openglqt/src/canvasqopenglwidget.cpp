/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2021 Inviwo Foundation
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

#include <modules/openglqt/canvasqopenglwidget.h>

#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/datastructures/image/layerram.h>
#include <inviwo/core/processors/canvasprocessor.h>
#include <inviwo/core/processors/processorwidget.h>
#include <inviwo/core/util/rendercontext.h>
#include <inviwo/core/network/networklock.h>

#include <inviwo/core/interaction/events/viewevent.h>

#include <modules/qtwidgets/inviwoqtutils.h>
#include <modules/openglqt/hiddencanvasqt.h>
#include <modules/opengl/openglcapabilities.h>
#include <modules/qtwidgets/inviwoqtutils.h>
#include <modules/qtwidgets/eventconverterqt.h>
#include <modules/openglqt/interactioneventmapperqt.h>

#include <warn/push>
#include <warn/ignore/all>

#include <QApplication>
#include <QOpenGLContext>
#include <QWindow>
#include <QMenu>
#include <QAction>
#include <QResizeEvent>
#include <QMouseEvent>
#include <warn/pop>

namespace inviwo {

CanvasQOpenGLWidget::CanvasQOpenGLWidget(QWidget* parent, std::string_view name)
    : QOpenGLWidget{parent}, CanvasGL{}, name_{name} {

    setFocusPolicy(Qt::StrongFocus);

    grabGesture(Qt::PanGesture);
    grabGesture(Qt::PinchGesture);

    setMouseTracking(true);
    setAttribute(Qt::WA_OpaquePaintEvent);

    installEventFilter(new utilqt::WidgetCloseEventFilter(this));
    installEventFilter(new InteractionEventMapperQt(
        this, this, [this]() { return getCanvasDimensions(); },
        [this]() { return getImageDimensions(); },
        [this](dvec2 pos) { return getDepthValueAtNormalizedCoord(pos); },
        [this](QMouseEvent* e) { doContextMenu(e); }));
}

CanvasQOpenGLWidget::~CanvasQOpenGLWidget() {
    RenderContext::getPtr()->unRegisterContext(contextId());
}

void CanvasQOpenGLWidget::activate() { makeCurrent(); }

void CanvasQOpenGLWidget::initializeGL() {
    OpenGLCapabilities::initializeGLEW();
    // QOpenGLWidget docs:
    // There is no need to call makeCurrent() because this has already been done
    // when this function is called.
    // Note however that the framebuffer is not yet available at this stage,
    // so do not issue draw calls from here.
    // Defer such calls to paintGL() instead.

    QOpenGLWidget::initializeGL();

    RenderContext::getPtr()->registerContext(contextId(), name_,
                                             std::make_unique<CanvasContextHolder>(this));
    setupDebug();
}

void CanvasQOpenGLWidget::glSwapBuffers() {
    // Do nothing:
    // QOpenGLWidget will swap buffers after paintGL and we are calling this from CanvasGL::update()
    // QOpenGLWidget docs:
    // triggering a buffer swap just for the QOpenGLWidget is not possible since there is no real,
    // onscreen native surface for it.
    // Instead, it is up to the widget stack to manage composition and buffer swaps on the gui
    // thread. When a thread is done updating the framebuffer, call update() on the GUI/main thread
    // to schedule composition.
}

void CanvasQOpenGLWidget::update() {
    QOpenGLWidget::update();  // this will trigger a paint event.
}

void CanvasQOpenGLWidget::paintGL() { CanvasGL::update(); }

void CanvasQOpenGLWidget::render(std::shared_ptr<const Image> image, LayerType layerType,
                                 size_t idx) {
    if (isVisible() && isValid()) {
        CanvasGL::render(image, layerType, idx);
    }
}

Canvas::ContextID CanvasQOpenGLWidget::activeContext() const {
    return static_cast<ContextID>(QOpenGLContext::currentContext());
}
Canvas::ContextID CanvasQOpenGLWidget::contextId() const {
    return static_cast<ContextID>(context());
}

std::unique_ptr<Canvas> CanvasQOpenGLWidget::createHiddenCanvas() {
    return HiddenCanvasQt::createHiddenQtCanvas();
}

void CanvasQOpenGLWidget::resizeEvent(QResizeEvent* event) {
    if (event->spontaneous()) {
        return;
    }
    image_.reset();
    pickingController_.setPickingSource(nullptr);

    setUpdatesEnabled(false);
    util::OnScopeExit enable([&]() { setUpdatesEnabled(true); });
    QOpenGLWidget::resizeEvent(event);
}

void CanvasQOpenGLWidget::releaseContext() {
    doneCurrent();
    context()->moveToThread(QApplication::instance()->thread());
}

void CanvasQOpenGLWidget::doContextMenu(QMouseEvent* event) {
    if (auto canvasProcessor = dynamic_cast<CanvasProcessor*>(ownerWidget_->getProcessor())) {
        if (!canvasProcessor->isContextMenuAllowed()) return;
    }

    QMenu menu(this);
    if (auto procssor = ownerWidget_->getProcessor()) {
        connect(menu.addAction(QIcon(":svgicons/edit-selectall.svg"), "&Select Processor"),
                &QAction::triggered, this, [procssor]() {
                    procssor->getMetaData<ProcessorMetaData>(ProcessorMetaData::CLASS_IDENTIFIER)
                        ->setSelected(true);
                });
    }
    connect(menu.addAction(QIcon(":svgicons/canvas-hide.svg"), "&Hide Canvas"), &QAction::triggered,
            this, [&]() { ownerWidget_->setVisible(false); });

    connect(menu.addAction(QIcon(":svgicons/fullscreen.svg"), "&Toggle Full Screen"),
            &QAction::triggered, this,
            [&]() { ownerWidget_->setFullScreen(!ownerWidget_->isFullScreen()); });

    if (auto image = image_.lock()) {
        menu.addSeparator();
        utilqt::addImageActions(menu, *image, layerType_, layerIdx_);
    }

    {
        menu.addSeparator();
        auto prop = [&](auto action) {
            return [this, action]() {
                ViewEvent e{action};
                propagateEvent(&e, nullptr);
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

size2_t CanvasQOpenGLWidget::getCanvasDimensions() const {
    const auto dpr = window()->devicePixelRatio();
    return dpr * utilqt::toGLM(size());
}

void CanvasQOpenGLWidget::propagateEvent(Event* e, Outport* source) {
    if (!propagator_) return;
    NetworkLock lock;
    pickingController_.propagateEvent(e, propagator_);
    if (e->hasBeenUsed()) return;
    propagator_->propagateEvent(e, source);
}

}  // namespace inviwo
