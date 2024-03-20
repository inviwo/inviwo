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

#pragma once

#include <modules/qtwidgets/qtwidgetsmoduledefine.h>  // for IVW_MODULE_QTWIDGETS_API

#include <inviwo/core/util/glmvec.h>    // for vec4, dvec2, vec3
#include <inviwo/core/util/observer.h>  // for Observable, Observer
#include <inviwo/core/datastructures/tfprimitive.h>

#include <memory>  // for unique_ptr
#include <vector>  // for vector

#include <QGraphicsItem>            // for QGraphicsItem, QGraphicsItem::Graph...
#include <QGraphicsSimpleTextItem>  // for QGraphicsSimpleTextItem
#include <QPointF>                  // for QPointF
#include <QVariant>                 // for QVariant

class QGraphicsScene;
class QGraphicsSceneHoverEvent;
class QPainter;
class QGraphicsSceneMouseEvent;
class QPointF;
class QStyleOptionGraphicsItem;
class QWidget;

namespace inviwo {

class TFControlPointConnection;

class IVW_MODULE_QTWIDGETS_API TFEditorPrimitive : public QGraphicsItem,
                                                   public TFPrimitiveObserver {
public:
    /**
     * Constructs a TransferFunction editor primitive. The graphics item is positioned
     * at the primitive location (scalar value and opacity) within the scene.
     * The graphical representation should thus be centered around (0,0) in local
     * coordinates.
     *
     * @param primitive the primitive
     */
    TFEditorPrimitive(TFPrimitive& primitive);
    virtual ~TFEditorPrimitive() = default;

    TFPrimitive& getPrimitive();
    const TFPrimitive& getPrimitive() const;

    void setPosition(double pos);
    double getPosition() const;

    void setColor(const vec4& color);
    void setColor(const vec3& color);
    void setAlpha(float alpha);
    const vec4& getColor() const;

    double getSize() const;

    void beginMouseDrag();
    void stopMouseDrag();

    virtual TFControlPointConnection* left() const { return nullptr; }
    virtual TFControlPointConnection* right() const { return nullptr; }
    virtual void setLeft(TFControlPointConnection*) {}
    virtual void setRight(TFControlPointConnection*) {}

    friend IVW_MODULE_QTWIDGETS_API bool operator==(const TFEditorPrimitive& lhs,
                                                    const TFEditorPrimitive& rhs);

    // compare TF primitives points by their scalar value
    friend IVW_MODULE_QTWIDGETS_API bool operator<(const TFEditorPrimitive& lhs,
                                                   const TFEditorPrimitive& rhs);

protected:
    virtual QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;
    virtual void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;
    virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;

    virtual int zLevel() const = 0;

    virtual void onTFPrimitiveChange(const TFPrimitive& p) override;
    
    QPointF constrainPosToXorY(QPointF pos) const;

    TFPrimitive& data_;
    bool isEditingPoint_;
    bool hovered_;

    void setHovered(bool hover);
    void updateLabel();

    std::unique_ptr<QGraphicsSimpleTextItem> tfPrimitiveLabel_;
    QPointF cachedPosition_;  //!< used for restricting to horizontal/vertical movement
    bool mouseDrag_;
};

IVW_MODULE_QTWIDGETS_API bool operator==(const TFEditorPrimitive& lhs,
                                         const TFEditorPrimitive& rhs);
IVW_MODULE_QTWIDGETS_API bool operator!=(const TFEditorPrimitive& lhs,
                                         const TFEditorPrimitive& rhs);
IVW_MODULE_QTWIDGETS_API bool operator<(const TFEditorPrimitive& lhs, const TFEditorPrimitive& rhs);
IVW_MODULE_QTWIDGETS_API bool operator>(const TFEditorPrimitive& lhs, const TFEditorPrimitive& rhs);
IVW_MODULE_QTWIDGETS_API bool operator<=(const TFEditorPrimitive& lhs,
                                         const TFEditorPrimitive& rhs);
IVW_MODULE_QTWIDGETS_API bool operator>=(const TFEditorPrimitive& lhs,
                                         const TFEditorPrimitive& rhs);

}  // namespace inviwo
