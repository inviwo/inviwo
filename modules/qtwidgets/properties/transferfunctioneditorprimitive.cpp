/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018 Inviwo Foundation
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

#include <modules/qtwidgets/properties/transferfunctioneditorprimitive.h>
#include <modules/qtwidgets/properties/transferfunctioneditor.h>
#include <modules/qtwidgets/inviwoqtutils.h>
#include <inviwo/core/datastructures/datamapper.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QGraphicsScene>
#include <QPainter>
#include <QGraphicsSimpleTextItem>
#include <QFont>
#include <warn/pop>

namespace inviwo {

TransferFunctionEditorPrimitive::TransferFunctionEditorPrimitive(TFPrimitive* primitive,
                                                                 QGraphicsScene* scene,
                                                                 const vec2& pos, float size)
    : size_(size), isEditingPoint_(false), hovered_(false), data_(primitive) {
    setFlags(ItemIgnoresTransformations | ItemIsFocusable | ItemIsMovable | ItemIsSelectable |
             ItemSendsGeometryChanges);
    setZValue(10);
    setAcceptHoverEvents(true);

    // create label for annotating TF primitives
    tfPrimitiveLabel_ = util::make_unique<QGraphicsSimpleTextItem>(this);
    tfPrimitiveLabel_->setVisible(hovered_);
    QFont font(tfPrimitiveLabel_->font());
    font.setPixelSize(14);
    tfPrimitiveLabel_->setFont(font);

    // update position first, then add to scene to avoid calling the virtual
    // function onItemPositionChange()
    updatePosition(
        QPointF(pos.x * scene->sceneRect().width(), pos.y * scene->sceneRect().height()));

    if (scene) {
        scene->addItem(this);
    }
}

void TransferFunctionEditorPrimitive::setPrimitive(TFPrimitive* primitive) { data_ = primitive; }

TFPrimitive* TransferFunctionEditorPrimitive::getPrimitive() { return data_; }

void TransferFunctionEditorPrimitive::setPosition(float pos) { data_->setPosition(pos); }

float TransferFunctionEditorPrimitive::getPosition() const { return data_->getPosition(); }

void TransferFunctionEditorPrimitive::setColor(const vec4& color) { data_->setColor(color); }

void TransferFunctionEditorPrimitive::setColor(const vec3& color) { data_->setColor(color); }

void TransferFunctionEditorPrimitive::setAlpha(float alpha) { data_->setAlpha(alpha); }

const vec4& TransferFunctionEditorPrimitive::getColor() const { return data_->getColor(); }

void TransferFunctionEditorPrimitive::setTFPosition(const vec2& tfpos) {
    if (!isEditingPoint_) {
        isEditingPoint_ = true;
        QRectF rect = scene()->sceneRect();
        QPointF newpos(tfpos.x * rect.width(), tfpos.y * rect.height());
        if (newpos != pos()) updatePosition(newpos);
        isEditingPoint_ = false;
        update();
    }
}

const QPointF& TransferFunctionEditorPrimitive::getCurrentPos() const { return currentPos_; }

void TransferFunctionEditorPrimitive::setSize(float s) {
    prepareGeometryChange();
    size_ = s;
    update();
}

float TransferFunctionEditorPrimitive::getSize() const { return hovered_ ? size_ + 5.0f : size_; }

void TransferFunctionEditorPrimitive::setHovered(bool hover) {
    prepareGeometryChange();
    hovered_ = hover;

    tfPrimitiveLabel_->setVisible(hover);
    updateLabel();

    update();
}

void TransferFunctionEditorPrimitive::paint(QPainter* painter,
                                            const QStyleOptionGraphicsItem* options,
                                            QWidget* widget) {
    IVW_UNUSED_PARAM(options);
    IVW_UNUSED_PARAM(widget);
    painter->setRenderHint(QPainter::Antialiasing, true);

    // set up pen and brush for drawing the primitive
    QPen pen = QPen();
    pen.setCosmetic(true);
    pen.setCapStyle(Qt::RoundCap);
    pen.setStyle(Qt::SolidLine);
    pen.setWidthF(3.0);
    isSelected() ? pen.setColor(QColor(213, 79, 79)) : pen.setColor(QColor(66, 66, 66));
    QBrush brush = QBrush(utilqt::toQColor(vec4(vec3(getColor()), 1.0f)));

    painter->setPen(pen);
    painter->setBrush(brush);

    // call primitive drawing function
    paintPrimitive(painter);
}

QVariant TransferFunctionEditorPrimitive::itemChange(GraphicsItemChange change,
                                                     const QVariant& value) {
    // check for scene() here in order to avoid callbacks as long as item is not added to scene
    if ((change == QGraphicsItem::ItemPositionChange) && scene()) {
        // constrain positions to valid view positions
        auto newpos = value.toPointF();
        QRectF rect = scene()->sceneRect();

        if (!rect.contains(newpos)) {
            newpos.setX(qMin(rect.right(), qMax(newpos.x(), rect.left())));
            newpos.setY(qMin(rect.bottom(), qMax(newpos.y(), rect.top())));
        }

        // allow for adjusting the position in derived classes
        currentPos_ = prepareItemPositionChange(newpos);

        if (!isEditingPoint_) {
            isEditingPoint_ = true;

            // update the associated transfer function primitive
            onItemPositionChange(vec2(static_cast<float>(currentPos_.x() / rect.width()),
                                      static_cast<float>(currentPos_.y() / rect.height())));
            // update label
            updateLabel();

            isEditingPoint_ = false;
        }

        // return the constrained position
        return currentPos_;
    } else if (change == QGraphicsItem::ItemSceneHasChanged) {
        // inform primitive about scene change
        onItemSceneHasChanged();
    }

    return QGraphicsItem::itemChange(change, value);
}

void TransferFunctionEditorPrimitive::hoverEnterEvent(QGraphicsSceneHoverEvent*) {
    // ensure that this primitive is showing up on top for the duration of the hover
    setZValue(1000);
    setHovered(true);
}

void TransferFunctionEditorPrimitive::hoverLeaveEvent(QGraphicsSceneHoverEvent*) {
    setHovered(false);
    setZValue(10);
}

void TransferFunctionEditorPrimitive::updatePosition(const QPointF& pos) {
    currentPos_ = pos;
    QGraphicsItem::setPos(pos);
}

void TransferFunctionEditorPrimitive::updateLabel() {
    if (!tfPrimitiveLabel_->isVisible()) {
        return;
    }

    QString label;
    if (auto tfe = qobject_cast<TransferFunctionEditor*>(scene())) {
        label = QString("%1 (%2) / %3")
                    .arg(tfe->getDataMapper().mapFromNormalizedToValue(getPosition()))
                    .arg(getPosition(), 0, 'g', 3)
                    .arg(getColor().a, 0, 'g', 3);
    } else {
        label = QString("%1 / %2").arg(getPosition(), 0, 'g', 3).arg(getColor().a, 0, 'g', 3);
    }

    tfPrimitiveLabel_->setText(label);

    const double distFromCenter = size_ * 0.7;
    QPointF pos(distFromCenter, distFromCenter);

    // adjust position based on quadrant the primitive is located in
    auto rect = tfPrimitiveLabel_->boundingRect();
    if (currentPos_.x() > scene()->sceneRect().width() * 0.5) {
        pos.rx() = -rect.width() - distFromCenter;
    }
    if (currentPos_.y() < scene()->sceneRect().height() * 0.5) {
        pos.ry() = -rect.height() - distFromCenter;
    }

    tfPrimitiveLabel_->setPos(pos);
}

bool operator==(const TransferFunctionEditorPrimitive& lhs,
                const TransferFunctionEditorPrimitive& rhs) {
    return (lhs.getPosition() == rhs.getPosition()) && (lhs.getColor() == rhs.getColor());
}

bool operator!=(const TransferFunctionEditorPrimitive& lhs,
                const TransferFunctionEditorPrimitive& rhs) {
    return !operator==(lhs, rhs);
}

bool operator<(const TransferFunctionEditorPrimitive& lhs,
               const TransferFunctionEditorPrimitive& rhs) {
    return lhs.getPosition() < rhs.getPosition();
}

bool operator>(const TransferFunctionEditorPrimitive& lhs,
               const TransferFunctionEditorPrimitive& rhs) {
    return rhs < lhs;
}

bool operator<=(const TransferFunctionEditorPrimitive& lhs,
                const TransferFunctionEditorPrimitive& rhs) {
    return !(rhs < lhs);
}

bool operator>=(const TransferFunctionEditorPrimitive& lhs,
                const TransferFunctionEditorPrimitive& rhs) {
    return !(lhs < rhs);
}

}  // namespace inviwo
