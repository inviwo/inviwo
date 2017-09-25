/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2017 Inviwo Foundation
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

#include <modules/animationqt/animationviewqt.h>
#include <modules/animationqt/animationeditorqt.h>
#include <modules/animation/animationcontroller.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QWheelEvent>
#include <QPainter>
#include <QScrollBar>
#include <qmath.h>
#include <warn/pop>

namespace inviwo {

namespace animation {

constexpr auto LineWidth = 0.5;

AnimationViewQt::AnimationViewQt(AnimationController& controller)
    : QGraphicsView(), controller_(controller) {
    setMouseTracking(true);
    setRenderHint(QPainter::Antialiasing, true);
    setViewportUpdateMode(QGraphicsView::SmartViewportUpdate);
    setCacheMode(QGraphicsView::CacheBackground);
    addObservation(&controller);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
}

void AnimationViewQt::mousePressEvent(QMouseEvent* e) {
    if (e->y() < TimelineHeight) {
        pressingOnTimeline_ = true;
        setTimelinePos(e->x());
    }

    QGraphicsView::mousePressEvent(e);
}

void AnimationViewQt::mouseMoveEvent(QMouseEvent* e) {
    if (pressingOnTimeline_) {
        setTimelinePos(e->x());
    }

    QGraphicsView::mouseMoveEvent(e);
}

void AnimationViewQt::mouseReleaseEvent(QMouseEvent* e) {
    pressingOnTimeline_ = false;
    QGraphicsView::mouseReleaseEvent(e);
}

void AnimationViewQt::wheelEvent(QWheelEvent* e) {
    QPointF numPixels = e->pixelDelta() / 5.0;
    QPointF numDegrees = e->angleDelta() / 8.0 / 15;

    if (e->modifiers() == Qt::ControlModifier) {
        setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
        if (!numPixels.isNull()) {
            zoom(qPow(1.05, numPixels.y()));
        } else if (!numDegrees.isNull()) {
            zoom(qPow(1.05, numDegrees.y()));
        }
    } else {
        QGraphicsView::wheelEvent(e);
    }
    e->accept();
}

void AnimationViewQt::setTimelinePos(int x) {
    auto time = mapToScene(x, 0).x() / static_cast<double>(WidthPerSecond);
    controller_.eval(controller_.getCurrentTime(), Seconds(time));
}

void AnimationViewQt::zoom(double dz) { scale(dz, 1.0); }

void AnimationViewQt::drawBackground(QPainter* painter, const QRectF& rect) {
    painter->fillRect(rect, QColor(89, 89, 89));

    // overlay grid
    int gridSpacing = WidthPerSecond;
    QRectF sRect = frameRect();
    sRect.setWidth(std::max(sceneRect().width(), rect.width()));
    qreal right = int(sRect.right()) - (int(sRect.right()) % gridSpacing);
    QVarLengthArray<QLineF, 100> lines;

    for (qreal x = sRect.left(); x <= right; x += gridSpacing) {
        lines.append(QLineF(x, sRect.top(), x, sRect.bottom()));
    }

    QPen gridPen;
    gridPen.setColor(QColor(102, 102, 102));
    gridPen.setWidthF(LineWidth);
    gridPen.setCosmetic(true);
    painter->setPen(gridPen);
    painter->drawLines(lines.data(), lines.size());
}

void AnimationViewQt::drawForeground(QPainter* painter, const QRectF& rect) {
    QRectF sRect(0, 0, 0, 0);
    sRect.setWidth(std::max(sceneRect().width(), rect.width()));
    sRect.setHeight(TimelineHeight);

    // Background rect
    QPen pen;
    pen.setColor(QColor(102, 102, 102));
    pen.setWidthF(LineWidth);
    pen.setCosmetic(true);
    painter->setPen(pen);
    painter->fillRect(sRect, QColor(180, 180, 180));

    int gridSpacing = WidthPerSecond;
    qreal right = int(sRect.right()) - (int(sRect.right()) % gridSpacing);
    QVarLengthArray<QLineF, 100> lines;
    QVarLengthArray<QPointF, 100> points;

    for (qreal x = sRect.left(); x <= right; x += gridSpacing) {
        lines.append(QLineF(x, sRect.top(), x, sRect.bottom()));
        points.append(QPointF(x, 30));
    }

    // Grid
    QPen gridPen;
    gridPen.setColor(QColor(102, 102, 102));
    gridPen.setWidthF(LineWidth);
    gridPen.setCosmetic(true);
    painter->setPen(gridPen);
    painter->drawLines(lines.data(), lines.size());

    // Little hack to render text with correct scale
    painter->save();
    painter->resetTransform();
    // Time stamps
    char buf[32];
    for (const auto& p : points) {
        snprintf(buf, 32, "%.4f", p.x() / static_cast<double>(WidthPerSecond));
        painter->drawText(mapFromScene(p.x(), p.y()), QString(buf));
    }
    painter->restore();

    // Current time
    auto x = controller_.getCurrentTime().count() * WidthPerSecond;
    QPen timePen;
    timePen.setColor(QColor(255, 255, 255));
    timePen.setWidthF(1.0);
    timePen.setCosmetic(true);
    timePen.setStyle(Qt::DashLine);
    painter->setPen(timePen);
    painter->drawLine(QLineF(x, rect.top(), x, rect.bottom()));
}

void AnimationViewQt::onStateChanged(AnimationController* controller, AnimationState oldState,
                                     AnimationState newState) {
    this->viewport()->update();
}

void AnimationViewQt::onTimeChanged(AnimationController* controller, Seconds oldTime, Seconds newTime) {
    this->viewport()->update();
}

}  // namespace

}  // namespace
