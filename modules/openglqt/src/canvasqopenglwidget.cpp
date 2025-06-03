/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2025 Inviwo Foundation
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

#include <inviwo/core/common/inviwoapplication.h>         // for InviwoApplication
#include <inviwo/core/datastructures/image/imagetypes.h>  // for LayerType
#include <inviwo/core/interaction/events/event.h>         // for Event
#include <inviwo/core/interaction/events/resizeevent.h>
#include <inviwo/core/interaction/events/contextmenuevent.h>
#include <inviwo/core/interaction/events/mouseevent.h>
#include <inviwo/core/interaction/events/eventpropagator.h>  // for EventPropagator
#include <inviwo/core/interaction/pickingcontroller.h>       // for PickingController
#include <inviwo/core/network/networklock.h>                 // for NetworkLock
#include <inviwo/core/properties/boolproperty.h>             // for BoolProperty
#include <inviwo/core/util/canvas.h>                         // for Canvas::ContextID, Canvas
#include <inviwo/core/util/glmvec.h>                         // for size2_t, dvec2
#include <inviwo/core/util/glm.h>                            // for invertY
#include <inviwo/core/util/raiiutils.h>                      // for OnScopeExit, OnScopeExit::Ex...
#include <inviwo/core/util/rendercontext.h>                  // for CanvasContextHolder, RenderC...
#include <inviwo/core/util/settings/systemsettings.h>        // for SystemSettings
#include <inviwo/core/util/logcentral.h>                     // for log::exception, log::report...
#include <inviwo/core/util/stdextensions.h>                  // for util::overloaded
#include <modules/opengl/canvasgl.h>                         // for CanvasGL
#include <modules/opengl/openglcapabilities.h>               // for OpenGLCapabilities
#include <modules/openglqt/hiddencanvasqt.h>                 // for HiddenCanvasQt
#include <modules/openglqt/interactioneventmapperqt.h>       // for InteractionEventMapperQt
#include <modules/qtwidgets/inviwoqtutils.h>                 // for toGLM, addImageActions, Widg...
#include <modules/qtwidgets/eventconverterqt.h>

#include <utility>  // for move

#include <QApplication>                       // for QApplication
#include <QCoreApplication>                   // for QCoreApplication
#include <QMenu>                              // for QMenu
#include <QMouseEvent>                        // for QMouseEvent
#include <QOpenGLContext>                     // for QOpenGLContext
#include <QResizeEvent>                       // for QResizeEvent
#include <QWidget>                            // for QWidget
#include <Qt>                                 // for PanGesture, PinchGesture
#include <glm/fwd.hpp>                        // for vec2
#include <glm/gtx/scalar_multiplication.hpp>  // for operator*

class QMouseEvent;

namespace inviwo {

class Image;
class Outport;

namespace {

dvec2 normalizePosition(QPointF pos, size2_t dim) {
    return util::invertY(utilqt::toGLM(pos), dim) / dvec2(dim - size2_t(1));
}

void addMenuEntries(QMenu* menu, std::span<const ContextMenuEntry> menuEntries,
                    const std::function<void(ContextMenuEvent&)>& propagator) {

    auto addSeparator = [menu](const ContextMenuSeparator&) { menu->addSeparator(); };
    auto addSubmenu = [menu, propagator](const ContextMenuSubmenu& menuEntry) {
        auto* submenu = menu->addMenu(utilqt::toQString(menuEntry.label));
        if (menuEntry.iconPath) {
            submenu->setIcon(QIcon(utilqt::toQString(*menuEntry.iconPath)));
        }
        addMenuEntries(submenu, menuEntry.childEntries, propagator);
    };
    auto addMenuAction = [menu, propagator](const ContextMenuAction& menuAction) {
        auto* action = menu->addAction(utilqt::toQString(menuAction.label));
        if (menuAction.iconPath) {
            action->setIcon(QIcon(utilqt::toQString(*menuAction.iconPath)));
        }
        QObject::connect(action, &QAction::triggered, [propagator, menuAction]() {
            try {
                RenderContext::getPtr()->activateDefaultRenderContext();
                ContextMenuEvent menuEvent{
                    menuAction.id, utilqt::getModifiers(QGuiApplication::keyboardModifiers()),
                    menuAction.data};
                propagator(menuEvent);
            } catch (Exception& e) {
                log::exception(e);
            } catch (fmt::format_error& e) {
                log::report(LogLevel::Error, {}, "Error using fmt formatting: {}\n{}", e.what(),
                            util::fmtHelp.view());
            } catch (std::exception& e) {
                log::exception(e);
            } catch (...) {
                log::exception();
            }
        });
    };

    for (const auto& entry : menuEntries) {
        std::visit(util::overloaded(addSeparator, addMenuAction, addSubmenu), entry);
    }
}

}  // namespace

CanvasQOpenGLWidget::CanvasQOpenGLWidget(QWidget* parent, std::string_view name)
    : QOpenGLWidget{parent}, CanvasGL{}, name_{name} {

    setFocusPolicy(Qt::StrongFocus);

    setMouseTracking(true);
    setAttribute(Qt::WA_OpaquePaintEvent);

    auto interactionEventMapper = new InteractionEventMapperQt(
        this, this, [this]() { return utilqt::toGLM(size()); },
        [this]() { return getImageDimensions(); },
        [this](dvec2 pos) { return getDepthValueAtNormalizedCoord(pos); },
        [this](std::span<ContextMenuEntry> entries, ContextMenuCategories actions,
               InteractionEvent* triggerEvent) {
            if (!contextMenuCallback_) return;

            QMenu menu(this);
            if (actions.contains(ContextMenuCategory::Callback)) {
                addMenuEntries(&menu, entries, [this](ContextMenuEvent& menuEvent) {
                    propagateEvent(&menuEvent, nullptr);
                });
            }

            if (auto image = image_.lock(); image && actions.contains(ContextMenuCategory::Image)) {
                if (!menu.actions().empty()) {
                    menu.addSeparator();
                }
                utilqt::addImageActions(menu, *image, layerType_, layerIdx_);
            }
            if (contextMenuCallback_(menu, actions)) {
                menu.exec(QCursor::pos());
            }
        },
        [this](Qt::CursorShape cursor) { setCursor(cursor); });

    auto& settings = InviwoApplication::getPtr()->getSystemSettings();
    auto setHandleTouch = [this, &settings, interactionEventMapper]() {
        const auto& touch = settings.enableTouchProperty_;
        const auto& gestures = settings.enableGesturesProperty_;

        if (gestures.get()) {
            grabGesture(Qt::PanGesture);
            grabGesture(Qt::PinchGesture);
        } else {
            ungrabGesture(Qt::PanGesture);
            ungrabGesture(Qt::PinchGesture);
        }
        setAttribute(Qt::WA_AcceptTouchEvents, touch.get());

        interactionEventMapper->handleTouch(touch.get());
        interactionEventMapper->handleGestures(gestures.get());
    };

    settings.enableTouchProperty_.onChange(setHandleTouch);
    settings.enableGesturesProperty_.onChange(setHandleTouch);
    setHandleTouch();

    installEventFilter(new utilqt::WidgetCloseEventFilter(this));
    installEventFilter(interactionEventMapper);
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

    // Propagated an event with the physical (pixel) dimensions of the canvas
    // Note: QWidget::size() will return logical dimensions
    const auto dpr = window()->devicePixelRatio();
    RenderContext::getPtr()->activateDefaultRenderContext();
    ResizeEvent resizeEvent{dpr * utilqt::toGLM(event->size()),
                            dpr * utilqt::toGLM(event->oldSize())};
    propagateEvent(&resizeEvent, nullptr);
}

void CanvasQOpenGLWidget::triggerResizeEventPropagation() {
    const auto dpr = window()->devicePixelRatio();

    RenderContext::getPtr()->activateDefaultRenderContext();
    ResizeEvent resizeEvent{dpr * utilqt::toGLM(size()), dpr * utilqt::toGLM(size())};
    propagateEvent(&resizeEvent, nullptr);
}

void CanvasQOpenGLWidget::releaseContext() {
    doneCurrent();
    context()->moveToThread(QApplication::instance()->thread());
}

void CanvasQOpenGLWidget::onContextMenu(
    std::function<bool(QMenu&, ContextMenuCategories)> callback) {
    contextMenuCallback_ = std::move(callback);
}

size2_t CanvasQOpenGLWidget::getCanvasDimensions() const {
    const auto dpr = window()->devicePixelRatio();
    return dpr * utilqt::toGLM(size());
}

void CanvasQOpenGLWidget::propagateEvent(Event* e, Outport*) {
    if (!propagator_) return;
    const NetworkLock lock;
    pickingController_.propagateEvent(e, propagator_);
}

}  // namespace inviwo
