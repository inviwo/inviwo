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

#include <inviwo/qt/editor/linkdialog/linkdialogprocessorgraphicsitems.h>
#include <inviwo/qt/editor/linkdialog/linkdialogpropertygraphicsitems.h>
#include <inviwo/qt/editor/connectiongraphicsitem.h>
#include <modules/qtwidgets/labelgraphicsitem.h>
#include <modules/qtwidgets/inviwoqtutils.h>
#include <inviwo/core/processors/processor.h>

namespace inviwo {

LinkDialogProcessorGraphicsItem::LinkDialogProcessorGraphicsItem(Side side, Processor* processor)
    : GraphicsItemData<Processor>(nullptr, side, processor) {
    setZValue(linkdialog::processorDepth);
    setFlags(ItemSendsGeometryChanges);

    setRect(-linkdialog::processorWidth / 2, -linkdialog::processorHeight / 2,
            linkdialog::processorWidth, linkdialog::processorHeight);

    auto name = new LabelGraphicsItem(this);
    name->setPos(rect().topLeft() + QPointF(linkdialog::offset, linkdialog::offset));
    name->setDefaultTextColor(Qt::white);
    auto idFont = QFont("Segoe", linkdialog::processorLabelHeight, QFont::Bold, false);
    idFont.setPixelSize(linkdialog::processorLabelHeight);
    name->setFont(idFont);
    name->setCrop(static_cast<int>(rect().width() - 2.0 * linkdialog::offset));

    auto id = new LabelGraphicsItem(this);
    id->setDefaultTextColor(Qt::lightGray);
    auto classFont = QFont("Segoe", linkdialog::processorLabelHeight, QFont::Normal, true);
    classFont.setPixelSize(linkdialog::processorLabelHeight);
    id->setFont(classFont);
    id->setCrop(static_cast<int>(rect().width() - 2.0 * linkdialog::offset));
    auto offset = id->boundingRect().height();
    id->setPos(rect().bottomLeft() + QPointF(linkdialog::offset, -linkdialog::offset - offset));

    name->setText(utilqt::toQString(processor->getDisplayName()));
    id->setText(utilqt::toQString(processor->getIdentifier()));

    for (auto& property : processor->getProperties()) {
        auto item = new LinkDialogPropertyGraphicsItem(this, property);
        properties_.push_back(item);
        item->hide();
        item->setParentItem(this);
    }

    LinkDialogTreeItem* prev = this;
    std::function<void(LinkDialogPropertyGraphicsItem*)> connect =
        [&connect, &prev](LinkDialogPropertyGraphicsItem* item) {
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

void LinkDialogProcessorGraphicsItem::paint(QPainter* p, const QStyleOptionGraphicsItem*,
                                            QWidget*) {
    p->save();

    QPen blackPen(QColor(0, 0, 0), 1);
    p->setPen(blackPen);
    p->setRenderHint(QPainter::Antialiasing, true);
    p->setViewTransformEnabled(false);

    p->setBrush(QColor(59, 61, 61));
    p->drawRoundedRect(rect(), linkdialog::processorRoundedCorners,
                       linkdialog::processorRoundedCorners);
    p->restore();
}

}  // namespace inviwo
