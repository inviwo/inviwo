/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2021-2022 Inviwo Foundation
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

#include <modules/openglqt/processors/canvaswithpropertiesprocessorwidgetqt.h>

#include <inviwo/core/common/inviwoapplication.h>          // for InviwoApplication
#include <inviwo/core/interaction/events/resizeevent.h>    // for ResizeEvent
#include <inviwo/core/metadata/processormetadata.h>        // for ProcessorMetaData, ProcessorMe...
#include <inviwo/core/network/networklock.h>               // for NetworkLock
#include <inviwo/core/network/processornetwork.h>          // for ProcessorNetwork
#include <inviwo/core/processors/canvasprocessorwidget.h>  // for CanvasProcessorWidget
#include <inviwo/core/processors/processor.h>              // for Processor
#include <inviwo/core/util/glmvec.h>                       // for ivec2, size2_t
#include <inviwo/core/util/logcentral.h>                   // for LogCentral, LogWarn
#include <inviwo/core/util/rendercontext.h>                // for RenderContext
#include <inviwo/core/util/stringconversion.h>             // for forEachStringPart, splitByFirst
#include <modules/openglqt/canvasqopenglwidget.h>          // for CanvasQOpenGLWidget
#include <modules/qtwidgets/inviwoqtutils.h>               // for setFullScreenAndOnTop, toGLM
#include <modules/qtwidgets/propertylistwidget.h>          // for PropertyListFrame

#include <cstddef>  // for size_t
#include <ostream>  // for operator<<, basic_ostream

#include <QAction>                            // for QAction
#include <QEvent>                             // for QEvent, QEvent::WindowStateChange
#include <QFrame>                             // for QFrame, QFrame::NoFrame
#include <QIcon>                              // for QIcon
#include <QMenu>                              // for QMenu
#include <QMoveEvent>                         // for QMoveEvent
#include <QPoint>                             // for QPoint
#include <QScrollArea>                        // for QScrollArea
#include <QSizePolicy>                        // for QSizePolicy, QSizePolicy::Mini...
#include <QSplitter>                          // for QSplitter
#include <QWidget>                            // for QWidget
#include <Qt>                                 // for ScrollBarAsNeeded, Tool, WA_Ma...
#include <glm/fwd.hpp>                        // for vec2
#include <glm/gtx/scalar_multiplication.hpp>  // for operator*
#include <glm/vec2.hpp>                       // for vec<>::(anonymous)

namespace inviwo {

class Canvas;

CanvasWithPropertiesProcessorWidgetQt::CanvasWithPropertiesProcessorWidgetQt(Processor* p)
    : CanvasProcessorWidget{p}
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

    const ivec2 dim = CanvasProcessorWidget::getDimensions();
    const ivec2 pos = CanvasProcessorWidget::getPosition();
    const auto space = utilqt::emToPx(this, utilqt::refSpaceEm());

    auto splitter = new QSplitter();

    canvas_->setEventPropagator(p);
    canvas_->setMinimumSize(16, 16);
    canvas_->onContextMenu([this](QMenu& menu) { return contextMenu(menu); });

    splitter->addWidget(canvas_.get());

    auto scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    scrollArea->setMinimumWidth(utilqt::emToPx(this, 30));
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
#ifdef __APPLE__
    // Scrollbars are overlayed in different way on mac...
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
#else
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
#endif
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setContentsMargins(0, space, 0, space);

    auto app = p->getNetwork()->getApplication();
    frame_ = new PropertyListFrame(nullptr, app->getPropertyWidgetFactory());
    frame_->setContentsMargins(space, 0, 0, 0);
    scrollArea->setWidget(frame_);
    splitter->addWidget(scrollArea);

    connect(splitter, &QSplitter::splitterMoved, this,
            &CanvasWithPropertiesProcessorWidgetQt::propagateResizeEvent);

    QSizePolicy sp(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    sp.setVerticalStretch(5);
    sp.setHorizontalStretch(5);
    canvas_->setSizePolicy(sp);

    setCentralWidget(splitter);

    if (auto mainWindow = utilqt::getApplicationMainWindow()) {
        // Move widget relative to main window to make sure that it is visible on screen.
        QPoint newPos =
            utilqt::movePointOntoDesktop(utilqt::toQPoint(pos), utilqt::toQSize(dim), true);

        if (!(newPos.x() == 0 && newPos.y() == 0)) {
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

    resize(dim.x, dim.y);
    setAttribute(Qt::WA_MacAlwaysShowToolWindow, true);

    utilqt::setFullScreenAndOnTop(this, CanvasProcessorWidget::isFullScreen(),
                                  CanvasProcessorWidget::isOnTop());

    Super::setVisible(CanvasProcessorWidget::isVisible());

    installEventFilter(new utilqt::WidgetCloseEventFilter(this));
}

void CanvasWithPropertiesProcessorWidgetQt::setProperties(std::string_view paths) {
    bool needUpdate = false;
    util::forEachStringPart(paths, "\n", [&, i = size_t{0}](std::string_view path) mutable {
        if (i >= addedPaths_.size() || addedPaths_[i] != path) needUpdate |= true;
        ++i;
    });
    if (!needUpdate) return;

    addedPaths_.clear();
    frame_->clear();
    if (auto net = getProcessor()->getNetwork()) {
        util::forEachStringPart(paths, "\n", [&](std::string_view path) {
            auto [proc, prop] = util::splitByFirst(path, ".");
            if (auto processor = net->getProcessorByIdentifier(proc)) {
                if (prop.empty()) {
                    frame_->add(processor);
                    addedPaths_.emplace_back(path);
                } else if (auto property = processor->getPropertyByPath(prop)) {
                    frame_->add(property);
                    addedPaths_.emplace_back(path);
                } else {
                    LogWarn("Property: " << prop << " not found in processor " << proc);
                }
            } else {
                LogWarn("Processor: " << proc << " not found");
            }
        });
    }
}

Canvas* CanvasWithPropertiesProcessorWidgetQt::getCanvas() const { return canvas_.get(); }

void CanvasWithPropertiesProcessorWidgetQt::resizeEvent(QResizeEvent* event) {
    Super::resizeEvent(event);
    propagateResizeEvent();
}

void CanvasWithPropertiesProcessorWidgetQt::showEvent(QShowEvent* event) {
    CanvasProcessorWidget::setVisible(true);
    Super::showEvent(event);
}
void CanvasWithPropertiesProcessorWidgetQt::hideEvent(QHideEvent* event) {
    CanvasProcessorWidget::setVisible(false);
    Super::hideEvent(event);
}

void CanvasWithPropertiesProcessorWidgetQt::moveEvent(QMoveEvent* event) {
    CanvasProcessorWidget::setPosition(utilqt::toGLM(event->pos()));
    Super::moveEvent(event);
}

void CanvasWithPropertiesProcessorWidgetQt::changeEvent(QEvent* event) {
    if (event->type() == QEvent::WindowStateChange) {
        CanvasProcessorWidget::setFullScreen(windowState().testFlag(Qt::WindowFullScreen));
    }
    Super::changeEvent(event);
}

void CanvasWithPropertiesProcessorWidgetQt::propagateResizeEvent() {
    CanvasProcessorWidget::setDimensions(utilqt::toGLM(size()));

    auto previousCanvasDimensions = canvasDimensions_;
    const auto dpr = window()->devicePixelRatio();
    canvasDimensions_ = dpr * utilqt::toGLM(canvas_->size());

    NetworkLock lock;
    RenderContext::getPtr()->activateDefaultRenderContext();
    ResizeEvent resizeEvent(canvasDimensions_, previousCanvasDimensions);
    getProcessor()->propagateEvent(&resizeEvent, nullptr);
}

bool CanvasWithPropertiesProcessorWidgetQt::contextMenu(QMenu& menu) {
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

void CanvasWithPropertiesProcessorWidgetQt::setVisible(bool visible) {
    Super::setVisible(visible);  // This will trigger show/hide events.
}

void CanvasWithPropertiesProcessorWidgetQt::setDimensions(ivec2 dimensions) {
    Super::resize(dimensions.x, dimensions.y);  // This will trigger a resize event.
}

void CanvasWithPropertiesProcessorWidgetQt::setPosition(ivec2 pos) {
    Super::move(pos.x, pos.y);  // This will trigger a move event.
}

void CanvasWithPropertiesProcessorWidgetQt::setFullScreen(bool fullScreen) {
    // This will trigger a change event.
    utilqt::setFullScreenAndOnTop(this, fullScreen, CanvasProcessorWidget::isOnTop());
}

void CanvasWithPropertiesProcessorWidgetQt::setOnTop(bool onTop) {
    utilqt::setFullScreenAndOnTop(this, CanvasProcessorWidget::isFullScreen(), onTop);
    CanvasProcessorWidget::setOnTop(onTop);
}

void CanvasWithPropertiesProcessorWidgetQt::updateVisible(bool visible) {
    Super::setVisible(visible);  // This will trigger show/hide events.
}
void CanvasWithPropertiesProcessorWidgetQt::updateDimensions(ivec2 dimensions) {
    Super::resize(dimensions.x, dimensions.y);  // This will trigger a resize event.
}

void CanvasWithPropertiesProcessorWidgetQt::updatePosition(ivec2 pos) {
    Super::move(pos.x, pos.y);  // This will trigger a move event.
}

void CanvasWithPropertiesProcessorWidgetQt::updateFullScreen(bool fullScreen) {
    // This will trigger a change event.
    utilqt::setFullScreenAndOnTop(this, fullScreen, CanvasProcessorWidget::isOnTop());
}

void CanvasWithPropertiesProcessorWidgetQt::updateOnTop(bool onTop) {
    utilqt::setFullScreenAndOnTop(this, CanvasProcessorWidget::isFullScreen(), onTop);
}

}  // namespace inviwo
