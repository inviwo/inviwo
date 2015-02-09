/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 * Version 0.9
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

#ifndef  IVW_LINKDIALOG_PROPERTYGRAPHICSITEMS_H
#define  IVW_LINKDIALOG_PROPERTYGRAPHICSITEMS_H

#include <inviwo/qt/editor/inviwoqteditordefine.h>
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

#include <inviwo/core/links/linkconditions.h>
#include <inviwo/core/network/processornetwork.h>
#include <inviwo/qt/widgets/labelgraphicsitem.h>
#include <inviwo/qt/editor/connectiongraphicsitem.h>
#include <inviwo/qt/widgets/inviwodockwidget.h>

#include <inviwo/qt/editor/linkdialog/linkdialoggraphicsitems.h>

namespace inviwo {

class DialogConnectionGraphicsItem;
class LinkDialogProcessorGraphicsItem;

class IVW_QTEDITOR_API LinkDialogPropertyGraphicsItem : public GraphicsItemData<Property> {

public:
    LinkDialogPropertyGraphicsItem(LinkDialogProcessorGraphicsItem*, Property*, LinkDialogPropertyGraphicsItem* parentPropertyGraphicsItem=0, int subPropertyLevel=0);
    ~LinkDialogPropertyGraphicsItem();

    void setProperty(Property* property);

    QSizeF sizeHint(Qt::SizeHint which, const QSizeF& constraint = QSizeF()) const;

    //override for qgraphicsitem_cast (refer qt documentation)
    enum { Type = UserType + LinkDialogPropertyGraphicsItemType };
    int type() const  {return Type; }

    QPointF getShortestBoundaryPointTo(LinkDialogPropertyGraphicsItem*);
    QPointF getShortestBoundaryPointTo(QPointF);

    void expand(bool expandSubProperties=false);
    void collapse(bool collapseSubProperties=false);
    bool hasSubProperties();
    bool isExpanded();
    void updatePositionBasedOnIndex(float animateExpansion=1.0);
    void setIndex(int index);
    const int getIndex() const;
    void setAnimate(bool animate);
    const bool getAnimate() const;
    void setPropertyItemIndex(int &currIndex);
    LinkDialogProcessorGraphicsItem* getProcessorItem() const {return processorGraphicsItem_;}
    int getLevel() const {return subPropertyLevel_;}

    QPointF calculateArrowCenter(size_t curPort, bool computeRight) const;
    QRectF calculateArrowRect(size_t curPort, bool computeRight) const;
    QRectF calculateArrowRect(DialogConnectionGraphicsItem* cItem, bool computeRight=true) const;
    DialogConnectionGraphicsItem* getArrowConnectionAt(const QPointF pos) const;
    bool isArrowPointedRight(DialogConnectionGraphicsItem* cItem);

    void prepareGeometryChange() {QGraphicsItem::prepareGeometryChange();}

    void addConnectionGraphicsItem(DialogConnectionGraphicsItem*);
    size_t getConnectionGraphicsItemCount() const;
    void removeConnectionGraphicsItem(DialogConnectionGraphicsItem*);
    const std::vector<DialogConnectionGraphicsItem*>& getConnectionGraphicsItems() const;
    std::vector<LinkDialogPropertyGraphicsItem*> getSubPropertyItemList(bool recursive=false) const;

protected:
    virtual void paint(QPainter* p, const QStyleOptionGraphicsItem* options, QWidget* widget);
    virtual QVariant itemChange(GraphicsItemChange change, const QVariant& value);

private:
    LabelGraphicsItem* classLabel_;
    LabelGraphicsItem* typeLabel_;
    LinkDialogProcessorGraphicsItem* processorGraphicsItem_;
    LinkDialogPropertyGraphicsItem* parentPropertyGraphicsItem_;
    std::vector<DialogConnectionGraphicsItem*> connectionItems_;
    std::vector<LinkDialogPropertyGraphicsItem*> subPropertyGraphicsItems_;
    int subPropertyLevel_;
    bool isExpanded_;
    int index_;
    bool animateEnabled_;
};

} //namespace

#endif //IVW_LINKDIALOG_PROPERTYGRAPHICSITEMS_H
