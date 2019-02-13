/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2019 Inviwo Foundation
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

#ifndef IVW_TRANSFERFUNCTIONEDITORPRIMITIVE_H
#define IVW_TRANSFERFUNCTIONEDITORPRIMITIVE_H

#include <modules/qtwidgets/qtwidgetsmoduledefine.h>

#include <inviwo/core/datastructures/tfprimitive.h>
#include <inviwo/core/util/observer.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QGraphicsItem>
#include <warn/pop>

class QGraphicsScene;
class QGraphicsSceneHoverEvent;
class QPainter;
class QGraphicsSimpleTextItem;
class QGraphicsMouseEvent;

namespace inviwo {

class TFEditorPrimitive;

class IVW_MODULE_QTWIDGETS_API TFEditorPrimitiveObserver : public Observer {
public:
    virtual void onTFPrimitiveDoubleClicked(const TFEditorPrimitive* p);
};

class IVW_MODULE_QTWIDGETS_API TFEditorPrimitive : public QGraphicsItem,
                                                   public Observable<TFEditorPrimitiveObserver> {
public:
    enum ItemType {
        TFEditorUnknownPrimitiveType = 30,
        TFEditorControlPointType,
        TFControlPointConnectionType,
        TFEditorIsovalueType,
        Number_of_InviwoWidgetGraphicsItemTypes
    };

    /**
     * Constructs a TransferFunction editor primitive. The graphics item is positioned
     * at the primitive location (scalar value and opacity) within the scene.
     * The graphical representation should thus be centered around (0,0) in local
     * coordinates.
     *
     * @param primitive pointer to the primitive
     * @param scene    item will be added to this QGraphicsScene
     * @param pos      normalized position of primitive (scalar value and opacity)
     * @param size     base size of primitive
     */
    TFEditorPrimitive(TFPrimitive& primitive, QGraphicsScene* scene = nullptr,
                      const vec2& pos = vec2(), double size = 14.0);
    virtual ~TFEditorPrimitive() = default;

    TFPrimitive& getPrimitive();
    const TFPrimitive& getPrimitive() const;

    void setPosition(double pos);
    double getPosition() const;

    void setColor(const vec4& color);
    void setColor(const vec3& color);
    void setAlpha(float alpha);
    const vec4& getColor() const;

    /**
     * set the position of the primitive using normalized coordinates within the transfer
     * function.
     *
     * @param tfpos   normalized position [0,1] corresponding to scalar value and opacity
     */
    void setTFPosition(const dvec2& tfpos);

    virtual const QPointF& getCurrentPos() const;

    void setSize(double s);
    double getSize() const;

    void setHovered(bool hover);

    void beginMouseDrag();
    void stopMouseDrag();

    friend IVW_MODULE_QTWIDGETS_API bool operator==(const TFEditorPrimitive& lhs,
                                                    const TFEditorPrimitive& rhs);

    // compare TF primitives points by their scalar value
    friend IVW_MODULE_QTWIDGETS_API bool operator<(const TFEditorPrimitive& lhs,
                                                   const TFEditorPrimitive& rhs);

protected:
    /**
     * paints the primitive and the label if visible. Pen and brush are set up prior
     * calling paintPrimitive() reflecting primitive color and selection.
     */
    virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* options,
                       QWidget* widget) override;
    virtual QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;
    virtual void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;
    virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;
    virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event) override;

    /**
     * draws the primitive. Gets called from within paint()
     *
     * @param painter   painter for drawing the object, pen and brush are set up to match
     *                  primitive color and selection status
     */
    virtual void paintPrimitive(QPainter* painter) = 0;

    /**
     * Allows to adjust the position prior updating the graphicsitem position,
     * e.g. when the movement of a TF point is restricted ("restrict" or "push").
     * This function is called in itemChange() before calling onItemPositionChange.
     *
     * @param pos   candidate for new position (in scene coords)
     * @return modified position which is used instead of pos
     */
    virtual QPointF prepareItemPositionChange(const QPointF& pos) { return pos; }

    /**
     * gets called in itemChange() after a position change of the item. The position
     * is already adjusted to lie within the scene bounding box and normalized.
     *
     * @param newPos   new, normalized position of the primitive
     */
    virtual void onItemPositionChange(const vec2& /*newPos*/) {}

    /**
     * gets called in itemChange() when a scene change has happend
     */
    virtual void onItemSceneHasChanged() {}

    int defaultZValue_ = 10;

    double size_;
    bool isEditingPoint_;
    QPointF currentPos_;  //!< position within scene rect (not normalized)
    bool hovered_;

    TFPrimitive& data_;

private:
    void updatePosition(const QPointF& pos);
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

#endif  // IVW_TRANSFERFUNCTIONEDITORPRIMITIVE_H
