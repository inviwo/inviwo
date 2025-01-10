/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2024-2025 Inviwo Foundation
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

#include <inviwo/qt/editor/processorerroritem.h>

#include <inviwo/qt/editor/editorgrapicsitem.h>
#include <inviwo/qt/editor/processorgraphicsitem.h>

#include <modules/qtwidgets/inviwoqtutils.h>

#include <QFontMetricsF>
#include <QGraphicsSimpleTextItem>
#include <QGraphicsLineItem>
#include <QPainterPathStroker>
#include <QBrush>
#include <QPen>
#include <QPainter>
#include <QRectF>

namespace inviwo {

namespace {

struct ErrorLine : QGraphicsLineItem {
    using QGraphicsLineItem::QGraphicsLineItem;

    void paint(QPainter* p, const QStyleOptionGraphicsItem*, QWidget*) override {
        auto stroker = QPainterPathStroker{};
        stroker.setWidth(2.5);
        stroker.setCapStyle(Qt::RoundCap);

        p->save();

        QPainterPath path{line().p1()};
        path.lineTo(line().p2());

        p->setBrush(QColor("#7a191b"));
        p->setPen(QPen(Qt::black, 0.5, Qt::SolidLine, Qt::RoundCap));
        p->drawPath(stroker.createStroke(path));

        p->restore();
    }
};

}  // namespace

ProcessorErrorItem::ProcessorErrorItem(QGraphicsItem* parent)
    : QGraphicsRectItem(parent)
    , text_{new QGraphicsSimpleTextItem(this)}
    , line_{new ErrorLine(this)}
    , hasError_{false}
    , active_{false}
    , pressing_{false} {

    setPos(ProcessorErrorItem::offset);

    text_->setFont(QFont("Segoe", 12, QFont::Normal, false));
    text_->setBrush(QBrush{Qt::white});
    hide();

    setFlags(ItemIsMovable | ItemIsSelectable | ItemIsFocusable | ItemSendsGeometryChanges);
    setZValue(depth::processorError);
    line_->setPen(QPen(Qt::black, 0.5, Qt::SolidLine, Qt::RoundCap));

    if (auto* p = qgraphicsitem_cast<ProcessorGraphicsItem*>(parentItem())) {
        setPos(p->rect().topRight() + ProcessorErrorItem::offset);
        const auto dest = mapFromParent(p->rect().topRight());
        line_->setLine(QLineF{QPointF{0.0, 0.0}, dest});
    }
}

void ProcessorErrorItem::paint(QPainter* p, const QStyleOptionGraphicsItem*, QWidget*) {
    const double roundedCorners = 4.5;
    p->save();
    p->setRenderHint(QPainter::Antialiasing, true);
    QColor borderColor("#282828");
    p->setBrush(QColor("#7a191b"));
    p->setPen(QPen(QBrush(borderColor), 2.0));
    p->drawRoundedRect(rect(), roundedCorners, roundedCorners);
    p->restore();
}

void ProcessorErrorItem::setText(std::string_view error) {
    QFontMetricsF fm{text_->font()};
    const auto rect = fm.boundingRect(QRect{}, 0, utilqt::toQString(error));
    setRect(0.0, -rect.height() - 10.0, rect.width() + 10.0, rect.height() + 10.0);
    text_->setPos(5.0, -rect.height() - 5.0);
    text_->setText(utilqt::toQString(error));
    hasError_ = true;
}
QString ProcessorErrorItem::text() const { return text_->text(); }

void ProcessorErrorItem::clear() {
    hasError_ = false;
    hide();
}
void ProcessorErrorItem::setActive(bool active) {
    active_ = active;
    if (!pressing_ && !active && !isSelected()) {
        hide();
    } else if (active) {
        setVisible(hasError_);
    }
}

void ProcessorErrorItem::mousePressEvent(QGraphicsSceneMouseEvent* e) {
    pressing_ = true;
    QGraphicsItem::mousePressEvent(e);
    pressing_ = false;
}

QVariant ProcessorErrorItem::itemChange(GraphicsItemChange change, const QVariant& value) {
    if (change == QGraphicsItem::ItemSelectedHasChanged) {
        if (!pressing_ && !active_ && !isSelected()) {
            hide();
        }
    } else if (change == QGraphicsItem::ItemPositionHasChanged) {
        if (auto* p = qgraphicsitem_cast<ProcessorGraphicsItem*>(parentItem())) {
            const auto dest = mapFromParent(p->rect().topRight());
            line_->setLine(QLineF{QPointF{0.0, 0.0}, dest});
        }
    }
    return QGraphicsItem::itemChange(change, value);
}

}  // namespace inviwo
