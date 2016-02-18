/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2015 Inviwo Foundation
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

#include <inviwo/qt/editor/processorpreview.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/processors/processorfactory.h>
#include <inviwo/qt/editor/processorgraphicsitem.h>
#include <inviwo/qt/editor/processorportgraphicsitem.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QPainter>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsTextItem>
#include <QGraphicsPathItem>
#include <QGraphicsRectItem>
#include <warn/pop>

namespace inviwo {

QImage utilqt::generatePreview(const QString& classIdentifier) {
    std::string cid = classIdentifier.toLocal8Bit().constData();

    try {
        auto processor = InviwoApplication::getPtr()->getProcessorFactory()->create(cid);
        auto item = new ProcessorGraphicsItem(processor.get());
        auto scene = util::make_unique<QGraphicsScene>(nullptr);
        scene->addItem(item);

        double yshift = 20.0;
        double offset = processor->getInports().size()*yshift;
        
        for (auto inport : processor->getInports()) {
            QFont classFont("Segoe", 8, QFont::Bold, true);
            classFont.setPixelSize(14);
            auto pl = new QGraphicsTextItem(QString::fromStdString(inport->getIdentifier()));
            pl->setDefaultTextColor(Qt::lightGray);
            scene->addItem(pl);
            auto pos = item->getInportGraphicsItem(inport)->pos();
            pl->setPos(pos + QPointF(0, -offset));

            double h = pl->boundingRect().height();
            auto t = QTransform().translate(1.0, -h / 2.0);
            pl->setTransform(t);

            auto path = QPainterPath(pos);
            path.lineTo(pos + QPointF(0, -offset));
            path.lineTo(pos + QPointF(2, -offset));
            auto li = new QGraphicsPathItem(path);
  
            li->setPen(QPen(Qt::lightGray));
            scene->addItem(li);

            offset -= yshift;
        }

        offset = processor->getOutports().size()*yshift;
        for (auto outport : processor->getOutports()) {
            QFont classFont("Segoe", 8, QFont::Bold, true);
            classFont.setPixelSize(14);
            auto pl = new QGraphicsTextItem(QString::fromStdString(outport->getIdentifier()));
            pl->setDefaultTextColor(Qt::lightGray);
            scene->addItem(pl);
            auto pos = item->getOutportGraphicsItem(outport)->pos();
            pl->setPos(pos + QPointF(1.0, offset));

            double h = pl->boundingRect().height();
            auto t = QTransform().translate(0.0, -h / 2.0);
            pl->setTransform(t);

            auto path = QPainterPath(pos);
            path.lineTo(pos + QPointF(0, offset));
            path.lineTo(pos + QPointF(2, offset));
            auto li = new QGraphicsPathItem(path);
            
            li->setPen(QPen(Qt::lightGray));
            scene->addItem(li);

            offset -= yshift;
        }

        float padBelow = processor->getOutports().empty() ? 10.0f : 0.0f;
        float padAbove = processor->getInports().empty() ? 10.0f : 0.0f;

        scene->clearSelection();  // Selections would also render to the file
        // Re-shrink the scene to it's bounding contents
        scene->setSceneRect(scene->itemsBoundingRect().adjusted(-10.0, -padAbove, 10.0, padBelow));
        QImage image(
            scene->sceneRect().size().toSize(),
            QImage::Format_ARGB32);   // Create the image with the exact size of the shrunk scene
        image.fill(Qt::transparent);  // Start all pixels transparent

        QPainter painter(&image);
        scene->render(&painter);

        return image;
    } catch (Exception&) {
        // We will just ignore this...
        return QImage();
    }
}

}  // namespace
