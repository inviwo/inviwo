/*********************************************************************************
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
#pragma once

#include <modules/qtwidgets/qtwidgetsmoduledefine.h>
#include <inviwo/core/datastructures/tfprimitiveset.h>
#include <inviwo/core/properties/transferfunctionproperty.h>

#include <QGraphicsItem>  // for QGraphicsItem::UserType

namespace inviwo {

class TFPropertyConcept;

class IVW_MODULE_QTWIDGETS_API TFEditorMask : public QGraphicsItem,
                                              public TFPropertyObserver,
                                              public TFPrimitiveSetObserver {
public:
    explicit TFEditorMask(TFPropertyConcept* c);
    virtual ~TFEditorMask() = default;

protected:
    static constexpr double shapeWidth = 12.0;
    static constexpr double penWidth = 1.5;
    static constexpr double width = shapeWidth + penWidth;

    virtual void onZoomVChange(const dvec2& zoomV) override;

    virtual QRectF maskRect(QPointF topLeft, QPointF bottomRight) const = 0;

    virtual QRectF boundingRect() const override;
    virtual QPainterPath shape() const override;
    virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* options,
                       QWidget* widget) override;

    TFPropertyConcept* concept_;
    double center_;
};

class IVW_MODULE_QTWIDGETS_API TFEditorMaskMin : public TFEditorMask {
public:
    explicit TFEditorMaskMin(TFPropertyConcept* c);

private:
    virtual void onTFMaskChanged(const TFPrimitiveSet& set, dvec2 mask) override;
    virtual QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;
    virtual QRectF maskRect(QPointF topLeft, QPointF bottomRight) const override;
};

class IVW_MODULE_QTWIDGETS_API TFEditorMaskMax : public TFEditorMask {
public:
    explicit TFEditorMaskMax(TFPropertyConcept* c);

private:
    virtual void onTFMaskChanged(const TFPrimitiveSet& set, dvec2 mask) override;
    virtual QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;
    virtual QRectF maskRect(QPointF topLeft, QPointF bottomRight) const override;
};

}  // namespace inviwo
