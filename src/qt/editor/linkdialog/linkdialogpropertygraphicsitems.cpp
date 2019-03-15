
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
#include <QVector2D>
#include <warn/pop>

#include <inviwo/qt/editor/linkdialog/linkdialogpropertygraphicsitems.h>
#include <inviwo/qt/editor/linkdialog/linkdialogprocessorgraphicsitems.h>
#include <inviwo/qt/editor/linkdialog/linkdialogcurvegraphicsitems.h>
#include <inviwo/qt/editor/linkdialog/linkdialogscene.h>
#include <inviwo/core/properties/compositeproperty.h>
#include <modules/qtwidgets/inviwoqtutils.h>
#include <inviwo/core/network/processornetwork.h>

#include <numeric>

namespace inviwo {

LinkDialogPropertyGraphicsItem::LinkDialogPropertyGraphicsItem(LinkDialogTreeItem* parent,
                                                               Property* prop)
    : GraphicsItemData<Property>(parent, parent->getSide(), prop) {

    setZValue(linkdialog::propertyDepth);
    setFlags(ItemSendsScenePositionChanges);

    int propWidth =
        linkdialog::propertyWidth - (getLevel() * linkdialog::propertyExpandCollapseOffset);
    setRect(-propWidth / 2, -linkdialog::propertyHeight / 2, propWidth, linkdialog::propertyHeight);

    auto displayName = new LabelGraphicsItem(this);
    displayName->setPos(rect().topLeft() + QPointF(linkdialog::offset, linkdialog::offset));
    displayName->setDefaultTextColor(Qt::black);
    auto dispFont = QFont("Segoe", linkdialog::propertyLabelHeight, QFont::Bold, false);
    dispFont.setPixelSize(linkdialog::propertyLabelHeight);
    displayName->setFont(dispFont);
    displayName->setCrop(static_cast<int>(rect().width() - 2.0 * linkdialog::offset));
    displayName->setText(QString::fromStdString(item_->getDisplayName()));

    auto classIdentifier = new LabelGraphicsItem(this);
    classIdentifier->setDefaultTextColor(Qt::black);
    auto classFont = QFont("Segoe", linkdialog::processorLabelHeight, QFont::Normal, true);
    classFont.setPixelSize(linkdialog::processorLabelHeight);
    classIdentifier->setFont(classFont);
    classIdentifier->setCrop(static_cast<int>(rect().width() - 2.0 * linkdialog::offset));
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
            subProperties_.push_back(item);
            item->hide();
            item->setParentItem(this);
        }
    }
}

void LinkDialogPropertyGraphicsItem::updatePositions() {
    auto visible = propertyVisible();
    setVisible(visible);
    auto pos = prev()->treeItemScenePos();
    if (visible) pos += QPointF(0.0f, prev()->treeItemRect().height());
    setPos(mapToParent(mapFromScene(pos)));
}

bool LinkDialogPropertyGraphicsItem::propertyVisible() const {
    if (auto p = qgraphicsitem_cast<LinkDialogPropertyGraphicsItem*>(parentItem())) {
        if (!p->isExpanded()) return false;
        if (!p->propertyVisible()) return false;
    }
    bool show = false;
    if (auto s = qobject_cast<LinkDialogGraphicsScene*>(scene())) {
        show = s->isShowingHidden();
    }
    return item_->getVisible() || !connections_.empty() || show;
}

bool LinkDialogPropertyGraphicsItem::hasSubProperties() const { return subProperties_.size() > 0; }

int LinkDialogPropertyGraphicsItem::getLevel() const { return parent()->getLevel() + 1; }

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

    if (connections_.size() == 1) {
        LinkDialogTreeItem* item = this;
        while (item) {
            item->updatePositions();
            item = item->next();
        }
    }
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

    if (connections_.empty()) {
        LinkDialogTreeItem* item = this;
        while (item) {
            item->updatePositions();
            item = item->next();
        }
    }
}

QRectF LinkDialogPropertyGraphicsItem::calculateArrowRect(size_t curPort) const {
    auto centerEdge = calculateArrowCenterLocal(curPort);
    QSizeF arrowDim(linkdialog::arrowWidth, linkdialog::arrowHeight);

    switch (getSide()) {
        case LinkDialogTreeItem::Side::Left:
            return QRectF(centerEdge + QPointF(-arrowDim.width(), -arrowDim.height() / 2),
                          arrowDim);
        case LinkDialogTreeItem::Side::Right:
            return QRectF(centerEdge + QPointF(0, -arrowDim.height() / 2), arrowDim);
        default:
            return QRectF();
    }
}

QRectF LinkDialogPropertyGraphicsItem::calculateArrowRect(
    DialogConnectionGraphicsItem* cItem) const {
    return calculateArrowRect(getConnectionIndex(cItem));
}

QVariant LinkDialogPropertyGraphicsItem::itemChange(GraphicsItemChange change,
                                                    const QVariant& value) {
    if (change == QGraphicsItem::ItemScenePositionHasChanged) {
        for (auto connection : connections_) {
            connection->updateShape();
        }
    }
    return QGraphicsItem::itemChange(change, value);
}

QPointF LinkDialogPropertyGraphicsItem::getConnectionPoint() {
    qreal dir = getSide() == Side::Left ? 1.0f : -1.0;
    QPointF offset{dir * rect().width() / 2, 0.0f};
    return scenePos() + offset;
}

QPointF LinkDialogPropertyGraphicsItem::calculateArrowCenterLocal(size_t curPort) const {
    QPointF bottom = getSide() == Side::Left ? rect().bottomRight() : rect().bottomLeft();
    QPointF top = getSide() == Side::Left ? rect().topRight() : rect().topLeft();

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
        auto props = elem->getSubPropertyItemList(recursive);
        for (auto& prop : props) subProps.push_back(prop);
    }

    return subProps;
}

void LinkDialogPropertyGraphicsItem::showToolTip(QGraphicsSceneHelpEvent* e) {
    showToolTipHelper(e, utilqt::toLocalQString(item_->getDescription()));
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

    QPen blackPen(QColor(0, 0, 0), 1, item_->getVisible() ? Qt::SolidLine : Qt::DashLine);
    p->setPen(blackPen);

    p->drawRoundedRect(rect(), linkdialog::propertyRoundedCorners,
                       linkdialog::propertyRoundedCorners);

    p->restore();
    p->save();

    auto isBidirectional = [](DialogConnectionGraphicsItem* item) {
        auto net = item->getPropertyLink().getSource()->getOwner()->getProcessor()->getNetwork();

        return net->isLinkedBidirectional(item->getPropertyLink().getSource(),
                                          item->getPropertyLink().getDestination());
    };

    for (auto& elem : connections_) {
        if (elem->getStartProperty() == this && !isBidirectional(elem)) continue;

        // If arrow points right, then get the rectangle aligned to the left-
        // boundary of property item (rectangle) and vice versa
        QRectF arrowRect = calculateArrowRect(elem);

        // set color of arrow
        p->setPen(Qt::black);
        p->setBrush(Qt::green);

        QPainterPath rectPath;
        if (getSide() == Side::Right) {
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

}  // namespace inviwo
