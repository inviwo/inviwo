/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2015 Inviwo Foundation
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

#include <inviwo/qt/editor/processorprogressgraphicsitem.h>

#include <QApplication>
#include <QPen>
#include <QPainter>
#include <QBrush>

namespace inviwo {

ProcessorProgressGraphicsItem::ProcessorProgressGraphicsItem(QGraphicsRectItem* parent,
                                                             ProgressBar* progressBar)
    : EditorGraphicsItem(parent), size_(126, 5.0f), progressBar_(progressBar) {
    setRect(-0.5f * size_.width(), 
            -0.5f * size_.height() + 3,
            size_.width(), size_.height());

    setPos(QPointF(0.0f, 9.0f));
    setCacheMode(QGraphicsItem::DeviceCoordinateCache);
    setVisible(progressBar_->isVisible());
    progressBar_->addObserver(this);

}

void ProcessorProgressGraphicsItem::paint(QPainter* p, const QStyleOptionGraphicsItem* options,
                                          QWidget* widget) {
    
    float progress = progressBar_->getProgress();

    p->save();
    p->setRenderHint(QPainter::Antialiasing, true);

    QColor progressColor = Qt::lightGray;
    QRectF progressBarRect = rect();
    QLinearGradient progressGrad(progressBarRect.topLeft(), progressBarRect.topRight());
    progressGrad.setColorAt(0.0f, progressColor);
    float left = std::max(0.0f, progress - 0.001f);
    float right = std::min(1.0f, progress + 0.001f);
    progressGrad.setColorAt(left, progressColor);
    progressGrad.setColorAt(right, Qt::black);
    progressGrad.setColorAt(1.0f, Qt::black);
    p->setPen(Qt::black);
    p->setBrush(progressGrad);
    p->drawRoundedRect(progressBarRect, 2.0, 2.0);
    QColor shadeColor(128, 128, 128);
    QLinearGradient shadingGrad(progressBarRect.topLeft(), progressBarRect.bottomLeft());
    shadingGrad.setColorAt(0.0f, QColor(shadeColor.red() * 0.6, shadeColor.green() * 0.6,
                                        shadeColor.blue() * 0.6, 120));
    shadingGrad.setColorAt(0.3f,
                           QColor(shadeColor.red(), shadeColor.green(), shadeColor.blue(), 120));
    shadingGrad.setColorAt(1.0f,
                           QColor(shadeColor.red(), shadeColor.green(), shadeColor.blue(), 120));
    p->setPen(Qt::NoPen);
    p->setBrush(shadingGrad);
    p->drawRoundedRect(progressBarRect, 2.0, 2.0);
    p->restore();
}

void ProcessorProgressGraphicsItem::progressChanged() {
    // mark item as dirty to force an redraw
    this->update();
    // let Qt take care of events like update for 25ms, but exclude user input (we do not want
    // any interference)
    qApp->processEvents(QEventLoop::ExcludeUserInputEvents, 25);
}

void ProcessorProgressGraphicsItem::progressBarVisibilityChanged() {
    setVisible(progressBar_->isVisible());
    // mark item as dirty to force an redraw
    this->update();
    // let Qt take care of events like update for 25ms, but exclude user input (we do not want
    // any interference)
    qApp->processEvents(QEventLoop::ExcludeUserInputEvents, 25);
}

}  // namespace
