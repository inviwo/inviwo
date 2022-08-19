/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2021 Inviwo Foundation
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

#include <inviwo/core/ports/inport.h>
#include <inviwo/core/ports/outport.h>

#include <inviwo/qt/editor/processorgraphicsitem.h>
#include <inviwo/qt/editor/connectiongraphicsitem.h>
#include <inviwo/qt/editor/processorportgraphicsitem.h>

#include <modules/qtwidgets/inviwoqtutils.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QGraphicsDropShadowEffect>
#include <QGraphicsSceneHoverEvent>
#include <QPainter>
#include <QPainterPath>
#include <warn/pop>

namespace inviwo {

CurveGraphicsItem::CurveGraphicsItem(QColor color, QColor borderColor, QColor selectedBorderColor)
    : color_(color)
    , borderColor_(borderColor)
    , selectedBorderColor_(selectedBorderColor)
    , infoLabel_{new QGraphicsTextItem(this)} {
    setZValue(DRAGING_ITEM_DEPTH);

    infoLabel_->setVisible(false);
    infoLabel_->setTextInteractionFlags(Qt::NoTextInteraction);
}

CurveGraphicsItem::~CurveGraphicsItem() = default;

QPainterPath CurveGraphicsItem::obtainCurvePath() const {
    return obtainCurvePath(getStartPoint(), getEndPoint());
}

QPainterPath CurveGraphicsItem::obtainCurvePath(QPointF startPoint, QPointF endPoint) const {
    const int startOff = 6;

    QPointF curvStart = startPoint + QPointF(0, startOff);
    QPointF curvEnd = endPoint - QPointF(0, startOff);

    double delta = std::abs(curvEnd.y() - curvStart.y());

    QPointF o = curvEnd - curvStart;
    double min = 37 - startOff * 2;
    min = std::min(min, std::sqrt(o.x() * o.x() + o.y() * o.y()));
    static const double max = 40.0;
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

void CurveGraphicsItem::paint(QPainter* p, const QStyleOptionGraphicsItem*, QWidget*) {
    const auto color = getColor();
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
    update();
}

void CurveGraphicsItem::setHoverInfo(const QString& info) {
    infoLabel_->setVisible(true);
    infoLabel_->setHtml(info);
}

void CurveGraphicsItem::resetHoverInfo() { infoLabel_->setVisible(false); }

void CurveGraphicsItem::updateShape() {
    path_ = obtainCurvePath();
    const auto p = path_.boundingRect();
    rect_ = QRectF(p.topLeft() - QPointF(5, 5), p.size() + QSizeF(10, 10));
    prepareGeometryChange();
}

QRectF CurveGraphicsItem::boundingRect() const { return rect_; }

void CurveGraphicsItem::setColor(QColor color) { color_ = color; }

void CurveGraphicsItem::setBorderColor(QColor borderColor) { borderColor_ = borderColor; }

void CurveGraphicsItem::setSelectedBorderColor(QColor selectedBorderColor) {
    selectedBorderColor_ = selectedBorderColor;
}

QColor CurveGraphicsItem::getColor() const { return color_; }

ConnectionDragGraphicsItem::ConnectionDragGraphicsItem(ProcessorOutportGraphicsItem* outport,
                                                       QPointF endPoint, QColor color)
    : CurveGraphicsItem(color), endPoint_{endPoint}, outport_(outport) {}

ConnectionDragGraphicsItem::~ConnectionDragGraphicsItem() = default;

ProcessorOutportGraphicsItem* ConnectionDragGraphicsItem::getOutportGraphicsItem() const {
    return outport_;
}

QPointF ConnectionDragGraphicsItem::getStartPoint() const {
    return outport_->mapToScene(outport_->rect().center());
}

QPointF ConnectionDragGraphicsItem::getEndPoint() const { return endPoint_; }
void ConnectionDragGraphicsItem::setEndPoint(QPointF endPoint) {
    endPoint_ = endPoint;
    infoLabel_->setPos(endPoint_);
    updateShape();
}

void ConnectionDragGraphicsItem::reactToPortHover(ProcessorInportGraphicsItem* inport) {
    if (inport != nullptr) {
        auto* port = inport->getPort();

        Document desc{};
        auto html = desc.append("html");
        html.append("head").append("style", R"(
            body {
                color: #9d9995;
                background: #323235;
            }
            div.name {
                font-size: 13pt;
                color: #c8ccd0;
                font-weight: bold;
            }
            div.help {
                font-size: 12pt;
            }
        )"_unindent);

        auto content = html.append("body");
        content.append("div", port->getIdentifier(), {{"class", "name"}});
        if (!port->getHelp().empty()) {
            content.append("div", "", {{"class", "help"}}).append(port->getHelp());
        }

        setHoverInfo(utilqt::toQString(desc.str()));

        if (port->canConnectTo(outport_->getPort())) {
            setBorderColor(Qt::green);
        } else {
            setBorderColor(Qt::red);
        }
    } else {
        resetBorderColors();
        resetHoverInfo();
    }
}

ConnectionGraphicsItem::ConnectionGraphicsItem(ProcessorOutportGraphicsItem* outport,
                                               ProcessorInportGraphicsItem* inport,
                                               const PortConnection& connection)
    : CurveGraphicsItem(utilqt::toQColor(connection.getInport()->getColorCode()))
    , outport_(outport)
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

ProcessorOutportGraphicsItem* ConnectionGraphicsItem::getOutportGraphicsItem() const {
    return outport_;
}

ProcessorGraphicsItem* ConnectionGraphicsItem::getOutProcessor() const {
    return outport_->getProcessor();
}

ProcessorGraphicsItem* ConnectionGraphicsItem::getInProcessor() const {
    return inport_->getProcessor();
}

Outport* ConnectionGraphicsItem::getOutport() const { return connection_.getOutport(); }

Inport* ConnectionGraphicsItem::getInport() const { return connection_.getInport(); }

PortConnection ConnectionGraphicsItem::getPortConnection() const { return connection_; }

QPointF ConnectionGraphicsItem::getStartPoint() const {
    return outport_->mapToScene(outport_->rect().center());
}

QPointF ConnectionGraphicsItem::getEndPoint() const {
    return inport_->mapToScene(inport_->rect().center());
}

void ConnectionGraphicsItem::showToolTip(QGraphicsSceneHelpEvent* e) {
    showPortInfo(e, getOutport());
}

QVariant ConnectionGraphicsItem::itemChange(GraphicsItemChange change, const QVariant& value) {
#include <warn/push>
#include <warn/ignore/switch-enum>
    switch (change) {
        case QGraphicsItem::ItemSelectedHasChanged:
            inport_->update();
            outport_->update();
            break;
        default:
            break;
    }
#include <warn/pop>
    return QGraphicsItem::itemChange(change, value);
}

}  // namespace inviwo
