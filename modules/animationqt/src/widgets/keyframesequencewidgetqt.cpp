/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2025 Inviwo Foundation
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

#include <modules/animationqt/widgets/keyframesequencewidgetqt.h>

#include <modules/animation/datastructures/keyframesequence.h>  // for KeyframeSequence
#include <modules/animationqt/widgets/editorconstants.h>        // for timeToScenePos, trackHeig...
#include <modules/animationqt/widgets/keyframewidgetqt.h>       // for KeyframeWidgetQt

#include <algorithm>  // for max
#include <cstddef>    // for size_t
#include <utility>    // for pair

#include <QApplication>     // for QApplication
#include <QBrush>           // for QBrush
#include <QColor>           // for QColor
#include <QFlags>           // for operator==, QFlags
#include <QGraphicsScene>   // for QGraphicsScene
#include <QGraphicsView>    // for QGraphicsView
#include <QLinearGradient>  // for QLinearGradient
#include <QList>            // for QList
#include <QPainter>         // for QPainter, QPainter::Antia...
#include <QPen>             // for QPen
#include <QPointF>          // for QPointF
#include <QTransform>       // for QTransform
#include <Qt>               // for LeftButton, MouseButtons
#include <qglobal.h>        // for qreal

class QStyleOptionGraphicsItem;
class QWidget;

namespace inviwo {

namespace animation {

class Keyframe;

KeyframeSequenceWidgetQt::KeyframeSequenceWidgetQt(KeyframeSequence& keyframeSequence,
                                                   QGraphicsItem* parent)
    : QGraphicsItem(parent), keyframeSequence_(keyframeSequence) {

    setFlags(ItemIsMovable | ItemIsSelectable | ItemSendsGeometryChanges |
             ItemSendsScenePositionChanges);

    keyframeSequence.addObserver(this);

    setX(mapFromScene(timeToScenePos(keyframeSequence_.getFirstTime()), 0).x());
    for (size_t i = 0; i < keyframeSequence_.size(); ++i) {
        keyframes_[&keyframeSequence_[i]] =
            std::make_unique<KeyframeWidgetQt>(keyframeSequence_[i], this);
    }

    QGraphicsItem::prepareGeometryChange();
    setSelected(keyframeSequence_.isSelected());
}

KeyframeSequenceWidgetQt::~KeyframeSequenceWidgetQt() = default;

void KeyframeSequenceWidgetQt::paint(QPainter* painter,
                                     [[maybe_unused]] const QStyleOptionGraphicsItem* options,
                                     [[maybe_unused]] QWidget* widget) {
    painter->setRenderHint(QPainter::Antialiasing, true);
    QPen pen = QPen();
    pen.setWidthF(1);
    pen.setCosmetic(true);
    pen.setCapStyle(Qt::RoundCap);
    pen.setStyle(Qt::SolidLine);
    isSelected() ? pen.setColor(QColor(66, 66, 132)) : pen.setColor(QColor(66, 66, 66));
    QLinearGradient Gradient(0, -trackHeight / 2 + trackHeightNudge, 0,
                             trackHeight / 2 - trackHeightNudge);
    Gradient.setColorAt(0, isSelected() ? QColor(63, 184, 255) : QColor(192, 192, 192));
    Gradient.setColorAt(1, isSelected() ? QColor(66, 66, 132) : QColor(66, 66, 66));
    QBrush brush = QBrush(Gradient);
    painter->setPen(pen);
    painter->setBrush(brush);
    auto rect = boundingRect();
    auto penWidth = pen.widthF();
    rect.adjust(0.5 * (keyframeWidth + penWidth), 0.5 * penWidth + trackHeightNudge,
                -0.5 * (keyframeWidth - penWidth), -0.5 * penWidth - trackHeightNudge);

    const auto start = mapFromScene(timeToScenePos(keyframeSequence_.getFirstTime()), 0).x();
    const auto stop = mapFromScene(timeToScenePos(keyframeSequence_.getLastTime()), 0).x();

    QRectF draw{start, -0.5 * trackHeight + trackHeightNudge, stop - start,
                trackHeight - 2 * trackHeightNudge};

    painter->drawRect(draw);
}

KeyframeSequence& KeyframeSequenceWidgetQt::getKeyframeSequence() { return keyframeSequence_; }

const KeyframeSequence& KeyframeSequenceWidgetQt::getKeyframeSequence() const {
    return keyframeSequence_;
}

void KeyframeSequenceWidgetQt::onKeyframeAdded(Keyframe* key, KeyframeSequence*) {
    prepareGeometryChange();
    keyframes_[key] = std::make_unique<KeyframeWidgetQt>(*key, this);
}

void KeyframeSequenceWidgetQt::onKeyframeRemoved(Keyframe* key, KeyframeSequence*) {
    prepareGeometryChange();
    keyframes_.erase(key);
}

void KeyframeSequenceWidgetQt::onKeyframeSequenceMoved(KeyframeSequence*) {
    prepareGeometryChange();
}

QRectF KeyframeSequenceWidgetQt::boundingRect() const { return childrenBoundingRect(); }

KeyframeWidgetQt* KeyframeSequenceWidgetQt::getKeyframeQt(const Keyframe* keyframe) const {
    auto it = keyframes_.find(keyframe);

    if (it != keyframes_.end()) {
        return it->second.get();
    } else {
        return nullptr;
    }
}

QVariant KeyframeSequenceWidgetQt::itemChange(GraphicsItemChange change, const QVariant& value) {
    if (change == ItemPositionChange) {
        // Dragging the keyframesequence to a new time is like snapping its left-most keyframe
        // - parent coordinates (== track coordinates in our case)
        auto xResult = value.toPointF().x();

        if (scene() && !scene()->views().empty() &&
            QApplication::mouseButtons() == Qt::LeftButton) {

            const qreal xFirstChild = childItems().empty() ? 0 : childItems().first()->x();
            const qreal xLeftBorderOfSequence = xResult + xFirstChild;
            const qreal xSnappedInScene =
                getSnapTime(xLeftBorderOfSequence, scene()->views().first()->transform().m11());
            xResult = std::max(xSnappedInScene, 0.0) - xFirstChild;
        }

        // Restrict vertical movement: y does not change
        return QPointF(xResult, y());
    } else if (change == ItemSelectedChange) {
        keyframeSequence_.setSelected(value.toBool());
    }

    return QGraphicsItem::itemChange(change, value);
}

}  // namespace animation

}  // namespace inviwo
