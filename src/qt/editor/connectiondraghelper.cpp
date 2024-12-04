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

#include <inviwo/qt/editor/connectiondraghelper.h>
#include <inviwo/qt/editor/networkeditor.h>
#include <inviwo/core/network/processornetwork.h>
#include <inviwo/qt/editor/connectiongraphicsitem.h>
#include <inviwo/qt/editor/processorportgraphicsitem.h>
#include <inviwo/core/util/rendercontext.h>

#include <modules/qtwidgets/inviwoqtutils.h>

#include <inviwo/core/ports/inport.h>
#include <inviwo/core/ports/outport.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QGraphicsItem>
#include <QGraphicsSceneMouseEvent>
#include <warn/pop>

namespace inviwo {

ConnectionOutDragHelper::ConnectionOutDragHelper(NetworkEditor& editor)
    : QObject(&editor), editor_{editor} {}

ConnectionOutDragHelper::~ConnectionOutDragHelper() = default;

bool ConnectionOutDragHelper::eventFilter(QObject*, QEvent* event) {
    if (outConnection_ && event->type() == QEvent::GraphicsSceneMouseMove) {

        auto e = static_cast<QGraphicsSceneMouseEvent*>(event);
        outConnection_->setEndPoint(e->scenePos());
        outConnection_->reactToPortHover(editor_.getProcessorInportGraphicsItemAt(e->scenePos()));
        e->accept();
    } else if (outConnection_ && event->type() == QEvent::GraphicsSceneMouseRelease) {
        auto e = static_cast<QGraphicsSceneMouseEvent*>(event);

        auto startPort = outConnection_->getOutportGraphicsItem()->getPort();
        reset();

        RenderContext::getPtr()->activateDefaultRenderContext();
        auto endItem = editor_.getProcessorInportGraphicsItemAt(e->scenePos());
        if (endItem && endItem->getPort()->canConnectTo(startPort)) {
            Inport* endPort = endItem->getPort();

            if (endPort->getNumberOfConnections() >= endPort->getMaxNumberOfConnections()) {
                editor_.getNetwork()->removeConnection(endPort->getConnectedOutport(), endPort);
            }
            editor_.getNetwork()->addConnection(startPort, endPort);
        }
        e->accept();
    }
    return false;
}

void ConnectionOutDragHelper::start(ProcessorOutportGraphicsItem* outport, QPointF endPoint,
                                    uvec3 color) {
    outConnection_ = std::make_unique<ConnectionOutportDragGraphicsItem>(outport, endPoint,
                                                                         utilqt::toQColor(color));
    editor_.addItem(outConnection_.get());
    outConnection_->show();

    editor_.installEventFilter(this);
}

void ConnectionOutDragHelper::reset() {
    outConnection_.reset();
    editor_.removeEventFilter(this);
}

ConnectionInDragHelper::ConnectionInDragHelper(NetworkEditor& editor)
    : QObject(&editor), editor_{editor} {}

ConnectionInDragHelper::~ConnectionInDragHelper() = default;

bool ConnectionInDragHelper::eventFilter(QObject*, QEvent* event) {
    if (inConnection_ && event->type() == QEvent::GraphicsSceneMouseMove) {
        auto e = static_cast<QGraphicsSceneMouseEvent*>(event);
        inConnection_->setStartPoint(e->scenePos());
        inConnection_->reactToPortHover(editor_.getProcessorOutportGraphicsItemAt(e->scenePos()));
        e->accept();
    } else if (inConnection_ && event->type() == QEvent::GraphicsSceneMouseRelease) {
        auto e = static_cast<QGraphicsSceneMouseEvent*>(event);

        auto endPort = inConnection_->getInportGraphicsItem()->getPort();
        reset();

        RenderContext::getPtr()->activateDefaultRenderContext();
        auto startItem = editor_.getProcessorOutportGraphicsItemAt(e->scenePos());
        if (startItem && endPort->canConnectTo(startItem->getPort())) {
            Outport* startPort = startItem->getPort();

            if (endPort->getNumberOfConnections() >= endPort->getMaxNumberOfConnections()) {
                editor_.getNetwork()->removeConnection(endPort->getConnectedOutport(), endPort);
            }
            editor_.getNetwork()->addConnection(startPort, endPort);
        }
        e->accept();
    }
    return false;
}

void ConnectionInDragHelper::start(ProcessorInportGraphicsItem* inport, QPointF startPoint,
                                   uvec3 color) {
    inConnection_ = std::make_unique<ConnectionInportDragGraphicsItem>(startPoint, inport,
                                                                       utilqt::toQColor(color));
    editor_.addItem(inConnection_.get());
    inConnection_->show();

    editor_.installEventFilter(this);
}

void ConnectionInDragHelper::reset() {
    inConnection_.reset();
    editor_.removeEventFilter(this);
}

}  // namespace inviwo
