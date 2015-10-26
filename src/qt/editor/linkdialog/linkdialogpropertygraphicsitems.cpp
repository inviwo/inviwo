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

#include <QGraphicsDropShadowEffect>
#include <QVector2D>

#include <inviwo/qt/editor/linkdialog/linkdialogpropertygraphicsitems.h>
#include <inviwo/qt/editor/linkdialog/linkdialogprocessorgraphicsitems.h>
#include <inviwo/qt/editor/linkdialog/linkdialogcurvegraphicsitems.h>
#include <inviwo/qt/editor/linkdialog/linkdialogscene.h>
#include <inviwo/core/properties/compositeproperty.h>
#include <inviwo/qt/widgets/inviwoqtutils.h>

#include <numeric>

namespace inviwo {

LinkDialogPropertyGraphicsItem::LinkDialogPropertyGraphicsItem(LinkDialogParent* parent,
                                                               Property* prop)
    : GraphicsItemData<Property>(parent->getSide(), prop)

    , isExpanded_(false)
    , animateEnabled_(false)
    , parent_(parent) {
    setZValue(linkdialog::propertyDepth);
    setFlags(ItemSendsScenePositionChanges);

    int propWidth = linkdialog::propertyWidth -
                    ((parent->getLevel() + 1) * linkdialog::propertyExpandCollapseOffset);
    setRect(-propWidth / 2, -linkdialog::propertyHeight / 2, propWidth, linkdialog::propertyHeight);

    auto displayName = new LabelGraphicsItem(this);
    displayName->setPos(rect().topLeft() + QPointF(linkdialog::offset, linkdialog::offset));
    displayName->setDefaultTextColor(Qt::black);
    auto dispFont = QFont("Segoe", linkdialog::propertyLabelHeight, QFont::Bold, false);
    dispFont.setPixelSize(linkdialog::propertyLabelHeight);
    displayName->setFont(dispFont);
    displayName->setCrop(15, 14);
    displayName->setText(QString::fromStdString(item_->getDisplayName()));


    auto classIdentifier = new LabelGraphicsItem(this);
    classIdentifier->setDefaultTextColor(Qt::black);
    auto classFont = QFont("Segoe", linkdialog::processorLabelHeight, QFont::Normal, true);
    classFont.setPixelSize(linkdialog::processorLabelHeight);
    classIdentifier->setFont(classFont);
    classIdentifier->setCrop(15, 14);
    auto offset = classIdentifier->boundingRect().height();
    classIdentifier->setPos(rect().bottomLeft() +
                            QPointF(linkdialog::offset, -linkdialog::offset - offset));
    std::string className = item_->getClassIdentifier();
    className = removeSubString(className, "Property");
    classIdentifier->setText(QString::fromStdString(className));

    if (auto compProp = dynamic_cast<CompositeProperty*>(prop)) {
        QPointF newPos(0.0f, rect().height());
        for (auto& subProperty : compProp->getProperties()) {
            auto item = new LinkDialogPropertyGraphicsItem(this, subProperty);
            item->hide();
            item->setParentItem(this);
            item->setPos(isExpanded_ ? newPos : QPointF(0.0, 0.0));
            size_t count = 1 + item->getTotalVisibleChildCount();
            newPos += QPointF(0, count * linkdialog::propertyHeight);
            subProperties_.push_back(item);
        }
    }
}

LinkDialogPropertyGraphicsItem::~LinkDialogPropertyGraphicsItem() {}

void LinkDialogPropertyGraphicsItem::setAnimate(bool animate) { animateEnabled_ = animate; }
const bool LinkDialogPropertyGraphicsItem::getAnimate() const { return animateEnabled_; }

void LinkDialogPropertyGraphicsItem::setExpanded(bool expand) {
    isExpanded_ = expand;
    for (auto& elem : subProperties_) {
        elem->setVisible(expand);
    }
    updatePositions();
}

bool LinkDialogPropertyGraphicsItem::isExpanded() const { return isExpanded_; }

bool LinkDialogPropertyGraphicsItem::hasSubProperties() const { return subProperties_.size() > 0; }

int LinkDialogPropertyGraphicsItem::getLevel() const { return parent_->getLevel() + 1; }

size_t LinkDialogPropertyGraphicsItem::getTotalVisibleChildCount() const {
    if (isExpanded_) {
        return std::accumulate(subProperties_.begin(), subProperties_.end(), 0,
                               [](size_t val, LinkDialogPropertyGraphicsItem* p) {
                                   return val + 1 + p->getTotalVisibleChildCount();
                               });
    } else {
        return 0;
    }
}

QSizeF LinkDialogPropertyGraphicsItem::sizeHint(Qt::SizeHint which,
                                                const QSizeF& constraint) const {
    switch (which) {
        case Qt::MinimumSize:
        case Qt::MaximumSize:
        case Qt::PreferredSize:
            return rect().size() + QSize(12, 12);

        case Qt::MinimumDescent:
        case Qt::NSizeHints:
        default:
            break;
    }

    return constraint;
}

void LinkDialogPropertyGraphicsItem::addConnectionGraphicsItem(
    DialogConnectionGraphicsItem* cItem) {
    connections_.push_back(cItem);
}

size_t LinkDialogPropertyGraphicsItem::getConnectionGraphicsItemCount() const {
    return connections_.size();
}

size_t LinkDialogPropertyGraphicsItem::getConnectionIndex(
    const DialogConnectionGraphicsItem* item) const {
    return std::find(connections_.begin(), connections_.end(), item) - connections_.begin();
}

void LinkDialogPropertyGraphicsItem::removeConnectionGraphicsItem(
    DialogConnectionGraphicsItem* cItem) {
    connections_.erase(std::remove(connections_.begin(), connections_.end(), cItem),
                       connections_.end());
}

QRectF LinkDialogPropertyGraphicsItem::calculateArrowRect(size_t curPort) const {
    auto centerEdge = calculateArrowCenterLocal(curPort);
    QSizeF arrowDim(linkdialog::arrowWidth, linkdialog::arrowHeight);

    switch (getSide()) {
        case LinkDialogParent::Side::Left:
            return QRectF(centerEdge + QPointF(-arrowDim.width(), -arrowDim.height() / 2),
                          arrowDim);
        case LinkDialogParent::Side::Right:
            return QRectF(centerEdge + QPointF(0, -arrowDim.height() / 2), arrowDim);
    }
}

QRectF LinkDialogPropertyGraphicsItem::calculateArrowRect(
    DialogConnectionGraphicsItem* cItem) const {
    return calculateArrowRect(getConnectionIndex(cItem));
}

void LinkDialogPropertyGraphicsItem::paint(QPainter* p, const QStyleOptionGraphicsItem* options,
                                           QWidget* widget) {
    IVW_UNUSED_PARAM(options);
    IVW_UNUSED_PARAM(widget);
    p->setPen(Qt::black);
    p->setRenderHint(QPainter::Antialiasing, true);
    p->save();
    // paint property
    QLinearGradient grad(rect().topLeft(), rect().bottomLeft());

    if (hasSubProperties()) {
        QColor bgColor = Qt::darkGray;
        grad.setColorAt(0.0f, bgColor);
        grad.setColorAt(1.0f, bgColor);
    } else {
        QColor bgColor = Qt::lightGray;
        grad.setColorAt(0.0f, bgColor);
        grad.setColorAt(1.0f, bgColor);
    }

    p->setBrush(grad);

    QPen blackPen(QColor(0, 0, 0), 1);
    QRectF bRect = rect();

    QPainterPath roundRectPath;
    roundRectPath.moveTo(bRect.left(), bRect.top() + linkdialog::propertyRoundedCorners);
    roundRectPath.lineTo(bRect.left(), bRect.bottom() - linkdialog::propertyRoundedCorners);
    roundRectPath.arcTo(bRect.left(), bRect.bottom() - (2 * linkdialog::propertyRoundedCorners),
                        (2 * linkdialog::propertyRoundedCorners),
                        (2 * linkdialog::propertyRoundedCorners), 180.0, 90.0);
    roundRectPath.lineTo(bRect.right() - linkdialog::propertyRoundedCorners, bRect.bottom());
    roundRectPath.arcTo(bRect.right() - (2 * linkdialog::propertyRoundedCorners),
                        bRect.bottom() - (2 * linkdialog::propertyRoundedCorners),
                        (2 * linkdialog::propertyRoundedCorners),
                        (2 * linkdialog::propertyRoundedCorners), 270.0, 90.0);
    roundRectPath.lineTo(bRect.right(), bRect.top() + linkdialog::propertyRoundedCorners);
    roundRectPath.arcTo(bRect.right() - (2 * linkdialog::propertyRoundedCorners), bRect.top(),
                        (2 * linkdialog::propertyRoundedCorners),
                        (2 * linkdialog::propertyRoundedCorners), 0.0, 90.0);
    roundRectPath.lineTo(bRect.left() + linkdialog::propertyRoundedCorners, bRect.top());
    roundRectPath.arcTo(bRect.left(), bRect.top(), (2 * linkdialog::propertyRoundedCorners),
                        (2 * linkdialog::propertyRoundedCorners), 90.0, 90.0);
    p->drawPath(roundRectPath);

    QPainterPath roundRectPath_Top;
    QPainterPath roundRectPath_Left;
    QPainterPath roundRectPath_Bottom;
    QPainterPath roundRectPath_Right;

    // Left
    p->setPen(blackPen);
    roundRectPath_Left.moveTo(bRect.left(), bRect.top());
    roundRectPath_Left.lineTo(bRect.left(), bRect.bottom());
    p->drawPath(roundRectPath_Left);

    // Bottom
    p->setPen(blackPen);
    roundRectPath_Bottom.moveTo(bRect.left(), bRect.bottom());
    roundRectPath_Bottom.lineTo(bRect.right(), bRect.bottom());
    p->drawPath(roundRectPath_Bottom);

    // Right
    p->setPen(blackPen);
    roundRectPath_Right.moveTo(bRect.right(), bRect.bottom());
    roundRectPath_Right.lineTo(bRect.right(), bRect.top());
    p->drawPath(roundRectPath_Right);

    // Top
    p->setPen(blackPen);
    roundRectPath_Top.moveTo(bRect.left(), bRect.top());
    roundRectPath_Top.lineTo(bRect.right(), bRect.top());
    p->drawPath(roundRectPath_Top);

    p->restore();
    p->save();
    QPoint arrowDim(linkdialog::arrowWidth, linkdialog::arrowHeight);

    for (auto& elem : connections_) {
        if (elem->getStartProperty() == this && !elem->isBidirectional()) continue;

        // If arrow points right, then get the rectangle aligned to the left-
        // boundary of property item (rectangle) and vice versa
        QRectF arrowRect = calculateArrowRect(elem);

        // set color of arrow
        p->setPen(Qt::black);
        p->setBrush(Qt::green);

        QPainterPath rectPath;
        if (getSide() == LinkDialogParent::Side::Right) {
            rectPath.moveTo(arrowRect.left(), arrowRect.top());
            rectPath.lineTo(arrowRect.left(), arrowRect.bottom());
            rectPath.lineTo(arrowRect.right(), arrowRect.bottom() - arrowRect.height() / 2);
            rectPath.closeSubpath();
        } else {
            rectPath.moveTo(arrowRect.right(), arrowRect.top());
            rectPath.lineTo(arrowRect.right(), arrowRect.bottom());
            rectPath.lineTo(arrowRect.left(), arrowRect.bottom() - arrowRect.height() / 2);
            rectPath.closeSubpath();
        }

        p->drawPath(rectPath);
    }

    p->restore();
}

QVariant LinkDialogPropertyGraphicsItem::itemChange(GraphicsItemChange change,
                                                    const QVariant& value) {
    if (change == QGraphicsItem::ItemScenePositionHasChanged) {
        for (auto connection : connections_) {
            connection->updateStartEndPoint();
        }
    }

    return QGraphicsItem::itemChange(change, value);
}

void LinkDialogPropertyGraphicsItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* e) {
    setExpanded(!isExpanded_);
}

QPointF LinkDialogPropertyGraphicsItem::getConnectionPoint() {
    qreal dir = getSide() == LinkDialogParent::Side::Left ? 1.0f : -1.0;
    QPointF offset{dir * rect().width() / 2, 0.0f};
    return scenePos() + offset;
}

QPointF LinkDialogPropertyGraphicsItem::calculateArrowCenterLocal(size_t curPort) const {
    QPointF bottom =
        getSide() == LinkDialogParent::Side::Left ? rect().bottomRight() : rect().bottomLeft();
    QPointF top = getSide() == LinkDialogParent::Side::Left ? rect().topRight() : rect().topLeft();

    size_t arrowCount = getConnectionGraphicsItemCount();
    if (arrowCount == 0) arrowCount++;

    return top + (bottom - top) * (curPort + 1.0) / (arrowCount + 1.0);
}

QPointF LinkDialogPropertyGraphicsItem::calculateArrowCenter(size_t curPort) const {
    return scenePos() + calculateArrowCenterLocal(curPort);
}

const std::vector<DialogConnectionGraphicsItem*>&
LinkDialogPropertyGraphicsItem::getConnectionGraphicsItems() const {
    return connections_;
}

std::vector<LinkDialogPropertyGraphicsItem*> LinkDialogPropertyGraphicsItem::getSubPropertyItemList(
    bool recursive) const {
    if (!recursive) return subProperties_;

    std::vector<LinkDialogPropertyGraphicsItem*> subProps;

    for (auto& elem : subProperties_) {
        subProps.push_back(elem);
        std::vector<LinkDialogPropertyGraphicsItem*> props =
            elem->getSubPropertyItemList(recursive);
        for (auto& prop : props) subProps.push_back(prop);
    }

    return subProps;
}

void LinkDialogPropertyGraphicsItem::updatePositions() {
    QPointF newPos(0.0f, rect().height());
    for (auto property : subProperties_) {
        property->setPos(isExpanded_? newPos : QPointF(0.0,0.0));
        size_t count = 1 + property->getTotalVisibleChildCount();
        newPos += QPointF(0, count * linkdialog::propertyHeight);
    }

    dynamic_cast<LinkDialogParent*>(parentItem())->updatePositions();
}

void LinkDialogPropertyGraphicsItem::showToolTip(QGraphicsSceneHelpEvent* e) {
    showToolTipHelper(e, utilqt::toLocalQString("todo"));
}

}  // namespace