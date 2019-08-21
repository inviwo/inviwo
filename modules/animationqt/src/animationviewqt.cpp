/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2019 Inviwo Foundation
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
#include <modules/animationqt/animationeditorqt.h>
#include <modules/animationqt/widgets/editorconstants.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QWheelEvent>
#include <QPainter>
#include <QScrollBar>
#include <qmath.h>
#include <QGridLayout>
#include <QScrollBar>
#include <QGraphicsScene>
#include <QStyleOption>
#include <warn/pop>

namespace inviwo {

namespace animation {

namespace {

class TimeLine : public QWidget {
public:
    TimeLine(AnimationViewQt* parent) : QWidget(parent), view_(parent) {
        setObjectName("AnimationTimeLine");
        setMouseTracking(true);
        setFixedHeight(timelineHeight);
    }
    virtual void paintEvent(QPaintEvent*) override {
        QPainter painter(this);

        // Draw grid
        auto gridPen = painter.pen();
        gridPen.setWidthF(1.0);
        gridPen.setCosmetic(true);
        auto font = painter.font();
        QFontMetrics fm{font};
        auto fontHeight = fm.lineSpacing();

        const auto gridSpacing = widthPerSecond;
        const auto viewStart = view_->mapToScene(view_->viewport()->rect().left(), 0).x();
        const auto viewEnd = view_->mapToScene(view_->viewport()->rect().right(), 0).x();

        const auto windowSize = rect().width();
        const auto digitSize = fm.horizontalAdvance('0');

        const auto tgcGuess =
            static_cast<int>(std::floor(windowSize / digitSize / std::abs(3) / 2.0));
        const auto divGuess =
            findDivisions(viewStart / widthPerSecond, viewEnd / widthPerSecond, tgcGuess);
        const auto digits = divGuess.integerDigits + divGuess.fractionalDigits +
                            (divGuess.fractionalDigits == 0 ? 0 : 1);
        const auto tgc = static_cast<int>(std::floor(windowSize / ((3 + digits) * digitSize)));
        const auto div = findDivisions(viewStart / widthPerSecond, viewEnd / widthPerSecond, tgc);

        QVector<QLineF> majorTicksLines;
        QVector<QLineF> minorTicksLines;
        QString text{"%1"};

        auto addMinor = [&](double pos) {
            for (int j = 1; j < 5; ++j) {
                const auto t =
                    view_->mapFromScene(widthPerSecond * (pos + j * div.step / 5), 0).x();
                minorTicksLines.push_back(QLineF(t, rect().top() + fontHeight, t, rect().bottom()));
            }
        };

        addMinor(div.start - div.step);

        for (size_t i = 0; i <= div.count; ++i) {
            const auto x = view_->mapFromScene(widthPerSecond * (div.start + i * div.step), 0).x();
            QRectF box(x - 0.5 * gridSpacing, rect().top(), gridSpacing, timelineHeight);
            painter.drawText(box, Qt::AlignHCenter | Qt::AlignTop | Qt::TextDontClip,
                             text.arg(div.start + i * div.step, 0, 'f', div.fractionalDigits));

            majorTicksLines.push_back(QLineF(x, rect().top() + fontHeight, x, rect().bottom()));

            addMinor(div.start + i * div.step);
        }
        addMinor(div.start + div.count * div.step);

        painter.drawLines(minorTicksLines);
        gridPen.setWidthF(2.0);
        painter.setPen(gridPen);
        painter.drawLines(majorTicksLines);

        // Current time
        const auto x =
            view_->mapFromScene(timeToScenePos(view_->getController().getCurrentTime()), 0).x();
        QPen timePen;
        timePen.setColor(QColor(255, 255, 255));
        timePen.setWidthF(1.0);
        timePen.setCosmetic(true);
        timePen.setStyle(Qt::DashLine);
        painter.setPen(timePen);
        painter.drawLine(QLineF(x, rect().top(), x, rect().bottom()));
    }

    virtual void mousePressEvent(QMouseEvent* e) override {
        pressingOnTimeline_ = true;
        view_->setTimelinePos(e->x());
        QWidget::mousePressEvent(e);
    }

    virtual void mouseMoveEvent(QMouseEvent* e) override {
        if (pressingOnTimeline_) {
            view_->setTimelinePos(e->x());
        }
        QWidget::mouseMoveEvent(e);
    }

    virtual void mouseReleaseEvent(QMouseEvent* e) override {
        pressingOnTimeline_ = false;
        QWidget::mouseReleaseEvent(e);
    }

private:
    bool pressingOnTimeline_ = false;
    AnimationViewQt* view_;
};

}  // namespace

AnimationViewQt::AnimationViewQt(AnimationController& controller, AnimationEditorQt* aScene)
    : QGraphicsView(), scene_{aScene}, controller_(controller) {

    setObjectName("AnimationView");
    setScene(scene_);
    setViewportMargins(0, timelineHeight, 0, 0);
    setSceneRect(QRectF());

    // Time Line widget
    timeline_ = new TimeLine{this};

    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

    horizontalScrollBar()->setTracking(true);
    verticalScrollBar()->setTracking(true);

    connect(horizontalScrollBar(), &QScrollBar::rangeChanged, this,
            [this]() { timeline_->update(); });

    connect(horizontalScrollBar(), &QScrollBar::valueChanged, this,
            [this]() { timeline_->update(); });

    QGridLayout* gridLayout = new QGridLayout();
    gridLayout->setSpacing(0);
    gridLayout->setMargin(0);

    gridLayout->addWidget(timeline_, 0, 0);
    gridLayout->addWidget(viewport(), 1, 0);

    setLayout(gridLayout);
    setFrameStyle(QFrame::NoFrame);

    setMouseTracking(true);
    setRenderHint(QPainter::Antialiasing, true);
    setViewportUpdateMode(QGraphicsView::BoundingRectViewportUpdate);
    setDragMode(QGraphicsView::NoDrag);
    setCacheMode(QGraphicsView::CacheBackground);
    controller_.AnimationControllerObservable::addObserver(this);
}

void AnimationViewQt::keyPressEvent(QKeyEvent* keyEvent) {
    if (keyEvent->modifiers() & Qt::ControlModifier) {
        setDragMode(QGraphicsView::ScrollHandDrag);
    } else if (keyEvent->modifiers() & Qt::ShiftModifier) {
        setDragMode(QGraphicsView::RubberBandDrag);
    }
    QGraphicsView::keyPressEvent(keyEvent);
}

void AnimationViewQt::keyReleaseEvent(QKeyEvent* keyEvent) {
    setDragMode(QGraphicsView::NoDrag);
    QGraphicsView::keyReleaseEvent(keyEvent);
}

void AnimationViewQt::mousePressEvent(QMouseEvent* e) {
    if (itemAt(e->pos()) == nullptr && e->modifiers() == Qt::NoModifier) {
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

    if (e->modifiers() & Qt::ControlModifier) {
        setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
        if (!numPixels.isNull()) {
            zoom(qPow(1.075, numPixels.y()));
        } else if (!numDegrees.isNull()) {
            zoom(qPow(1.075, numDegrees.y()));
        }
        timeline_->update();
        e->accept();
    } else {
        horizontalScrollBar()->event(e);
    }
}

void AnimationViewQt::setTimelinePos(int x) {
    const auto time = mapToScene(x, 0).x() / static_cast<double>(widthPerSecond);
    controller_.eval(controller_.getCurrentTime(), Seconds(time));
}

void AnimationViewQt::zoom(double dz) {
    if (dz < 1.0 || transform().m11() < 250.0) {
        scale(dz, 1.0);
    }
}

AnimationController& AnimationViewQt::getController() { return controller_; }

void AnimationViewQt::drawBackground(QPainter*, const QRectF&) {}

void AnimationViewQt::drawForeground(QPainter* painter, const QRectF& rect) {
    // Current time
    auto x = timeToScenePos(controller_.getCurrentTime());
    QPen timePen;
    timePen.setColor(QColor(255, 255, 255));
    timePen.setWidthF(1.0);
    timePen.setCosmetic(true);
    timePen.setStyle(Qt::DashLine);
    painter->setPen(timePen);
    painter->drawLine(QLineF(x, rect.top(), x, rect.bottom()));
}

void AnimationViewQt::onStateChanged(AnimationController*, AnimationState, AnimationState) {
    this->viewport()->update();
}

void AnimationViewQt::onTimeChanged(AnimationController*, Seconds, Seconds) {
    timeline_->update();
    this->viewport()->update();
}

}  // namespace animation

}  // namespace inviwo
