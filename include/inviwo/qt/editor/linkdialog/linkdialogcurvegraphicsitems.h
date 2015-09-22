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

#ifndef  IVW_LINKDIALOG_CURVEGRAPHICSITEMS_H
#define  IVW_LINKDIALOG_CURVEGRAPHICSITEMS_H

#include <inviwo/qt/editor/inviwoqteditordefine.h>
#include <inviwo/qt/widgets/labelgraphicsitem.h>
#include <inviwo/qt/editor/connectiongraphicsitem.h>
#include <inviwo/qt/widgets/inviwodockwidget.h>
#include <inviwo/qt/editor/linkdialog/linkdialoggraphicsitems.h>

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
#include <warn/pop>

namespace inviwo {

class PropertyLink;

class IVW_QTEDITOR_API DialogCurveGraphicsItem : public CurveGraphicsItem {

public:
    DialogCurveGraphicsItem(QPointF startPoint, QPointF endPoint, uvec3 color=uvec3(38,38,38));
    virtual ~DialogCurveGraphicsItem();

    //override for qgraphicsitem_cast (refer qt documentation)
    enum { Type = UserType + LinkDialogCurveGraphicsItemType };
    int type() const override {return Type; }
protected:
    virtual QPainterPath obtainCurvePath() const override;
    virtual QPainterPath obtainCurvePath(QPointF startPoint, QPointF endPoint) const override;
};

//////////////////////////////////////////////////////////////////////////

class LinkDialogPropertyGraphicsItem;
class LinkDialogProcessorGraphicsItem;

class IVW_QTEDITOR_API DialogConnectionGraphicsItem : public DialogCurveGraphicsItem {
public:
    DialogConnectionGraphicsItem(LinkDialogPropertyGraphicsItem* startProperty, LinkDialogPropertyGraphicsItem* endProperty,
                                 PropertyLink* propertyLink);
    virtual ~DialogConnectionGraphicsItem();

    PropertyLink* getPropertyLink() const { return propertyLink_; }
    LinkDialogPropertyGraphicsItem* getStartProperty() const { return startPropertyGraphicsItem_; }
    LinkDialogPropertyGraphicsItem* getEndProperty() const { return endPropertyGraphicsItem_; }

    bool isBidirectional();

    void setStartArrowHeadIndex(size_t index) {startArrowHeadIndex_ = index;}
    void setEndArrowHeadIndex(size_t index) {endArrowHeadIndex_ = index;}
    size_t getStartArrowHeadIndex() { return startArrowHeadIndex_;}
    size_t getEndArrowHeadIndex() { return endArrowHeadIndex_;}
    void updateStartEndPoint();
    void updateConnectionDrawing();
    void cleanup();

protected:
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* e);
    

private:
    LinkDialogPropertyGraphicsItem* startPropertyGraphicsItem_;
    LinkDialogPropertyGraphicsItem* endPropertyGraphicsItem_;
    size_t startArrowHeadIndex_;
    size_t endArrowHeadIndex_;
    PropertyLink* propertyLink_;
};

} //namespace

#endif //IVW_LINKDIALOG_CURVEGRAPHICSITEMS_H
