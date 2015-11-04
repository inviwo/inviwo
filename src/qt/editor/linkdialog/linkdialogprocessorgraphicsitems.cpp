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
#include <inviwo/qt/editor/linkdialog/linkdialogscene.h>
#include <inviwo/core/links/linkconditions.h>
#include <inviwo/core/network/processornetwork.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QTimeLine>
#include <warn/pop>



namespace inviwo {

LinkDialogProcessorGraphicsItem::LinkDialogProcessorGraphicsItem(Side side, Processor* processor)
    : GraphicsItemData<Processor>(side, processor), animateExpansion_(1.0) {
    setZValue(linkdialog::processorDepth);
    setFlags(ItemSendsGeometryChanges);

    setRect(-linkdialog::processorWidth / 2, -linkdialog::processorHeight / 2,
            linkdialog::processorWidth, linkdialog::processorHeight);

    auto identifier = new LabelGraphicsItem(this);
    identifier->setPos(rect().topLeft() + QPointF(linkdialog::offset, linkdialog::offset));
    identifier->setDefaultTextColor(Qt::white);
    auto idFont = QFont("Segoe", linkdialog::processorLabelHeight, QFont::Bold, false);
    idFont.setPixelSize(linkdialog::processorLabelHeight);
    identifier->setFont(idFont);
    identifier->setCrop(20, 19);

    auto classIdentifier = new LabelGraphicsItem(this);
    classIdentifier->setDefaultTextColor(Qt::lightGray);
    auto classFont = QFont("Segoe", linkdialog::processorLabelHeight, QFont::Normal, true);
    classFont.setPixelSize(linkdialog::processorLabelHeight);
    classIdentifier->setFont(classFont);
    classIdentifier->setCrop(20, 19);
    auto offset = classIdentifier->boundingRect().height();
    classIdentifier->setPos(rect().bottomLeft() +
                            QPointF(linkdialog::offset, -linkdialog::offset - offset));

    identifier->setText(QString::fromStdString(processor->getIdentifier()));
    classIdentifier->setText(QString::fromStdString(processor->getClassIdentifier()));

    QPointF newPos(0.0f, rect().height());
    for (auto& property : processor->getProperties()) {
        auto item = new LinkDialogPropertyGraphicsItem(this, property);
        properties_.push_back(item);
        item->setParentItem(this);
        item->setPos(newPos);
        size_t count = 1 + item->getTotalVisibleChildCount();
        newPos += QPointF(0, count * linkdialog::propertyHeight);
        item->show();
    }
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

int LinkDialogProcessorGraphicsItem::getLevel() const { return -1; }

void LinkDialogProcessorGraphicsItem::updatePositions() {
    QPointF newPos(0.0f, rect().height());
    for (auto property : properties_) {
        property->setPos(newPos);
        size_t count = 1 + property->getTotalVisibleChildCount();
        newPos += QPointF(0, count * linkdialog::propertyHeight);
    }
}

void LinkDialogProcessorGraphicsItem::paint(QPainter* p, const QStyleOptionGraphicsItem* options,
                                            QWidget* widget) {
    IVW_UNUSED_PARAM(options);
    IVW_UNUSED_PARAM(widget);
    p->save();

    QPen blackPen(QColor(0, 0, 0), 1);

    p->setPen(blackPen);
    p->setRenderHint(QPainter::Antialiasing, true);
    p->setViewTransformEnabled(false);
    QColor topColor(140, 140, 140);
    QColor middleColor(59, 61, 61);
    QColor bottomColor(40, 40, 40);
    // paint processor
    QLinearGradient grad(rect().topLeft(), rect().bottomLeft());

    grad.setColorAt(0.0f, topColor);
    grad.setColorAt(0.2f, middleColor);
    grad.setColorAt(1.0f, bottomColor);

    p->setBrush(grad);
    QPainterPath roundRectPath;
    QRectF bRect = rect();
    roundRectPath.moveTo(bRect.left(), bRect.top() + linkdialog::processorRoundedCorners);
    roundRectPath.lineTo(bRect.left(), bRect.bottom() - linkdialog::processorRoundedCorners);
    roundRectPath.arcTo(bRect.left(), bRect.bottom() - (2 * linkdialog::processorRoundedCorners),
                        (2 * linkdialog::processorRoundedCorners),
                        (2 * linkdialog::processorRoundedCorners), 180.0, 90.0);
    roundRectPath.lineTo(bRect.right() - linkdialog::processorRoundedCorners, bRect.bottom());
    roundRectPath.arcTo(bRect.right() - (2 * linkdialog::processorRoundedCorners),
                        bRect.bottom() - (2 * linkdialog::processorRoundedCorners),
                        (2 * linkdialog::processorRoundedCorners),
                        (2 * linkdialog::processorRoundedCorners), 270.0, 90.0);
    roundRectPath.lineTo(bRect.right(), bRect.top() + linkdialog::processorRoundedCorners);
    roundRectPath.arcTo(bRect.right() - (2 * linkdialog::processorRoundedCorners), bRect.top(),
                        (2 * linkdialog::processorRoundedCorners),
                        (2 * linkdialog::processorRoundedCorners), 0.0, 90.0);
    roundRectPath.lineTo(bRect.left() + linkdialog::processorRoundedCorners, bRect.top());
    roundRectPath.arcTo(bRect.left(), bRect.top(), (2 * linkdialog::processorRoundedCorners),
                        (2 * linkdialog::processorRoundedCorners), 90.0, 90.0);
    p->drawPath(roundRectPath);
    p->restore();
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
        animateExpansion_ += static_cast<float>(incr);
    }
}

void LinkDialogProcessorGraphicsItem::animationEnd() {
    animateExpansion_ = 1.0f;
    delete sender();
    for (auto& elem : properties_) elem->setAnimate(false);
}

}  // namespace