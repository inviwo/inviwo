; /*********************************************************************************
   *
   * Inviwo - Interactive Visualization Workshop
   *
   * Copyright (c) 2024 Inviwo Foundation
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

#include <modules/qtwidgets/tf/tfeditormask.h>
#include <modules/qtwidgets/tf/tfpropertyconcept.h>
#include <modules/qtwidgets/inviwoqtutils.h>

#include <QPainterPath>
#include <QRectF>
#include <QPainter>
#include <QPen>
#include <QBrush>
#include <QGraphicsScene>
#include <QGraphicsView>

namespace inviwo {

TFEditorMask::TFEditorMask(TFPropertyConcept* c) : concept_(c), center_{0.5} {
    setFlags(ItemIgnoresTransformations | ItemIsFocusable | ItemIsMovable |
             ItemSendsGeometryChanges);

    setZValue(15.0);
    setAcceptHoverEvents(true);

    concept_->addObserver(this);
    for (auto* set : concept_->sets()) {
        set->addObserver(this);
    }
}

void TFEditorMask::onZoomVChange(const dvec2& zoomV) {
    center_ = 0.5 * (zoomV.x + zoomV.y);
    setPos(QPointF{pos().x(), center_});
}

QRectF TFEditorMask::boundingRect() const {
    auto bRect = QRectF(-width / 2.0, -width / 2.0, width, width);

    if (scene()->views().empty()) {
        return bRect;
    }
    auto* view = scene()->views().front();
    auto trans = deviceTransform(view->viewportTransform()).inverted();
    const auto topLeft = trans.map(view->rect().topLeft());
    const auto bottomRight = trans.map(view->rect().bottomRight());
    return bRect.united(maskRect(topLeft, bottomRight));
}

QPainterPath TFEditorMask::shape() const {

    QPainterPath path(QPointF{0.0, width / 2.0});
    path.lineTo(QPointF{width / 2.0, 0.0});
    path.lineTo(QPointF{0.0, -width / 2.0});
    path.lineTo(QPointF{-width / 2.0, 0.0});
    path.lineTo(QPointF{0.0, width / 2.0});

    return path;
}

void TFEditorMask::paint(QPainter* painter,
                         [[maybe_unused]] const QStyleOptionGraphicsItem* options,
                         [[maybe_unused]] QWidget* widget) {

    painter->setRenderHint(QPainter::Antialiasing, true);

    if (!scene()->views().empty()) {
        // draw vertical line
        const utilqt::Save saved{painter};

        painter->setPen(
            utilqt::cosmeticPen(QColor(64, 64, 64, 150), penWidth, Qt::SolidLine, Qt::RoundCap));
        painter->setBrush(QColor(64, 64, 64, 100));

        auto* view = scene()->views().front();
        auto trans = deviceTransform(view->viewportTransform()).inverted();
        const auto topLeft = trans.map(view->rect().topLeft());
        const auto bottomRight = trans.map(view->rect().bottomRight());

        painter->drawRect(maskRect(topLeft, bottomRight));
    }

    // set up pen and brush for drawing the primitive
    painter->setPen(utilqt::cosmeticPen(QColor(33, 33, 33), penWidth, Qt::SolidLine, Qt::RoundCap));
    painter->setBrush(QColor(66, 66, 66));

    QPainterPath path(QPointF{0.0, shapeWidth / 2.0});
    path.lineTo(QPointF{shapeWidth / 2.0, 0.0});
    path.lineTo(QPointF{0.0, -shapeWidth / 2.0});
    path.lineTo(QPointF{-shapeWidth / 2.0, 0.0});
    path.lineTo(QPointF{0.0, shapeWidth / 2.0});
    painter->drawPath(path);
}

TFEditorMaskMin::TFEditorMaskMin(TFPropertyConcept* c) : TFEditorMask{c} {}

QVariant TFEditorMaskMin::itemChange(GraphicsItemChange change, const QVariant& value) {
    if ((change == QGraphicsItem::ItemPositionChange) && scene()) {
        // constrain positions to valid view positions
        return utilqt::clamp(QPointF{value.toPointF().x(), center_}, scene()->sceneRect());
    } else if (change == QGraphicsItem::ItemPositionHasChanged) {
        const auto mask = concept_->getMask();
        if (value.toPointF().x() != mask.x) {
            concept_->setMask(value.toPointF().x(), mask.y);
        }
        return {};
    } else if (change == QGraphicsItem::ItemSceneHasChanged) {
        setPos(QPointF{concept_->getMask().x, center_});
    }
    return QGraphicsItem::itemChange(change, value);
}

QRectF TFEditorMaskMin::maskRect(QPointF topLeft, QPointF bottomRight) const {
    return QRectF(topLeft, QPointF{0.0, bottomRight.y()});
}

void TFEditorMaskMin::onTFMaskChanged(const TFPrimitiveSet&, dvec2 mask) {
    if (pos().x() != mask.x) {
        setPos({mask.x, center_});
    }
}

TFEditorMaskMax::TFEditorMaskMax(TFPropertyConcept* c) : TFEditorMask{c} {}

QVariant TFEditorMaskMax::itemChange(GraphicsItemChange change, const QVariant& value) {
    if ((change == QGraphicsItem::ItemPositionChange) && scene()) {
        // constrain positions to valid view positions
        return utilqt::clamp(QPointF{value.toPointF().x(), center_}, scene()->sceneRect());
    } else if (change == QGraphicsItem::ItemPositionHasChanged) {
        const auto mask = concept_->getMask();
        if (value.toPointF().x() != mask.y) {
            concept_->setMask(mask.x, value.toPointF().x());
        }
        return {};
    } else if (change == QGraphicsItem::ItemSceneHasChanged) {
        setPos(QPointF{concept_->getMask().y, center_});
    }
    return QGraphicsItem::itemChange(change, value);
}

QRectF TFEditorMaskMax::maskRect(QPointF topLeft, QPointF bottomRight) const {
    return QRectF(QPointF(0.0, topLeft.y()), bottomRight);
}

void TFEditorMaskMax::onTFMaskChanged(const TFPrimitiveSet&, dvec2 mask) {
    if (pos().x() != mask.y) {
        setPos({mask.y, center_});
    }
}

}  // namespace inviwo
