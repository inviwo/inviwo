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

#include <QGraphicsDropShadowEffect>
#include <QVector2D>

#include <inviwo/qt/editor/linkdialog/linkdialogpropertygraphicsitems.h>
#include <inviwo/qt/editor/linkdialog/linkdialogprocessorgraphicsitems.h>
#include <inviwo/qt/editor/linkdialog/linkdialogcurvegraphicsitems.h>

namespace inviwo {

LinkDialogPropertyGraphicsItem::LinkDialogPropertyGraphicsItem (
    LinkDialogProcessorGraphicsItem* processor, Property* prop,
    LinkDialogPropertyGraphicsItem* parentPropertyGraphicsItem, int subPropertyLevel)
    : GraphicsItemData<Property>()
    , parentPropertyGraphicsItem_(parentPropertyGraphicsItem)
    , subPropertyLevel_(subPropertyLevel)
    , isExpanded_(false)
    , index_(0)
    , animateEnabled_(false) {
    setZValue(LINKDIALOG_PROPERTY_GRAPHICSITEM_DEPTH);
    // setFlags(ItemIsMovable | ItemIsSelectable | ItemIsFocusable | ItemSendsGeometryChanges);
    int propWidth = propertyItemWidth - (subPropertyLevel_ * propertyExpandCollapseOffset);
    setRect(-propWidth / 2, -propertyItemHeight / 2, propWidth, propertyItemHeight);
    QGraphicsDropShadowEffect* processorShadowEffect = new QGraphicsDropShadowEffect();
    processorShadowEffect->setOffset(3.0);
    processorShadowEffect->setBlurRadius(3.0);
    setGraphicsEffect(processorShadowEffect);
    classLabel_ = new LabelGraphicsItem(this);
    classLabel_->setPos(-propWidth / 2.0 + propertyLabelHeight / 2.0,
                        -propertyItemHeight / 2.0 + propertyLabelHeight);
    classLabel_->setDefaultTextColor(Qt::black);
    classLabel_->setFont(QFont("Segoe", propertyLabelHeight, QFont::Black, false));
    classLabel_->setCrop(9, 8);
    typeLabel_ = new LabelGraphicsItem(this);
    typeLabel_->setPos(-propWidth / 2.0 + propertyLabelHeight / 2.0,
                       -propertyItemHeight / 2.0 + propertyLabelHeight * 2.5);
    typeLabel_->setDefaultTextColor(Qt::black);
    typeLabel_->setFont(QFont("Segoe", processorLabelHeight, QFont::Normal, true));
    typeLabel_->setCrop(9, 8);
    processorGraphicsItem_ = processor;
    setProperty(prop);

    CompositeProperty* compProp = IS_COMPOSITE_PROPERTY(prop);
    if (compProp) {
        // LogWarn("Found composite sub properties")
        std::vector<Property*> subProperties = compProp->getProperties();
        for (size_t j = 0; j < subProperties.size(); j++) {
            LinkDialogPropertyGraphicsItem* compItem = new LinkDialogPropertyGraphicsItem(
                processor, subProperties[j], this, subPropertyLevel_ + 1);
            compItem->hide();
            subPropertyGraphicsItems_.push_back(compItem);
        }
    }
}

LinkDialogPropertyGraphicsItem::~LinkDialogPropertyGraphicsItem() {}

void LinkDialogPropertyGraphicsItem::setIndex(int index) {
    index_ = index;
}

const int LinkDialogPropertyGraphicsItem::getIndex() const { 
    return index_;
}

void LinkDialogPropertyGraphicsItem::setAnimate(bool animate) {
    animateEnabled_ = animate;
}

const bool LinkDialogPropertyGraphicsItem::getAnimate() const { 
    return animateEnabled_;
}


void LinkDialogPropertyGraphicsItem::setPropertyItemIndex(int &currIndex) {
    setIndex(currIndex);
    currIndex++;
    if (isExpanded_) {
        for (size_t i=0; i<subPropertyGraphicsItems_.size(); i++)
            subPropertyGraphicsItems_[i]->setPropertyItemIndex(currIndex);
    }
    else {
        for (size_t i=0; i<subPropertyGraphicsItems_.size(); i++)
            subPropertyGraphicsItems_[i]->setIndex(index_);
    }
    
    /*
    //For debugging the indexing
    QString label(glm::to_string(index_).c_str());
    label+= QString::fromStdString(getGraphicsItemData()->getDisplayName());
    classLabel_->setText(label);
    */
    
}

void LinkDialogPropertyGraphicsItem::updatePositionBasedOnIndex(float animateExpansion) {
    if (!processorGraphicsItem_) return;

    QPointF tl = processorGraphicsItem_->rect().topLeft();
    QPointF br = processorGraphicsItem_->rect().bottomRight();
    QPointF processorMappedDim = processorGraphicsItem_->mapToParent(tl) - processorGraphicsItem_->mapToParent(br);

    tl = rect().topLeft();
    br = rect().bottomRight();
    QPointF propertyMappedDim = mapToParent(tl) -  mapToParent(br);

    qreal initialOffset = fabs(processorMappedDim.y());
    QPointF p = processorGraphicsItem_->pos();

    qreal px = p.x() + fabs((float)subPropertyLevel_*propertyExpandCollapseOffset/2);
    qreal py = p.y() + initialOffset + index_*fabs(propertyMappedDim.y());

    int propWidth = 0;
    int propHeight = propertyItemHeight;

    bool parentExpanded = (parentPropertyGraphicsItem_ && parentPropertyGraphicsItem_->isExpanded());
    if (parentExpanded) {
        //offsetting with respect to parent ( using current sub-property level)
        propWidth =  propertyItemWidth-(subPropertyLevel_*propertyExpandCollapseOffset);

        if (parentPropertyGraphicsItem_->getAnimate()) {
            px = p.x() + fabs((float)subPropertyLevel_*propertyExpandCollapseOffset/2)*animateExpansion;
            float diff = parentPropertyGraphicsItem_->getIndex()+(index_-parentPropertyGraphicsItem_->getIndex())*animateExpansion;
            py = p.y() + initialOffset + diff*fabs(propertyMappedDim.y());
        }
    }
    else {
        //no offsetting if parent is collapsed
        propWidth =  propertyItemWidth;
        px = p.x();
    }

    

    setRect(-propWidth/2, -propertyItemHeight/2, propWidth, propHeight);

    setPos(QPointF(px,py));
    

    //LogWarn("SubProperty Level is : " << subPropertyLevel_ << " Index " << index_ << " Mapped dim y" << propertyMappedDim.y() << " (" << px << "," << py << ")")
}

void LinkDialogPropertyGraphicsItem::expand(bool expandSubProperties) {
    if (!subPropertyGraphicsItems_.size()) return;
    isExpanded_ = true;
    if (!expandSubProperties) return;
    for (size_t i=0; i<subPropertyGraphicsItems_.size(); i++) {
        subPropertyGraphicsItems_[i]->expand(true);
        subPropertyGraphicsItems_[i]->show();
    }
}

void LinkDialogPropertyGraphicsItem::collapse(bool collapseSubProperties) {
    if (!subPropertyGraphicsItems_.size()) return;
    isExpanded_ = false;
    if (!collapseSubProperties) return;
    for (size_t i=0; i<subPropertyGraphicsItems_.size(); i++) {
        subPropertyGraphicsItems_[i]->collapse(true);
        subPropertyGraphicsItems_[i]->hide();
    }
}

bool LinkDialogPropertyGraphicsItem::hasSubProperties() {
    return (subPropertyGraphicsItems_.size()>0);
}

bool LinkDialogPropertyGraphicsItem::isExpanded() {
    return isExpanded_;
}


QSizeF LinkDialogPropertyGraphicsItem::sizeHint(Qt::SizeHint which, const QSizeF& constraint) const
{
    switch (which) {
        case Qt::MinimumSize:
        case Qt::MaximumSize:
        case Qt::PreferredSize:
            return rect().size() + QSize(12, 12);

        default:
            break;
    }

    return constraint;
}

void LinkDialogPropertyGraphicsItem::addConnectionGraphicsItem(DialogConnectionGraphicsItem* cItem) {
    connectionItems_.push_back(cItem);
}

size_t LinkDialogPropertyGraphicsItem::getConnectionGraphicsItemCount() const {
    return connectionItems_.size();
}

void LinkDialogPropertyGraphicsItem::removeConnectionGraphicsItem(DialogConnectionGraphicsItem* cItem) {
    connectionItems_.erase(std::remove(connectionItems_.begin(), connectionItems_.end(), cItem), connectionItems_.end());
}

DialogConnectionGraphicsItem* LinkDialogPropertyGraphicsItem::getArrowConnectionAt(const QPointF pos) const {
    QPointF itemPos = mapFromScene(pos);

    for (size_t i=0; i<connectionItems_.size(); i++) {
        QRectF arrowRect = calculateArrowRect(i+1, true);

        if (arrowRect.contains(itemPos))
            return connectionItems_[i];
    }

    for (size_t i=0; i<connectionItems_.size(); i++) {
        QRectF arrowRect = calculateArrowRect(i+1, false);

        if (arrowRect.contains(itemPos))
            return connectionItems_[i];
    }

    return 0;
}

QRectF LinkDialogPropertyGraphicsItem::calculateArrowRect(size_t curPort, bool computeRight) const {
    QPointF arrowDim(arrowDimensionWidth, arrowDimensionHeight);
    QPointF rectDim(0, rect().height()/(getConnectionGraphicsItemCount()+1));
    qreal x = rect().right()-arrowDim.x();

    if (!computeRight)
        x = rect().left();

    qreal y = rect().top()+(curPort*rectDim.y()) - arrowDim.y();
    return QRectF(x, y, arrowDim.x(), 2*arrowDim.y());
}

QRectF LinkDialogPropertyGraphicsItem::calculateArrowRect(DialogConnectionGraphicsItem* cItem, bool computeRight) const {
    for (size_t i=0; i<connectionItems_.size(); i++) {
        if (connectionItems_[i]==cItem)
            return calculateArrowRect(i+1, computeRight);
    }

    return QRectF();
}

bool LinkDialogPropertyGraphicsItem::isArrowPointedRight(DialogConnectionGraphicsItem* cItem) {
    QPointF c = pos();
    QPointF bl = rect().bottomLeft();
    QPointF br = rect().bottomRight();
    QPointF propertyMappedDim ;
    propertyMappedDim = mapToParent(br) -  mapToParent(bl);
    QPointF rightBoundaryCenter = c + (propertyMappedDim/2.0);
    propertyMappedDim = mapToParent(bl) -  mapToParent(br);
    QPointF leftBoundaryCenter = c + (propertyMappedDim/2.0);
    qreal dist1 = std::min(QVector2D(rightBoundaryCenter - cItem->getStartPoint()).length(),
                           QVector2D(rightBoundaryCenter - cItem->getEndPoint()).length());
    qreal dist2 = std::min(QVector2D(leftBoundaryCenter - cItem->getStartPoint()).length(),
                           QVector2D(leftBoundaryCenter - cItem->getEndPoint()).length());

    if (dist1<dist2) return false;

    return true;
}

void LinkDialogPropertyGraphicsItem::paint(QPainter* p, const QStyleOptionGraphicsItem* options, QWidget* widget) {
    IVW_UNUSED_PARAM(options);
    IVW_UNUSED_PARAM(widget);
    p->setPen(Qt::black);
    p->setRenderHint(QPainter::Antialiasing, true);
    p->save();
    // paint property
    QLinearGradient grad(rect().topLeft(), rect().bottomLeft());

    if (isSelected()) {
        QColor bgColor = Qt::blue;
        grad.setColorAt(0.0f, bgColor);
        grad.setColorAt(1.0f, bgColor);
    } else {        
        if (subPropertyLevel_) {
            QColor bgColor = Qt::darkGray;
            grad.setColorAt(0.0f, bgColor);
            grad.setColorAt(1.0f, bgColor);
        } else {
            QColor bgColor = Qt::lightGray;
            grad.setColorAt(0.0f, bgColor);
            grad.setColorAt(1.0f, bgColor);
        }
    }

    p->setBrush(grad);

    QPen blackPen(QColor(0, 0, 0), 3);
    QPen greyPen(QColor(96, 96, 96), 1);
    QRectF bRect = rect();
    
    QPainterPath roundRectPath;
    roundRectPath.moveTo(bRect.left(), bRect.top()+propertyRoundedCorners);
    roundRectPath.lineTo(bRect.left(), bRect.bottom()-propertyRoundedCorners);
    roundRectPath.arcTo(bRect.left(), bRect.bottom()-(2*propertyRoundedCorners), (2*propertyRoundedCorners), (2*propertyRoundedCorners), 180.0,
    90.0);
    roundRectPath.lineTo(bRect.right()-propertyRoundedCorners, bRect.bottom());
    roundRectPath.arcTo(bRect.right()-(2*propertyRoundedCorners), bRect.bottom()-(2*propertyRoundedCorners), (2*propertyRoundedCorners),
    (2*propertyRoundedCorners), 270.0, 90.0);
    roundRectPath.lineTo(bRect.right(), bRect.top()+propertyRoundedCorners);
    roundRectPath.arcTo(bRect.right()-(2*propertyRoundedCorners), bRect.top(), (2*propertyRoundedCorners), (2*propertyRoundedCorners), 0.0,
    90.0);
    roundRectPath.lineTo(bRect.left()+propertyRoundedCorners, bRect.top());
    roundRectPath.arcTo(bRect.left(), bRect.top(), (2*propertyRoundedCorners), (2*propertyRoundedCorners), 90.0, 90.0);
    p->drawPath(roundRectPath);
    
    QPainterPath roundRectPath_Top;
    QPainterPath roundRectPath_Left;
    QPainterPath roundRectPath_Bottom;
    QPainterPath roundRectPath_Right;   

    //Left
    p->setPen(blackPen);
    roundRectPath_Left.moveTo(bRect.left(), bRect.top());
    roundRectPath_Left.lineTo(bRect.left(), bRect.bottom());    
    p->drawPath(roundRectPath_Left);

    //Bottom
    p->setPen(blackPen);
    roundRectPath_Bottom.moveTo(bRect.left(), bRect.bottom());
    roundRectPath_Bottom.lineTo(bRect.right(), bRect.bottom());
    p->drawPath(roundRectPath_Bottom);

    //Right
    p->setPen(blackPen);
    roundRectPath_Right.moveTo(bRect.right(), bRect.bottom());
    roundRectPath_Right.lineTo(bRect.right(), bRect.top());
    p->drawPath(roundRectPath_Right);

    //Top
    p->setPen(blackPen);
    roundRectPath_Top.moveTo(bRect.left(), bRect.top());
    roundRectPath_Top.lineTo(bRect.right(), bRect.top());
    p->drawPath(roundRectPath_Top);
    
    p->restore();
    p->save();
    QPoint arrowDim(arrowDimensionWidth, arrowDimensionHeight);

    for (size_t i=0; i<connectionItems_.size(); i++) {
        //Determine if arrow need to be drawn pointing left or right side
        bool right = isArrowPointedRight(connectionItems_[i]);
        //If arrow points right, then get the rectangle aligned to the left-
        //boundary of property item (rectangle) and vice versa
        QRectF arrowRect = calculateArrowRect(connectionItems_[i], !right);
        QPainterPath rectPath;

        //determine color of start and end arrow
        if (connectionItems_[i]->getEndProperty()==this) {
            arrowRect = calculateArrowRect(connectionItems_[i]->getEndArrowHeadIndex(), !right);
            p->setPen(Qt::black);
            p->setBrush(Qt::green);
        }
        else if (connectionItems_[i]->getStartProperty()==this) {
            arrowRect = calculateArrowRect(connectionItems_[i]->getStartArrowHeadIndex(), !right);
            p->setPen(Qt::transparent);
            p->setBrush(Qt::transparent);
        }

        if (connectionItems_[i]->isBidirectional()) {
            p->setPen(Qt::black);
            p->setBrush(Qt::green);
        }

        if (right) {
            rectPath.moveTo(arrowRect.left(), arrowRect.top());
            rectPath.lineTo(arrowRect.left(), arrowRect.bottom());
            rectPath.lineTo(arrowRect.right(), arrowRect.bottom()-arrowRect.height()/2);
            rectPath.closeSubpath();
        } else {
            rectPath.moveTo(arrowRect.right(), arrowRect.top());
            rectPath.lineTo(arrowRect.right(), arrowRect.bottom());
            rectPath.lineTo(arrowRect.left(), arrowRect.bottom()-arrowRect.height()/2);
            rectPath.closeSubpath();
        }

        p->drawPath(rectPath);
    }

    p->restore();

}

QVariant LinkDialogPropertyGraphicsItem::itemChange(GraphicsItemChange change, const QVariant& value) {
    return QGraphicsItem::itemChange(change, value);
}

void LinkDialogPropertyGraphicsItem::setProperty(Property* prop) {
    setGraphicsItemData(prop);

    if (prop) {
        std::string className = prop->getClassIdentifier();
        className = removeSubString(className, "Property");
        QString label = QString::fromStdString(prop->getDisplayName());
        classLabel_->setText(label);
        typeLabel_->setText(QString::fromStdString(className));
    } else {
        classLabel_->setText("");
        typeLabel_->setText("");
    }
}

QPointF LinkDialogPropertyGraphicsItem::getShortestBoundaryPointTo(LinkDialogPropertyGraphicsItem* inProperty) {
    return getShortestBoundaryPointTo(inProperty->pos());
}

QPointF LinkDialogPropertyGraphicsItem::getShortestBoundaryPointTo(QPointF inPos) {
    QPointF c = pos();
    QPointF bl = rect().bottomLeft();
    QPointF br = rect().bottomRight();
    QPointF propertyMappedDim ;
    propertyMappedDim = mapToParent(br) -  mapToParent(bl);
    QPointF rightBoundaryCenter1 = c + (propertyMappedDim/2.0);
    propertyMappedDim = mapToParent(bl) -  mapToParent(br);
    QPointF leftBoundaryCenter1 = c + (propertyMappedDim/2.0);
    QVector2D vec1(leftBoundaryCenter1 - inPos);
    QVector2D vec2(rightBoundaryCenter1 - inPos);

    if (vec1.length()>vec2.length())
        return rightBoundaryCenter1;

    return leftBoundaryCenter1;
}

QPointF LinkDialogPropertyGraphicsItem::calculateArrowCenter(size_t curPort, bool computeRight) const {
    size_t arrowCount = getConnectionGraphicsItemCount();
    QPointF o = pos();

    if (computeRight) {
        QPointF br = o + QPointF(rect().width()/2, rect().height()/2);
        QPointF tr = o + QPointF(rect().width()/2, -rect().height()/2);
        QPointF vec(br - tr);
        vec = vec/(arrowCount+1) ;

        if (arrowCount==0) vec = vec/2;

        vec*=curPort;
        return tr+vec;
    } else {
        QPointF bl = o + QPointF(-rect().width()/2, rect().height()/2);
        QPointF tl = o + QPointF(-rect().width()/2, -rect().height()/2);
        QPointF vec(bl - tl);
        vec = vec/(arrowCount+1) ;

        if (arrowCount==0) vec = vec/2;

        vec*=curPort;
        return tl+vec;
    }
}

const std::vector<DialogConnectionGraphicsItem*>& LinkDialogPropertyGraphicsItem::getConnectionGraphicsItems() const {
    return connectionItems_;
}

std::vector<LinkDialogPropertyGraphicsItem*> LinkDialogPropertyGraphicsItem::getSubPropertyItemList(bool recursive) const {
    if (!recursive)
        return subPropertyGraphicsItems_;

    std::vector<LinkDialogPropertyGraphicsItem*> subProps;

    for (size_t i=0; i<subPropertyGraphicsItems_.size(); i++) {
        subProps.push_back(subPropertyGraphicsItems_[i]);
        std::vector<LinkDialogPropertyGraphicsItem*> props = subPropertyGraphicsItems_[i]->getSubPropertyItemList(recursive);
        for (size_t j=0; j<props.size(); j++)
            subProps.push_back(props[j]);
    }

    return subProps;
}

} //namespace