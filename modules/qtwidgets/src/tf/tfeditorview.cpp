
/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2026 Inviwo Foundation
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

#include <modules/qtwidgets/tf/tfeditorview.h>

#include <inviwo/core/algorithm/axislabeling.h>
#include <inviwo/core/datastructures/datamapper.h>
#include <inviwo/core/datastructures/histogram.h>
#include <inviwo/core/network/networklock.h>
#include <inviwo/core/util/glmvec.h>
#include <inviwo/core/util/zip.h>
#include <modules/qtwidgets/tf/tfpropertyconcept.h>
#include <modules/qtwidgets/inviwoqtutils.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <algorithm>
#include <bitset>
#include <cmath>
#include <cstddef>
#include <functional>
#include <memory>
#include <type_traits>
#include <vector>

#include <QColor>
#include <QFlags>
#include <QFont>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QLineF>
#include <QList>
#include <QPainter>
#include <QPen>
#include <QPoint>
#include <QPointF>
#include <QPolygonF>
#include <QRect>
#include <QRectF>
#include <QString>
#include <QVector>
#include <QWheelEvent>
#include <Qt>
#include <QBrush>
#include <QScrollBar>
#include <QtGlobal>
#include <glm/vec2.hpp>

class QLineF;
class QResizeEvent;
class QWheelEvent;
class QWidget;

namespace inviwo {

TFEditorView::TFEditorView(TFPropertyConcept* tfProperty, QGraphicsScene* scene, QWidget* parent)
    : QGraphicsView(scene, parent)
    , property_{tfProperty}
    , histogramState_{
          .mode = tfProperty->getHistogramMode(),
          .selection = tfProperty->getHistogramSelection(),
      }
    , leftPressed_{false}
    , zooming_{false} {

    setMouseTracking(true);
    setRenderHint(QPainter::Antialiasing, true);
    setViewportUpdateMode(QGraphicsView::SmartViewportUpdate);

    setFocusPolicy(Qt::StrongFocus);
    scale(1.0, -1.0);  // put origin to bottom left corner
    setAlignment(Qt::AlignLeft | Qt::AlignBottom);
    setMinimumSize(255, 100);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    setDragMode(QGraphicsView::NoDrag);
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);

    setCornerWidget(new QWidget());

    property_->addObserver(this);

    connect(scene, &QGraphicsScene::sceneRectChanged, this, [this](const QRectF&) {
        if (leftPressed_) return;
        onSceneRectChanged();
    });
    histogramChangeHandle_ =
        property_->onHistogramChange([this](TFPropertyConcept::HistogramChange change,
                                            const std::vector<Histogram1D>& histograms) {
            histogramState_.change = change;
            histogramState_.histograms = histograms;
            histogramState_.polygons = HistogramState::createHistogramPolygons(
                histogramState_.histograms, histogramState_.mode);
            resetCachedContent();
            update();
        });

    setSceneRect(scene->sceneRect());
    updateZoomFromProperty();
}

TFEditorView::~TFEditorView() = default;

void TFEditorView::onZoomHChange(const dvec2&) { updateZoomFromProperty(); }

void TFEditorView::onZoomVChange(const dvec2&) { updateZoomFromProperty(); }

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

void TFEditorView::keyPressEvent(QKeyEvent* event) { QGraphicsView::keyPressEvent(event); }
void TFEditorView::keyReleaseEvent(QKeyEvent* event) { QGraphicsView::keyReleaseEvent(event); }

void TFEditorView::wheelEvent(QWheelEvent* event) {
    util::KeepTrueWhileInScope keepTrue(&zooming_);
    if (event->modifiers() & Qt::ControlModifier) {
        const QPointF numPixels = event->pixelDelta() / 5.0;
        const QPointF numDegrees = event->angleDelta() / 8.0 / 15;
        double dx = 1.0;
        double dy = 1.0;
        if (!numPixels.isNull()) {
            dx = qPow(1.025, std::max(-15.0, std::min(15.0, numPixels.x())));
            dy = qPow(1.025, std::max(-15.0, std::min(15.0, -numPixels.y())));
        } else if (!numDegrees.isNull()) {
            dx = qPow(1.025, std::max(-15.0, std::min(15.0, numDegrees.x())));
            dy = qPow(1.025, std::max(-15.0, std::min(15.0, -numDegrees.y())));
        }

        if (event->modifiers() & Qt::ShiftModifier) {
            dx = dy;
            dy = 1.0;
        } else if (event->modifiers() & Qt::AltModifier) {
            dx = 1.0;
        }

        const auto hight = static_cast<double>(viewport()->height() - 1);
        const auto width = static_cast<double>(viewport()->width() - 1);

        const auto sceneHeight = scene()->sceneRect().height();
        const auto sceneWidth = scene()->sceneRect().width();

        static constexpr double maxZoom = 100.0;

        const auto t = transform();
        const auto nt = QTransform{
            std::clamp(t.m11() * dx, width / sceneWidth, maxZoom * width / sceneWidth),
            0.0,
            0.0,
            std::clamp(t.m22() * dy, -(maxZoom * hight / sceneHeight), -(hight / sceneHeight)),
            t.dx(),
            t.dy()};

        setTransform(nt, false);
        setSceneRect(scene()->sceneRect());
        event->accept();

    } else if (event->modifiers() & Qt::ShiftModifier) {
        // horizontal scrolling
        const auto modifiers = event->modifiers();
        // remove the shift key temporarily from the event
        event->setModifiers(modifiers ^ Qt::ShiftModifier);
        horizontalScrollBar()->event(event);
        // restore previous modifiers
        event->setModifiers(modifiers);
        event->accept();

    } else {
        QGraphicsView::wheelEvent(event);
    }

    const auto [currentHRange, currentVRange] = getZoom();
    property_->setZoomH(currentHRange.x, currentHRange.y);
    property_->setZoomV(currentVRange.x, currentVRange.y);
}

void TFEditorView::mousePressEvent(QMouseEvent* event) {
    leftPressed_ = event->buttons() & Qt::LeftButton;
    QGraphicsView::mousePressEvent(event);
}
void TFEditorView::mouseReleaseEvent(QMouseEvent* event) {
    leftPressed_ = event->buttons() & Qt::LeftButton;
    QGraphicsView::mouseReleaseEvent(event);
    if (!leftPressed_) onSceneRectChanged();
}

void TFEditorView::resizeEvent(QResizeEvent* event) {
    QGraphicsView::resizeEvent(event);
    resetCachedContent();
    updateZoomFromProperty();
}

std::pair<dvec2, dvec2> TFEditorView::getZoom() const {
    const auto bl = viewport()->rect().bottomLeft();
    const auto tr = viewport()->rect().topRight();
    const auto min = mapToScene(bl);
    const auto max = mapToScene(tr);

    return {{min.x(), max.x()}, {min.y(), max.y()}};
}

void TFEditorView::fitViewToScene() {
    util::KeepTrueWhileInScope keepTrue(&zooming_);
    const auto newSceneRect = scene()->sceneRect();
    fitViewToRect(newSceneRect);
    setSceneRect(newSceneRect);

    const auto [currentHRange, currentVRange] = getZoom();
    property_->setZoomH(currentHRange.x, currentHRange.y);
    property_->setZoomV(currentVRange.x, currentVRange.y);
}

void TFEditorView::onSceneRectChanged() {
    util::KeepTrueWhileInScope keepTrue(&zooming_);
    const auto newSceneRect = scene()->sceneRect();

    const auto [maxHRange, maxVRange] = std::pair<dvec2, dvec2>{
        {newSceneRect.left(), newSceneRect.right()}, {newSceneRect.top(), newSceneRect.bottom()}};
    const auto [currentHRange, currentVRange] = getZoom();

    auto newHRange = currentHRange;
    auto newVRange = currentVRange;

    if (currentHRange.x < maxHRange.x) newHRange.x = maxHRange.x;
    if (currentHRange.y > maxHRange.y) newHRange.y = maxHRange.y;
    if (currentVRange.x < maxVRange.x) newVRange.x = maxVRange.x;
    if (currentVRange.y > maxVRange.y) newVRange.y = maxVRange.y;

    if (newHRange != currentHRange || newVRange != currentVRange) {
        const auto newRect = QRectF{QPointF{newHRange.x, newVRange.y},
                                    QSizeF{newHRange.y - newHRange.x, newVRange.y - newVRange.x}};
        fitViewToRect(newRect);
        property_->setZoomH(newHRange.x, newHRange.y);
        property_->setZoomV(newVRange.x, newVRange.y);
    } else {
        property_->setZoomH(currentHRange.x, currentHRange.y);
        property_->setZoomV(currentVRange.x, currentVRange.y);
    }
    setSceneRect(newSceneRect);
}

void TFEditorView::updateZoomFromProperty() {
    if (zooming_) return;
    const auto zh = property_->getZoomH();
    const auto zv = property_->getZoomV();
    const auto [currentHz, currentVz] = getZoom();
    if (util::almostEqual(zh, currentHz) && util::almostEqual(zv, currentVz)) {
        return;
    }

    const auto newRect = QRectF{QPointF{zh.x, zv.x}, QSizeF{zh.y - zh.x, zv.y - zv.x}};
    fitViewToRect(newRect);
    setSceneRect(scene()->sceneRect());
}

void TFEditorView::fitViewToRect(const QRectF& sceneRect) {
    const auto hight = static_cast<double>(viewport()->height() - 1);
    const auto width = static_cast<double>(viewport()->width() - 1);
    const auto sceneHeight = sceneRect.height();
    const auto sceneWidth = sceneRect.width();
    const auto nt = QTransform{width / sceneWidth, 0.0, 0.0, -hight / sceneHeight, 0.0, 0.0};

    setTransform(nt);
    centerOn(sceneRect.center());
}

void TFEditorView::drawGrid(QPainter* painter, const QRectF& updateRect,
                            const DataMapper& sceneDM) {
    static constexpr QColor colorBg(89, 89, 89);
    static constexpr QColor colorGrid(122, 122, 122);
    static constexpr QColor colorOrigin(132, 136, 145);

    const utilqt::Save saved{painter};
    painter->fillRect(updateRect, colorBg);

    const QPen gridPen = utilqt::cosmeticPen(colorGrid, 1.0);
    const QPen originPen = utilqt::cosmeticPen(colorOrigin, 2.0);
    std::optional<double> zeroPos;

    // s* stands for scene coordinates,
    // v* for value coordinates
    // w* for widget coordinates

    const auto sRight = mapToScene(rect().topRight()).x();
    const auto sLeft = mapToScene(rect().topLeft()).x();

    const auto vRight = sceneDM.mapFromNormalizedToValue(sRight);
    const auto vLeft = sceneDM.mapFromNormalizedToValue(sLeft);

    const auto vRange = plot::labelingExtendedWilkinson(vLeft, vRight, 10);
    for (const auto x : plot::rangeView(vRange)) {
        if (plot::almostEqual(x, 0.0)) {
            painter->setPen(originPen);
            zeroPos = x;
        } else {
            painter->setPen(gridPen);
        }
        const auto sx = sceneDM.mapFromValueToNormalized(x);
        if (sx >= updateRect.left() && sx <= updateRect.right()) {
            painter->drawLine(QLineF(sx, updateRect.top(), sx, updateRect.bottom()));
        }
    }

    painter->setFont(QFont{"Segoe", 12, QFont::Normal, false});
    painter->resetTransform();
    painter->setBackgroundMode(Qt::OpaqueMode);  // Default is Qt::TransparentMode
    painter->setBackground(QBrush(colorBg));     // Set background color

    const auto sStart = sceneDM.mapFromValueToNormalized(vRange.start);
    const auto sStop = sceneDM.mapFromValueToNormalized(vRange.stop);

    const auto wStart = QPoint{mapFromScene(QPointF(sStart, 0.0)).x(), rect().top()};
    const auto wStop = QPoint{mapFromScene(QPointF(sStop, 0.0)).x(), rect().top()};

    static constexpr double tw = 100.0;
    static constexpr double th = 25.0;
    static constexpr double hpad = -2.0;
    static constexpr double vpad = 2.0;

    if (zeroPos && zeroPos != vRange.start && zeroPos != vRange.stop) {
        const auto sZero = sceneDM.mapFromValueToNormalized(*zeroPos);
        const auto zero = QPoint{mapFromScene(QPointF(sZero, 0.0)).x(), rect().top()};
        painter->drawText(QRectF{zero + QPointF{-tw / 2.0, vpad}, QSizeF{tw, th}},
                          Qt::AlignHCenter | Qt::AlignTop,
                          utilqt::toQString(fmt::format("{:.4g}", *zeroPos)));
    }

    painter->drawText(QRectF{wStart + QPointF{hpad, vpad}, QSizeF{tw, th}},
                      Qt::AlignLeft | Qt::AlignTop,
                      utilqt::toQString(fmt::format("{:.4g}", vRange.start)));
    painter->drawText(QRectF{wStop + QPointF{-tw - hpad, vpad}, QSizeF{tw, th}},
                      Qt::AlignRight | Qt::AlignTop,
                      utilqt::toQString(fmt::format("{:.4g}", vRange.stop)));
}

void TFEditorView::drawBackground(QPainter* painter, const QRectF& updateRect) {
    if (updateRect.height() <= 0 || updateRect.width() <= 0) {
        return;
    }

    // DataMapper for mapping between value and scene coordinates. Initialized to identity
    // mapping use the property DataMapper if relative and available, otherwise use default
    // identity mapping this means that "scene coordinates" are equivalent to "normalized
    // coordinates"
    DataMapper sceneDM{dvec2{0.0, 1.0}, dvec2{0.0, 1.0}};
    if (property_->allRelative()) {
        if (const auto* dm = property_->getDataMap()) {
            sceneDM = *dm;
        }
    }

    drawGrid(painter, updateRect, sceneDM);
    histogramState_.paintHistograms(painter, sceneRect(), rect(), sceneDM);
}

namespace {
enum class ColorType { Text = 0, Line, Fill };

QColor getColor(size_t channel, size_t nChannels, ColorType type) {
    static constexpr std::array<QColor, 4> colors{
        QColor{170, 68, 68, 255},   // Red
        QColor{68, 170, 85, 255},   // Green
        QColor{68, 102, 170, 255},  // Blue
        QColor{170, 68, 154, 255}   // Purple
    };

    static constexpr std::array<int, 3> alpha{255, 150, 100};

    auto color = colors[nChannels == 1 ? 2 : channel % colors.size()];
    color.setAlpha(alpha[static_cast<int>(type)]);

    if (type == ColorType::Text) {
        color = color.lighter();
    }
    return color;
}

QRect textRect(QRect rect, size_t count = 0) {
    auto newRect = QRect(0, 0, rect.width(), rect.height()).adjusted(20, 30, -20, -10);
    newRect.adjust(0, 20 * static_cast<int>(count), 0, 0);
    return newRect;
}

void setPenAndFont(QPainter* painter, ColorType type, size_t channel = 0, size_t nChannels = 1) {
    painter->setPen(QPen{getColor(channel, nChannels, type)});
    auto font = painter->font();
    font.setPointSize(12);
    painter->setFont(font);
}

}  // namespace

QPolygonF TFEditorView::HistogramState::createHistogramPolygon(const Histogram1D& histogram,
                                                               HistogramMode mode) {
    const auto stepSize = 1.0 / histogram.counts.size();

    QPolygonF polygon{};
    polygon << QPointF(0.0, 0.0);

    if (mode == HistogramMode::Log) {
        const auto maxCount = static_cast<double>(std::max(histogram.maxCount, size_t{1}));
        const auto scale = std::log10(maxCount);
        for (size_t i = 0; i < histogram.counts.size(); i++) {
            const double height =
                std::log10(1.0 + static_cast<double>(histogram.counts[i])) / scale;
            polygon << QPointF(static_cast<double>(i) * stepSize, height)
                    << QPointF(static_cast<double>(i + 1) * stepSize, height);
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
        const auto scaleSafe = std::max(scale, 1.0);
        for (size_t i = 0; i < histogram.counts.size(); i++) {
            const auto height = std::min(static_cast<double>(histogram.counts[i]) / scaleSafe, 1.0);
            polygon << QPointF(static_cast<double>(i) * stepSize, height)
                    << QPointF(static_cast<double>(i + 1) * stepSize, height);
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

void TFEditorView::HistogramState::paintHistogram(QPainter* painter, const QPolygonF& polygon,
                                                  size_t channel, size_t nChannels,
                                                  const QRectF& sceneRect, const DataMapper& dataDM,
                                                  const DataMapper& sceneDM) {
    // Histogram polygon are defined in normalized [0,1] coordinates. And is mapped
    // to value range using the dataDM. We use the sceneDM to map from value range to the scene
    // coordinates which are represented as the normalized range of the sceneDM.
    const auto sStart = sceneDM.mapFromValueToNormalized(dataDM.mapFromNormalizedToValue(0.0));
    const auto sStop = sceneDM.mapFromValueToNormalized(dataDM.mapFromNormalizedToValue(1.0));

    const utilqt::Save saved{painter};
    painter->setPen(utilqt::cosmeticPen(getColor(channel, nChannels, ColorType::Line), 2.0));
    painter->setBrush(QBrush{getColor(channel, nChannels, ColorType::Fill), Qt::SolidPattern});

    painter->setTransform(QTransform::fromScale(sStop - sStart, sceneRect.height()) *
                              QTransform::fromTranslate(sStart, sceneRect.y()),
                          true);
    painter->drawPolygon(polygon);
}

void TFEditorView::HistogramState::paintLabel(QPainter* painter, size_t channel, size_t count,
                                              size_t nChannels, const QRect& rect,
                                              std::string_view overflow) {
    const utilqt::Save saved{painter};
    painter->resetTransform();
    setPenAndFont(painter, ColorType::Text, channel, nChannels);
    painter->drawText(textRect(rect, count), Qt::AlignRight | Qt::AlignTop,
                      utilqt::toQString(fmt::format("Channel: {}{}", channel + 1, overflow)));
}

void TFEditorView::HistogramState::paintState(QPainter* painter, const QRect& rect) const {
    const utilqt::Save saved{painter};
    painter->resetTransform();
    setPenAndFont(painter, ColorType::Text);
    if (change == TFPropertyConcept::HistogramChange::Requested) {
        painter->drawText(textRect(rect), Qt::AlignRight | Qt::AlignTop,
                          QString("Calculating histogram..."));
    } else if (change == TFPropertyConcept::HistogramChange::NoData) {
        painter->drawText(textRect(rect), Qt::AlignRight | Qt::AlignTop,
                          QString("Histogram not available"));
    }
}

void TFEditorView::HistogramState::paintHistograms(QPainter* painter, const QRectF& sceneRect,
                                                   const QRect& rect,
                                                   const DataMapper& dataMap) const {
    if (mode == HistogramMode::Off) return;

    paintState(painter, rect);

    size_t total = 0;
    for (auto&& [channel, histogram] : util::enumerate(polygons)) {
        if (!selection[channel]) continue;
        ++total;
    }

    for (auto&& [channel, polygon, histogram] : util::enumerate(polygons, histograms)) {
        if (!selection[channel]) continue;
        paintHistogram(painter, polygon, channel, total, sceneRect, histogram.dataMap, dataMap);
    }

    size_t count = 0;
    for (auto&& [channel, histogram] : util::enumerate(polygons)) {
        if (!selection[channel]) continue;
        const auto overflow = histograms[channel].overflow > 0;
        const auto underflow = histograms[channel].underflow > 0;
        if (overflow || underflow) {
            const auto outside = histograms[channel].overflow + histograms[channel].underflow;
            if (outside > histograms[channel].totalCounts / 10000) {
                paintLabel(painter, channel, count, total, rect,
                           fmt::format(", underflow: {:5.2f}%, overflow: {:5.2f}%",
                                       100.0 * static_cast<double>(histograms[channel].underflow) /
                                           static_cast<double>(histograms[channel].totalCounts),
                                       100.0 * static_cast<double>(histograms[channel].overflow) /
                                           static_cast<double>(histograms[channel].totalCounts)));
            } else {
                paintLabel(
                    painter, channel, count, total, rect,
                    fmt::format(", underflow: {}, overflow: {}", histograms[channel].underflow,
                                histograms[channel].overflow));
            }

        } else {
            paintLabel(painter, channel, count, total, rect, "");
        }

        ++count;
    }
}

}  // namespace inviwo
