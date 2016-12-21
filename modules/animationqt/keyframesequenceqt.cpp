/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016 Inviwo Foundation
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

#include <warn/push>
#include <warn/ignore/all>
 //#include <QTextStream>
#include <QGraphicsLineItem>
#include <QGraphicsScene>
 //#include <QGraphicsSceneEvent>
#include <QGraphicsView>
#include <QPainter>
 //#include <QKeyEvent>
#include <warn/pop>

namespace inviwo {

namespace animation {

KeyframeSequenceQt::KeyframeSequenceQt(KeyframeSequence& keyframeSequence) : QGraphicsItem(), keyframeSequence_(keyframeSequence) {
	keyframeSequence.addObserver(this);

    for (size_t i = 0; i < keyframeSequence_.size(); ++i) {
        auto& keyframe = keyframeSequence_[i];
        auto keyframeQt = new KeyframeQt(keyframe);

        keyframeQt->setParentItem(this);
		keyframeQt->setPos(QPointF(keyframe.getTime().count() * WidthPerTimeUnit, 0));
    }

	updateRect();
}

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

    painter->drawRect(rect_);
}

void KeyframeSequenceQt::onKeyframeAdded(Keyframe *key) {
	auto keyframeQt = new KeyframeQt(*key);
	keyframeQt->setParentItem(this);
	keyframeQt->setPos(QPointF(key->getTime().count() * WidthPerTimeUnit, 0));
}

QRectF KeyframeSequenceQt::boundingRect() const { return rect_; }

void KeyframeSequenceQt::updateRect() {
	auto startTime = keyframeSequence_.getFirst().getTime().count();
	auto endTime = keyframeSequence_.getLast().getTime().count();

	auto l = startTime * WidthPerTimeUnit;
	auto t = -TrackHeight / 2.0;
	auto w = (endTime - startTime) * WidthPerTimeUnit;
	auto h = TrackHeight;
	rect_ = QRectF(l, t, w, h);
}

}  // namespace

}  // namespace
