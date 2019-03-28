/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2019 Inviwo Foundation
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

#include <inviwo/qt/editor/linkdraghelper.h>
#include <inviwo/qt/editor/networkeditor.h>
#include <inviwo/core/network/processornetwork.h>
#include <inviwo/qt/editor/editorgrapicsitem.h>
#include <inviwo/qt/editor/linkgraphicsitem.h>
#include <inviwo/qt/editor/processorgraphicsitem.h>
#include <inviwo/qt/editor/processorlinkgraphicsitem.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QGraphicsItem>
#include <QGraphicsSceneMouseEvent>
#include <warn/pop>

namespace inviwo {

LinkDragHelper::LinkDragHelper(NetworkEditor &editor) : QObject(&editor), editor_{editor} {}

LinkDragHelper::~LinkDragHelper() = default;

void LinkDragHelper::start(ProcessorLinkGraphicsItem *item, QPointF endPos) {
    link_ = std::make_unique<LinkConnectionDragGraphicsItem>(item, endPos);
    editor_.addItem(link_.get());
    link_->setZValue(DRAGING_ITEM_DEPTH);
    link_->show();
}

void LinkDragHelper::reset() { link_.reset(); }

bool LinkDragHelper::eventFilter(QObject *, QEvent *event) {
    if (link_ && event->type() == QEvent::GraphicsSceneMouseMove) {
        auto e = static_cast<QGraphicsSceneMouseEvent *>(event);
        link_->setEndPoint(e->scenePos());
        link_->reactToProcessorHover(editor_.getProcessorGraphicsItemAt(e->scenePos()));
        e->accept();
    } else if (link_ && event->type() == QEvent::GraphicsSceneMouseRelease) {
        auto e = static_cast<QGraphicsSceneMouseEvent *>(event);
        // link drag mode
        auto startProcessor =
            link_->getSrcProcessorLinkGraphicsItem()->getProcessorGraphicsItem()->getProcessor();

        reset();

        if (auto endProcessorItem = editor_.getProcessorGraphicsItemAt(e->scenePos())) {
            Processor *endProcessor = endProcessorItem->getProcessor();
            if (startProcessor != endProcessor) {
                editor_.showLinkDialog(startProcessor, endProcessor);
            }
        }
        e->accept();
    }
    return false;
}

}  // namespace inviwo
