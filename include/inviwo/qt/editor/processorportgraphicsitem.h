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

#ifndef IVW_PROCESSORPORTGRAPHICSITEM_H
#define IVW_PROCESSORPORTGRAPHICSITEM_H

#include <inviwo/qt/editor/inviwoqteditordefine.h>
#include <inviwo/qt/editor/editorgrapicsitem.h>
#include <warn/push>
#include <warn/ignore/all>
#include <QEvent>
#include <QPointF>
#include <QColor>
#include <warn/pop>

namespace inviwo {

class Port;
class ConnectionGraphicsItem;
class ProcessorGraphicsItem;
class ProcessorPortGraphicsItem;

class IVW_QTEDITOR_API ProcessorPortConnectionIndicator : public EditorGraphicsItem {
public:
    ProcessorPortConnectionIndicator(ProcessorPortGraphicsItem* parent, bool up, QColor color);
    virtual ~ProcessorPortConnectionIndicator() = default;

protected:
    void paint(QPainter* p, const QStyleOptionGraphicsItem* options, QWidget* widget);

private:
    ProcessorPortGraphicsItem* portConnectionItem_;
    bool up_;
    QColor color_;
};

class IVW_QTEDITOR_API ProcessorPortGraphicsItem : public EditorGraphicsItem {
public:
    ProcessorPortGraphicsItem(ProcessorGraphicsItem* parent, const QPointF& pos, bool up,
                              Port* port);
    virtual ~ProcessorPortGraphicsItem();

    void addConnection(ConnectionGraphicsItem* connection);
    void removeConnection(ConnectionGraphicsItem* connection);
    std::vector<ConnectionGraphicsItem*>& getConnections();
    ProcessorGraphicsItem* getProcessor();
    virtual void showToolTip(QGraphicsSceneHelpEvent* e) override = 0;

protected:
    virtual QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;
    virtual void updateConnectionPositions() = 0;

    std::vector<ConnectionGraphicsItem*> connections_;
    ProcessorGraphicsItem* processor_;
    ProcessorPortConnectionIndicator* connectionIndicator_;
    float size_;
    float lineWidth_;
};

class IVW_QTEDITOR_API ProcessorInportGraphicsItem : public ProcessorPortGraphicsItem {
public:
    ProcessorInportGraphicsItem(ProcessorGraphicsItem* parent, const QPointF& pos, Inport* port);
    virtual ~ProcessorInportGraphicsItem() = default;
    Inport* getPort();

    // override for qgraphicsitem_cast (refer qt documentation)
    enum { Type = UserType + ProcessorInportGraphicsType };
    virtual int type() const override { return Type; }

    virtual void showToolTip(QGraphicsSceneHelpEvent* e) override;

protected:
    virtual void paint(QPainter* p, const QStyleOptionGraphicsItem* options,
                       QWidget* widget) override;
    virtual void mousePressEvent(QGraphicsSceneMouseEvent* e) override;
    virtual void updateConnectionPositions() override;
    Inport* inport_;
};

class IVW_QTEDITOR_API ProcessorOutportGraphicsItem : public ProcessorPortGraphicsItem {
public:
    ProcessorOutportGraphicsItem(ProcessorGraphicsItem* parent, const QPointF& pos, Outport* port);
    virtual ~ProcessorOutportGraphicsItem() = default;

    Outport* getPort();
    // override for qgraphicsitem_cast (refer qt documentation)
    enum { Type = UserType + ProcessorOutportGraphicsType };
    virtual int type() const override { return Type; }

    virtual void showToolTip(QGraphicsSceneHelpEvent* e) override;

protected:
    virtual void paint(QPainter* p, const QStyleOptionGraphicsItem* options,
                       QWidget* widget) override;
    virtual void mousePressEvent(QGraphicsSceneMouseEvent* e) override;

    virtual void updateConnectionPositions() override;
    Outport* outport_;
};

}  // namespace inviwo

#endif  // IVW_PROCESSORPORTGRAPHICSITEM_H
