/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2024 Inviwo Foundation
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
#pragma once

#include <inviwo/qt/editor/inviwoqteditordefine.h>
#include <inviwo/qt/editor/editorgrapicsitem.h>

#include <QGraphicsRectItem>
#include <QString>
#include <QPointF>

#include <string_view>

namespace inviwo {

class IVW_QTEDITOR_API ProcessorErrorItem : public QGraphicsRectItem {
public:
    ProcessorErrorItem(QGraphicsItem* parent);

    virtual void paint(QPainter* p, const QStyleOptionGraphicsItem* options,
                       QWidget* widget) override;

    void setText(std::string_view error);
    void clear();
    void setActive(bool active);

    QString text() const;

    void mousePressEvent(QGraphicsSceneMouseEvent* e) override;

    // override for qgraphicsitem_cast (refer qt documentation)
    enum { Type = static_cast<int>(UserType) + static_cast<int>(ProcessorErrorItemType) };
    virtual int type() const override { return Type; }

    static constexpr QPointF offset{3.0f, -3.0f};

protected:
    virtual QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;

    QGraphicsSimpleTextItem* text_;
    QGraphicsLineItem* line_;
    bool hasError_;
    bool active_;
    bool pressing_;
};

}  // namespace inviwo
