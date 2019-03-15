/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2019 Inviwo Foundation
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

#include <modules/animationqt/widgets/keyframewidgetqt.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QGraphicsLineItem>
#include <QGraphicsItem>
#include <QPainter>
#include <QRadialGradient>
#include <QApplication>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <warn/pop>

namespace inviwo {

namespace animation {

KeyframeWidgetQt::KeyframeWidgetQt(Keyframe& keyframe, QGraphicsItem* parent)
    : QGraphicsItem(parent), keyframe_(keyframe) {

    setFlags(ItemIgnoresTransformations | ItemIsMovable | ItemIsSelectable |
             ItemSendsGeometryChanges | ItemSendsScenePositionChanges);
    keyframe_.addObserver(this);

    setX(mapFromScene(timeToScenePos(keyframe_.getTime()), 0.0).x());
    setSelected(keyframe_.isSelected());
}

void KeyframeWidgetQt::paint(QPainter* painter, const QStyleOptionGraphicsItem* options,
                             QWidget* widget) {
    IVW_UNUSED_PARAM(options);
    IVW_UNUSED_PARAM(widget);
    painter->setRenderHint(QPainter::Antialiasing, true);
    QPen pen = QPen();
    pen.setWidthF(1);
    pen.setCosmetic(true);
    pen.setCapStyle(Qt::RoundCap);
    pen.setStyle(Qt::SolidLine);
    isSelected() ? pen.setColor(QColor(66, 66, 132)) : pen.setColor(QColor(66, 66, 66));
    QRadialGradient Gradient(QPointF(0, 0), trackHeight / 2 - trackHeightNudge,
                             QPointF(0, -trackHeight / 4));
    Gradient.setColorAt(0, isSelected() ? QColor(63, 184, 255) : QColor(220, 220, 220));
    Gradient.setColorAt(1, isSelected() ? QColor(66, 66, 132) : QColor(128, 128, 128));
    QBrush brush = QBrush(Gradient);
    painter->setPen(pen);
    painter->setBrush(brush);
    auto penWidth = pen.widthF();
    int hs = static_cast<int>((keyframeWidth - penWidth) / 2.0f);
    QPoint p[4] = {{-hs, 0},
                   {0, -trackHeight / 2 + trackHeightNudge},
                   {hs, 0},
                   {0, trackHeight / 2 - trackHeightNudge}};
    painter->drawPolygon(p, 4);
}

void KeyframeWidgetQt::lock() { isEditing_ = true; }

void KeyframeWidgetQt::unlock() { isEditing_ = false; }

bool KeyframeWidgetQt::islocked() const { return isEditing_; }

void KeyframeWidgetQt::onKeyframeTimeChanged(Keyframe*, Seconds) {
    if (!isEditing_) {
        KeyframeWidgetQtLock lock(this);
        const auto newPos = mapToParent(mapFromScene(timeToScenePos(keyframe_.getTime()), 0.0));
        if (newPos != pos()) {
            setPos(newPos);
        }
    }
}

QRectF KeyframeWidgetQt::boundingRect() const {
    return QRectF(-keyframeWidth / 2.0f, -keyframeHeight / 2.0f, keyframeWidth, keyframeHeight);
}

QVariant KeyframeWidgetQt::itemChange(GraphicsItemChange change, const QVariant& value) {
    if (change == ItemPositionChange) {
        // Dragging a keyframe to a new time
        qreal xResult = value.toPointF().x();
        if (scene() && !scene()->views().empty() &&
            QApplication::mouseButtons() == Qt::LeftButton) {
            // Snap to a grid depending on the current scale
            // - We get parent coordinates here, and need scene coordinates to snap to something
            // globally
            const qreal xInScene = mapToScene(mapFromParent(xResult, 0.0)).x();
            const qreal xSnappedInScene =
                getSnapTime(xInScene, scene()->views().first()->transform().m11());
            xResult = mapToParent(mapFromScene(std::max(xSnappedInScene, 0.0), 0.0)).x();
        }

        // Restrict vertical movement: y does not change
        return QPointF(xResult, y());

    } else if (change == ItemScenePositionHasChanged) {
        if (!isEditing_) {
            KeyframeWidgetQtLock lock(this);
            keyframe_.setTime(scenePosToTime(value.toPointF().x()));
        }
    } else if (change == ItemSelectedChange) {
        keyframe_.setSelected(value.toBool());
    }

    return QGraphicsItem::itemChange(change, value);
}

KeyframeWidgetQtLock::KeyframeWidgetQtLock(KeyframeWidgetQt* keyframe) : keyframe_(keyframe) {
    if (keyframe_) keyframe_->lock();
}

KeyframeWidgetQtLock::~KeyframeWidgetQtLock() {
    if (keyframe_) keyframe_->unlock();
}

}  // namespace animation

}  // namespace inviwo
