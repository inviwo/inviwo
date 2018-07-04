/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2018 Inviwo Foundation
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
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

#include <modules/animationqt/keyframesequenceqt.h>
#include <modules/animationqt/keyframeqt.h>
#include <modules/animationqt/trackqt.h>
#include <modules/animationqt/animationeditorqt.h>
#include <modules/animation/datastructures/keyframesequence.h>
#include <modules/animation/datastructures/animationtime.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QApplication>
#include <QGraphicsLineItem>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QPainter>
#include <QLinearGradient>
#include <warn/pop>

namespace inviwo {

namespace animation {

KeyframeSequenceQt::KeyframeSequenceQt(KeyframeSequence& keyframeSequence, TrackQt* parent)
    : QGraphicsItem(parent), keyframeSequence_(keyframeSequence), trackQt_(*parent) {

    setFlags(ItemIsMovable | ItemIsSelectable | ItemSendsGeometryChanges |
             ItemSendsScenePositionChanges);

    keyframeSequence.addObserver(this);
    auto firstKeyframePos = keyframeSequence_.getFirst().getTime().count() * WidthPerSecond;
    setX(firstKeyframePos);
    for (size_t i = 0; i < keyframeSequence_.size(); ++i) {
        keyframes_.push_back(std::make_unique<KeyframeQt>(keyframeSequence_[i], this));
    }

    QGraphicsItem::prepareGeometryChange();
    setSelected(keyframeSequence_.isSelected());
}

KeyframeSequenceQt::~KeyframeSequenceQt() = default;

void KeyframeSequenceQt::paint(QPainter* painter, const QStyleOptionGraphicsItem* options,
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
    QLinearGradient Gradient(0, -TrackHeight / 2 + TrackHeightNudge, 0,
                             TrackHeight / 2 - TrackHeightNudge);
    Gradient.setColorAt(0, isSelected() ? QColor(63, 184, 255) : QColor(192, 192, 192));
    Gradient.setColorAt(1, isSelected() ? QColor(66, 66, 132) : QColor(66, 66, 66));
    QBrush brush = QBrush(Gradient);
    painter->setPen(pen);
    painter->setBrush(brush);
    auto rect = boundingRect();
    auto penWidth = pen.widthF();
    rect.adjust(0.5 * (KeyframeWidth + penWidth), 0.5 * penWidth + TrackHeightNudge,
                -0.5 * (KeyframeWidth - penWidth), -0.5 * penWidth - TrackHeightNudge);
    painter->drawRect(rect);
}

KeyframeSequence& KeyframeSequenceQt::getKeyframeSequence() { return keyframeSequence_; }

const KeyframeSequence& KeyframeSequenceQt::getKeyframeSequence() const {
    return keyframeSequence_;
}

void KeyframeSequenceQt::onKeyframeAdded(Keyframe* key, KeyframeSequence*) {
    QGraphicsItem::prepareGeometryChange();
    keyframes_.push_back(std::make_unique<KeyframeQt>(*key, this));
}

void KeyframeSequenceQt::onKeyframeRemoved(Keyframe* key, KeyframeSequence*) {
    if (util::erase_remove_if(keyframes_, [&](auto& keyframeqt) {
            if (&(keyframeqt->getKeyframe()) == key) {
                this->prepareGeometryChange();
                this->scene()->removeItem(keyframeqt.get());
                return true;
            } else {
                return false;
            }
        }) > 0) {
    }
}

void KeyframeSequenceQt::onKeyframeSequenceMoved(KeyframeSequence*) {
    QGraphicsItem::prepareGeometryChange();
}

QRectF KeyframeSequenceQt::boundingRect() const { return childrenBoundingRect(); }

KeyframeQt* KeyframeSequenceQt::getKeyframeQt(const Keyframe* keyframe) const {
    auto it = util::find_if(
        keyframes_, [&](auto& keyframeqt) { return &(keyframeqt->getKeyframe()) == keyframe; });

    if (it != keyframes_.end()) {
        return it->get();
    } else {
        return nullptr;
    }
}

QVariant KeyframeSequenceQt::itemChange(GraphicsItemChange change, const QVariant& value) {
    if (change == ItemPositionChange) {
        // Dragging the keyframesequence to a new time is like snapping its left-most keyframe
        // - parent coordinates (== scene coordinates in our case)
        qreal xResult = value.toPointF().x();

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
