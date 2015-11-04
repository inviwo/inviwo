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

#include <inviwo/qt/widgets/properties/transferfunctioncontrolpointconnection.h>
#include <inviwo/qt/widgets/properties/transferfunctioneditorcontrolpoint.h>
#include <warn/push>
#include <warn/ignore/all>
#include <QPainterPathStroker>
#include <QGraphicsScene>
#include <QRectF>
#include <QPainter>
#include <warn/pop>

namespace inviwo {

TransferFunctionControlPointConnection::TransferFunctionControlPointConnection()
    : QGraphicsItem(), left_(nullptr), right_(nullptr), path_(), shape_(), rect_() {
    updateShape();
}

TransferFunctionControlPointConnection::~TransferFunctionControlPointConnection() {}

void TransferFunctionControlPointConnection::paint(QPainter* p,
                                                   const QStyleOptionGraphicsItem* options,
                                                   QWidget* widget) {
    IVW_UNUSED_PARAM(options);
    IVW_UNUSED_PARAM(widget);

    p->save();
    p->setRenderHint(QPainter::Antialiasing, true);

    QPen pathPen(QColor(66, 66, 66));
    pathPen.setWidth(3.0);
    pathPen.setCosmetic(true);
    
    p->setPen(pathPen);
    p->drawPath(path_);
    p->restore();
}

void TransferFunctionControlPointConnection::updateShape() {
    if (left_ == nullptr && right_ == nullptr) {
        path_ = QPainterPath();
    }
    
    path_ = QPainterPath(getStart());
    path_.lineTo(getStop());
    
    rect_ = path_.boundingRect();
    
    QPainterPathStroker pathStrocker;
    pathStrocker.setWidth(10.0);
    shape_ = pathStrocker.createStroke(path_);
    
    prepareGeometryChange();
    update();
}

QPointF TransferFunctionControlPointConnection::getStart() const {
    QPointF start;
    if(left_) {
        start = left_->getCurrentPos();
    } else if (right_ && scene()) {
        start = QPointF(scene()->sceneRect().left(), right_->getCurrentPos().y());
    }
    return start;
}

QPointF TransferFunctionControlPointConnection::getStop() const {
    QPointF stop;
    if(right_) {
        stop = right_->getCurrentPos();
    } else if (left_ && scene()) {
        stop = QPointF(scene()->sceneRect().right(), left_->getCurrentPos().y());
    }
    return stop;
}

QRectF TransferFunctionControlPointConnection::boundingRect() const {
    return rect_;
}

QPainterPath TransferFunctionControlPointConnection::shape() const {
    return shape_;
}

bool operator==(const TransferFunctionControlPointConnection& lhs,
                const TransferFunctionControlPointConnection& rhs) {
    return lhs.getStart() == rhs.getStart() && lhs.getStop() == rhs.getStop();
}

bool operator!=(const TransferFunctionControlPointConnection& lhs,
                const TransferFunctionControlPointConnection& rhs) {
    return !operator==(lhs, rhs);
}

bool operator<(const TransferFunctionControlPointConnection& lhs,
               const TransferFunctionControlPointConnection& rhs) {
    return 0.5f * (lhs.getStart().x() + lhs.getStop().x()) <
           0.5f * (rhs.getStart().x() + rhs.getStop().x());
}

bool operator>(const TransferFunctionControlPointConnection& lhs,
               const TransferFunctionControlPointConnection& rhs) {
    return rhs < lhs;
}

bool operator<=(const TransferFunctionControlPointConnection& lhs,
                const TransferFunctionControlPointConnection& rhs) {
    return !(rhs < lhs);
}

bool operator>=(const TransferFunctionControlPointConnection& lhs,
                const TransferFunctionControlPointConnection& rhs) {
    return !(lhs < rhs);
}

}  // namespace
