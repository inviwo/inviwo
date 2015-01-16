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

#ifndef IVW_CONNECTIONGRAPHICSITEM_H
#define IVW_CONNECTIONGRAPHICSITEM_H

#include <inviwo/qt/editor/inviwoqteditordefine.h>
#include <QGraphicsLineItem>
#include <QPainterPath>
#include <QEvent>
#include <QColor>
#include <QPointF>

#include <inviwo/qt/editor/editorgrapicsitem.h>

namespace inviwo {

class Port;
class PortConnection;
class Inport;
class Outport;
class ProcessorGraphicsItem;
class ProcessorOutportGraphicsItem;
class ProcessorInportGraphicsItem;
class PortInspectionManager;

class IVW_QTEDITOR_API CurveGraphicsItem : public EditorGraphicsItem {
public:
    CurveGraphicsItem(QPointF startPoint, QPointF endPoint, uvec3 color = uvec3(38, 38, 38));
    ~CurveGraphicsItem();

    virtual QPainterPath shape() const;
    virtual QPointF getStartPoint() const;
    virtual QPointF getEndPoint() const;
    virtual QPointF getMidPoint() const;
    virtual QColor getColor() const;

    virtual void updateShape();

    void setStartPoint(QPointF startPoint);
    void setEndPoint(QPointF endPoint);
    void setMidPoint(QPointF midPoint);
    void setColor(QColor color);

    void clearMidPoint();
    void resetBorderColors();
    void setBorderColor(QColor borderColor);
    void setSelectedBorderColor(QColor selectedBorderColor);

    // override for qgraphicsitem_cast (refer qt documentation)
    enum { Type = UserType + CurveGraphicsType };
    int type() const { return Type; }

    /**
     * Overloaded paint method from QGraphicsItem. Here the actual representation is drawn.
     */
    virtual void paint(QPainter* p, const QStyleOptionGraphicsItem* options, QWidget* widget);
    QRectF boundingRect() const;

    virtual QPainterPath obtainCurvePath() const;
    virtual QPainterPath obtainCurvePath(QPointF startPoint, QPointF endPoint) const;

protected:
    QPointF startPoint_;
    QPointF endPoint_;
    QPointF midPoint_;
    bool useMidPoint_;

    QColor color_;
    QColor borderColor_;
    QColor selectedBorderColor_;

    QPainterPath path_;
    QRectF rect_;
};

class IVW_QTEDITOR_API ConnectionDragGraphicsItem : public CurveGraphicsItem {
public:
    ConnectionDragGraphicsItem(ProcessorOutportGraphicsItem* outport, QPointF endPoint,
                               uvec3 color = uvec3(38, 38, 38));
    ~ConnectionDragGraphicsItem();

    // Override
    virtual QPointF getStartPoint() const;

    void reactToPortHover(ProcessorInportGraphicsItem* inport);
    ProcessorOutportGraphicsItem* getOutportGraphicsItem() const;

    // override for qgraphicsitem_cast (refer qt documentation)
    enum { Type = UserType + ConnectionDragGraphicsType };
    int type() const { return Type; }

protected:
    ProcessorOutportGraphicsItem* outport_;
};

/**
 * Graphical representation of the connection between two ports in the network editor.
 */

class IVW_QTEDITOR_API ConnectionGraphicsItem : public ConnectionDragGraphicsItem {
public:
    /**
     * Construct a new graphical representation between the outport of the outProcessor
     * and the inport of the inProcessor. Data is assumed to flow from outport to inport.
     * While the processors are provided through their graphical representations
     * (@see ProcessorGraphicsItem), the ports are directly specified as used in the data
     * flow network.
     */
    ConnectionGraphicsItem(ProcessorOutportGraphicsItem* outport,
                           ProcessorInportGraphicsItem* inport, PortConnection* connection);

    ~ConnectionGraphicsItem();

    ProcessorGraphicsItem* getOutProcessor() const;
    ProcessorGraphicsItem* getInProcessor() const;
    Outport* getOutport() const;
    Inport* getInport() const;
    ProcessorInportGraphicsItem* getInportGraphicsItem() const;

    // override for qgraphicsitem_cast (refer qt documentation)
    enum { Type = UserType + ConnectionGraphicsType };
    int type() const { return Type; }

    // Override
    virtual QPointF getEndPoint() const;

    virtual void showToolTip(QGraphicsSceneHelpEvent* e);

private:
    ProcessorInportGraphicsItem* inport_;
    PortConnection* connection_;  //< Non-owning reference
};

}  // namespace

#endif  // IVW_CONNECTIONGRAPHICSITEM_H