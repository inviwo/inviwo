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

#include <inviwo/core/processors/processorfactory.h>
#include <inviwo/qt/editor/processorgraphicsitem.h>
#include <QPainter>
#include <QGraphicsScene>
#include <QGraphicsView>


namespace inviwo {

QImage utilqt::generatePreview(const QString& classIdentifier) {
    std::string cid = classIdentifier.toLocal8Bit().constData();

    auto processor = ProcessorFactory::getPtr()->create(cid);
    auto item = new ProcessorGraphicsItem(processor.get());
    auto scene = util::make_unique<QGraphicsScene>(nullptr);
    scene->addItem(item);

    scene->clearSelection();  // Selections would also render to the file
    // Re-shrink the scene to it's bounding contents
    scene->setSceneRect(scene->itemsBoundingRect());
    QImage image(
        scene->sceneRect().size().toSize(),
        QImage::Format_ARGB32);   // Create the image with the exact size of the shrunk scene
    image.fill(Qt::transparent);  // Start all pixels transparent

    QPainter painter(&image);
    scene->render(&painter);

    return image;
}

}  // namespace
