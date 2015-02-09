/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 * Version 0.9
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

#include <inviwo/core/links/linkconditions.h>
#include <inviwo/core/network/processornetwork.h>

namespace inviwo {

DialogCurveGraphicsItem::DialogCurveGraphicsItem(QPointF startPoint, QPointF endPoint, uvec3 color) :
    CurveGraphicsItem(startPoint, endPoint, color) {
    setZValue(LINKDIALOG_CONNECTION_GRAPHICSITEM_DEPTH);
}

DialogCurveGraphicsItem::~DialogCurveGraphicsItem() { }

QPainterPath DialogCurveGraphicsItem::obtainCurvePath(QPointF startPoint, QPointF endPoint) const {
    float delta = endPoint.x()-startPoint.x();
    QPointF ctrlPoint1 = QPointF(startPoint.x()+delta/4.0, startPoint.y());
    QPointF ctrlPoint2 = QPointF(endPoint.x()-delta/4.0, endPoint.y());
    QPainterPath bezierCurve;
    bezierCurve.moveTo(startPoint);
    bezierCurve.cubicTo(ctrlPoint1, ctrlPoint2, endPoint);
    return bezierCurve;
}

//////////////////////////////////////////////////////////////////////////

DialogConnectionGraphicsItem::DialogConnectionGraphicsItem(
    LinkDialogPropertyGraphicsItem* startProperty, LinkDialogPropertyGraphicsItem* endProperty,
    PropertyLink* propertyLink)
    : DialogCurveGraphicsItem(startProperty->getShortestBoundaryPointTo(endProperty),
                              endProperty->getShortestBoundaryPointTo(startProperty),
                              uvec3(38, 38, 38))
    , startPropertyGraphicsItem_(startProperty)
    , endPropertyGraphicsItem_(endProperty)
    , propertyLink_(propertyLink) {
    setFlags(ItemIsSelectable | ItemIsFocusable);

    startPropertyGraphicsItem_->addConnectionGraphicsItem(this);
    setStartArrowHeadIndex(startPropertyGraphicsItem_->getConnectionGraphicsItemCount());
    endPropertyGraphicsItem_->addConnectionGraphicsItem(this);
    setEndArrowHeadIndex(endPropertyGraphicsItem_->getConnectionGraphicsItemCount());
}

DialogConnectionGraphicsItem::~DialogConnectionGraphicsItem() {    
    cleanup();
}

void DialogConnectionGraphicsItem::cleanup() { 
    if (startPropertyGraphicsItem_) {
        startPropertyGraphicsItem_->removeConnectionGraphicsItem(this);
        startPropertyGraphicsItem_ = NULL;
    }
    if (endPropertyGraphicsItem_) {
        endPropertyGraphicsItem_->removeConnectionGraphicsItem(this);
        endPropertyGraphicsItem_ = NULL;
    }
    propertyLink_ = NULL;
}

void DialogConnectionGraphicsItem::updateStartEndPoint() {
    QPoint arrowDim(arrowDimensionWidth, arrowDimensionHeight);
    //Start Property
    QPointF aCenterR = startPropertyGraphicsItem_->calculateArrowCenter(startArrowHeadIndex_, true);
    QPointF aCenterL = startPropertyGraphicsItem_->calculateArrowCenter(startArrowHeadIndex_, false);
    QPointF arrowCenter;
    QPointF start = getStartPoint();
    QVector2D vec1(aCenterR - start);
    QVector2D vec2(aCenterL - start);
    arrowCenter = aCenterR;

    if (vec2.length()<vec1.length())
        arrowCenter = aCenterL;

    setStartPoint(arrowCenter);
    
    //End Property
    aCenterR = endPropertyGraphicsItem_->calculateArrowCenter(endArrowHeadIndex_, true);
    aCenterL = endPropertyGraphicsItem_->calculateArrowCenter(endArrowHeadIndex_, false);
    QPointF end = getEndPoint();
    vec1 = QVector2D(aCenterR - end);
    vec2 = QVector2D(aCenterL - end);
    arrowCenter = aCenterR;

    if (vec2.length()<vec1.length())
        arrowCenter = aCenterL;

    setEndPoint(arrowCenter);
}

bool DialogConnectionGraphicsItem::isBidirectional() {
    return InviwoApplication::getPtr()->getProcessorNetwork()->isLinkedBidirectional(
               propertyLink_->getSourceProperty(), propertyLink_->getDestinationProperty());
}

void DialogConnectionGraphicsItem::updateConnectionDrawing() {
    if (!propertyLink_) return;
    startPropertyGraphicsItem_->prepareGeometryChange();
    endPropertyGraphicsItem_->prepareGeometryChange();
    updateStartEndPoint();
}

void DialogConnectionGraphicsItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* e) {
    QGraphicsItem::mouseDoubleClickEvent(e);
}

//////////////////////////////////////////////////////////////////////////


} //namespace