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

#pragma once

#include <modules/qtwidgets/qtwidgetsmoduledefine.h>  // for IVW_MODULE_QTWIDGETS_API

#include <inviwo/core/datastructures/tfprimitive.h>  // for TFPrimitive, TFPrimitiveObserver
#include <modules/qtwidgets/tf/tfeditorprimitive.h>  // for TFEditorPrimitive, TFEditorPrimitiv...
#include <modules/qtwidgets/tf/tfmovemode.h>

#include <inviwo/core/util/glmvec.h>  // for dvec2

#include <QGraphicsItem>  // for QGraphicsItem::UserType
#include <QPainterPath>   // for QPainterPath
#include <QPointF>        // for QPointF
#include <QRectF>         // for QRectF

class QGraphicsScene;
class QPainter;
class QPointF;
class QRectF;

namespace inviwo {

class TFControlPointConnection;

class IVW_MODULE_QTWIDGETS_API TFEditorControlPoint : public TFEditorPrimitive {
public:
    explicit TFEditorControlPoint(TFPrimitive& primitive);
    ~TFEditorControlPoint() = default;

    virtual TFControlPointConnection* left() const override { return left_; }
    virtual TFControlPointConnection* right() const override { return right_; }
    virtual void setLeft(TFControlPointConnection* left) override { left_ = left; }
    virtual void setRight(TFControlPointConnection* right) override { right_ = right; }

protected:
    virtual QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;
    virtual QRectF boundingRect() const override;
    virtual QPainterPath shape() const override;

    TFMoveMode moveMode() const;

    static constexpr int tfZLevel = 10;
    virtual int zLevel() const override { return tfZLevel; }

private:
    virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* options,
                       QWidget* widget) override;

    TFControlPointConnection* left_ = nullptr;   // Non-owning reference
    TFControlPointConnection* right_ = nullptr;  // Non-owning reference
};

}  // namespace inviwo
