/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2015 Inviwo Foundation
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

#include <inviwo/qt/editor/processorlinkgraphicsitem.h>
#include <inviwo/qt/editor/processorgraphicsitem.h>
#include <inviwo/qt/editor/networkeditor.h>
#include <inviwo/qt/editor/linkgraphicsitem.h>

#include <QPen>
#include <QPainter>
#include <QGraphicsSceneMouseEvent>

namespace inviwo {

ProcessorLinkGraphicsItem::ProcessorLinkGraphicsItem(ProcessorGraphicsItem* parent)
    : EditorGraphicsItem(parent), processor_(parent), leftItem_(nullptr), rightItem_(nullptr) {
    setFlags(ItemSendsScenePositionChanges);
    setCacheMode(QGraphicsItem::DeviceCoordinateCache);

    leftItem_ =
        new LinkItem(this, (parent->rect().bottomRight() + parent->rect().topRight()) / 2.0, 90.0f);
    rightItem_ =
        new LinkItem(this, (parent->rect().bottomLeft() + parent->rect().topLeft()) / 2.0, -90.0f);
}

QPointF ProcessorLinkGraphicsItem::getLeftPos() const {
    return leftItem_->mapToScene(leftItem_->rect().center());
}

QPointF ProcessorLinkGraphicsItem::getRightPos() const {
    return rightItem_->mapToScene(rightItem_->rect().center());
}

ProcessorGraphicsItem* ProcessorLinkGraphicsItem::getProcessorGraphicsItem() const {
    return processor_;
}

QVariant ProcessorLinkGraphicsItem::itemChange(GraphicsItemChange change, const QVariant& value) {
    if (change == QGraphicsItem::ItemScenePositionHasChanged) {
        updateLinkPositions();
    }
    return QGraphicsItem::itemChange(change, value);
}

void ProcessorLinkGraphicsItem::addLink(LinkConnectionGraphicsItem* link) {
    links_.push_back(link);
    updateLinkPositions();
}

void ProcessorLinkGraphicsItem::removeLink(LinkConnectionGraphicsItem* link) {
    links_.erase(std::find(links_.begin(),links_.end(), link));
}

void ProcessorLinkGraphicsItem::updateLinkPositions() {
    for (auto& elem : links_) {
        elem->updateShape();
    }
}

ProcessorLinkGraphicsItem::LinkItem::LinkItem(ProcessorLinkGraphicsItem* parent, QPointF pos,
                                              float angle)
    : EditorGraphicsItem(parent), parent_(parent), pos_(pos), angle_(angle), size_(4.0f), lineWidth_(1.3f) {
    setRect(-0.5f * size_ - 2.0 * lineWidth_, -0.5f * size_ - 2.0 * lineWidth_, size_ + 4.0 * lineWidth_,
            size_ + 4.0 * lineWidth_);
    setCacheMode(QGraphicsItem::DeviceCoordinateCache);
    setPos(pos);
}

ProcessorLinkGraphicsItem::LinkItem::~LinkItem() {}

void ProcessorLinkGraphicsItem::LinkItem::mousePressEvent(QGraphicsSceneMouseEvent* e) {
    NetworkEditor::getPtr()->initiateLink(parent_, e->scenePos());
    e->accept();
}

void ProcessorLinkGraphicsItem::LinkItem::paint(QPainter* p,
                                                const QStyleOptionGraphicsItem* options,
                                                QWidget* widget) {
    p->save();
    p->setBrush(Qt::NoBrush);
    p->setRenderHint(QPainter::Antialiasing, true);
    p->setPen(QPen(QColor(164, 164, 164), lineWidth_, Qt::SolidLine, Qt::RoundCap));

    QPainterPath linkpath1;
    linkpath1.arcTo(QRectF(QPointF(-size_, size_), QPointF(size_, -size_)), angle_, 180);
    linkpath1.closeSubpath();
    p->drawPath(linkpath1);
    p->restore();
}

}  // namespace
