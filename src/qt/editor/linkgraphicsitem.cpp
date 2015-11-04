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

#include <warn/push>
#include <warn/ignore/all>
#include <QGraphicsDropShadowEffect>
#include <QPainter>
#include <QString>
#include <QPainterPath>
#include <QVector2D>
#include <warn/pop>

#include <inviwo/qt/editor/linkgraphicsitem.h>
#include <inviwo/core/ports/port.h>
#include <inviwo/core/links/propertylink.h>
#include <inviwo/qt/editor/processorlinkgraphicsitem.h>
#include <inviwo/qt/editor/processorgraphicsitem.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/network/processornetwork.h>

namespace inviwo {

LinkGraphicsItem::LinkGraphicsItem(QPointF startPoint, QPointF endPoint, ivec3 color,
                                   QPointF startDir, QPointF endDir)
    : startPoint_(startPoint)
    , endPoint_(endPoint)
    , color_(color.r, color.g, color.b)
    , startDir_(startDir)
    , endDir_(endDir) {
    setZValue(LINKGRAPHICSITEM_DEPTH);
    QGraphicsDropShadowEffect* processorShadowEffect = new QGraphicsDropShadowEffect();
    processorShadowEffect->setOffset(3.0);
    processorShadowEffect->setBlurRadius(3.0);
    setGraphicsEffect(processorShadowEffect);
}

LinkGraphicsItem::~LinkGraphicsItem() {}

void LinkGraphicsItem::paint(QPainter* p, const QStyleOptionGraphicsItem* options,
                             QWidget* widget) {
    IVW_UNUSED_PARAM(options);
    IVW_UNUSED_PARAM(widget);

    if (isSelected())
        p->setPen(QPen(Qt::darkRed, 2.0, Qt::DotLine, Qt::RoundCap));
    else
        p->setPen(QPen(color_, 2.0, Qt::DotLine, Qt::RoundCap));

    p->drawPath(path_);
}

QPainterPath LinkGraphicsItem::obtainCurvePath() const {
    QPainterPath bezierCurve;
    float dist =
        1.0f +
        std::min(50.0f, 2.0f * static_cast<float>(QVector2D(startPoint_ - endPoint_).length()));

    bezierCurve.moveTo(startPoint_);
    bezierCurve.cubicTo(startPoint_ + dist * startDir_, endPoint_ + dist * endDir_, endPoint_);
    return bezierCurve;
}

QPainterPath LinkGraphicsItem::shape() const {
    QPainterPathStroker pathStrocker;
    pathStrocker.setWidth(10.0);
    return pathStrocker.createStroke(path_);
}

QRectF LinkGraphicsItem::boundingRect() const { return rect_; }

void LinkGraphicsItem::setStartPoint(QPointF startPoint) {
    updateShape();
    startPoint_ = startPoint;
}

void LinkGraphicsItem::setEndPoint(QPointF endPoint) {
    updateShape();
    endPoint_ = endPoint;
}

QPointF LinkGraphicsItem::getStartPoint() const { return startPoint_; }

void LinkGraphicsItem::setStartDir(QPointF dir) {
    updateShape();
    startDir_ = dir;
}

QPointF LinkGraphicsItem::getStartDir() const { return startDir_; }

void LinkGraphicsItem::setEndDir(QPointF dir) {
    updateShape();
    endDir_ = dir;
}

QPointF LinkGraphicsItem::getEndDir() const { return endDir_; }

QPointF LinkGraphicsItem::getEndPoint() const { return endPoint_; }

void LinkGraphicsItem::updateShape() {
    prepareGeometryChange();

    QPointF topLeft =
        QPointF(std::min(startPoint_.x(), endPoint_.x()), std::min(startPoint_.y(), endPoint_.y()));
    rect_ = QRectF(topLeft.x() - 40.0, topLeft.y() - 10.0,
                   std::abs(startPoint_.x() - endPoint_.x()) + 80.0,
                   std::abs(startPoint_.y() - endPoint_.y()) + 20.0);

    path_ = obtainCurvePath();
}

//////////////////////////////////////////////////////////////////////////

LinkConnectionDragGraphicsItem::LinkConnectionDragGraphicsItem(ProcessorLinkGraphicsItem* outLink,
                                                               QPointF endPos)
    : LinkGraphicsItem(QPointF(0, 0), endPos)
    , inLeft_(endPoint_)
    , inRight_(endPoint_)
    , outLink_(outLink) {}

LinkConnectionDragGraphicsItem::~LinkConnectionDragGraphicsItem() {}

ProcessorLinkGraphicsItem* LinkConnectionDragGraphicsItem::getSrcProcessorLinkGraphicsItem() const {
    return outLink_;
}

ProcessorGraphicsItem* LinkConnectionDragGraphicsItem::getSrcProcessorGraphicsItem() const {
    return outLink_->getProcessorGraphicsItem();
}

QPainterPath LinkConnectionDragGraphicsItem::obtainCurvePath() const {
    QPointF inLeft = inLeft_;
    QPointF inRight = inRight_;
    QPointF outRight = outLink_->getRightPos();
    QPointF outLeft = outLink_->getLeftPos();

    QPointF start;
    QPointF stop;
    QPointF ctrlPointStart;
    QPointF ctrlPointStop;
    QPointF qp = QPointF(1, 0);
    QPainterPath bezierCurve;

    if (outLeft.x() <= inLeft.x()) {
        start = outLeft;
        stop = inLeft;
        ctrlPointStart = qp;
        ctrlPointStop = -qp;
    } else if (outRight.x() >= inRight.x()) {
        start = outRight;
        stop = inRight;
        ctrlPointStart = -qp;
        ctrlPointStop = qp;
    } else {
        start = outLeft;
        stop = inRight;
        ctrlPointStart = qp;
        ctrlPointStop = qp;
    }

    float dist =
        1.0f + std::min(50.0f, 2.0f * static_cast<float>(QVector2D(start - stop).length()));
    bezierCurve.moveTo(start);
    bezierCurve.cubicTo(start + dist * ctrlPointStart, stop + dist * ctrlPointStop, stop);
    return bezierCurve;
}

void LinkConnectionDragGraphicsItem::reactToProcessorHover(ProcessorGraphicsItem* processor) {
    if (processor != nullptr) {
        inLeft_ = processor->getLinkGraphicsItem()->getRightPos();
        inRight_ = processor->getLinkGraphicsItem()->getLeftPos();
    } else {
        inLeft_ = endPoint_;
        inRight_ = endPoint_;
    }
}

void LinkConnectionDragGraphicsItem::updateShape() {
    QPointF inLeft = inLeft_;
    QPointF inRight = inRight_;
    QPointF outRight = outLink_->getRightPos();
    QPointF outLeft = outLink_->getLeftPos();

    QPointF start;
    QPointF stop;

    if (outLeft.x() < inLeft.x()) {
        start = outLeft;
        stop = inLeft;
    } else if (outRight.x() > inRight.x()) {
        start = outRight;
        stop = inRight;
    } else {
        start = outLeft;
        stop = inRight;
    }

    QPointF topLeft = QPointF(std::min(start.x(), stop.x()), std::min(start.y(), stop.y()));
    QPointF bottomRight = QPointF(std::max(start.x(), stop.x()), std::max(start.y(), stop.y()));
    rect_ = QRectF(topLeft.x() - 30, topLeft.y() - 10, bottomRight.x() - topLeft.x() + 70,
                   bottomRight.y() - topLeft.y() + 20);

    path_ = obtainCurvePath();

    prepareGeometryChange();
}

//////////////////////////////////////////////////////////////////////////

LinkConnectionGraphicsItem::LinkConnectionGraphicsItem(ProcessorLinkGraphicsItem* outLink,
                                                       ProcessorLinkGraphicsItem* inLink)
    : LinkConnectionDragGraphicsItem(outLink, QPointF(0, 0)), inLink_(inLink) {
    setFlags(ItemIsSelectable | ItemIsFocusable);

    outLink_->addLink(this);
    inLink_->addLink(this);

    setVisible(outLink_->isVisible() && inLink_->isVisible());
}

LinkConnectionGraphicsItem::~LinkConnectionGraphicsItem() {
    outLink_->removeLink(this);
    inLink_->removeLink(this);
}

QPainterPath LinkConnectionGraphicsItem::obtainCurvePath() const {
    QPointF inRight = inLink_->getRightPos();
    QPointF inLeft = inLink_->getLeftPos();
    QPointF outRight = outLink_->getRightPos();
    QPointF outLeft = outLink_->getLeftPos();

    QPointF start;
    QPointF stop;
    QPointF ctrlPointStart;
    QPointF ctrlPointStop;
    QPointF qp = QPointF(1, 0);
    QPainterPath bezierCurve;

    if (outLeft.x() <= inRight.x()) {
        start = outLeft;
        stop = inRight;
        ctrlPointStart = qp;
        ctrlPointStop = -qp;
    } else if (outRight.x() >= inLeft.x()) {
        start = outRight;
        stop = inLeft;
        ctrlPointStart = -qp;
        ctrlPointStop = qp;
    } else {
        start = outLeft;
        stop = inLeft;
        ctrlPointStart = qp;
        ctrlPointStop = qp;
    }

    float dist =
        1.0f + std::min(50.0f, 2.0f * static_cast<float>(QVector2D(start - stop).length()));
    bezierCurve.moveTo(start);
    bezierCurve.cubicTo(start + dist * ctrlPointStart, stop + dist * ctrlPointStop, stop);
    return bezierCurve;
}

ProcessorLinkGraphicsItem* LinkConnectionGraphicsItem::getDestProcessorLinkGraphicsItem() const {
    return inLink_;
}

ProcessorGraphicsItem* LinkConnectionGraphicsItem::getDestProcessorGraphicsItem() const {
    return inLink_->getProcessorGraphicsItem();
}

void LinkConnectionGraphicsItem::updateShape() {
    QPointF inRight = inLink_->getRightPos();
    QPointF inLeft = inLink_->getLeftPos();
    QPointF outRight = outLink_->getRightPos();
    QPointF outLeft = outLink_->getLeftPos();

    QPointF start;
    QPointF stop;

    if (outLeft.x() < inRight.x()) {
        start = outLeft;
        stop = inRight;
    } else if (outRight.x() > inLeft.x()) {
        start = outRight;
        stop = inLeft;
    } else {
        start = outLeft;
        stop = inLeft;
    }

    QPointF topLeft = QPointF(std::min(start.x(), stop.x()), std::min(start.y(), stop.y()));
    QPointF bottomRight = QPointF(std::max(start.x(), stop.x()), std::max(start.y(), stop.y()));
    rect_ = QRectF(topLeft.x() - 30, topLeft.y() - 10, bottomRight.x() - topLeft.x() + 70,
                   bottomRight.y() - topLeft.y() + 20);

    path_ = obtainCurvePath();

    prepareGeometryChange();
}

std::string getLinkInfoTableRows(const std::vector<PropertyLink*>& links,
                                 const std::string& imgName) {
    std::string str;
    std::vector<PropertyLink*>::const_iterator it = links.begin();
    while (it != links.end()) {
        Property* srcProperty = (*it)->getSourceProperty();
        Property* dstProperty = (*it)->getDestinationProperty();
        str += "<tr><td align='center'>" + srcProperty->getDisplayName() +
               "</td><td width='30px' align='center' valign='middle'><img src='" + imgName +
               "'></td><td align='center'>" + dstProperty->getDisplayName() + "</td></tr>";
        ++it;
    }
    return str;
}

class LinkConnectionGraphicsItemMatchReverse {
public:
    LinkConnectionGraphicsItemMatchReverse(PropertyLink* link) : link_(link) {}
    bool operator()(const PropertyLink* link) const {
        return link->getDestinationProperty() == link_->getSourceProperty() &&
               link->getSourceProperty() == link_->getDestinationProperty();
    }

private:
    PropertyLink* link_;
};

void LinkConnectionGraphicsItem::showToolTip(QGraphicsSceneHelpEvent* e) {
    Processor* p1 = inLink_->getProcessorGraphicsItem()->getProcessor();
    Processor* p2 = outLink_->getProcessorGraphicsItem()->getProcessor();

    const std::vector<PropertyLink*> propertyLinks =
        InviwoApplication::getPtr()->getProcessorNetwork()->getLinksBetweenProcessors(p1, p2);

    // collect all links based on their direction

    std::vector<PropertyLink*> bidirectional;
    std::vector<PropertyLink*> outgoing;  // from processor 1
    std::vector<PropertyLink*> incoming;  // toward processor 1

    for (const auto& propertyLink : propertyLinks) {
        Processor* linkSrc = dynamic_cast<Processor*>(
            (propertyLink)->getSourceProperty()->getOwner()->getProcessor());

        if (linkSrc == p1) {
            // forward link
            std::vector<PropertyLink*>::iterator sit =
                std::find_if(incoming.begin(), incoming.end(),
                             LinkConnectionGraphicsItemMatchReverse(propertyLink));
            if (sit != incoming.end()) {
                bidirectional.push_back(propertyLink);
                incoming.erase(sit);
            } else {
                outgoing.push_back(propertyLink);
            }
        } else {  // if (linkSrc == processorB)
            std::vector<PropertyLink*>::iterator sit =
                std::find_if(outgoing.begin(), outgoing.end(),
                             LinkConnectionGraphicsItemMatchReverse(propertyLink));
            if (sit != outgoing.end()) {
                bidirectional.push_back(propertyLink);
                outgoing.erase(sit);
            } else {
                incoming.push_back(propertyLink);
            }
        }
    }

    // set up a HTML table containing three columns:
    //    props of outProcesser, link indicator, props of inProcessor
    std::string info =
        "<html><head/><body style=''>\
           <table border='0' cellspacing='2' cellpadding='0' style='border-color:white;white-space:pre;'>";
    // put in the table header consisting of both processor names
    info += "<tr style='color:#bbb;font-weight:bold;'><td align='center'>" + p1->getIdentifier() +
            "</td><td align='center'></td><td align='center'>" + p2->getIdentifier() + "</td></tr>";

    // add outgoing links first
    info.append(getLinkInfoTableRows(outgoing, ":/icons/linkarrow_right.png"));
    // add bidirectional links
    info.append(getLinkInfoTableRows(bidirectional, ":/icons/linkarrow_bidirectional.png"));
    // add incoming links
    info.append(getLinkInfoTableRows(incoming, ":/icons/linkarrow_left.png"));

    info.append("</table></body></html>");

    showToolTipHelper(e, QString(info.c_str()));
}

}  // namespace