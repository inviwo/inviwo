/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2019 Inviwo Foundation
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

#ifndef IVW_LINKDIALOG_GRAPHICSITEMS_H
#define IVW_LINKDIALOG_GRAPHICSITEMS_H

#include <inviwo/qt/editor/inviwoqteditordefine.h>
#include <inviwo/core/properties/property.h>
#include <modules/qtwidgets/labelgraphicsitem.h>
#include <inviwo/qt/editor/connectiongraphicsitem.h>
#include <modules/qtwidgets/inviwodockwidget.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QGraphicsView>
#include <QGraphicsRectItem>
#include <QToolTip>
#include <warn/pop>

namespace inviwo {

namespace linkdialog {
static const qreal connectionDepth = 1.0f;
static const qreal processorDepth = 2.0f;
static const qreal propertyDepth = 3.0f;
static const qreal connectionDragDepth = 4.0f;

static const int offset = 2;

static const int processorWidth = 250;  // all other parameters depends on processor width.
static const int processorHeight = 40;
static const int processorRoundedCorners = 5;
static const int processorLabelHeight = 12;

static const int dialogWidth = processorWidth * 3;
static const int dialogHeight = processorHeight * 12;

static const int propertyLabelHeight = 12;
static const int propertyWidth = processorWidth * 7 / 8;
static const int propertyHeight = processorHeight;
static const int propertyRoundedCorners = 3;

static const int propertyExpandCollapseButtonSize = 8;
static const int propertyExpandCollapseOffset = 16;

static const int arrowWidth = propertyWidth / 15;
static const int arrowHeight = arrowWidth / 2;
}  // namespace linkdialog

enum InviwoLinkUserGraphicsItemType {
    LinkDialogCurveGraphicsItemType = 3,
    LinkDialogProcessorGraphicsItemType = 4,
    LinkDialogPropertyGraphicsItemType = 5,
    LinkDialogDragCurveGraphicsItemType = 6,
};

class IVW_QTEDITOR_API LinkDialogTreeItem {
public:
    LinkDialogTreeItem(LinkDialogTreeItem* parent);
    virtual ~LinkDialogTreeItem() = default;
    enum class Side { Left, Right };
    virtual int getLevel() const = 0;
    virtual Side getSide() const = 0;
    virtual void updatePositions() = 0;
    virtual QPointF treeItemScenePos() const = 0;
    virtual QRectF treeItemRect() const = 0;

    void setPrev(LinkDialogTreeItem* prev);
    void setNext(LinkDialogTreeItem* next);
    LinkDialogTreeItem* parent() const;
    LinkDialogTreeItem* next() const;
    LinkDialogTreeItem* prev() const;

    void setExpanded(bool expand);
    bool isExpanded() const;

private:
    LinkDialogTreeItem* parent_ = nullptr;
    LinkDialogTreeItem* prev_ = nullptr;
    LinkDialogTreeItem* next_ = nullptr;
    bool isExpanded_ = false;
};

template <typename T>
class GraphicsItemData : public QGraphicsRectItem, public LinkDialogTreeItem {
public:
    GraphicsItemData(LinkDialogTreeItem* parent, Side side, T* item)
        : QGraphicsRectItem(), LinkDialogTreeItem(parent), item_(item), side_(side) {}
    T* getItem() { return item_; }
    void setItem(T* item) { item_ = item; }
    virtual LinkDialogTreeItem::Side getSide() const override { return side_; }
    virtual QPointF treeItemScenePos() const override { return scenePos(); }
    virtual QRectF treeItemRect() const override { return rect(); }

    void showToolTipHelper(QGraphicsSceneHelpEvent* e, QString string) const {
        QGraphicsView* v = scene()->views().first();
        QRectF rect = this->mapRectToScene(this->rect());
        QRect viewRect = v->mapFromScene(rect).boundingRect();
        QToolTip::showText(e->screenPos(), string, v, viewRect);
    }

protected:
    T* item_;
    const Side side_;
};

inline LinkDialogTreeItem::LinkDialogTreeItem(LinkDialogTreeItem* parent) : parent_(parent) {}
inline void LinkDialogTreeItem::setNext(LinkDialogTreeItem* next) { next_ = next; }
inline void LinkDialogTreeItem::setPrev(LinkDialogTreeItem* prev) { prev_ = prev; }
inline LinkDialogTreeItem* LinkDialogTreeItem::parent() const { return parent_; }
inline LinkDialogTreeItem* LinkDialogTreeItem::next() const { return next_; }
inline LinkDialogTreeItem* LinkDialogTreeItem::prev() const { return prev_; }

inline void LinkDialogTreeItem::setExpanded(bool expand) { isExpanded_ = expand; }
inline bool LinkDialogTreeItem::isExpanded() const { return isExpanded_; }

}  // namespace inviwo

#endif  // IVW_LINKDIALOG_GRAPHICSITEMS_H
