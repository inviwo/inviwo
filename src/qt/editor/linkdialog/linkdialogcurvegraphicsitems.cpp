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

#include <QVector2D>
#include <QGraphicsSceneMouseEvent>

#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/qt/editor/linkdialog/linkdialogcurvegraphicsitems.h>
#include <inviwo/qt/editor/linkdialog/linkdialogpropertygraphicsitems.h>
#include <inviwo/qt/editor/linkdialog/linkdialogscene.h>

#include <inviwo/core/links/linkconditions.h>
#include <inviwo/core/network/processornetwork.h>

namespace inviwo {

DialogCurveGraphicsItem::DialogCurveGraphicsItem(QPointF startPoint, QPointF endPoint, uvec3 color)
    : CurveGraphicsItem(startPoint, endPoint, color) {
    setZValue(linkdialog::connectionDepth);
}

DialogCurveGraphicsItem::~DialogCurveGraphicsItem() {}

QPainterPath DialogCurveGraphicsItem::obtainCurvePath(QPointF startPoint, QPointF endPoint) const {
    float delta = endPoint.x() - startPoint.x();
    QPointF ctrlPoint1 = QPointF(startPoint.x() + delta / 4.0, startPoint.y());
    QPointF ctrlPoint2 = QPointF(endPoint.x() - delta / 4.0, endPoint.y());
    QPainterPath bezierCurve;
    bezierCurve.moveTo(startPoint);
    bezierCurve.cubicTo(ctrlPoint1, ctrlPoint2, endPoint);
    return bezierCurve;
}

QPainterPath DialogCurveGraphicsItem::obtainCurvePath() const {
    return CurveGraphicsItem::obtainCurvePath();
}

DialogConnectionGraphicsItem::DialogConnectionGraphicsItem(
    LinkDialogPropertyGraphicsItem* startProperty, LinkDialogPropertyGraphicsItem* endProperty,
    PropertyLink* propertyLink)
    : DialogCurveGraphicsItem(startProperty->pos(), endProperty->pos(), uvec3(38, 38, 38))
    , startPropertyGraphicsItem_(startProperty)
    , endPropertyGraphicsItem_(endProperty)
    , propertyLink_(propertyLink) {
    setFlags(ItemIsSelectable | ItemIsFocusable);

    setZValue(linkdialog::connectionDepth);

    startPropertyGraphicsItem_->addConnectionGraphicsItem(this);
    endPropertyGraphicsItem_->addConnectionGraphicsItem(this);
}

DialogConnectionGraphicsItem::~DialogConnectionGraphicsItem() { 
    if (startPropertyGraphicsItem_) {
        startPropertyGraphicsItem_->removeConnectionGraphicsItem(this);
        startPropertyGraphicsItem_ = nullptr;
    }
    if (endPropertyGraphicsItem_) {
        endPropertyGraphicsItem_->removeConnectionGraphicsItem(this);
        endPropertyGraphicsItem_ = nullptr;
    }
    propertyLink_ = nullptr;
}

void DialogConnectionGraphicsItem::updateStartEndPoint() {
    auto si = startPropertyGraphicsItem_->getConnectionIndex(this);
    setStartPoint(startPropertyGraphicsItem_->calculateArrowCenter(si));

    auto ei = endPropertyGraphicsItem_->getConnectionIndex(this);
    setEndPoint(endPropertyGraphicsItem_->calculateArrowCenter(ei));
    
    if (startPropertyGraphicsItem_->isVisible() || endPropertyGraphicsItem_->isVisible()){
        setColor(QColor(38, 38, 38));
        setZValue(linkdialog::connectionDepth);
    } else {
        setColor(QColor(138, 138, 138));
        setZValue(linkdialog::connectionDepth-0.5);
    }
}

bool DialogConnectionGraphicsItem::isBidirectional() {
    auto linkscene = qobject_cast<LinkDialogGraphicsScene*>(scene());
    return linkscene->getNetwork()->isLinkedBidirectional(
        propertyLink_->getSourceProperty(), propertyLink_->getDestinationProperty());
}

void DialogConnectionGraphicsItem::updateConnectionDrawing() {
    if (!propertyLink_) return;
    startPropertyGraphicsItem_->prepareGeometryChange();
    endPropertyGraphicsItem_->prepareGeometryChange();
    updateStartEndPoint();
}

void DialogConnectionGraphicsItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* e) {
    // Toggle directionality

    auto linkscene = qobject_cast<LinkDialogGraphicsScene*>(scene());
    if(e->modifiers() == Qt::ShiftModifier || e->modifiers() == Qt::ControlModifier) {
        linkscene->makePropertyLinkBidirectional(this, !isBidirectional());
    }else if (isBidirectional()) {
        linkscene->makePropertyLinkBidirectional(this, false);
    } else {
        linkscene->switchPropertyLinkDirection(this);
    }
    e->accept();
}

}  // namespace