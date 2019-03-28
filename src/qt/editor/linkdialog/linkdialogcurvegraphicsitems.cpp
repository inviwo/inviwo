/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2019 Inviwo Foundation
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

#include <warn/push>
#include <warn/ignore/all>
#include <QPainterPath>
#include <QColor>
#include <warn/pop>

#include <inviwo/qt/editor/linkdialog/linkdialogcurvegraphicsitems.h>
#include <inviwo/qt/editor/linkdialog/linkdialogpropertygraphicsitems.h>
#include <inviwo/qt/editor/linkdialog/linkdialogscene.h>

namespace inviwo {

DialogCurveGraphicsItem::DialogCurveGraphicsItem(QPointF startPoint, QPointF endPoint, QColor color)
    : CurveGraphicsItem(color), startPoint_{startPoint}, endPoint_{endPoint} {
    setZValue(linkdialog::connectionDepth);
}

DialogCurveGraphicsItem::~DialogCurveGraphicsItem() = default;

QPainterPath DialogCurveGraphicsItem::obtainCurvePath(QPointF startPoint, QPointF endPoint) const {
    double delta = endPoint.x() - startPoint.x();
    QPointF ctrlPoint1 = QPointF(startPoint.x() + delta / 4.0, startPoint.y());
    QPointF ctrlPoint2 = QPointF(endPoint.x() - delta / 4.0, endPoint.y());
    QPainterPath bezierCurve;
    bezierCurve.moveTo(startPoint);
    bezierCurve.cubicTo(ctrlPoint1, ctrlPoint2, endPoint);
    return bezierCurve;
}

QPointF DialogCurveGraphicsItem::getStartPoint() const { return startPoint_; }
QPointF DialogCurveGraphicsItem::getEndPoint() const { return endPoint_; }

void DialogCurveGraphicsItem::setStartPoint(QPointF startPoint) {
    startPoint_ = startPoint;
    updateShape();
}
void DialogCurveGraphicsItem::setEndPoint(QPointF endPoint) {
    endPoint_ = endPoint;
    updateShape();
}

DialogConnectionGraphicsItem::DialogConnectionGraphicsItem(
    LinkDialogPropertyGraphicsItem* startProperty, LinkDialogPropertyGraphicsItem* endProperty,
    const PropertyLink& propertyLink)
    : CurveGraphicsItem(QColor(38, 38, 38))
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
}

QPointF DialogConnectionGraphicsItem::getStartPoint() const {
    auto si = startPropertyGraphicsItem_->getConnectionIndex(this);
    return startPropertyGraphicsItem_->calculateArrowCenter(si);
}
QPointF DialogConnectionGraphicsItem::getEndPoint() const {
    auto ei = endPropertyGraphicsItem_->getConnectionIndex(this);
    return endPropertyGraphicsItem_->calculateArrowCenter(ei);
}

QPainterPath DialogConnectionGraphicsItem::obtainCurvePath(QPointF startPoint,
                                                           QPointF endPoint) const {
    double delta = endPoint.x() - startPoint.x();
    QPointF ctrlPoint1 = QPointF(startPoint.x() + delta / 4.0, startPoint.y());
    QPointF ctrlPoint2 = QPointF(endPoint.x() - delta / 4.0, endPoint.y());
    QPainterPath bezierCurve;
    bezierCurve.moveTo(startPoint);
    bezierCurve.cubicTo(ctrlPoint1, ctrlPoint2, endPoint);
    return bezierCurve;
}

void DialogConnectionGraphicsItem::paint(QPainter* p, const QStyleOptionGraphicsItem* options,
                                         QWidget* widget) {
    if (startPropertyGraphicsItem_->isVisible() || endPropertyGraphicsItem_->isVisible()) {
        setColor(QColor(38, 38, 38));
        setZValue(linkdialog::connectionDepth);
    } else {
        setColor(QColor(138, 138, 138));
        setZValue(linkdialog::connectionDepth - 0.5);
    }
    CurveGraphicsItem::paint(p, options, widget);
}

}  // namespace inviwo
