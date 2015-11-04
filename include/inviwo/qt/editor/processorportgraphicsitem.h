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

#ifndef IVW_PROCESSORPORTGRAPHICSITEM_H
#define IVW_PROCESSORPORTGRAPHICSITEM_H

#include <inviwo/qt/editor/inviwoqteditordefine.h>
#include <inviwo/qt/editor/editorgrapicsitem.h>
#include <warn/push>
#include <warn/ignore/all>
#include <QEvent>
#include <QPointF>
#include <warn/pop>

namespace inviwo {

class Port;
class ConnectionGraphicsItem;
class ProcessorGraphicsItem;

class IVW_QTEDITOR_API ProcessorPortGraphicsItem : public EditorGraphicsItem {
public:
    ProcessorPortGraphicsItem(ProcessorGraphicsItem* parent, const QPointF& pos, bool up,
                              Port* port);
    virtual ~ProcessorPortGraphicsItem();

    void addConnection(ConnectionGraphicsItem* connection);
    void removeConnection(ConnectionGraphicsItem* connection);
    std::vector<ConnectionGraphicsItem*> getConnections();
    ProcessorGraphicsItem* getProcessor();
    virtual void showToolTip(QGraphicsSceneHelpEvent* e);

protected:
    void paint(QPainter* p, const QStyleOptionGraphicsItem* options, QWidget* widget);
    // events
    virtual QVariant itemChange(GraphicsItemChange change, const QVariant& value);

    // Members
    std::vector<ConnectionGraphicsItem*> connections_;

private:
    virtual void updateConnectionPositions() = 0;
    ProcessorGraphicsItem* processor_;
    Port* port_;
    float size_;
    float lineWidth_;
    bool up_;
};

class IVW_QTEDITOR_API ProcessorInportGraphicsItem : public ProcessorPortGraphicsItem {
public:
    ProcessorInportGraphicsItem(ProcessorGraphicsItem* parent, const QPointF& pos, Inport* port);
    virtual ~ProcessorInportGraphicsItem() {}
    Inport* getPort();

    // override for qgraphicsitem_cast (refer qt documentation)
    enum { Type = UserType + ProcessorInportGraphicsType };
    int type() const { return Type; }

    virtual void showToolTip(QGraphicsSceneHelpEvent* e);

protected:   
    // events
    virtual void mousePressEvent(QGraphicsSceneMouseEvent* e);

private:
    virtual void updateConnectionPositions();
    Inport* port_;
};

class IVW_QTEDITOR_API ProcessorOutportGraphicsItem : public ProcessorPortGraphicsItem {
public:
    ProcessorOutportGraphicsItem(ProcessorGraphicsItem* parent, const QPointF& pos, Outport* port);
    virtual ~ProcessorOutportGraphicsItem() {}

    Outport* getPort();
    // override for qgraphicsitem_cast (refer qt documentation)
    enum { Type = UserType + ProcessorOutportGraphicsType };
    int type() const { return Type; }

protected:
    // events
    virtual void mousePressEvent(QGraphicsSceneMouseEvent* e);

private:
    virtual void updateConnectionPositions();
    Outport* port_;
};

}  // namespace

#endif  // IVW_PROCESSORPORTGRAPHICSITEM_H
