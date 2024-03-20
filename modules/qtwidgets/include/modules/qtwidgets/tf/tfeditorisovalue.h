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
#include <inviwo/core/util/glmvec.h>                 // for dvec2
#include <modules/qtwidgets/tf/tfeditorprimitive.h>  // for TFEditorPrimitive, TFEditorPrimitiv...

#include <QGraphicsItem>  // for QGraphicsItem::UserType
#include <QPainterPath>   // for QPainterPath
#include <QRectF>         // for QRectF

class QGraphicsScene;
class QPainter;
class QRectF;

namespace inviwo {

class IVW_MODULE_QTWIDGETS_API TFEditorIsovalue : public TFEditorPrimitive {
public:
    TFEditorIsovalue(TFPrimitive& primitive);
    ~TFEditorIsovalue() = default;

protected:
    virtual QRectF boundingRect() const override;
    virtual QPainterPath shape() const override;

    static constexpr int isoZLevel = 5;
    virtual int zLevel() const override { return isoZLevel; }

private:
    virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* options,
                       QWidget* widget) override;
};

}  // namespace inviwo
