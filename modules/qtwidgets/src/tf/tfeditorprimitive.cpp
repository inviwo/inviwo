/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2024 Inviwo Foundation
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

#include <modules/qtwidgets/tf/tfeditorprimitive.h>

#include <inviwo/core/datastructures/datamapper.h>   // for DataMapper
#include <inviwo/core/datastructures/tfprimitive.h>  // for TFPrimitive
#include <inviwo/core/datastructures/unitsystem.h>   // for Axis
#include <inviwo/core/util/glmvec.h>                 // for vec4, vec3, dvec2
#include <modules/qtwidgets/inviwoqtutils.h>         // for clamp, toQColor, toQString
#include <modules/qtwidgets/tf/tfeditor.h>           // for TFEditor
#include <modules/qtwidgets/tf/tfcontrolpointconnection.h>

#include <cmath>    // for abs
#include <cstdlib>  // for abs

#include <QBrush>                   // for QBrush
#include <QColor>                   // for QColor
#include <QFlags>                   // for QFlags, operator==
#include <QFont>                    // for QFont
#include <QGraphicsScene>           // for QGraphicsScene
#include <QGraphicsSimpleTextItem>  // for QGraphicsSimpleTextItem
#include <QGuiApplication>          // for QGuiApplication
#include <QObject>                  // for qobject_cast
#include <QPainter>                 // for QPainter, QPainter::Antialiasing
#include <QPen>                     // for QPen
#include <QPoint>                   // for operator!=, operator-
#include <QRectF>                   // for QRectF
#include <QString>                  // for QString
#include <Qt>                       // for ShiftModifier, RoundCap, SolidLine
#include <fmt/core.h>               // for basic_string_view, format
#include <glm/vec4.hpp>             // for operator==, vec<>::(anonymous)

class QGraphicsSceneMouseEvent;
class QStyleOptionGraphicsItem;
class QWidget;

namespace inviwo {

TFEditorPrimitive::TFEditorPrimitive(TFPrimitive& primitive)
    : data_(primitive)
    , isEditing_(false)
    , hovered_(false)
    , label_{nullptr}
    , cachedPosition_{} {

    setFlags(ItemIgnoresTransformations | ItemIsFocusable | ItemIsMovable | ItemIsSelectable |
             ItemSendsGeometryChanges);
    setAcceptHoverEvents(true);

    primitive.addObserver(this);
}

TFPrimitive& TFEditorPrimitive::getPrimitive() { return data_; }

const TFPrimitive& TFEditorPrimitive::getPrimitive() const { return data_; }

void TFEditorPrimitive::setPosition(double pos) { data_.setPosition(pos); }

double TFEditorPrimitive::getPosition() const { return data_.getPosition(); }

void TFEditorPrimitive::setColor(const vec4& color) { data_.setColor(color); }

void TFEditorPrimitive::setColor(const vec3& color) { data_.setColor(color); }

void TFEditorPrimitive::setAlpha(float alpha) { data_.setAlpha(alpha); }

const vec4& TFEditorPrimitive::getColor() const { return data_.getColor(); }

void TFEditorPrimitive::onTFPrimitiveChange(const TFPrimitive& p) {
    if (!isEditing_) {
        isEditing_ = true;
        const QPointF newPos(p.getPosition(), p.getAlpha());
        if (newPos != pos()) {
            setPos(newPos);
        }
        isEditing_ = false;
        update();
    }
}

double TFEditorPrimitive::getSize() const { return hovered_ ? 14.0 + 5.0 : 14.0; }

void TFEditorPrimitive::setHovered(bool hover) {
    prepareGeometryChange();
    hovered_ = hover;

    if (hovered_ && !label_) {
        // create label for annotating TF primitives
        label_ = std::make_unique<QGraphicsSimpleTextItem>(this);
        QFont font(label_->font());
        font.setPixelSize(14);
        label_->setFont(font);
    }

    if (label_) {
        label_->setVisible(hover);
        updateLabel();
    }

    update();
}

void TFEditorPrimitive::beginMouseDrag() {
    cachedPosition_ = scenePos();
}


QPointF TFEditorPrimitive::constrainPosToXorY(QPointF pos) const {
    const bool shiftPressed =
        ((QGuiApplication::queryKeyboardModifiers() & Qt::ShiftModifier) == Qt::ShiftModifier);
    // restrict movement to either horizontal or vertical direction while shift is pressed
    if (shiftPressed) {
        // adjust position of mouse event
        auto delta = pos - cachedPosition_;
        if (std::abs(delta.x()) > std::abs(delta.y())) {
            // horizontal movement is dominating
            pos.setY(cachedPosition_.y());
        } else {
            // vertical movement is dominating
            pos.setX(cachedPosition_.x());
        }
    }
    return pos;
}

QVariant TFEditorPrimitive::itemChange(GraphicsItemChange change, const QVariant& value) {
    // check for scene() here in order to avoid callbacks as long as item is not added to scene
    if ((change == QGraphicsItem::ItemPositionChange) && scene()) {
        // constrain positions to valid view positions
        return utilqt::clamp(constrainPosToXorY(value.toPointF()), scene()->sceneRect());
    } else if (change == QGraphicsItem::ItemPositionHasChanged) {
        if (!isEditing_) {
            isEditing_ = true;
            // update the associated transfer function primitive
            data_.setPositionAlpha(dvec2{scenePos().x(), scenePos().y()});
            isEditing_ = false;
        }

        updateLabel();

    } else if (change == QGraphicsItem::ItemSceneHasChanged) {
        setPos(QPointF(data_.getPosition(), data_.getAlpha()));
    }

    return QGraphicsItem::itemChange(change, value);
}

void TFEditorPrimitive::hoverEnterEvent(QGraphicsSceneHoverEvent*) {
    // ensure that this primitive is showing up on top for the duration of the hover
    setZValue(1000);
    setHovered(true);
}

void TFEditorPrimitive::hoverLeaveEvent(QGraphicsSceneHoverEvent*) {
    setHovered(false);
    setZValue(zLevel());
}

void TFEditorPrimitive::updateLabel() {
    if (!label_ || !label_->isVisible()) {
        return;
    }

    QString label;
    if (auto tfe = qobject_cast<TFEditor*>(scene())) {
        label = utilqt::toQString(
            fmt::format("{:0.3g}{: [} ({:0.3g}) / {:0.3g}",
                        tfe->getDataMapper().mapFromNormalizedToValue(getPosition()),
                        tfe->getDataMapper().valueAxis.unit, getPosition(), getColor().a));

    } else {
        label = QString("%1 / %2").arg(getPosition(), 0, 'g', 3).arg(getColor().a, 0, 'g', 3);
    }

    label_->setText(label);

    const double distFromCenter = getSize() * 0.7;
    QPointF pos(distFromCenter, distFromCenter);

    // adjust position based on quadrant the primitive is located in
    auto rect = label_->boundingRect();
    if (scenePos().x() > scene()->sceneRect().width() * 0.5) {
        pos.rx() = -rect.width() - distFromCenter;
    }
    if (scenePos().y() < scene()->sceneRect().height() * 0.5) {
        pos.ry() = -rect.height() - distFromCenter;
    }

    label_->setPos(pos);
}

bool operator==(const TFEditorPrimitive& lhs, const TFEditorPrimitive& rhs) {
    return (lhs.getPosition() == rhs.getPosition()) && (lhs.getColor() == rhs.getColor());
}

bool operator!=(const TFEditorPrimitive& lhs, const TFEditorPrimitive& rhs) {
    return !operator==(lhs, rhs);
}

bool operator<(const TFEditorPrimitive& lhs, const TFEditorPrimitive& rhs) {
    return lhs.getPosition() < rhs.getPosition();
}

bool operator>(const TFEditorPrimitive& lhs, const TFEditorPrimitive& rhs) { return rhs < lhs; }

bool operator<=(const TFEditorPrimitive& lhs, const TFEditorPrimitive& rhs) { return !(rhs < lhs); }

bool operator>=(const TFEditorPrimitive& lhs, const TFEditorPrimitive& rhs) { return !(lhs < rhs); }

}  // namespace inviwo
