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

#ifndef IVW_LINKDIALOG_CURVEGRAPHICSITEMS_H
#define IVW_LINKDIALOG_CURVEGRAPHICSITEMS_H

#include <inviwo/qt/editor/inviwoqteditordefine.h>
#include <inviwo/qt/editor/connectiongraphicsitem.h>
#include <inviwo/qt/editor/linkdialog/linkdialoggraphicsitems.h>
#include <inviwo/core/links/propertylink.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QPainterPath>
#include <warn/pop>

namespace inviwo {

class IVW_QTEDITOR_API DialogCurveGraphicsItem : public CurveGraphicsItem {
public:
    DialogCurveGraphicsItem(QPointF startPoint, QPointF endPoint,
                            QColor color = QColor(38, 38, 38));
    virtual ~DialogCurveGraphicsItem();

    // override for qgraphicsitem_cast (refer qt documentation)
    enum { Type = UserType + LinkDialogCurveGraphicsItemType };
    int type() const override { return Type; }

    // Override
    virtual QPointF getStartPoint() const override;
    virtual QPointF getEndPoint() const override;

    void setStartPoint(QPointF startPoint);
    void setEndPoint(QPointF endPoint);

    virtual QPainterPath obtainCurvePath(QPointF startPoint, QPointF endPoint) const override;

protected:
    QPointF startPoint_;
    QPointF endPoint_;
};

class LinkDialogPropertyGraphicsItem;
class LinkDialogProcessorGraphicsItem;

class IVW_QTEDITOR_API DialogConnectionGraphicsItem : public CurveGraphicsItem {
public:
    DialogConnectionGraphicsItem(LinkDialogPropertyGraphicsItem* startProperty,
                                 LinkDialogPropertyGraphicsItem* endProperty,
                                 const PropertyLink& propertyLink);
    virtual ~DialogConnectionGraphicsItem();

    const PropertyLink& getPropertyLink() const { return propertyLink_; }
    LinkDialogPropertyGraphicsItem* getStartProperty() const { return startPropertyGraphicsItem_; }
    LinkDialogPropertyGraphicsItem* getEndProperty() const { return endPropertyGraphicsItem_; }

    // Override
    virtual QPointF getStartPoint() const override;
    virtual QPointF getEndPoint() const override;

    virtual QPainterPath obtainCurvePath(QPointF startPoint, QPointF endPoint) const override;

    virtual void paint(QPainter* p, const QStyleOptionGraphicsItem* options,
                       QWidget* widget) override;

private:
    LinkDialogPropertyGraphicsItem* startPropertyGraphicsItem_;
    LinkDialogPropertyGraphicsItem* endPropertyGraphicsItem_;
    PropertyLink propertyLink_;
};

}  // namespace inviwo

#endif  // IVW_LINKDIALOG_CURVEGRAPHICSITEMS_H
