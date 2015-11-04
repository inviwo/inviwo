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

#ifndef IVW_TRANSFERFUNCTIONCONTROLPOINTCONNECTION_H
#define IVW_TRANSFERFUNCTIONCONTROLPOINTCONNECTION_H

#include <inviwo/qt/widgets/inviwoqtwidgetsdefine.h>
#include <inviwo/qt/widgets/properties/propertywidgetqt.h>
#include <warn/push>
#include <warn/ignore/all>
#include <QGraphicsItem>
#include <QPainterPath>
#include <QPointF>
#include <warn/pop>

namespace inviwo {
class TransferFunctionEditorControlPoint;

class IVW_QTWIDGETS_API TransferFunctionControlPointConnection : public QGraphicsItem {
public:
    TransferFunctionControlPointConnection();
    virtual ~TransferFunctionControlPointConnection();

    void updateShape();

    // override for qgraphicsitem_cast (refer qt documentation)
    enum { Type = UserType + TransferFunctionControlPointConnectionType };
    int type() const { return Type; }

    friend IVW_QTWIDGETS_API bool operator==(const TransferFunctionControlPointConnection& lhs,
                           const TransferFunctionControlPointConnection& rhs);

    // Compare points by their "x" value
    friend IVW_QTWIDGETS_API bool operator<(const TransferFunctionControlPointConnection& lhs,
                          const TransferFunctionControlPointConnection& rhs);

    TransferFunctionEditorControlPoint* left_;   // Non-owning reference
    TransferFunctionEditorControlPoint* right_;  // Non-owning reference

    QPointF getStart() const;
    QPointF getStop() const;

protected:
    // Overload
    void paint(QPainter* p, const QStyleOptionGraphicsItem* options, QWidget* widget);
    QRectF boundingRect() const;
    QPainterPath shape() const;

private:
    QPainterPath path_;
    QPainterPath shape_;
    QRectF rect_;
};

IVW_QTWIDGETS_API bool operator==(const TransferFunctionControlPointConnection& lhs,
                                  const TransferFunctionControlPointConnection& rhs);
IVW_QTWIDGETS_API bool operator!=(const TransferFunctionControlPointConnection& lhs,
                                  const TransferFunctionControlPointConnection& rhs);
IVW_QTWIDGETS_API bool operator<(const TransferFunctionControlPointConnection& lhs,
                                 const TransferFunctionControlPointConnection& rhs);
IVW_QTWIDGETS_API bool operator>(const TransferFunctionControlPointConnection& lhs,
                                 const TransferFunctionControlPointConnection& rhs);
IVW_QTWIDGETS_API bool operator<=(const TransferFunctionControlPointConnection& lhs,
                                  const TransferFunctionControlPointConnection& rhs);
IVW_QTWIDGETS_API bool operator>=(const TransferFunctionControlPointConnection& lhs,
                                  const TransferFunctionControlPointConnection& rhs);

}  // namespace

#endif  // IVW_TRANSFERFUNCTIONCONTROLPOINTCONNECTION_H
