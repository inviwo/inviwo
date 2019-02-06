/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2018 Inviwo Foundation
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
#include <warn/pop>

#include <inviwo/qt/editor/linkgraphicsitem.h>
#include <inviwo/core/ports/port.h>
#include <inviwo/core/links/propertylink.h>
#include <inviwo/qt/editor/processorlinkgraphicsitem.h>
#include <inviwo/qt/editor/processorgraphicsitem.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/network/processornetwork.h>

#include <modules/qtwidgets/inviwoqtutils.h>

namespace inviwo {

LinkGraphicsItem::LinkGraphicsItem(ivec3 color) : color_(color.r, color.g, color.b) {
    setZValue(LINKGRAPHICSITEM_DEPTH);
}

LinkGraphicsItem::~LinkGraphicsItem() = default;

void LinkGraphicsItem::paint(QPainter* p, const QStyleOptionGraphicsItem*, QWidget*) {
    if (isSelected()) {
        p->setPen(QPen(Qt::darkRed, 2.0, Qt::DotLine, Qt::RoundCap));
    } else {
        p->setPen(QPen(color_, 2.0, Qt::DotLine, Qt::RoundCap));
    }
    p->drawPath(path_);
}

QPainterPath LinkGraphicsItem::obtainCurvePath() const {
    QPainterPath bezierCurve;

    const auto start = getStartPoint();
    const auto end = getEndPoint();

    const auto diff = start - end;
    const auto dist = std::sqrt(QPointF::dotProduct(diff, diff));
    const auto strength = 1.0 + std::min(50.0, 2.0 * dist);

    bezierCurve.moveTo(getStartPoint());
    bezierCurve.cubicTo(getStartPoint() + strength * getStartDir(),
                        getEndPoint() + strength * getEndDir(), getEndPoint());
    return bezierCurve;
}

QPainterPath LinkGraphicsItem::shape() const {
    QPainterPathStroker pathStrocker;
    pathStrocker.setWidth(10.0);
    return pathStrocker.createStroke(path_);
}

QRectF LinkGraphicsItem::boundingRect() const { return rect_; }

void LinkGraphicsItem::updateShape() {
    const auto start = utilqt::toGLM(getStartPoint());
    const auto end = utilqt::toGLM(getEndPoint());

    rect_ = QRectF(utilqt::toQPoint(glm::min(start, end)), utilqt::toQPoint(glm::max(start, end)))
                .marginsAdded(QMarginsF{40.0, 10.0, 40.0, 10.0});

    path_ = obtainCurvePath();

    prepareGeometryChange();
}

LinkConnectionDragGraphicsItem::LinkConnectionDragGraphicsItem(ProcessorLinkGraphicsItem* outLink,
                                                               QPointF endPos)
    : LinkGraphicsItem(), inLeft_(endPos), inRight_(endPos), outLink_(outLink) {}

LinkConnectionDragGraphicsItem::~LinkConnectionDragGraphicsItem() = default;

ProcessorLinkGraphicsItem* LinkConnectionDragGraphicsItem::getSrcProcessorLinkGraphicsItem() const {
    return outLink_;
}

ProcessorGraphicsItem* LinkConnectionDragGraphicsItem::getSrcProcessorGraphicsItem() const {
    return outLink_->getProcessorGraphicsItem();
}

QPointF LinkConnectionDragGraphicsItem::compare(const QPointF startLeft, const QPointF& startRight,
                                                const QPointF& endLeft, const QPointF& endRight,
                                                const QPointF& left, const QPointF& center,
                                                const QPointF& right) {

    if (startLeft.x() > endRight.x()) {
        return left;
    } else if (startRight.x() < endLeft.x()) {
        return right;
    } else {
        return center;
    }
}

QPointF LinkConnectionDragGraphicsItem::getStartPoint() const {
    const auto startLeft = outLink_->getLeftPos();
    const auto startRight = outLink_->getRightPos();
    return compare(startLeft, startRight, inLeft_, inRight_, startLeft, startRight, startRight);
}
QPointF LinkConnectionDragGraphicsItem::getEndPoint() const {
    const auto startLeft = outLink_->getLeftPos();
    const auto startRight = outLink_->getRightPos();
    return compare(startLeft, startRight, inLeft_, inRight_, inRight_, inRight_, inLeft_);
}

QPointF LinkConnectionDragGraphicsItem::getStartDir() const {
    const auto qp = QPointF(1, 0);
    const auto startLeft = outLink_->getLeftPos();
    const auto startRight = outLink_->getRightPos();
    return compare(startLeft, startRight, inLeft_, inRight_, -qp, qp, qp);
}
QPointF LinkConnectionDragGraphicsItem::getEndDir() const {
    const auto qp = QPointF(1, 0);
    const auto startLeft = outLink_->getLeftPos();
    const auto startRight = outLink_->getRightPos();
    return compare(startLeft, startRight, inLeft_, inRight_, qp, qp, -qp);
}

void LinkConnectionDragGraphicsItem::setEndPoint(QPointF endPoint) {
    inLeft_ = endPoint;
    inRight_ = endPoint;
    updateShape();
}

void LinkConnectionDragGraphicsItem::setEndPoint(QPointF endPointLeft, QPointF endPointRight) {
    inLeft_ = endPointLeft;
    inRight_ = endPointRight;
    updateShape();
}

void LinkConnectionDragGraphicsItem::reactToProcessorHover(ProcessorGraphicsItem* processor) {
    if (processor != nullptr) {
        inRight_ = processor->getLinkGraphicsItem()->getRightPos();
        inLeft_ = processor->getLinkGraphicsItem()->getLeftPos();
        updateShape();
    }
}

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

QPointF LinkConnectionGraphicsItem::getStartPoint() const {
    const auto startLeft = outLink_->getLeftPos();
    const auto startRight = outLink_->getRightPos();
    const auto endRight = inLink_->getRightPos();
    const auto endLeft = inLink_->getLeftPos();
    return compare(startLeft, startRight, endLeft, endRight, startLeft, startRight, startRight);
}
QPointF LinkConnectionGraphicsItem::getEndPoint() const {
    const auto startLeft = outLink_->getLeftPos();
    const auto startRight = outLink_->getRightPos();
    const auto endRight = inLink_->getRightPos();
    const auto endLeft = inLink_->getLeftPos();
    return compare(startLeft, startRight, endLeft, endRight, endRight, endRight, endLeft);
}

QPointF LinkConnectionGraphicsItem::getStartDir() const {
    const auto qp = QPointF(1, 0);
    const auto startLeft = outLink_->getLeftPos();
    const auto startRight = outLink_->getRightPos();
    const auto endRight = inLink_->getRightPos();
    const auto endLeft = inLink_->getLeftPos();
    return compare(startLeft, startRight, endLeft, endRight, -qp, qp, qp);
}
QPointF LinkConnectionGraphicsItem::getEndDir() const {
    const auto qp = QPointF(1, 0);
    const auto startLeft = outLink_->getLeftPos();
    const auto startRight = outLink_->getRightPos();
    const auto endRight = inLink_->getRightPos();
    const auto endLeft = inLink_->getLeftPos();
    return compare(startLeft, startRight, endLeft, endRight, qp, qp, -qp);
}

ProcessorLinkGraphicsItem* LinkConnectionGraphicsItem::getDestProcessorLinkGraphicsItem() const {
    return inLink_;
}

ProcessorGraphicsItem* LinkConnectionGraphicsItem::getDestProcessorGraphicsItem() const {
    return inLink_->getProcessorGraphicsItem();
}

std::string getLinkInfoTableRows(const std::vector<PropertyLink>& links,
                                 const std::string& imgName) {
    std::string str;
    for (auto& item : links) {
        auto srcProperty = item.getSource();
        auto dstProperty = item.getDestination();
        str += "<tr><td align='center'>" + srcProperty->getDisplayName() +
               "</td><td width='30px' align='center' valign='middle'><img src='" + imgName +
               "'></td><td align='center'>" + dstProperty->getDisplayName() + "</td></tr>";
    }
    return str;
}

class LinkConnectionGraphicsItemMatchReverse {
public:
    LinkConnectionGraphicsItemMatchReverse(PropertyLink link) : link_(link) {}
    bool operator()(const PropertyLink& link) const {
        return link.getDestination() == link_.getSource() &&
               link.getSource() == link_.getDestination();
    }

private:
    PropertyLink link_;
};

void LinkConnectionGraphicsItem::showToolTip(QGraphicsSceneHelpEvent* e) {
    Processor* p1 = inLink_->getProcessorGraphicsItem()->getProcessor();
    Processor* p2 = outLink_->getProcessorGraphicsItem()->getProcessor();

    auto propertyLinks =
        InviwoApplication::getPtr()->getProcessorNetwork()->getLinksBetweenProcessors(p1, p2);

    // collect all links based on their direction

    std::vector<PropertyLink> bidirectional;
    std::vector<PropertyLink> outgoing;  // from processor 1
    std::vector<PropertyLink> incoming;  // toward processor 1

    for (const auto& propertyLink : propertyLinks) {
        Processor* linkSrc =
            dynamic_cast<Processor*>(propertyLink.getSource()->getOwner()->getProcessor());

        if (linkSrc == p1) {
            // forward link
            auto sit = std::find_if(incoming.begin(), incoming.end(),
                                    LinkConnectionGraphicsItemMatchReverse(propertyLink));
            if (sit != incoming.end()) {
                bidirectional.push_back(propertyLink);
                incoming.erase(sit);
            } else {
                outgoing.push_back(propertyLink);
            }
        } else {  // if (linkSrc == processorB)
            auto sit = std::find_if(outgoing.begin(), outgoing.end(),
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

}  // namespace inviwo
