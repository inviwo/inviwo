/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2024 Inviwo Foundation
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

#include <modules/qtwidgets/tf/tfeditorcontrolpoint.h>

#include <inviwo/core/datastructures/tfprimitive.h>         // for TFPrimitive, operator==
#include <inviwo/core/util/glmvec.h>                        // for dvec2
#include <modules/qtwidgets/tf/tfcontrolpointconnection.h>  // for TFControlPointConnection
#include <modules/qtwidgets/tf/tfeditor.h>                  // for TFEditor
#include <modules/qtwidgets/tf/tfeditorprimitive.h>         // for TFEditorPrimitive
#include <modules/qtwidgets/inviwoqtutils.h>

#include <QGraphicsScene>                // for QGraphicsScene
#include <QObject>                       // for qobject_cast
#include <QPainter>                      // for QPainter
#include <glm/ext/scalar_constants.hpp>  // for epsilon

class QPointF;

namespace inviwo {

TFEditorControlPoint::TFEditorControlPoint(TFPrimitive& primitive) : TFEditorPrimitive(primitive) {
    setZValue(tfZLevel);
}

QRectF TFEditorControlPoint::boundingRect() const {
    double bBoxSize = getSize() + 5.0;  //<! consider size of pen
    auto bRect = QRectF(-bBoxSize / 2.0, -bBoxSize / 2.0, bBoxSize, bBoxSize);

    return bRect;
}

QPainterPath TFEditorControlPoint::shape() const {
    QPainterPath path;
    const auto radius = getSize() * 0.5 + 1.5;  //<! consider size of pen
    path.addEllipse(QPointF(0.0, 0.0), radius, radius);
    return path;
}

void TFEditorControlPoint::paint(QPainter* painter,
                                 [[maybe_unused]] const QStyleOptionGraphicsItem* options,
                                 [[maybe_unused]] QWidget* widget) {

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

    const auto radius = getSize() * 0.5;
    painter->drawEllipse(QPointF(0.0, 0.0), radius, radius);
}

TFMoveMode TFEditorControlPoint::moveMode() const {
    if (auto tfe = qobject_cast<TFEditor*>(scene())) {
        return tfe->getMoveMode();
    }
    return TFMoveMode::Free;
}

QVariant TFEditorControlPoint::itemChange(GraphicsItemChange change, const QVariant& value) {
    if ((change == QGraphicsItem::ItemPositionChange) && scene()) {
        // constrain positions to valid view positions
        const auto sceneRect = scene()->sceneRect();
        auto newPos = utilqt::clamp(constrainPosToXorY(value.toPointF()), sceneRect);

        const double d = 2.0 * sceneRect.width() * glm::epsilon<float>();
        if (moveMode() == TFMoveMode::Restrict) {
            if (left_ && left_->left && left_->left->scenePos().x() > newPos.x()) {
                newPos.setX(left_->left->scenePos().x() + d);
            }
            if (right_ && right_->right && right_->right->scenePos().x() < newPos.x()) {
                newPos.setX(right_->right->scenePos().x() - d);
            }
        }
        return newPos;  // return the constrained position
    } else if (change == QGraphicsItem::ItemPositionHasChanged) {

        if (!isEditing_) {
            isEditing_ = true;
            // update the associated transfer function primitive
            data_.setPositionAlpha(dvec2{scenePos().x(), scenePos().y()});
            isEditing_ = false;
        }

        updateLabel();

        const double d = 2.0 * scene()->sceneRect().width() * glm::epsilon<float>();
        if (moveMode() == TFMoveMode::Free) {
            if ((left_ && left_->left && *(left_->left) > *this) ||
                (right_ && right_->right && *(right_->right) < *this)) {
                if (auto tfe = qobject_cast<TFEditor*>(scene())) {
                    tfe->updateConnections();
                    return {};
                }
            }
        } else if (moveMode() == TFMoveMode::Push) {
            if (left_ && left_->left && *(left_->left) > *this) {
                left_->left->setPos(QPointF(scenePos().x() - d, left_->left->scenePos().y()));
            }
            if (right_ && right_->right && *(right_->right) < *this) {
                right_->right->setPos(QPointF(scenePos().x() + d, right_->right->scenePos().y()));
            }
        }
        if (auto r = right()) {
            r->updateShape();
        }
        if (auto l = left()) {
            l->updateShape();
        }
        return {};
    }

    return TFEditorPrimitive::itemChange(change, value);
}

}  // namespace inviwo
