/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2015 Inviwo Foundation
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

#include <QGraphicsDropShadowEffect>
#include <QGraphicsSceneHoverEvent>
#include <QPainter>
#include <QPainterPath>

#include <inviwo/core/ports/inport.h>
#include <inviwo/core/ports/outport.h>
#include <inviwo/core/network/portconnection.h>

#include <inviwo/qt/editor/processorgraphicsitem.h>
#include <inviwo/qt/editor/connectiongraphicsitem.h>
#include <inviwo/qt/editor/processorportgraphicsitem.h>

namespace inviwo {


CurveGraphicsItem::CurveGraphicsItem(QPointF startPoint, QPointF endPoint, uvec3 color)
    : startPoint_(startPoint)
    , endPoint_(endPoint)
    , midPoint_(QPointF(0,0))
    , useMidPoint_(false)
    , color_(color.r, color.g, color.b)
    , borderColor_()
    , selectedBorderColor_() {

    setZValue(DRAGING_ITEM_DEPTH);
    QGraphicsDropShadowEffect* shadowEffect = new QGraphicsDropShadowEffect();
    shadowEffect->setOffset(3.0);
    shadowEffect->setBlurRadius(3.0);
    setGraphicsEffect(shadowEffect);
    resetBorderColors();
}

CurveGraphicsItem::~CurveGraphicsItem() {}

QPainterPath CurveGraphicsItem::obtainCurvePath() const {
    if (useMidPoint_) {
    return obtainCurvePath(getStartPoint(), getMidPoint()) + 
        obtainCurvePath(getMidPoint(), getEndPoint());    
    } else {
        return obtainCurvePath(getStartPoint(), getEndPoint());
    }
}

QPainterPath CurveGraphicsItem::obtainCurvePath(QPointF startPoint, QPointF endPoint) const {
    const int startOff = 6;

    QPointF curvStart = startPoint + QPointF(0, startOff);
    QPointF curvEnd = endPoint - QPointF(0, startOff);

    float delta = std::abs(curvEnd.y() - curvStart.y());

    QPointF o = curvEnd - curvStart;
    int min = 37 - startOff * 2;
    min = std::min(min, static_cast<int>(std::sqrt(o.x() * o.x() + o.y() * o.y())));
    static const int max = 40;
    if (delta < min) delta = min;
    if (delta > max) delta = max;

    QPointF off(0, delta);
    QPointF ctrlPoint1 = curvStart + off;
    QPointF ctrlPoint2 = curvEnd - off;

    QPainterPath bezierCurve;
    bezierCurve.moveTo(startPoint);
    bezierCurve.lineTo(curvStart);
    bezierCurve.cubicTo(ctrlPoint1, ctrlPoint2, curvEnd);
    bezierCurve.lineTo(endPoint);

    return bezierCurve;
}

void CurveGraphicsItem::paint(QPainter* p, const QStyleOptionGraphicsItem* options,
                              QWidget* widget) {
    IVW_UNUSED_PARAM(options);
    IVW_UNUSED_PARAM(widget);

    QColor color = getColor();

    if (isSelected()) {
        p->setPen(QPen(selectedBorderColor_, 4.0, Qt::SolidLine, Qt::RoundCap));
    } else {
        p->setPen(QPen(borderColor_, 3.0, Qt::SolidLine, Qt::RoundCap));
    }
    p->drawPath(path_);
    p->setPen(QPen(color, 2.0, Qt::SolidLine, Qt::RoundCap));
    p->drawPath(path_);
}

QPainterPath CurveGraphicsItem::shape() const {
    QPainterPathStroker pathStrocker;
    pathStrocker.setWidth(10.0);
    return pathStrocker.createStroke(path_);
}

void CurveGraphicsItem::resetBorderColors() {
    setBorderColor(Qt::black);
    setSelectedBorderColor(Qt::darkRed);
}

void CurveGraphicsItem::updateShape() {   
    path_ = obtainCurvePath();

    QRectF p = path_.boundingRect();
    rect_ = QRectF(p.topLeft() - QPointF(5, 5), p.size() + QSizeF(10, 10));

    prepareGeometryChange();
}

QRectF CurveGraphicsItem::boundingRect() const {
    return rect_;
}

QPointF CurveGraphicsItem::getStartPoint() const { return startPoint_; }
QPointF CurveGraphicsItem::getMidPoint() const { return midPoint_; }
QPointF CurveGraphicsItem::getEndPoint() const { return endPoint_; }

void CurveGraphicsItem::setStartPoint(QPointF startPoint) {
    startPoint_ = startPoint;
    updateShape();
}
void CurveGraphicsItem::setMidPoint(QPointF midPoint) {
    useMidPoint_ = true;
    midPoint_ = midPoint;
    updateShape();
}
void CurveGraphicsItem::setEndPoint(QPointF endPoint) {
    endPoint_ = endPoint;
    updateShape();
}
void CurveGraphicsItem::clearMidPoint() {  
    midPoint_ = QPointF(0,0);
    useMidPoint_ = false;
    updateShape();
}

void CurveGraphicsItem::setColor(QColor color) { color_ = color; }

void CurveGraphicsItem::setBorderColor(QColor borderColor) { borderColor_ = borderColor; }

void CurveGraphicsItem::setSelectedBorderColor(QColor selectedBorderColor) {
    selectedBorderColor_ = selectedBorderColor;
}

QColor CurveGraphicsItem::getColor() const {
    return color_;
}



//////////////////////////////////////////////////////////////////////////


ConnectionDragGraphicsItem::ConnectionDragGraphicsItem(ProcessorOutportGraphicsItem* outport,
                                                       QPointF endPoint,
                                                       uvec3 color)
    : CurveGraphicsItem(QPointF(0.0f, 0.0f), endPoint, color)
    , outport_(outport) {
}

ConnectionDragGraphicsItem::~ConnectionDragGraphicsItem() {}

ProcessorOutportGraphicsItem* ConnectionDragGraphicsItem::getOutportGraphicsItem() const {
    return outport_;
}

QPointF ConnectionDragGraphicsItem::getStartPoint() const {
    return outport_->mapToScene(outport_->rect().center());
}

void ConnectionDragGraphicsItem::reactToPortHover(ProcessorInportGraphicsItem* inport) {
    if (inport != NULL) {
        if (inport->getPort()->canConnectTo(outport_->getPort())) {
            setBorderColor(Qt::green);
        } else {
            setBorderColor(Qt::red);
        }
    } else {
        resetBorderColors();
    }
}

//////////////////////////////////////////////////////////////////////////


ConnectionGraphicsItem::ConnectionGraphicsItem(ProcessorOutportGraphicsItem* outport,
                                               ProcessorInportGraphicsItem* inport,
                                               PortConnection* connection)
    : ConnectionDragGraphicsItem(outport, QPointF(0.0f,0.0f), connection->getInport()->getColorCode())
    , inport_(inport)
    , connection_(connection) {
    setFlags(ItemIsSelectable | ItemIsFocusable);
    setZValue(CONNECTIONGRAPHICSITEM_DEPTH);
    outport_->addConnection(this);
    inport_->addConnection(this);
}

ConnectionGraphicsItem::~ConnectionGraphicsItem() {
    outport_->removeConnection(this);
    inport_->removeConnection(this);
}

ProcessorInportGraphicsItem* ConnectionGraphicsItem::getInportGraphicsItem() const {
    return inport_;
}

ProcessorGraphicsItem* ConnectionGraphicsItem::getOutProcessor() const {
    return outport_->getProcessor();
}

ProcessorGraphicsItem* ConnectionGraphicsItem::getInProcessor() const {
    return inport_->getProcessor();
}

Outport* ConnectionGraphicsItem::getOutport() const { return connection_->getOutport(); }

Inport* ConnectionGraphicsItem::getInport() const { return connection_->getInport(); }

QPointF ConnectionGraphicsItem::getEndPoint() const {   
    return inport_->mapToScene(inport_->rect().center());
}

void ConnectionGraphicsItem::showToolTip(QGraphicsSceneHelpEvent* e) {
    showPortInfo(e, getOutport());
}

}  // namespace