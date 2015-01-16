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

#ifndef  IVW_LINKDIALOG_GRAPHICSITEMS_H
#define  IVW_LINKDIALOG_GRAPHICSITEMS_H

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
#include <QTimeLine>

#include <inviwo/core/properties/property.h>
#include <inviwo/qt/widgets/labelgraphicsitem.h>
#include <inviwo/qt/editor/connectiongraphicsitem.h>
#include <inviwo/qt/widgets/inviwodockwidget.h>

static const qreal LINKDIALOG_PROCESSOR_GRAPHICSITEM_DEPTH = 1.0f;
static const qreal LINKDIALOG_PROPERTY_GRAPHICSITEM_DEPTH = 2.0f;
static const qreal LINKDIALOG_CONNECTION_GRAPHICSITEM_DEPTH = 3.0f;

static const int processorItemWidth = 250; //all other parameters depends on processor width.
static const int processorItemHeight = 50;
static const int processorRoundedCorners = 9;
static const int processorLabelHeight = 8;

static const int linkDialogWidth = processorItemWidth*4;
static const int linkDialogHeight = processorItemHeight*12;

static const int propertyLabelHeight = 8;
static const int propertyItemWidth = processorItemWidth*3/4;
static const int propertyItemHeight = processorItemHeight*3/4 + 2*propertyLabelHeight;
static const int propertyRoundedCorners = 0;

static const int propertyExpandCollapseButtonSize = 8;
static const int propertyExpandCollapseOffset = 16;

static const int arrowDimensionWidth = propertyItemWidth/15;
static const int arrowDimensionHeight = arrowDimensionWidth/2;

// WE should not use macros for this kind of things. //Peter.
#define IS_SUB_PROPERTY(prop) (prop->getOwner()->getProcessor() != prop->getOwner())
#define IS_COMPOSITE_PROPERTY(prop) dynamic_cast<CompositeProperty*>(prop)

namespace inviwo {

IVW_QTEDITOR_API enum InviwoLinkUserGraphicsItemType {
    LinkDialogProcessorGraphicsItemType = 4,
    LinkDialogPropertyGraphicsItemType = 5,
    LinkDialogCurveGraphicsItemType = 6
};

template <typename T>
class IVW_QTEDITOR_API GraphicsItemData : public QGraphicsRectItem {
public:
    GraphicsItemData(T* item=0) : QGraphicsRectItem() {item_ = item;}
    T* getGraphicsItemData() {return item_;}
    void setGraphicsItemData(T* item) {item_ = item;}
private:
    T* item_;
};

} //namespace

#endif //IVW_LINKDIALOG_GRAPHICSITEMS_H
