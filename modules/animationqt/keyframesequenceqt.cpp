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
#include <warn/pop>

namespace inviwo {

namespace animation {

KeyframeSequenceQt::KeyframeSequenceQt(KeyframeSequence& keyframeSequence, QGraphicsItem* parent)
    : QGraphicsItem(parent), keyframeSequence_(keyframeSequence) {
    setFlags(ItemIsMovable | ItemSendsGeometryChanges);

    keyframeSequence.addObserver(this);
    auto firstKeyframePos = keyframeSequence_.getFirst().getTime().count() * WidthPerSecond;
    setX(firstKeyframePos);
    for (size_t i = 0; i < keyframeSequence_.size(); ++i) {
        keyframes_.push_back(std::make_unique<KeyframeQt>(keyframeSequence_[i], this));
    }
    prepareGeometryChange();
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
    isSelected() ? pen.setColor(QColor(213, 79, 79)) : pen.setColor(QColor(66, 66, 66));
    QBrush brush = QBrush(QColor::fromRgb(128, 128, 128));
    painter->setPen(pen);
    painter->setBrush(brush);
    auto rect = boundingRect();
    rect.adjust(0.5 * KeyframeWidth, 0, -0.5 * KeyframeWidth, 0);
    painter->drawRect(rect);
}

KeyframeSequence& KeyframeSequenceQt::getKeyframeSequence() { return keyframeSequence_; }

const KeyframeSequence& KeyframeSequenceQt::getKeyframeSequence() const {
    return keyframeSequence_;
}

void KeyframeSequenceQt::onKeyframeAdded(Keyframe* key, KeyframeSequence* seq) {
    keyframes_.push_back(std::make_unique<KeyframeQt>(*key, this));
    prepareGeometryChange();
}

void KeyframeSequenceQt::onKeyframeRemoved(Keyframe* key, KeyframeSequence* seq) {
    if (util::erase_remove_if(keyframes_, [&](auto& keyframeqt) {
            if (&(keyframeqt->getKeyframe()) == key) {
                this->scene()->removeItem(keyframeqt.get());
                return true;
            } else {
                return false;
            }
        }) > 0) {
        prepareGeometryChange();
    }
}

void KeyframeSequenceQt::onKeyframeSequenceMoved(KeyframeSequence* key) { prepareGeometryChange(); }

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
    // Only restrict movement on user interaction
    if (change == ItemPositionChange && scene() && QApplication::mouseButtons() == Qt::LeftButton) {
        // Snap to frame per second
        double xV = round(value.toPointF().x() / WidthPerFrame) * WidthPerFrame;
        auto dt = Seconds((xV - x()) / static_cast<double>(WidthPerSecond));
        
        // No need to update time within keyframes in here,
        // since the position changed event will be propagated to keyframes.
        if (dt < Seconds(0.0)) {
            // Do not allow it to move before t=0
            auto maxMove = -keyframeSequence_.getFirst().getTime().count();
            dt = Seconds(std::max(dt.count(), maxMove));
            xV = x() + dt.count() * static_cast<double>(WidthPerSecond);
        } 
        // Restrict vertical movement
        return QPointF(static_cast<float>(xV), y());
    }
    
    return QGraphicsItem::itemChange(change, value);
}

}  // namespace

}  // namespace
