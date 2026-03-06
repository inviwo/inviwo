/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2026 Inviwo Foundation
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

#include <inviwo/core/interaction/events/resizeevent.h>
#include <inviwo/core/metadata/processormetadata.h>
#include <inviwo/core/network/networklock.h>
#include <inviwo/core/processors/canvasprocessor.h>
#include <inviwo/core/processors/canvasprocessorwidget.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/processors/processorwidget.h>
#include <inviwo/core/util/glmvec.h>
#include <inviwo/core/util/raiiutils.h>
#include <inviwo/core/util/rendercontext.h>
#include <modules/openglqt/canvasqopenglwidget.h>
#include <modules/qtwidgets/inviwoqtutils.h>

#include <string>
#include <string_view>

#include <QAction>
#include <QGridLayout>
#include <QIcon>
#include <QMainWindow>
#include <QMenu>
#include <QMoveEvent>
#include <QPoint>
#include <Qt>
#include <glm/fwd.hpp>
#include <glm/gtx/scalar_multiplication.hpp>
#include <glm/vec2.hpp>

class QHideEvent;
class QMoveEvent;
class QResizeEvent;
class QShowEvent;

namespace inviwo {

class Canvas;

CanvasProcessorWidgetQt::CanvasProcessorWidgetQt(Processor* p)
    : CanvasProcessorWidget(p)
    , QMainWindow{utilqt::getApplicationMainWindow(),
                  CanvasProcessorWidget::isOnTop() ? Qt::Tool : Qt::Window}
    , canvas_{std::unique_ptr<CanvasQOpenGLWidget, std::function<void(CanvasQOpenGLWidget*)>>(
          new CanvasQOpenGLWidget(nullptr, p->getDisplayName()),
          [](CanvasQOpenGLWidget* c) {
              c->activate();
              c->setParent(nullptr);
              delete c;
              RenderContext::getPtr()->activateDefaultRenderContext();
          })}
    , nameChange_{p->onDisplayNameChange([this](std::string_view newName, std::string_view) {
        setWindowTitle(utilqt::toQString(newName));
        RenderContext::getPtr()->setContextName(canvas_->contextId(), newName);
    })} {

    setWindowTitle(utilqt::toQString(p->getDisplayName()));
    setDockNestingEnabled(true);

    setMinimumSize(32, 32);
    setFocusPolicy(Qt::NoFocus);
    setAttribute(Qt::WA_OpaquePaintEvent);
    setAttribute(Qt::WA_ShowWithoutActivating);

    const ivec2 pysicalDim = CanvasProcessorWidget::getDimensions();
    const ivec2 pos = CanvasProcessorWidget::getPosition();

    const auto dpr = window()->devicePixelRatio();
    const ivec2 logicalDim = pysicalDim / dpr;
    canvas_->setEventPropagator(this);
    canvas_->onContextMenu(
        [this](QMenu& menu, ContextMenuCategories actions) { return contextMenu(menu, actions); });

    setCentralWidget(canvas_.get());

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

    setAttribute(Qt::WA_MacAlwaysShowToolWindow, true);
    utilqt::setFullScreenAndOnTop(this, CanvasProcessorWidget::isFullScreen(),
                                  CanvasProcessorWidget::isOnTop());

    {
        // ignore internal state updates, i.e. position, when showing the widget
        // On Windows, the widget hasn't got a decoration yet. So it will be positioned using the
        // decoration offset, i.e. the "adjusted" position.
        const util::KeepTrueWhileInScope ignore(&ignoreEvents_);
        resize(logicalDim.x, logicalDim.y);
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
    utilqt::setFullScreenAndOnTop(this, fullScreen, CanvasProcessorWidget::isOnTop());
    CanvasProcessorWidget::setFullScreen(fullScreen);
}

void CanvasProcessorWidgetQt::setOnTop(bool onTop) {
    utilqt::setFullScreenAndOnTop(this, CanvasProcessorWidget::isFullScreen(), onTop);
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

void CanvasProcessorWidgetQt::resizeEvent(QResizeEvent* event) { Super::resizeEvent(event); }

void CanvasProcessorWidgetQt::propagateEvent(Event* event, Outport* source) {
    if (auto* re = event->getAs<ResizeEvent>()) {
        CanvasProcessorWidget::setDimensions(re->size());
    }

    getProcessor()->propagateEvent(event, source);
}

void CanvasProcessorWidgetQt::propagateResizeEvent() { canvas_->triggerResizeEventPropagation(); }

bool CanvasProcessorWidgetQt::contextMenu(QMenu& menu, ContextMenuCategories actions) {
    if (auto* canvasProcessor = dynamic_cast<CanvasProcessor*>(getProcessor())) {
        if (!canvasProcessor->isContextMenuAllowed()) return false;
    }

    if (actions.contains(ContextMenuCategory::Widget)) {
        if (!menu.actions().empty()) {
            menu.addSeparator();
        }
        connect(menu.addAction(QIcon(":svgicons/edit-selectall.svg"), "&Select Processor"),
                &QAction::triggered, this, [this]() {
                    getProcessor()
                        ->getMetaData<ProcessorMetaData>(ProcessorMetaData::classIdentifier)
                        ->setSelected(true);
                });
        connect(menu.addAction(QIcon(":svgicons/canvas-hide.svg"), "&Hide Canvas"),
                &QAction::triggered, this, [&]() { setVisible(false); });

        connect(menu.addAction(QIcon(":svgicons/fullscreen.svg"), "&Toggle Full Screen"),
                &QAction::triggered, this, [&]() { setFullScreen(!Super::isFullScreen()); });

        auto* ontop = menu.addAction("On Top");
        ontop->setCheckable(true);
        ontop->setChecked(isOnTop());
        connect(ontop, &QAction::triggered, this, [&]() { setOnTop(!isOnTop()); });
    }

    if (actions.contains(ContextMenuCategory::View)) {
        if (!menu.actions().empty()) {
            menu.addSeparator();
        }
        utilqt::addViewActions(menu, getProcessor());
    }
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
    util::KeepTrueWhileInScope ignore(&ignoreEvents_);
    setDimensions(dim);
}
void CanvasProcessorWidgetQt::updatePosition(ivec2 pos) {
    util::KeepTrueWhileInScope ignore(&ignoreEvents_);
    setPosition(pos);
}

void CanvasProcessorWidgetQt::updateFullScreen(bool fullScreen) {
    utilqt::setFullScreenAndOnTop(this, fullScreen, CanvasProcessorWidget::isOnTop());
}

void CanvasProcessorWidgetQt::updateOnTop(bool onTop) {
    utilqt::setFullScreenAndOnTop(this, CanvasProcessorWidget::isFullScreen(), onTop);
}

}  // namespace inviwo
