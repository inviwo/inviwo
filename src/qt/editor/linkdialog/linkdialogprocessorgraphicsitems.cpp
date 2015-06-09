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

#include <inviwo/qt/editor/linkdialog/linkdialogprocessorgraphicsitems.h>
#include <inviwo/qt/editor/linkdialog/linkdialogpropertygraphicsitems.h>
#include <inviwo/qt/editor/linkdialog/linkdialogcurvegraphicsitems.h>

#include <QGraphicsDropShadowEffect>
#include <QTimeLine>

#include <inviwo/core/links/linkconditions.h>
#include <inviwo/core/network/processornetwork.h>

namespace inviwo {

LinkDialogProcessorGraphicsItem::LinkDialogProcessorGraphicsItem()
    : GraphicsItemData<Processor>(), animateExpansion_(1.0) {
    setZValue(LINKDIALOG_PROCESSOR_GRAPHICSITEM_DEPTH);
    // setFlags(ItemIsMovable | ItemIsSelectable | ItemIsFocusable | ItemSendsGeometryChanges);
    setRect(-processorItemWidth / 2, -processorItemHeight / 2, processorItemWidth,
            processorItemHeight);
    QGraphicsDropShadowEffect* processorShadowEffect = new QGraphicsDropShadowEffect();
    processorShadowEffect->setOffset(3.0);
    processorShadowEffect->setBlurRadius(3.0);
    setGraphicsEffect(processorShadowEffect);
    nameLabel_ = new LabelGraphicsItem(this);
    nameLabel_->setPos(-processorItemWidth / 2.0 + processorLabelHeight / 2.0,
                       -processorItemHeight / 2.0 + processorLabelHeight);
    nameLabel_->setDefaultTextColor(Qt::white);
    nameLabel_->setFont(QFont("Segoe", processorLabelHeight, QFont::Black, false));
    nameLabel_->setCrop(9, 8);
    classLabel_ = new LabelGraphicsItem(this);
    classLabel_->setPos(-processorItemWidth / 2.0 + processorLabelHeight / 2.0,
                        -processorItemHeight / 2.0 + processorLabelHeight * 2.5);
    classLabel_->setDefaultTextColor(Qt::lightGray);
    classLabel_->setFont(QFont("Segoe", processorLabelHeight, QFont::Normal, true));
    classLabel_->setCrop(9, 8);
}

LinkDialogProcessorGraphicsItem::~LinkDialogProcessorGraphicsItem() {}

QSizeF LinkDialogProcessorGraphicsItem::sizeHint(Qt::SizeHint which,
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

void LinkDialogProcessorGraphicsItem::paint(QPainter* p, const QStyleOptionGraphicsItem* options,
                                            QWidget* widget) {
    IVW_UNUSED_PARAM(options);
    IVW_UNUSED_PARAM(widget);
    p->save();
    p->setPen(Qt::black);
    p->setRenderHint(QPainter::Antialiasing, true);
    QColor topColor(140, 140, 140);
    QColor middleColor(59, 61, 61);
    QColor bottomColor(40, 40, 40);
    // paint processor
    QLinearGradient grad(rect().topLeft(), rect().bottomLeft());

    if (isSelected()) {
        grad.setColorAt(0.0f, topColor);
        grad.setColorAt(0.2f, middleColor);
        grad.setColorAt(0.5f, Qt::darkRed);
        grad.setColorAt(1.0f, bottomColor);
    } else {
        grad.setColorAt(0.0f, topColor);
        grad.setColorAt(0.2f, middleColor);
        grad.setColorAt(1.0f, bottomColor);
    }

    p->setBrush(grad);
    QPainterPath roundRectPath;
    QRectF bRect = rect();
    roundRectPath.moveTo(bRect.left(), bRect.top() + processorRoundedCorners);
    roundRectPath.lineTo(bRect.left(), bRect.bottom() - processorRoundedCorners);
    roundRectPath.arcTo(bRect.left(), bRect.bottom() - (2 * processorRoundedCorners),
                        (2 * processorRoundedCorners), (2 * processorRoundedCorners), 180.0, 90.0);
    roundRectPath.lineTo(bRect.right() - processorRoundedCorners, bRect.bottom());
    roundRectPath.arcTo(bRect.right() - (2 * processorRoundedCorners),
                        bRect.bottom() - (2 * processorRoundedCorners),
                        (2 * processorRoundedCorners), (2 * processorRoundedCorners), 270.0, 90.0);
    roundRectPath.lineTo(bRect.right(), bRect.top() + processorRoundedCorners);
    roundRectPath.arcTo(bRect.right() - (2 * processorRoundedCorners), bRect.top(),
                        (2 * processorRoundedCorners), (2 * processorRoundedCorners), 0.0, 90.0);
    roundRectPath.lineTo(bRect.left() + processorRoundedCorners, bRect.top());
    roundRectPath.arcTo(bRect.left(), bRect.top(), (2 * processorRoundedCorners),
                        (2 * processorRoundedCorners), 90.0, 90.0);
    p->drawPath(roundRectPath);
    p->restore();
}

QVariant LinkDialogProcessorGraphicsItem::itemChange(GraphicsItemChange change,
                                                     const QVariant& value) {
    return QGraphicsItem::itemChange(change, value);
}

void LinkDialogProcessorGraphicsItem::setProcessor(Processor* processor, bool expandProperties) {
    setGraphicsItemData(processor);

    if (processor) {
        nameLabel_->setText(QString::fromStdString(processor->getIdentifier()));
        classLabel_->setText(QString::fromStdString(processor->getClassIdentifier()));
        propertyGraphicsItems_.clear();
        std::vector<Property*> properties = processor->getProperties();

        for (auto& propertie : properties) {
            LinkDialogPropertyGraphicsItem* compItem =
                new LinkDialogPropertyGraphicsItem(this, propertie);
            compItem->show();
            propertyGraphicsItems_.push_back(compItem);
            std::vector<LinkDialogPropertyGraphicsItem*> subPropGraphicsItems =
                compItem->getSubPropertyItemList(true);
            for (auto& subPropGraphicsItem : subPropGraphicsItems) {
                propertyGraphicsItems_.push_back(subPropGraphicsItem);
            }
        }

        updatePropertyItemPositions();
    } else {
        nameLabel_->setText("");
        classLabel_->setText("");
    }
}

void LinkDialogProcessorGraphicsItem::updatePropertyItemPositions(bool animateExpansion) {
    int globalIndex = 0;
    for (auto& elem : propertyGraphicsItems_) {
        if (elem->getLevel() == 0) elem->setPropertyItemIndex(globalIndex);
    }

    if (animateExpansion) {
        animationStart();
    } else {
        animateExpansion_ = 1.0f;
        updateAll();
    }
}

void LinkDialogProcessorGraphicsItem::animationStart() {
    animateExpansion_ = 0.1f;

    QTimeLine* anim = new QTimeLine(50, this);
    anim->setUpdateInterval(20);
    connect(anim, SIGNAL(valueChanged(qreal)), SLOT(animate(qreal)));
    connect(anim, SIGNAL(finished()), SLOT(animationEnd()));
    anim->start();
}

void LinkDialogProcessorGraphicsItem::animate(qreal incr) {
    if (animateExpansion_ > 1.0 || animateExpansion_ < 0.0f) {
        animationEnd();
    } else {
        animateExpansion_ += incr;
        updateAll();
    }
}

void LinkDialogProcessorGraphicsItem::updateAll() {
    for (auto& elem : propertyGraphicsItems_) elem->updatePositionBasedOnIndex(animateExpansion_);

    for (auto connections : propertyGraphicsItems_) {
        for (auto& connection : connections->getConnectionGraphicsItems())
            connection->updateConnectionDrawing();
    }
}

void LinkDialogProcessorGraphicsItem::animationEnd() {
    animateExpansion_ = 1.0f;
    delete sender();
    updateAll();
    for (auto& elem : propertyGraphicsItems_) elem->setAnimate(false);
}

}  // namespace