/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2015 Inviwo Foundation
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
#include <inviwo/qt/widgets/labelgraphicsitem.h>
#include <inviwo/qt/editor/connectiongraphicsitem.h>
#include <inviwo/qt/widgets/inviwodockwidget.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QGraphicsView>
#include <QDialog>
#include <QGraphicsRectItem>
#include <QPushButton>
#include <QGraphicsLineItem>
#include <QGraphicsPolygonItem>
#include <QPainterPath>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QComboBox>
#include <QStandardItemModel>
#include <QEventLoop>
#include <QCheckBox>
#include <QTimeLine>
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
}

enum IVW_QTEDITOR_API InviwoLinkUserGraphicsItemType {
    LinkDialogCurveGraphicsItemType = 3,
    LinkDialogProcessorGraphicsItemType = 4,
    LinkDialogPropertyGraphicsItemType = 5,
    LinkDialogDragCurveGraphicsItemType = 6,
};


class IVW_QTEDITOR_API LinkDialogParent {
public:
    enum class Side { Left, Right };
    virtual int getLevel() const = 0;
    virtual Side getSide() const = 0;
    virtual void updatePositions() = 0;
};

template <typename T>
class GraphicsItemData : public QGraphicsRectItem, public LinkDialogParent {
public:
    GraphicsItemData(Side side, T* item) : QGraphicsRectItem(), item_(item), side_(side) {}
    T* getItem() { return item_; }
    void setItem(T* item) { item_ = item; }
    LinkDialogParent::Side getSide() const { return side_; }

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

}  // namespace

#endif  // IVW_LINKDIALOG_GRAPHICSITEMS_H
