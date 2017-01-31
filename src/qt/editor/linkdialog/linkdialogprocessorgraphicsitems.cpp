/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2017 Inviwo Foundation
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
#include <modules/qtwidgets/inviwoqtutils.h>
#include <inviwo/core/links/linkconditions.h>
#include <inviwo/core/network/processornetwork.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QTimeLine>
#include <warn/pop>



namespace inviwo {

LinkDialogProcessorGraphicsItem::LinkDialogProcessorGraphicsItem(Side side, Processor* processor)
    : GraphicsItemData<Processor>(nullptr, side, processor) {
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
    identifier->setCrop(rect().width() - 2.0 * linkdialog::offset);

    auto classIdentifier = new LabelGraphicsItem(this);
    classIdentifier->setDefaultTextColor(Qt::lightGray);
    auto classFont = QFont("Segoe", linkdialog::processorLabelHeight, QFont::Normal, true);
    classFont.setPixelSize(linkdialog::processorLabelHeight);
    classIdentifier->setFont(classFont);
    classIdentifier->setCrop(rect().width() - 2.0 * linkdialog::offset);
    auto offset = classIdentifier->boundingRect().height();
    classIdentifier->setPos(rect().bottomLeft() +
                            QPointF(linkdialog::offset, -linkdialog::offset - offset));

    identifier->setText(QString::fromStdString(processor->getIdentifier()));
    classIdentifier->setText(QString::fromStdString(processor->getClassIdentifier()));

    for (auto& property : processor->getProperties()) {
        auto item = new LinkDialogPropertyGraphicsItem(this, property);
        properties_.push_back(item);
        item->hide();
        item->setParentItem(this);
    }
    
    
    LinkDialogTreeItem* prev = this;
    std::function<void(LinkDialogPropertyGraphicsItem*)> connect = [this, &connect, &prev](
        LinkDialogPropertyGraphicsItem* item) {
        prev->setNext(item);
        item->setPrev(prev);
        prev = item;
        for (auto i : item->getSubPropertyItemList()) {
            connect(i);
        }
    };
    for (auto item : properties_) connect(item);

    LinkDialogTreeItem* item = this;
    while (item) {
        item->updatePositions();
        item = item->next();
    }
}

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

void LinkDialogProcessorGraphicsItem::updatePositions() {}

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
    p->drawRoundedRect(rect(), linkdialog::processorRoundedCorners,
                       linkdialog::processorRoundedCorners);
    p->restore();
}


}  // namespace
