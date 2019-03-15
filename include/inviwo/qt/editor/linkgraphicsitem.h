/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2019 Inviwo Foundation
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
#include <warn/push>
#include <warn/ignore/all>
#include <QGraphicsLineItem>
#include <QPainterPath>
#include <QEvent>
#include <warn/pop>

namespace inviwo {

class Port;
class ProcessorLinkGraphicsItem;
class ProcessorGraphicsItem;
class ProcessorLink;

class IVW_QTEDITOR_API LinkGraphicsItem : public EditorGraphicsItem {
public:
    LinkGraphicsItem(ivec3 color = ivec3(0xbd, 0xcd, 0xd5));
    ~LinkGraphicsItem();

    virtual QPointF getStartPoint() const = 0;
    virtual QPointF getEndPoint() const = 0;
    virtual QPointF getStartDir() const = 0;
    virtual QPointF getEndDir() const = 0;

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

    QColor color_;
    QPainterPath path_;
    QRectF rect_;
};

class IVW_QTEDITOR_API LinkConnectionDragGraphicsItem : public LinkGraphicsItem {
public:
    LinkConnectionDragGraphicsItem(ProcessorLinkGraphicsItem* outLink, QPointF endPos);
    ~LinkConnectionDragGraphicsItem();

    virtual QPointF getStartPoint() const override;
    virtual QPointF getEndPoint() const override;
    virtual QPointF getStartDir() const override;
    virtual QPointF getEndDir() const override;
    virtual void setEndPoint(QPointF endPoint);
    virtual void setEndPoint(QPointF endPointLeft, QPointF endPointRight);

    void reactToProcessorHover(ProcessorGraphicsItem* processor);
    virtual ProcessorLinkGraphicsItem* getSrcProcessorLinkGraphicsItem() const;
    virtual ProcessorGraphicsItem* getSrcProcessorGraphicsItem() const;

    enum { Type = UserType + LinkConnectionDragGraphicsType };
    virtual int type() const override { return Type; }

protected:
    static QPointF compare(const QPointF startLeft, const QPointF& startRight,
                           const QPointF& endLeft, const QPointF& endRight, const QPointF& left,
                           const QPointF& center, const QPointF& right);

    QPointF inLeft_;
    QPointF inRight_;

    ProcessorLinkGraphicsItem* outLink_;  //< non-owning reference
};

class IVW_QTEDITOR_API LinkConnectionGraphicsItem : public LinkConnectionDragGraphicsItem {
public:
    LinkConnectionGraphicsItem(ProcessorLinkGraphicsItem* outLink,
                               ProcessorLinkGraphicsItem* inLink);
    ~LinkConnectionGraphicsItem();

    virtual QPointF getStartPoint() const override;
    virtual QPointF getEndPoint() const override;
    virtual QPointF getStartDir() const override;
    virtual QPointF getEndDir() const override;

    virtual ProcessorLinkGraphicsItem* getDestProcessorLinkGraphicsItem() const;
    virtual ProcessorGraphicsItem* getDestProcessorGraphicsItem() const;

    enum { Type = UserType + LinkConnectionGraphicsType };
    virtual int type() const override { return Type; }

    virtual void showToolTip(QGraphicsSceneHelpEvent* e) override;

protected:
    ProcessorLinkGraphicsItem* inLink_;  //< non-owning reference
};

}  // namespace inviwo

#endif  // IVW_CONNECTIONGRAPHICSITEM_H
