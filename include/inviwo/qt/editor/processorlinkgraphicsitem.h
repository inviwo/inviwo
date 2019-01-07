/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2018 Inviwo Foundation
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

#ifndef IVW_PROCESSORLINKGRAPHICSITEM_H
#define IVW_PROCESSORLINKGRAPHICSITEM_H

#include <inviwo/qt/editor/inviwoqteditordefine.h>
#include <inviwo/qt/editor/editorgrapicsitem.h>
#include <warn/push>
#include <warn/ignore/all>
#include <QEvent>
#include <QPointF>
#include <warn/pop>

namespace inviwo {

class ProcessorGraphicsItem;
class LinkConnectionGraphicsItem;

class IVW_QTEDITOR_API ProcessorLinkGraphicsItem : public QGraphicsItem {
public:
    ProcessorLinkGraphicsItem(ProcessorGraphicsItem* parent);
    virtual ~ProcessorLinkGraphicsItem() {}

    // override for qgraphicsitem_cast (refer qt documentation)
    enum { Type = UserType + ProcessorLinkGraphicsType };
    int type() const override { return Type; }

    ProcessorGraphicsItem* getProcessorGraphicsItem() const;

    void addLink(LinkConnectionGraphicsItem* link);
    void removeLink(LinkConnectionGraphicsItem* link);

    QPointF getLeftPos() const;
    QPointF getRightPos() const;

protected:
    void updateLinkPositions();
    virtual QRectF boundingRect() const override;
    virtual void paint(QPainter* p, const QStyleOptionGraphicsItem* options,
                       QWidget* widget) override;
    // events
    virtual QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;

private:
    class IVW_QTEDITOR_API LinkItem : public EditorGraphicsItem {
    public:
        LinkItem(ProcessorLinkGraphicsItem* parent, QPointF pos, float angle);
        virtual ~LinkItem();

    protected:
        virtual void paint(QPainter* p, const QStyleOptionGraphicsItem* options,
                           QWidget* widget) override;

        // events
        virtual void mousePressEvent(QGraphicsSceneMouseEvent* e) override;

    private:
        ProcessorLinkGraphicsItem* parent_;
        const QPointF pos_;
        const float angle_;
        const float size_;
        const float lineWidth_;
    };

    ProcessorGraphicsItem* processor_;
    LinkItem* leftItem_;
    LinkItem* rightItem_;

    std::vector<LinkConnectionGraphicsItem*> links_;
};

}  // namespace inviwo

#endif  // IVW_PROCESSORLINKGRAPHICSITEM_H
