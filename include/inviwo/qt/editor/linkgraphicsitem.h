/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 * Version 0.9
 *
 * Copyright (c) 2012-2015 Inviwo Foundation
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

#ifndef IVW_LINKGRAPHICSITEM_H
#define IVW_LINKGRAPHICSITEM_H

#include <inviwo/qt/editor/inviwoqteditordefine.h>
#include <inviwo/qt/editor/editorgrapicsitem.h>
#include <QGraphicsLineItem>
#include <QPainterPath>
#include <QEvent>

namespace inviwo {

class Port;
class ProcessorLinkGraphicsItem;
class ProcessorGraphicsItem;
class ProcessorLink;

class IVW_QTEDITOR_API LinkGraphicsItem : public EditorGraphicsItem {
public:
    LinkGraphicsItem(QPointF startPoint, QPointF endPoint, ivec3 color = ivec3(255, 255, 255),
                     QPointF startDir = QPointF(1.0f, 0.0), QPointF endDir = QPointF(0.0f, 0.0f));
    ~LinkGraphicsItem();

    QPointF getStartPoint() const;
    QPointF getEndPoint() const;
    QPointF getStartDir() const;
    QPointF getEndDir() const;

    void setStartPoint(QPointF startPoint);
    void setEndPoint(QPointF endPoint);
    void setStartDir(QPointF dir);
    void setEndDir(QPointF dir);

    virtual void updateShape();

    /**
     * Overloaded paint method from QGraphicsItem. Here the actual representation is drawn.
     */
    void paint(QPainter* p, const QStyleOptionGraphicsItem* options, QWidget* widget);
    virtual QRectF boundingRect() const;
    virtual QPainterPath shape() const;

    // override for qgraphicsitem_cast (refer qt documentation)
    enum { Type = UserType + LinkGraphicsType };
    int type() const { return Type; }

protected:
    virtual QPainterPath obtainCurvePath() const;

    QPointF startPoint_;
    QPointF endPoint_;
    QColor color_;
    QPointF startDir_;
    QPointF endDir_;

    QPainterPath path_;
    QRectF rect_;
};

class IVW_QTEDITOR_API LinkConnectionDragGraphicsItem : public LinkGraphicsItem {
public:
    LinkConnectionDragGraphicsItem(ProcessorLinkGraphicsItem* outLink,
                                   QPointF endPos);
    ~LinkConnectionDragGraphicsItem();

    void reactToProcessorHover(ProcessorGraphicsItem* processor);
    virtual ProcessorLinkGraphicsItem* getSrcProcessorLinkGraphicsItem() const;
    virtual ProcessorGraphicsItem* getSrcProcessorGraphicsItem() const;
    virtual void updateShape();

    enum { Type = UserType + LinkConnectionDragGraphicsType };
    int type() const { return Type; }

protected:
    QPointF inLeft_;
    QPointF inRight_;

    virtual QPainterPath obtainCurvePath() const;
    ProcessorLinkGraphicsItem* outLink_;  //< non-onwing reference
};

class IVW_QTEDITOR_API LinkConnectionGraphicsItem : public LinkConnectionDragGraphicsItem {
public:
    LinkConnectionGraphicsItem(ProcessorLinkGraphicsItem* outLink,
                               ProcessorLinkGraphicsItem* inLink);
    ~LinkConnectionGraphicsItem();

    virtual ProcessorLinkGraphicsItem* getDestProcessorLinkGraphicsItem() const;
    virtual ProcessorGraphicsItem* getDestProcessorGraphicsItem() const;
    virtual void updateShape();

    enum { Type = UserType + LinkConnectionGraphicsType };
    int type() const { return Type; }

    virtual void showToolTip(QGraphicsSceneHelpEvent* e);

protected:
    virtual QPainterPath obtainCurvePath() const;
    ProcessorLinkGraphicsItem* inLink_;  //< non-onwing reference
};

}  // namespace

#endif  // IVW_CONNECTIONGRAPHICSITEM_H