/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2019 Inviwo Foundation
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

#include <modules/qtwidgets/tf/tfeditorisovalue.h>
#include <modules/qtwidgets/tf/tfcontrolpointconnection.h>
#include <modules/qtwidgets/tf/tfeditor.h>
#include <modules/qtwidgets/tf/tfeditorview.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QGraphicsScene>
#include <QPainter>
#include <warn/pop>

namespace inviwo {

TFEditorIsovalue::TFEditorIsovalue(TFPrimitive& primitive, QGraphicsScene* scene, double size)
    : TFEditorPrimitive(primitive, scene, vec2(primitive.getPosition(), primitive.getAlpha()),
                        size) {
    // ensure that Isovalue primitives are rendered behind TF control points
    defaultZValue_ = 5;
    setZValue(defaultZValue_);
    data_.addObserver(this);
}

void TFEditorIsovalue::onTFPrimitiveChange(const TFPrimitive& p) {
    setTFPosition(vec2(p.getPosition(), p.getAlpha()));
}

QRectF TFEditorIsovalue::boundingRect() const {
    double bBoxSize = getSize() + 5.0;  //<! consider size of pen
    auto bRect = QRectF(-bBoxSize / 2.0, -bBoxSize / 2.0, bBoxSize, bBoxSize);

    // add box of vertical line
    double verticalScaling = getVerticalSceneScaling();
    QRectF rect = scene()->sceneRect();
    return bRect.united(QRectF(QPointF(-1.0, -rect.height() * verticalScaling),
                               QPointF(1.0, rect.height() * verticalScaling)));
}

QPainterPath TFEditorIsovalue::shape() const {
    QPainterPath path;
    const auto width = getSize() + 3.0;  //<! consider size of pen;
    path.addRect(QRectF(QPointF(-0.5 * width, -0.5 * width), QSizeF(width, width)));

    // do not add the vertical line to the shape in order to make only the square box selectable

    return path;
}

void TFEditorIsovalue::paintPrimitive(QPainter* painter) {
    // draw vertical line
    painter->save();

    QPen pen = QPen();
    pen.setCosmetic(true);
    pen.setCapStyle(Qt::RoundCap);

    pen.setWidthF(2.0);
    pen.setColor(QColor(64, 64, 64, 255));
    pen.setStyle(Qt::DashLine);
    // ensure line is always covering entire scene
    QRectF rect = scene()->sceneRect();
    painter->setPen(pen);
    painter->setBrush(Qt::NoBrush);

    double verticalScaling = getVerticalSceneScaling();
    painter->drawLine(QLineF(QPointF(0.0, -rect.height() * verticalScaling),
                             QPointF(0.0, rect.height() * verticalScaling)));

    painter->restore();

    // draw square for indicating isovalue
    const auto width = getSize();
    painter->drawRect(QRectF(QPointF(-0.5f * width, -0.5f * width), QSizeF(width, width)));
}

void TFEditorIsovalue::onItemPositionChange(const vec2& newPos) { data_.setPositionAlpha(newPos); }

void TFEditorIsovalue::onItemSceneHasChanged() { onTFPrimitiveChange(data_); }

double TFEditorIsovalue::getVerticalSceneScaling() const {
    double verticalScaling = 1.0;
    if (auto tfe = qobject_cast<TFEditor*>(scene())) {
        verticalScaling = 1.0 / tfe->getZoom().y;
        ;
    }
    return verticalScaling;
}

bool operator==(const TFEditorIsovalue& lhs, const TFEditorIsovalue& rhs) {
    return lhs.data_ == rhs.data_;
}

bool operator!=(const TFEditorIsovalue& lhs, const TFEditorIsovalue& rhs) {
    return !operator==(lhs, rhs);
}

bool operator<(const TFEditorIsovalue& lhs, const TFEditorIsovalue& rhs) {
    return lhs.currentPos_.x() < rhs.currentPos_.x();
}

bool operator>(const TFEditorIsovalue& lhs, const TFEditorIsovalue& rhs) { return rhs < lhs; }

bool operator<=(const TFEditorIsovalue& lhs, const TFEditorIsovalue& rhs) { return !(rhs < lhs); }

bool operator>=(const TFEditorIsovalue& lhs, const TFEditorIsovalue& rhs) { return !(lhs < rhs); }

}  // namespace inviwo
