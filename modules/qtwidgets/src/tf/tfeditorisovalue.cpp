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

#include <modules/qtwidgets/tf/tfeditorisovalue.h>

#include <inviwo/core/datastructures/tfprimitive.h>  // for TFPrimitive, operator==
#include <inviwo/core/util/glmvec.h>                 // for dvec2
#include <modules/qtwidgets/tf/tfeditor.h>           // for TFEditor
#include <modules/qtwidgets/tf/tfeditorprimitive.h>  // for TFEditorPrimitive
#include <modules/qtwidgets/inviwoqtutils.h>

#include <QColor>          // for QColor
#include <QGraphicsScene>  // for QGraphicsScene
#include <QLineF>          // for QLineF
#include <QObject>         // for qobject_cast
#include <QPainter>        // for QPainter
#include <QPen>            // for QPen
#include <QPointF>         // for QPointF
#include <QSizeF>          // for QSizeF
#include <Qt>              // for DashLine, NoBrush, RoundCap
#include <glm/vec2.hpp>    // for vec<>::(anonymous)
#include <QGraphicsView>

namespace inviwo {

TFEditorIsovalue::TFEditorIsovalue(TFPrimitive& primitive) : TFEditorPrimitive(primitive) {
    // ensure that Isovalue primitives are rendered behind TF control points
    setZValue(isoZLevel);
}

QRectF TFEditorIsovalue::boundingRect() const {
    const double bBoxSize = getSize() + 5.0;  //<! consider size of pen
    auto bRect = QRectF(-bBoxSize / 2.0, -bBoxSize / 2.0, bBoxSize, bBoxSize);

    // add box of vertical line
    auto* view = scene()->views().front();
    auto trans = deviceTransform(view->viewportTransform()).inverted();

    const auto top = trans.map(view->rect().topLeft()).y();
    const auto bottom = trans.map(view->rect().bottomLeft()).y();
    return bRect.united(QRectF(QPointF(-1.0, top), QPointF(1.0, bottom)));
}

QPainterPath TFEditorIsovalue::shape() const {
    QPainterPath path;
    const auto width = getSize() + 3.0;  //<! consider size of pen;
    path.addRect(QRectF(QPointF(-0.5 * width, -0.5 * width), QSizeF(width, width)));

    // do not add the vertical line to the shape in order to make only the square box selectable
    return path;
}

void TFEditorIsovalue::paint(QPainter* painter,
                             [[maybe_unused]] const QStyleOptionGraphicsItem* options,
                             [[maybe_unused]] QWidget* widget) {

    painter->setRenderHint(QPainter::Antialiasing, true);

    {
        // draw vertical line
        const utilqt::Save saved{painter};
        painter->setPen(
            utilqt::cosmeticPen(QColor(64, 64, 64, 255), 2.0, Qt::DashLine, Qt::RoundCap));
        painter->setBrush(Qt::NoBrush);

        auto* view = scene()->views().front();
        auto trans = deviceTransform(view->viewportTransform()).inverted();
        const auto top = trans.map(view->rect().topLeft()).y();
        const auto bottom = trans.map(view->rect().bottomLeft()).y();

        painter->drawLine(QLineF(QPointF(0.0, top), QPointF(0.0, bottom)));
    }

    {
        // set up pen and brush for drawing the primitive
        const auto pen =
            utilqt::cosmeticPen(isSelected() ? QColor{213, 79, 79} : QColor{66, 66, 66}, 3.0,
                                Qt::SolidLine, Qt::RoundCap);
        const auto brush = QBrush(utilqt::toQColor(vec4(vec3(getColor()), 1.0f)));
        painter->setPen(pen);
        painter->setBrush(brush);

        // draw square for indicating isovalue
        const auto width = getSize();
        painter->drawRect(QRectF(QPointF(-0.5f * width, -0.5f * width), QSizeF(width, width)));
    }
}
}  // namespace inviwo
