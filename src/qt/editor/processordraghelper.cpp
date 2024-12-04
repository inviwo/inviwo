/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2024 Inviwo Foundation
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

#include <inviwo/qt/editor/processordraghelper.h>

#include <inviwo/core/processors/processor.h>
#include <inviwo/core/processors/processorutils.h>
#include <inviwo/core/network/networkutils.h>
#include <inviwo/core/network/autolinker.h>
#include <inviwo/core/util/zip.h>
#include <inviwo/core/util/raiiutils.h>
#include <inviwo/core/network/networklock.h>
#include <inviwo/core/util/rendercontext.h>
#include <inviwo/core/util/glm.h>
#include <inviwo/core/util/assertion.h>

#include <modules/qtwidgets/inviwoqtutils.h>
#include <modules/qtwidgets/textlabeloverlay.h>

#include <inviwo/qt/editor/connectiongraphicsitem.h>
#include <inviwo/qt/editor/processorgraphicsitem.h>
#include <inviwo/qt/editor/processorportgraphicsitem.h>
#include <inviwo/qt/editor/linkgraphicsitem.h>

#include <inviwo/qt/editor/networkeditor.h>
#include <inviwo/qt/editor/processormimedata.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QGraphicsItem>
#include <QGraphicsSceneDragDropEvent>
#include <QMimeData>
#include <QGraphicsView>
#include <warn/pop>

namespace inviwo {

ProcessorDragHelper::ProcessorDragHelper(NetworkEditor& editor)
    : QObject(&editor), editor_{editor}, automator_{editor} {}

ProcessorDragHelper::~ProcessorDragHelper() = default;

namespace {

template <QEvent::Type E, typename T>
auto eventIs(QEvent* event) -> T* {
    if (event->type() == E) {
        auto* e = dynamic_cast<QGraphicsSceneDragDropEvent*>(event);
        IVW_ASSERT(e, "If this is invalid the combination of E and T is wrong");
        return e;
    }
    return nullptr;
};

}  // namespace

bool ProcessorDragHelper::eventFilter(QObject*, QEvent* event) {
    if (auto* enterEvent =
            eventIs<QEvent::GraphicsSceneDragEnter, QGraphicsSceneDragDropEvent>(event)) {
        if (const auto* mime = ProcessorMimeData::toProcessorMimeData(enterEvent->mimeData())) {
            return enter(enterEvent, mime);
        }
    } else if (auto* moveEvent =
                   eventIs<QEvent::GraphicsSceneDragMove, QGraphicsSceneDragDropEvent>(event)) {
        if (const auto* mime = ProcessorMimeData::toProcessorMimeData(moveEvent->mimeData())) {
            return move(moveEvent, mime);
        }
    } else if (auto* leaveEvent =
                   eventIs<QEvent::GraphicsSceneDragLeave, QGraphicsSceneDragDropEvent>(event)) {
        return leave(leaveEvent);
    } else if (auto* dropEvent =
                   eventIs<QEvent::GraphicsSceneDrop, QGraphicsSceneDragDropEvent>(event)) {
        // for some reason, QGraphicsView accepts the event before it arrives in this event filter
        // @see QGraphicsView::dropEvent(QDropEvent*)
        dropEvent->setAccepted(false);
        if (const auto* mime = ProcessorMimeData::toProcessorMimeData(dropEvent->mimeData())) {
            return drop(dropEvent, mime);
        }
    }
    event->ignore();
    return false;
}

bool ProcessorDragHelper::enter(QGraphicsSceneDragDropEvent* e, const ProcessorMimeData* mime) {
    e->acceptProposedAction();

    auto processor = mime->processor();
    const auto zoom = 1.0 / editor_.views().front()->transform().m11();
    automator_.enter(e->scenePos(), e->modifiers(), *processor, zoom);
    return true;
}
bool ProcessorDragHelper::move(QGraphicsSceneDragDropEvent* e, const ProcessorMimeData* mime) {
    e->accept();
    auto processor = mime->processor();
    util::setPosition(processor, utilqt::toGLM(e->scenePos()));

    const auto zoom = 1.0 / editor_.views().front()->transform().m11();
    automator_.move(e->scenePos(), e->modifiers(), *processor, zoom);

    return true;
}

bool ProcessorDragHelper::leave(QGraphicsSceneDragDropEvent* e) {
    e->accept();
    automator_.leave();
    return true;
}
bool ProcessorDragHelper::drop(QGraphicsSceneDragDropEvent* e, const ProcessorMimeData* mime) {
    e->accept();

    auto network = editor_.getNetwork();
    NetworkLock lock(network);

    try {
        auto processor = mime->get();
        if (!processor) {
            LogError("Unable to get processor from drag object");
            return true;
        }
        editor_.clearSelection();
        RenderContext::getPtr()->activateDefaultRenderContext();
        util::setPosition(processor.get(), utilqt::toGLM(NetworkEditor::snapToGrid(e->scenePos())));

        auto* addedProcessor = network->addProcessor(std::move(processor));
        automator_.drop(e->scenePos(), e->modifiers(), *addedProcessor);

    } catch (Exception& exception) {
        util::log(exception.getContext(),
                  "Unable to create processor " + utilqt::fromQString(mime->text()) + " due to " +
                      exception.getMessage(),
                  LogLevel::Error);
    }

    return true;
}

void ProcessorDragHelper::clear(ConnectionGraphicsItem* connection) {
    automator_.clear(connection);
}
void ProcessorDragHelper::clear(ProcessorGraphicsItem* processor) { automator_.clear(processor); }

}  // namespace inviwo
