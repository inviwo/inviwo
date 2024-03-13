/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2024 Inviwo Foundation
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
#include <inviwo/core/util/glmvec.h>                 // for dvec2
#include <inviwo/core/util/zip.h>                    // for enumerate, zipIterator, zipper
#include <modules/qtwidgets/tf/tfeditorview.h>       // for TFEditorView
#include <modules/qtwidgets/tf/tfpropertyconcept.h>  // for TFPropertyConcept
#include <modules/qtwidgets/inviwoqtutils.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <algorithm>    // for min, max
#include <bitset>       // for __bitset<>::reference, bitset, bitse...
#include <cmath>        // for log10, pow
#include <cstddef>      // for size_t
#include <functional>   // for __base
#include <memory>       // for shared_ptr, shared_ptr<>::element_type
#include <type_traits>  // for remove_extent_t
#include <vector>       // for vector, __vector_base<>::value_type

#include <QColor>          // for QColor
#include <QFlags>          // for QFlags
#include <QFont>           // for QFont
#include <QGraphicsScene>  // for QGraphicsScene
#include <QGraphicsView>   // for QGraphicsView, QGraphicsView::CacheB...
#include <QLineF>          // for QLineF
#include <QList>           // for QList
#include <QPainter>        // for QPainter, QPainter::Antialiasing
#include <QPen>            // for QPen
#include <QPoint>          // for operator/, QPoint
#include <QPointF>         // for QPointF
#include <QPolygonF>       // for QPolygonF
#include <QRect>           // for QRect
#include <QRectF>          // for QRectF
#include <QString>         // for QString
#include <QVector>         // for QVector
#include <QWheelEvent>     // for QWheelEvent
#include <Qt>              // for operator|, AlignRight, AlignTop, Con...
#include <QBrush>          // for QBrush
#include <QtGlobal>        // for operator==, QT_VERSION, QT_VERSION_C...
#include <glm/vec2.hpp>    // for vec<>::(anonymous), vec, operator!=

class QLineF;
class QResizeEvent;
class QWheelEvent;
class QWidget;

namespace inviwo {

TFEditorView::TFEditorView(TFPropertyConcept* tfProperty, QGraphicsScene* scene, QWidget* parent)
    : QGraphicsView(scene, parent)
    , tfPropertyPtr_{tfProperty}
    , histogramState_{.mode = tfProperty->getHistogramMode(),
                      .selection = tfProperty->getHistogramSelection()}
    , maskHorizontal_{0.0, 1.0} {

    setMouseTracking(true);
    setRenderHint(QPainter::Antialiasing, true);
    setViewportUpdateMode(QGraphicsView::SmartViewportUpdate);

    this->setCacheMode(QGraphicsView::CacheBackground);

    tfPropertyPtr_->addObserver(this);

    histogramChangeHandle_ =
        tfPropertyPtr_->onHistogramChange([this](TFPropertyConcept::HistogramChange change,
                                                 const std::vector<Histogram1D>& histograms) {
            histogramState_.change = change;
            histogramState_.histograms = std::move(histograms);
            histogramState_.polygons = HistogramState::createHistogramPolygons(
                histogramState_.histograms, histogramState_.mode);
            resetCachedContent();
            update();
        });
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
    if (histogramState_.mode != mode) {
        histogramState_.mode = mode;

        histogramState_.polygons = HistogramState::createHistogramPolygons(
            histogramState_.histograms, histogramState_.mode);
        resetCachedContent();
        update();
    }
}

void TFEditorView::onHistogramSelectionChange(HistogramSelection selection) {
    if (histogramState_.selection != selection) {
        histogramState_.selection = selection;
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
        auto zoomCenter = event->position().x() / width() * zoomExtent + horizontal.x;

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

QPolygonF TFEditorView::HistogramState::createHistogramPolygon(const Histogram1D& histogram,
                                                               HistogramMode mode) {
    const auto stepSize = 1.0 / histogram.counts.size();

    QPolygonF polygon{};
    polygon << QPointF(0.0, 0.0);

    if (mode == HistogramMode::Log) {
        double maxCount = histogram.maxCount;
        for (size_t i = 0; i < histogram.counts.size(); i++) {
            double height = std::log10(1.0 + histogram.counts[i]) / std::log10(maxCount);
            polygon << QPointF(i * stepSize, height) << QPointF((i + 1) * stepSize, height);
        }
    } else {
        const double scale = [&]() {
            switch (mode) {
                case HistogramMode::All:  // show all
                    return histogram.histStats.percentiles[100];
                case HistogramMode::P99:  // show 99%
                    return histogram.histStats.percentiles[99];
                case HistogramMode::P95:  // show 95%
                    return histogram.histStats.percentiles[95];
                case HistogramMode::P90:  // show 90%
                    return histogram.histStats.percentiles[90];
                default:
                    return histogram.histStats.percentiles[100];
            }
        }();
        for (size_t i = 0; i < histogram.counts.size(); i++) {
            double height = histogram.counts[i] / scale;
            height = std::min(height, 1.0);
            polygon << QPointF(i * stepSize, height) << QPointF((i + 1) * stepSize, height);
        }
    }
    polygon << QPointF(1.0f, 0.0f) << QPointF(0.0f, 0.0f);

    return polygon;
}

std::vector<QPolygonF> TFEditorView::HistogramState::createHistogramPolygons(
    const std::vector<Histogram1D>& histograms, HistogramMode mode) {
    std::vector<QPolygonF> polygons;

    if (mode != HistogramMode::Off) {
        for (const auto& histogram : histograms) {
            polygons.push_back(createHistogramPolygon(histogram, mode));
        }
    }

    return polygons;
}

void TFEditorView::drawBackground(QPainter* painter, const QRectF& rect) {
    painter->fillRect(rect, QColor(89, 89, 89));

    // overlay grid
    const QColor colorGrid(102, 102, 102);
    const QColor colorOrigin(102, 106, 115);
    const int gridSpacing = 50;

    const auto sRect = sceneRect();
    QPen gridPen;
    gridPen.setCosmetic(true);

    double gridOrigin = sRect.left();  // horizontal origin of the grid

    // adjust grid origin if there is a data mapper available
    const auto& datamap = tfPropertyPtr_->getDataMap();
    if ((datamap.valueRange.x < 0.0) && (datamap.valueRange.y > 0.0)) {
        gridOrigin = datamap.mapFromValueToNormalized(0.0) * sRect.width() + sRect.left();

        // draw line at zero
        gridPen.setWidthF(3.0f);
        gridPen.setColor(colorOrigin);
        painter->setPen(gridPen);
        painter->drawLine(
            QLineF(QPointF(gridOrigin, sRect.bottom()), QPointF(gridOrigin, sRect.top())));
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

    histogramState_.paintHistograms(painter, sceneRect(), this->rect());
}

void TFEditorView::HistogramState::paintHistogram(QPainter* painter, const QPolygonF& histogram,
                                                  const QRectF& sceneRect) const {
    utilqt::Save saved{painter};
    QPen pen;
    pen.setColor(QColor(68, 102, 170, 150));
    pen.setWidthF(2.0f);
    pen.setCosmetic(true);
    painter->setPen(pen);

    QBrush brush;
    brush.setColor(QColor(68, 102, 170, 100));
    brush.setStyle(Qt::SolidPattern);
    painter->setBrush(brush);

    painter->setTransform(QTransform::fromScale(sceneRect.width(), sceneRect.height()), true);
    painter->drawPolygon(histogram);
}

void TFEditorView::HistogramState::paintLabel(QPainter* painter, size_t index, size_t count,
                                              const QRect& rect) const {
    utilqt::Save saved{painter};
    painter->resetTransform();
    QPen pen;
    pen.setColor(QColor(68, 102, 170, 255).lighter());
    painter->setPen(pen);
    auto font = painter->font();
    font.setPointSize(12);
    painter->setFont(font);
    painter->drawText(QRect(0, 0, rect.width(), rect.height())
                          .adjusted(20, 10, -20, -10 - 10 * static_cast<int>(count)),
                      Qt::AlignRight | Qt::AlignTop,
                      utilqt::toQString(fmt::format("Channel: {}", index + 1)));
}

void TFEditorView::HistogramState::paintState(QPainter* painter, const QRect& rect) const {
    utilqt::Save saved{painter};
    painter->resetTransform();
    QPen pen;
    pen.setColor(QColor(68, 102, 170, 255).lighter());
    painter->setPen(pen);
    auto font = painter->font();
    font.setPointSize(12);
    painter->setFont(font);
    if (change == TFPropertyConcept::HistogramChange::Requested) {
        painter->drawText(QRect(0, 0, rect.width(), rect.height()).adjusted(20, 10, -20, -10),
                          Qt::AlignRight | Qt::AlignTop, QString("Calculating histogram..."));
    } else if (change == TFPropertyConcept::HistogramChange::NoData) {
        painter->drawText(QRect(0, 0, rect.width(), rect.height()).adjusted(20, 10, -20, -10),
                          Qt::AlignRight | Qt::AlignTop, QString("Histogram not available"));
    }
}

void TFEditorView::HistogramState::paintHistograms(QPainter* painter, const QRectF& sceneRect,
                                                   const QRect& rect) const {
    if (mode == HistogramMode::Off) return;

    paintState(painter, rect);

    size_t count = 0;
    for (auto&& [i, histogram] : util::enumerate(polygons)) {
        if (!selection[i]) continue;
        paintLabel(painter, i, count, rect);
        paintHistogram(painter, histogram, sceneRect);
        ++count;
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
