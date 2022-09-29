/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2022 Inviwo Foundation
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

#include <inviwo/core/datastructures/datamapper.h>   // for DataMapper
#include <inviwo/core/datastructures/histogram.h>    // for HistogramMode, HistogramContainer
#include <inviwo/core/network/networklock.h>         // for NetworkLock
#include <inviwo/core/ports/volumeport.h>            // for VolumeInport
#include <inviwo/core/util/glmvec.h>                 // for dvec2
#include <inviwo/core/util/zip.h>                    // for enumerate, zipIterator, zipper
#include <modules/qtwidgets/tf/tfeditorview.h>       // for TFEditorView
#include <modules/qtwidgets/tf/tfpropertyconcept.h>  // for TFPropertyConcept

#include <algorithm>                                 // for min, max
#include <bitset>                                    // for __bitset<>::reference, bitset, bitse...
#include <cmath>                                     // for log10, pow
#include <cstddef>                                   // for size_t
#include <functional>                                // for __base
#include <memory>                                    // for shared_ptr, shared_ptr<>::element_type
#include <type_traits>                               // for remove_extent_t
#include <vector>                                    // for vector, __vector_base<>::value_type

#include <warn/push>
#include <warn/ignore/all>
#include <QColor>                                    // for QColor
#include <QFlags>                                    // for QFlags
#include <QFont>                                     // for QFont
#include <QGraphicsScene>                            // for QGraphicsScene
#include <QGraphicsView>                             // for QGraphicsView, QGraphicsView::CacheB...
#include <QLineF>                                    // for QLineF
#include <QList>                                     // for QList
#include <QPainter>                                  // for QPainter, QPainter::Antialiasing
#include <QPen>                                      // for QPen
#include <QPoint>                                    // for operator/, QPoint
#include <QPointF>                                   // for QPointF
#include <QPolygonF>                                 // for QPolygonF
#include <QRect>                                     // for QRect
#include <QRectF>                                    // for QRectF
#include <QString>                                   // for QString
#include <QVector>                                   // for QVector
#include <QWheelEvent>                               // for QWheelEvent
#include <QtCore/qnamespace.h>                       // for operator|, AlignRight, AlignTop, Con...
#include <QtGlobal>                                  // for operator==, QT_VERSION, QT_VERSION_C...
#include <glm/vec2.hpp>                              // for vec<>::(anonymous), vec, operator!=
#include <qbrush.h>                                  // for QBrush

class QLineF;
class QResizeEvent;
class QWheelEvent;
class QWidget;

#include <warn/pop>

namespace inviwo {

TFEditorView::TFEditorView(util::TFPropertyConcept* tfProperty, QGraphicsScene* scene,
                           QWidget* parent)
    : QGraphicsView(scene, parent)
    , tfPropertyPtr_{tfProperty}
    , volumeInport_{tfProperty->getVolumeInport()}
    , histogramMode_{tfProperty->getHistogramMode()}
    , histogramSelection_{tfProperty->getHistogramSelection()}
    , maskHorizontal_{0.0, 1.0} {

    setMouseTracking(true);
    setRenderHint(QPainter::Antialiasing, true);
    setViewportUpdateMode(QGraphicsView::SmartViewportUpdate);

    this->setCacheMode(QGraphicsView::CacheBackground);

    tfPropertyPtr_->addObserver(this);

    if (volumeInport_) {
        const auto portChange = [this]() { updateHistogram(); };
        callbackOnChange = volumeInport_->onChangeScoped(portChange);
        callbackOnConnect = volumeInport_->onConnectScoped(portChange);
        callbackOnDisconnect = volumeInport_->onDisconnectScoped(portChange);
    }
    updateHistogram();
}

TFEditorView::~TFEditorView() = default;

void TFEditorView::onMaskChange(const dvec2& mask) {
    if (maskHorizontal_ != mask) {
        maskHorizontal_ = mask;
        update();
    }
}

void TFEditorView::onZoomHChange(const dvec2&) { updateZoom(); }

void TFEditorView::onZoomVChange(const dvec2&) { updateZoom(); }

void TFEditorView::onHistogramModeChange(HistogramMode mode) {
    if (histogramMode_ != mode) {
        histogramMode_ = mode;
        updateHistogram();
    }
}

void TFEditorView::onHistogramSelectionChange(HistogramSelection selection) {
    if (histogramSelection_ != selection) {
        histogramSelection_ = selection;
        resetCachedContent();
        update();
    }
}

void TFEditorView::wheelEvent(QWheelEvent* event) {
    const QPointF numPixels = event->pixelDelta() / 5.0;
    const QPointF numDegrees = event->angleDelta() / 8.0 / 15;

    const dvec2 scrollStep(0.2, 0.2);

    dvec2 delta;
    if (!numPixels.isNull()) {
        delta = dvec2(numPixels.x(), numPixels.y());
    } else if (!numDegrees.isNull()) {
        delta = dvec2(numDegrees.x(), numDegrees.y());
    } else {
        return;
    }

    NetworkLock lock(tfPropertyPtr_->getProperty());

    if (event->modifiers() == Qt::ControlModifier) {
        // zoom only horizontally relative to wheel event position
        double zoomFactor = std::pow(1.05, std::max(-15.0, std::min(15.0, -delta.y)));

        dvec2 horizontal = tfPropertyPtr_->getZoomH();
        double zoomExtent = horizontal.y - horizontal.x;

        // off-center zooming
        // relative position within current zoom range

#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
        auto zoomCenter = event->posF().x() / width() * zoomExtent + horizontal.x;
#else
        auto zoomCenter = event->position().x() / width() * zoomExtent + horizontal.x;
#endif

        double lower = zoomCenter + (horizontal.x - zoomCenter) * zoomFactor;
        double upper = zoomCenter + (horizontal.y - zoomCenter) * zoomFactor;

        tfPropertyPtr_->setZoomH(std::max(0.0, lower), std::min(1.0, upper));
    } else {
        // vertical scrolling (+ optional horizontal if two-axis wheel)

        if (event->modifiers() & Qt::ShiftModifier) {
            // horizontal scrolling: map vertical wheel movement to horizontal direction
            delta.x = -delta.y;
            delta.y = 0.0;
        }

        dvec2 horizontal = tfPropertyPtr_->getZoomH();
        dvec2 vertical = tfPropertyPtr_->getZoomV();
        dvec2 extent(horizontal.y - horizontal.x, vertical.y - vertical.x);
        // scale scroll step with current zoom range
        delta *= scrollStep * extent;

        // separate horizontal and vertical scrolling
        if (delta.x < 0.0) {
            horizontal.x = std::max(0.0, horizontal.x + delta.x);
            horizontal.y = horizontal.x + extent.x;
        } else if (delta.x > 0.0) {
            horizontal.y = std::min(1.0, horizontal.y + delta.x);
            horizontal.x = horizontal.y - extent.x;
        }
        // vertical
        if (delta.y < 0.0) {
            vertical.x = std::max(0.0, vertical.x + delta.y);
            vertical.y = vertical.x + extent.y;
        } else if (delta.y > 0.0) {
            vertical.y = std::min(1.0, vertical.y + delta.y);
            vertical.x = vertical.y - extent.y;
        }

        tfPropertyPtr_->setZoomH(horizontal.x, horizontal.y);
        tfPropertyPtr_->setZoomV(vertical.x, vertical.y);
    }

    event->accept();
}

void TFEditorView::resizeEvent(QResizeEvent* event) {
    QGraphicsView::resizeEvent(event);
    resetCachedContent();
    updateZoom();
}

void TFEditorView::drawForeground(QPainter* painter, const QRectF& rect) {
    QPen pen;
    pen.setCosmetic(true);
    pen.setWidthF(1.5);
    pen.setColor(Qt::lightGray);
    painter->setPen(pen);

    QRectF sRect = sceneRect();

    if (maskHorizontal_.x > 0.0) {
        double leftMaskBorder = maskHorizontal_.x * sRect.width();
        QRectF r(0.0, rect.top(), leftMaskBorder, rect.height());
        QLineF line(leftMaskBorder, rect.top(), leftMaskBorder, rect.bottom());
        painter->fillRect(r, QColor(25, 25, 25, 100));
        painter->drawLine(line);
    }

    if (maskHorizontal_.y < 1.0) {
        double rightMaskBorder = maskHorizontal_.y * sRect.width();
        QRectF r(rightMaskBorder, rect.top(), sRect.right() - rightMaskBorder, rect.height());
        QLineF line(rightMaskBorder, rect.top(), rightMaskBorder, rect.bottom());
        painter->fillRect(r, QColor(25, 25, 25, 100));
        painter->drawLine(line);
    }

    QGraphicsView::drawForeground(painter, rect);
}

void TFEditorView::updateHistogram(const HistogramContainer& histCont) {
    histograms_.clear();
    if (histCont.empty()) return;

    const auto sRect = sceneRect();

    for (size_t channel = 0; channel < histCont.size(); ++channel) {
        const auto& normHistogramData = histCont[channel].getData();
        const auto stepSize = sRect.width() / normHistogramData.size();

        histograms_.push_back(QPolygonF());
        double scale = 1.0;
        switch (histogramMode_) {
            case HistogramMode::Off:  // Don't show
                return;
            case HistogramMode::All:  // show all
                scale = 1.0;
                break;
            case HistogramMode::P99:  // show 99%
                scale = histCont[channel].histStats_.percentiles[99];
                break;
            case HistogramMode::P95:  // show 95%
                scale = histCont[channel].histStats_.percentiles[95];
                break;
            case HistogramMode::P90:  // show 90%
                scale = histCont[channel].histStats_.percentiles[90];
                break;
            case HistogramMode::Log:  // show log%
                scale = 1.0;
                break;
        }
        double height;
        double maxCount = histCont[channel].getMaximumBinValue();

        histograms_.back() << QPointF(0.0, 0.0);

        for (size_t i = 0; i < normHistogramData.size(); i++) {
            if (histogramMode_ == HistogramMode::Log) {
                height = std::log10(1.0 + maxCount * normHistogramData[i]) / std::log10(maxCount);

            } else {
                height = normHistogramData[i] / scale;
                height = std::min(height, 1.0);
            }
            height *= sRect.height();
            histograms_.back() << QPointF(i * stepSize, height)
                               << QPointF((i + 1) * stepSize, height);
        }
        histograms_.back() << QPointF(sRect.width(), 0.0f) << QPointF(0.0f, 0.0f);
    }
}

void TFEditorView::updateHistogram() {
    if (histogramMode_ != HistogramMode::Off && volumeInport_ && volumeInport_->isReady()) {
        if (auto volume = volumeInport_->getData()) {
            if (volume->hasHistograms()) {
                updateHistogram(volume->getHistograms());
            } else if (!histCalculation_) {
                histograms_.clear();
                histCalculation_ = volume->calculateHistograms(2048);
                histCalculation_->whenDone([this](const HistogramContainer& histograms) {
                    updateHistogram(histograms);
                    resetCachedContent();
                    update();
                    histCalculation_.reset();
                });
            }
        } else {
            histograms_.clear();
        }
    } else {
        histograms_.clear();
    }
    resetCachedContent();
    update();
}

void TFEditorView::drawBackground(QPainter* painter, const QRectF& rect) {
    painter->fillRect(rect, QColor(89, 89, 89));

    // overlay grid
    const QColor colorGrid(102, 102, 102);
    const QColor colorOrigin(102, 106, 115);
    const int gridSpacing = 50;

    QRectF sRect = sceneRect();
    QPen gridPen;
    gridPen.setCosmetic(true);

    double gridOrigin = sRect.left();  // horizontal origin of the grid
    // adjust grid origin if there is a data mapper available
    if (volumeInport_ && volumeInport_->hasData()) {
        auto& datamap = volumeInport_->getData()->dataMap_;
        if ((datamap.valueRange.x < 0.0) && (datamap.valueRange.y > 0.0)) {
            gridOrigin = datamap.mapFromValueToNormalized(0.0) * sRect.width() + sRect.left();

            // draw line at zero
            gridPen.setWidthF(3.0f);
            gridPen.setColor(colorOrigin);
            painter->setPen(gridPen);
            painter->drawLine(
                QLineF(QPointF(gridOrigin, sRect.bottom()), QPointF(gridOrigin, sRect.top())));
        }
    }

    QVector<QLineF> lines;

    // add grid lines left of origin
    double x = gridOrigin - gridSpacing;
    while (x > sRect.left()) {
        lines.push_back(QLineF(x, sRect.bottom(), x, sRect.top()));
        x -= gridSpacing;
    }
    // add grid lines right of origin
    x = gridOrigin + gridSpacing;
    while (x < sRect.right()) {
        lines.push_back(QLineF(x, sRect.bottom(), x, sRect.top()));
        x += gridSpacing;
    }

    // draw grid
    gridPen.setColor(colorGrid);
    gridPen.setWidthF(1.0);
    painter->setPen(gridPen);
    painter->drawLines(lines);

    // histogram
    if (histCalculation_ && histogramMode_ != HistogramMode::Off) {
        painter->save();
        painter->resetTransform();

        QPen pen;
        pen.setColor(QColor(180, 180, 180, 255));
        painter->setPen(pen);
        auto font = painter->font();
        font.setPointSize(12);
        painter->setFont(font);
        painter->drawText(QRect(0, 0, width(), height()).adjusted(20, 10, -20, -10),
                          Qt::AlignRight | Qt::AlignTop, QString("Calculating histogram..."));
        painter->restore();
    }

    for (auto&& [i, elem] : util::enumerate(histograms_)) {
        if (!histogramSelection_[i]) continue;

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

void TFEditorView::updateZoom() {
    const auto rect = scene()->sceneRect();
    const auto zh = tfPropertyPtr_->getZoomH();
    const auto zv = tfPropertyPtr_->getZoomV();
    fitInView(zh.x * rect.width(), zv.x * rect.height(), (zh.y - zh.x) * rect.width(),
              (zv.y - zv.x) * rect.height(), Qt::IgnoreAspectRatio);
}

}  // namespace inviwo
