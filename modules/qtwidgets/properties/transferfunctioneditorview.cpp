/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2016 Inviwo Foundation
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
#include <modules/qtwidgets/properties/transferfunctioneditorview.h>
#include <modules/qtwidgets/properties/transferfunctionpropertydialog.h>
#include <modules/qtwidgets/properties/transferfunctioneditorcontrolpoint.h>
#include <modules/qtwidgets/properties/transferfunctioneditor.h>
#include <inviwo/core/datastructures/volume/volumeram.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QVarLengthArray>
#include <QGraphicsItem>
#include <QtEvents>
#include <QGraphicsScene>
#include <QPainter>
#include <QBrush>
#include <warn/pop>

namespace inviwo {

TransferFunctionEditorView::TransferFunctionEditorView(TransferFunctionProperty* tfProperty)
    : QGraphicsView()
    , tfProperty_(tfProperty)
    , volumeInport_(tfProperty->getVolumeInport())
    , histogramMode_(tfProperty->getHistogramMode())
    , maskHorizontal_(0.0f, 1.0f) {

    setMouseTracking(true);
    setRenderHint(QPainter::Antialiasing, true);
    setViewportUpdateMode(QGraphicsView::SmartViewportUpdate);

    this->setCacheMode(QGraphicsView::CacheBackground);

    tfProperty_->TransferFunctionPropertyObservable::addObserver(this);

    if (volumeInport_) {
        const auto portChange = [this]() {
            if (histogramMode_ != HistogramMode::Off && volumeInport_->hasData()) {
                updateHistogram();
                resetCachedContent();
                update();
            }
        };

        volumeInport_->onInvalid(this, &TransferFunctionEditorView::onVolumeInportInvalid);
        volumeInport_->onChange(portChange);
        volumeInport_->onConnect(portChange);
        volumeInport_->onDisconnect([this](){
            histograms_.clear();
            resetCachedContent();
            update();
        });
    }
    updateHistogram();
}

TransferFunctionEditorView::~TransferFunctionEditorView() {
    stopHistCalculation_ = true;
    if (volumeInport_) volumeInport_->removeOnInvalid(this);
}

void TransferFunctionEditorView::onMaskChange(const vec2& mask) {
    if (maskHorizontal_ != mask) {
        maskHorizontal_ = mask;
        update();
    }
}

void TransferFunctionEditorView::onZoomHChange(const vec2& zoomH) { updateZoom(); }

void TransferFunctionEditorView::onZoomVChange(const vec2& zoomV) { updateZoom(); }

void TransferFunctionEditorView::onHistogramModeChange(HistogramMode mode) {
    if (histogramMode_ != mode) {
        histogramMode_ = mode;
        if (histogramMode_ != HistogramMode::Off) updateHistogram();
        resetCachedContent();
        update();
    }
}

void TransferFunctionEditorView::resizeEvent(QResizeEvent* event) {
    QGraphicsView::resizeEvent(event);
    resetCachedContent();
    updateZoom();
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

void TransferFunctionEditorView::onVolumeInportInvalid() {
    stopHistCalculation_ = true;
    resetCachedContent();
    update();
}

void TransferFunctionEditorView::updateHistogram() {
    histograms_.clear();
    QRectF sRect = sceneRect();

    const HistogramContainer* histCont = getNormalizedHistograms();
    if (!histCont) return;

    for (size_t channel = 0; channel < histCont->size(); ++channel) {
        histograms_.push_back(QPolygonF());
        const std::vector<double>* normHistogramData = (*histCont)[channel].getData();
        double histSize = static_cast<double>(normHistogramData->size());
        double stepSize = sRect.width() / histSize;

        double scale = 1.0;
        switch (histogramMode_) {
            case HistogramMode::Off:  // Don't show
                return;
            case HistogramMode::All:  // show all
                scale = 1.0;
                break;
            case HistogramMode::P99:  // show 99%
                scale = (*histCont)[channel].histStats_.percentiles[99];
                break;
            case HistogramMode::P95:  // show 95%
                scale = (*histCont)[channel].histStats_.percentiles[95];
                break;
            case HistogramMode::P90:  // show 90%
                scale = (*histCont)[channel].histStats_.percentiles[90];
                break;
            case HistogramMode::Log:  // show log%
                scale = 1.0;
                break;
        }
        double height;
        double maxCount = (*histCont)[channel].getMaximumBinValue();

        histograms_.back() << QPointF(0.0f, 0.0f);

        for (double i = 0; i < histSize; i++) {
            if (histogramMode_ == HistogramMode::Log) {
                height =
                    std::log10(1.0 + maxCount * normHistogramData->at(static_cast<size_t>(i))) /
                    std::log10(maxCount);

            } else {
                height = normHistogramData->at(static_cast<size_t>(i)) / scale;
                height = std::min(height, 1.0);
            }
            height *= sRect.height();
            histograms_.back() << QPointF(i * stepSize, height)
                               << QPointF((i + 1) * stepSize, height);
        }
        histograms_.back() << QPointF(sRect.width(), 0.0f) << QPointF(0.0f, 0.0f);
    }
}

const HistogramContainer* TransferFunctionEditorView::getNormalizedHistograms() {
    if (volumeInport_ && volumeInport_->hasData()) {
        if (const auto volumeRAM = volumeInport_->getData()->getRepresentation<VolumeRAM>()) {
            if (volumeRAM->hasHistograms()) {
                return volumeRAM->getHistograms(2048, size3_t(1));
            } else if (!histCalculation_.valid()) {

                const auto done = [this]() {
                    histCalculation_.get();
                    updateHistogram();
                    resetCachedContent();
                    update();
                };

                const auto histcalc =
                    [& stop = stopHistCalculation_, volume = volumeInport_->getData(), done ]()
                        ->void {
                    auto ram = volume->getRepresentation<VolumeRAM>();
                    ram->calculateHistograms(2048, size3_t(1), stop);
                    dispatchFront(done);
                    return;
                };
                stopHistCalculation_ = false;
                histCalculation_ = dispatchPool(histcalc);
            }
        }
    }

    return nullptr;
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
    gridPen.setWidthF(1.0);
    gridPen.setCosmetic(true);
    painter->setPen(gridPen);
    painter->drawLines(lines.data(), lines.size());

    // histogram
    if (histogramMode_ != HistogramMode::Off) {
        for (auto& elem : histograms_) {
            QPen pen;
            pen.setColor(QColor(68, 102, 170, 150));
            pen.setWidthF(2.0f);
            pen.setCosmetic(true);
            painter->setPen(pen);

            QBrush brush;
            brush.setColor(QColor(68, 102, 170, 100));
            brush.setStyle(Qt::SolidPattern);
            painter->setBrush(brush);

            painter->drawPolygon(elem);
        }
    }
}

void TransferFunctionEditorView::updateZoom() {
    const auto rect = scene()->sceneRect();
    const auto zh = tfProperty_->getZoomH();
    const auto zv = tfProperty_->getZoomV();
    fitInView(zh.x * rect.width(), zv.x * rect.height(), (zh.y - zh.x) * rect.width(),
              (zv.y - zv.x) * rect.height(), Qt::IgnoreAspectRatio);
}

}  // namespace inviwo
