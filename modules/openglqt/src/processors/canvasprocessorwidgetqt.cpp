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

#include <modules/openglqt/processors/canvasprocessorwidgetqt.h>
#include <inviwo/core/processors/canvasprocessor.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/util/raiiutils.h>
#include <inviwo/core/util/rendercontext.h>
#include <inviwo/core/network/networklock.h>
#include <inviwo/core/interaction/events/resizeevent.h>
#include <modules/qtwidgets/inviwoqtutils.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QResizeEvent>
#include <QMoveEvent>
#include <QGridLayout>
#include <QApplication>
#include <QAction>
#include <QMenu>
#include <warn/pop>

namespace inviwo {

CanvasProcessorWidgetQt::CanvasProcessorWidgetQt(Processor* p)
    : CanvasProcessorWidget(p), QWidget{utilqt::getApplicationMainWindow(), Qt::Window}, canvas_{} {

    setWindowTitle(utilqt::toQString(p->getDisplayName()));
    nameChange_ = p->onDisplayNameChange([this](std::string_view newName, std::string_view) {
        setWindowTitle(utilqt::toQString(newName));
        RenderContext::getPtr()->setContextName(canvas_->contextId(), newName);
    });

    setMinimumSize(32, 32);
    setFocusPolicy(Qt::NoFocus);
    setAttribute(Qt::WA_OpaquePaintEvent);

    const ivec2 pysicalDim = CanvasProcessorWidget::getDimensions();
    const ivec2 pos = CanvasProcessorWidget::getPosition();

    const auto dpr = window()->devicePixelRatio();
    const ivec2 logicalDim = pysicalDim / dpr;

    canvas_ = std::unique_ptr<CanvasQOpenGLWidget, std::function<void(CanvasQOpenGLWidget*)>>(
        new CanvasQOpenGLWidget(nullptr, p->getDisplayName()), [&](CanvasQOpenGLWidget* c) {
            c->activate();
            layout()->removeWidget(c);
            delete c;
            RenderContext::getPtr()->activateDefaultRenderContext();
        });
    canvas_->setEventPropagator(p);
    canvas_->onContextMenu([this](QMenu& menu) { return contextMenu(menu); });

    QGridLayout* gridLayout = new QGridLayout(this);
    gridLayout->setContentsMargins(0, 0, 0, 0);
    gridLayout->addWidget(canvas_.get(), 0, 0);

    if (auto mainWindow = utilqt::getApplicationMainWindow()) {
        // Move widget relative to main window to make sure that it is visible on screen.
        QPoint newPos =
            utilqt::movePointOntoDesktop(utilqt::toQPoint(pos), utilqt::toQSize(logicalDim), true);

        if (!(newPos.x() == 0 && newPos.y() == 0)) {
            util::KeepTrueWhileInScope ignore(&ignoreEvents_);
            // prevent move events, since this will automatically save the "adjusted" position.
            // The processor widget already has its correct pos, i.e. the one de-serialized from
            // file.
            Super::move(newPos);
        } else {  // We guess that this is a new widget and give a new position
            newPos = mainWindow->pos();
            newPos += utilqt::offsetWidget();
            Super::move(newPos);
        }
    }

    {
        // Trigger both resize event and move event by showing and hiding the widget
        // in order to set the correct, i.e. the de-serialized, size and position.
        //
        // Otherwise, a spontaneous event will be triggered which will set the widget
        // to its "initial" size of 160 by 160 at (0, 0) thereby overwriting our values.
        util::KeepTrueWhileInScope ignore(&ignoreEvents_);
        Super::setVisible(true);
        resize(static_cast<int>(logicalDim.x), static_cast<int>(logicalDim.y));

        Super::setVisible(false);
    }

    setWindowFlag(Qt::WindowStaysOnTopHint, CanvasProcessorWidget::isOnTop());
    connect(qApp, &QApplication::applicationStateChanged, this, [this](Qt::ApplicationState state) {
        utilqt::setOnTop(this, isOnTop() && state == Qt::ApplicationActive);
    });

    {
        // ignore internal state updates, i.e. position, when showing the widget
        // On Windows, the widget hasn't got a decoration yet. So it will be positioned using the
        // decoration offset, i.e. the "adjusted" position.
        util::KeepTrueWhileInScope ignore(&ignoreEvents_);
        Super::setVisible(ProcessorWidget::isVisible());
    }
    RenderContext::getPtr()->activateDefaultRenderContext();

    installEventFilter(new utilqt::WidgetCloseEventFilter(this));
}

CanvasProcessorWidgetQt::~CanvasProcessorWidgetQt() = default;

void CanvasProcessorWidgetQt::setVisible(bool visible) {
    if (Super::isVisible() != visible) {
        Super::setVisible(visible);  // This will trigger show/hide events.
    }
}

void CanvasProcessorWidgetQt::setFullScreen(bool fullScreen) {
    if (fullScreen != Super::isFullScreen()) {
        utilqt::setFullScreen(this, fullScreen);
    }
    CanvasProcessorWidget::setFullScreen(fullScreen);
}

void CanvasProcessorWidgetQt::setOnTop(bool onTop) {
    utilqt::setOnTop(this, onTop);
    CanvasProcessorWidget::setOnTop(onTop);
}

void CanvasProcessorWidgetQt::setPosition(ivec2 pos) {
    if (pos != utilqt::toGLM(Super::pos())) {
        Super::move(pos.x, pos.y);  // This will trigger a move event.
    }
}

void CanvasProcessorWidgetQt::setDimensions(ivec2 dimensions) {
    if (dimensions != utilqt::toGLM(Super::size())) {
        const auto dpr = window()->devicePixelRatio();
        const ivec2 logicalDim = dimensions / dpr;
        resize(logicalDim.x, logicalDim.y);  // This will trigger a resize event.
    }
}

Canvas* CanvasProcessorWidgetQt::getCanvas() const { return canvas_.get(); }

void CanvasProcessorWidgetQt::resizeEvent(QResizeEvent* event) {
    Super::resizeEvent(event);
    util::KeepTrueWhileInScope ignore(&resizeOngoing_);
    propagateResizeEvent();
}

void CanvasProcessorWidgetQt::propagateResizeEvent() {
    auto previousCanvasDimensions = canvasDimensions_;

    const auto dpr = window()->devicePixelRatio();
    canvasDimensions_ = dpr * utilqt::toGLM(canvas_->size());
    CanvasProcessorWidget::setDimensions(canvasDimensions_);

    NetworkLock lock;
    RenderContext::getPtr()->activateDefaultRenderContext();
    ResizeEvent resizeEvent(canvasDimensions_, previousCanvasDimensions);
    getProcessor()->propagateEvent(&resizeEvent, nullptr);
}

bool CanvasProcessorWidgetQt::contextMenu(QMenu& menu) {
    if (auto canvasProcessor = dynamic_cast<CanvasProcessor*>(getProcessor())) {
        if (!canvasProcessor->isContextMenuAllowed()) return false;
    }

    connect(menu.addAction(QIcon(":svgicons/edit-selectall.svg"), "&Select Processor"),
            &QAction::triggered, this, [this]() {
                getProcessor()
                    ->getMetaData<ProcessorMetaData>(ProcessorMetaData::CLASS_IDENTIFIER)
                    ->setSelected(true);
            });
    connect(menu.addAction(QIcon(":svgicons/canvas-hide.svg"), "&Hide Canvas"), &QAction::triggered,
            this, [&]() { setVisible(false); });

    connect(menu.addAction(QIcon(":svgicons/fullscreen.svg"), "&Toggle Full Screen"),
            &QAction::triggered, this, [&]() { setFullScreen(!Super::isFullScreen()); });

    auto ontop = menu.addAction("On Top");
    ontop->setCheckable(true);
    ontop->setChecked(isOnTop());
    connect(ontop, &QAction::triggered, this, [&]() { setOnTop(!isOnTop()); });

    menu.addSeparator();
    utilqt::addViewActions(menu, getProcessor());
    return true;
}

void CanvasProcessorWidgetQt::showEvent(QShowEvent* event) {
    if (ignoreEvents_) return;
    CanvasProcessorWidget::setVisible(true);
    Super::showEvent(event);
}

void CanvasProcessorWidgetQt::hideEvent(QHideEvent* event) {
    if (ignoreEvents_) return;
    CanvasProcessorWidget::setVisible(false);
    Super::hideEvent(event);
}

void CanvasProcessorWidgetQt::moveEvent(QMoveEvent* event) {
    if (ignoreEvents_) return;
    CanvasProcessorWidget::setPosition(utilqt::toGLM(event->pos()));
    Super::moveEvent(event);
}

void CanvasProcessorWidgetQt::updateVisible(bool visible) {
    util::KeepTrueWhileInScope ignore(&ignoreEvents_);
    setVisible(visible);
}
void CanvasProcessorWidgetQt::updateDimensions(ivec2 dim) {
    if (resizeOngoing_) return;
    util::KeepTrueWhileInScope ignore(&ignoreEvents_);
    setDimensions(dim);
}
void CanvasProcessorWidgetQt::updatePosition(ivec2 pos) {
    util::KeepTrueWhileInScope ignore(&ignoreEvents_);
    setPosition(pos);
}

void CanvasProcessorWidgetQt::updateFullScreen(bool fullScreen) {
    if (fullScreen != Super::isFullScreen()) {
        utilqt::setFullScreen(this, fullScreen);
    }
}

void CanvasProcessorWidgetQt::updateOnTop(bool onTop) { utilqt::setOnTop(this, onTop); }

}  // namespace inviwo
