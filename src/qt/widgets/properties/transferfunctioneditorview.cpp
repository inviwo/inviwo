/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 * Version 0.9
 *
 * Copyright (c) 2013-2015 Inviwo Foundation
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

#include <inviwo/core/datastructures/histogram.h>
#include <inviwo/core/properties/transferfunctionproperty.h>
#include <inviwo/qt/widgets/properties/transferfunctioneditorview.h>
#include <inviwo/qt/widgets/properties/transferfunctionpropertydialog.h>
#include <inviwo/qt/widgets/properties/transferfunctioneditorcontrolpoint.h>
#include <inviwo/qt/widgets/properties/transferfunctioneditor.h>
#include <inviwo/core/datastructures/volume/volumeram.h>
#include <QVarLengthArray>
#include <QGraphicsItem>
#include <QThread>
#include <QtEvents>
#include <QGraphicsScene>
#include <QPainter>
#include <QBrush>

namespace inviwo {

TransferFunctionEditorView::TransferFunctionEditorView(TransferFunctionProperty* tfProperty)
    : TransferFunctionObserver()
    , tfProperty_(tfProperty)
    , volumeInport_(tfProperty->getVolumeInport())
    , showHistogram_(tfProperty->getShowHistogram())
    , histogramTheadWorking_(false)
    , workerThread_(NULL)
    , invalidatedHistogram_(true)
    , maskHorizontal_(0.0f, 1.0f) {

    setMouseTracking(true);
    setRenderHint(QPainter::Antialiasing, true);
    setViewportUpdateMode(QGraphicsView::SmartViewportUpdate);

    this->setCacheMode(QGraphicsView::CacheBackground);

    // TODO: enable this block for OpenGL rendering of the Transfer Function
    /*
        QGLFormat format;
        // we only need a Depth buffer and a RGB frame buffer
        // double buffering is nice, too!
        format.setOption(QGL::DeprecatedFunctions);
        format.setDoubleBuffer(true);
        format.setDepth(true);
        format.setRgba(true);
        format.setAlpha(true);
        format.setAccum(false);
        format.setStencil(false);
        format.setStereo(false);
        this->setViewport(new QGLWidget(format));
    */

    if (volumeInport_) {
        volumeInport_->onInvalid(this, &TransferFunctionEditorView::onVolumeInportInvalid);
        volumeInport_->onChange(this, &TransferFunctionEditorView::onVolumeInportChange);
    }
}

TransferFunctionEditorView::~TransferFunctionEditorView() {
    if (workerThread_) {
        const VolumeRAM* volumeRAM = volumeInport_->getData()->getRepresentation<VolumeRAM>();
        volumeRAM->stopHistogramCalculation();
        workerThread_ = NULL;
    }
    if (volumeInport_) {
        volumeInport_->onInvalid(this, &TransferFunctionEditorView::onVolumeInportInvalid, false);
    }
}

void TransferFunctionEditorView::resizeEvent(QResizeEvent* event) {
    updateZoom();
    invalidatedHistogram_ = true;
    QGraphicsView::resizeEvent(event);
    this->resetCachedContent();
}

void TransferFunctionEditorView::drawForeground(QPainter* painter, const QRectF& rect) {
    QPen pen;
    pen.setCosmetic(true);
    pen.setWidthF(1.5);
    pen.setColor(Qt::lightGray);
    painter->setPen(pen);

    QRectF sRect = sceneRect();

    if (maskHorizontal_.x > 0.0f) {
        double leftMaskBorder = maskHorizontal_.x * sRect.width();
        QRectF r(0.0, rect.top(), leftMaskBorder, rect.height());
        QLineF line(leftMaskBorder, rect.top(), leftMaskBorder, rect.bottom());
        painter->fillRect(r, QColor(25, 25, 25, 100));
        painter->drawLine(line);
    }

    if (maskHorizontal_.y < 1.0f) {
        double rightMaskBorder = maskHorizontal_.y * sRect.width();
        QRectF r(rightMaskBorder, rect.top(), sRect.right() - rightMaskBorder, rect.height());
        QLineF line(rightMaskBorder, rect.top(), rightMaskBorder, rect.bottom());
        painter->fillRect(r, QColor(25, 25, 25, 100));
        painter->drawLine(line);
    }

    QGraphicsView::drawForeground(painter, rect);
}

void TransferFunctionEditorView::onTransferFunctionChange() {
    if (volumeInport_ != tfProperty_->getVolumeInport()) {
        volumeInport_ = tfProperty_->getVolumeInport();
        volumeInport_->onInvalid(this, &TransferFunctionEditorView::onVolumeInportInvalid);
        volumeInport_->onChange(this, &TransferFunctionEditorView::onVolumeInportChange);
    }

    this->viewport()->update();
}

void TransferFunctionEditorView::onControlPointChanged(const TransferFunctionDataPoint* p) {
    onTransferFunctionChange();
}

void TransferFunctionEditorView::onVolumeInportInvalid() {
    if (workerThread_) {
        const VolumeRAM* volumeRAM = volumeInport_->getData()->getRepresentation<VolumeRAM>();
        volumeRAM->stopHistogramCalculation();
        workerThread_ = NULL;
    }
    invalidatedHistogram_ = true;
    this->resetCachedContent();
}

void TransferFunctionEditorView::setShowHistogram(int type) {
    showHistogram_ = type;
    invalidatedHistogram_ = true;
    this->resetCachedContent();
}

void TransferFunctionEditorView::histogramThreadFinished() {
    workerThread_ = NULL;
    histogramTheadWorking_ = false;
    invalidatedHistogram_ = true;
    this->resetCachedContent();
}

void TransferFunctionEditorView::updateHistogram() {
    if (!invalidatedHistogram_) return;

    histograms_.clear();

    int channel = 0;
    const NormalizedHistogram* normHistogram;
    QRectF sRect = sceneRect();

    while((normHistogram = getNormalizedHistogram(channel)) !=NULL) {
        ++channel;
        histograms_.push_back(QPolygonF());        
        const std::vector<double>* normHistogramData = normHistogram->getData();
        double histSize = static_cast<double>(normHistogramData->size());
        double stepSize = sRect.width() / histSize;

        double scale = 1.0;
        switch (showHistogram_) {
            case 0:  // Don't show
                return;
            case 1:  // show all
                scale = 1.0;
                break;
            case 2:  // show 99%
                scale = normHistogram->histStats_.percentiles[99];
                break;
            case 3:  // show 95%
                scale = normHistogram->histStats_.percentiles[95];
                break;
            case 4:  // show 90%
                scale = normHistogram->histStats_.percentiles[90];
                break;
            case 5:  // show log%
                scale = 1.0;
                break;
        }
        double height;
        double maxCount = normHistogram->getMaximumBinValue();

        histograms_.back() << QPointF(0.0f,0.0f);

        for (double i = 0; i < histSize; i++) {
            if (showHistogram_ == 5) {
                height =
                    std::log10(1.0 + maxCount * normHistogramData->at(static_cast<size_t>(i))) /
                    std::log10(maxCount);

            } else {
                height = normHistogramData->at(static_cast<size_t>(i)) / scale;
                height = std::min(height, 1.0);
            }
            height *= sRect.height();
            histograms_.back() << QPointF(i*stepSize, height) << QPointF((i+1)*stepSize, height);
        }
        histograms_.back() << QPointF(sRect.width(),0.0f) << QPointF(0.0f,0.0f);
    }

    invalidatedHistogram_ = false;
}

const NormalizedHistogram* TransferFunctionEditorView::getNormalizedHistogram(int channel) {
    if (volumeInport_ && volumeInport_->hasData()) {
        const VolumeRAM* volumeRAM = volumeInport_->getData()->getRepresentation<VolumeRAM>();

        if (volumeRAM) {
            if (volumeRAM->hasNormalizedHistogram())
                return volumeRAM->getNormalizedHistogram(1, 2048, channel);
            else if (!histogramTheadWorking_) {
                histogramTheadWorking_ = true;
                workerThread_ = new QThread();
                HistogramWorkerQt* worker = new HistogramWorkerQt(volumeRAM, 2048);
                worker->moveToThread(workerThread_);
                connect(workerThread_, SIGNAL(started()), worker, SLOT(process()));
                connect(worker, SIGNAL(finished()), workerThread_, SLOT(quit()));
                connect(worker, SIGNAL(finished()), worker, SLOT(deleteLater()));
                connect(workerThread_, SIGNAL(finished()), workerThread_, SLOT(deleteLater()));
                connect(workerThread_, SIGNAL(finished()), this, SLOT(histogramThreadFinished()));
                workerThread_->start();
            }
        }
    } 

    return NULL;
}

void TransferFunctionEditorView::drawBackground(QPainter* painter, const QRectF& rect) {
    painter->fillRect(rect, QColor(89, 89, 89));

    // overlay grid
    int gridSpacing = 25;
    QRectF sRect = sceneRect();
    qreal right = int(sRect.right()) - (int(sRect.right()) % gridSpacing);
    QVarLengthArray<QLineF, 100> lines;

    for (qreal x = sRect.left(); x <= right; x += gridSpacing) {
        lines.append(QLineF(x, sRect.top(), x, sRect.bottom()));
    }

    QPen gridPen;
    gridPen.setColor(QColor(102, 102, 102));
    gridPen.setWidth(1.0);
    gridPen.setCosmetic(true);
    painter->setPen(gridPen);
    painter->drawLines(lines.data(), lines.size());

    // histogram
    if (showHistogram_ > 0) {
        updateHistogram();
        for (int i = 0; i < static_cast<int>(histograms_.size()); ++i) {
            QPen pen;
            pen.setColor(QColor(68, 102, 170, 150));
            pen.setWidthF(2.0f);
            pen.setCosmetic(true);
            painter->setPen(pen);

            QBrush brush;
            brush.setColor(QColor(68, 102, 170, 100));
            brush.setStyle(Qt::SolidPattern);
            painter->setBrush(brush);
            
            painter->drawPolygon(histograms_[i]);
        }
    }
}

void TransferFunctionEditorView::updateZoom() {
    fitInView(
        tfProperty_->getZoomH().x * scene()->sceneRect().width(),
        tfProperty_->getZoomV().x * scene()->sceneRect().height(),
        (tfProperty_->getZoomH().y - tfProperty_->getZoomH().x) * scene()->sceneRect().width(),
        (tfProperty_->getZoomV().y - tfProperty_->getZoomV().x) * scene()->sceneRect().height(),
        Qt::IgnoreAspectRatio);
}

void TransferFunctionEditorView::setMask(float maskMin, float maskMax) {
    if (maskMax < maskMin) {
        maskMax = maskMin;
    }
    maskHorizontal_ = vec2(maskMin, maskMax);
    this->viewport()->update();
}

void TransferFunctionEditorView::onVolumeInportChange() {
    if (volumeInport_ && volumeInport_->hasData()) {
        TransferFunctionEditor* editor = dynamic_cast<TransferFunctionEditor*>(this->scene());
        editor->setDataMap(volumeInport_->getData()->dataMap_);
        if (editor) {
            QList<QGraphicsItem*> items = editor->items();
            for (QList<QGraphicsItem*>::iterator it = items.begin(); it != items.end(); it++) {
                TransferFunctionEditorControlPoint* cp =
                    qgraphicsitem_cast<TransferFunctionEditorControlPoint*>(*it);
                if (cp) {
                    cp->setDataMap(volumeInport_->getData()->dataMap_);
                }
            }
        }
    }

    if (showHistogram_ && volumeInport_ && volumeInport_->hasData()) {
        const NormalizedHistogram* histogram = getNormalizedHistogram();
        if (histogram && histogram->dataRange_ != volumeInport_->getData()->dataMap_.dataRange) {
            invalidatedHistogram_ = true;
            updateHistogram();
            this->viewport()->update();
        }
    }
}

void HistogramWorkerQt::process() {
    volumeRAM_->getNormalizedHistogram(1, numBins_);
    emit finished();
}

HistogramWorkerQt::HistogramWorkerQt(const VolumeRAM* volumeRAM, std::size_t numBins /*= 2048u*/) 
    : volumeRAM_(volumeRAM), numBins_(numBins) {
}

HistogramWorkerQt::~HistogramWorkerQt() {
    volumeRAM_ = NULL;
}

}  // namespace inviwo