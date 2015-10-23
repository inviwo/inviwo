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

#ifndef IVW_LINKDIALOG_PROPERTYGRAPHICSITEMS_H
#define IVW_LINKDIALOG_PROPERTYGRAPHICSITEMS_H

#include <inviwo/qt/editor/inviwoqteditordefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/links/linkconditions.h>
#include <inviwo/core/network/processornetwork.h>
#include <inviwo/qt/widgets/labelgraphicsitem.h>
#include <inviwo/qt/editor/connectiongraphicsitem.h>
#include <inviwo/qt/widgets/inviwodockwidget.h>
#include <inviwo/qt/editor/linkdialog/linkdialoggraphicsitems.h>

#include <warn/push>
#include <warn/ignore/all>
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
#include <warn/pop>

namespace inviwo {

class DialogConnectionGraphicsItem;
class LinkDialogProcessorGraphicsItem;
class LinkDialogGraphicsScene;

class IVW_QTEDITOR_API LinkDialogPropertyGraphicsItem : public GraphicsItemData<Property> {
public:
    LinkDialogPropertyGraphicsItem(LinkDialogParent* parent, Property* prop);
    ~LinkDialogPropertyGraphicsItem();

    QSizeF sizeHint(Qt::SizeHint which, const QSizeF& constraint = QSizeF()) const;

    // override for qgraphicsitem_cast (refer qt documentation)
    enum { Type = UserType + LinkDialogPropertyGraphicsItemType };
    virtual int type() const override { return Type; }

    QPointF getConnectionPoint();

    void setExpanded(bool expand);
    bool isExpanded() const;
    bool hasSubProperties() const;

    void setAnimate(bool animate);
    const bool getAnimate() const;

    virtual int getLevel() const override;
    size_t getTotalVisibleChildCount() const;

    QPointF calculateArrowCenter(size_t curPort) const;

    void prepareGeometryChange() { QGraphicsItem::prepareGeometryChange(); }

    void addConnectionGraphicsItem(DialogConnectionGraphicsItem*);
    size_t getConnectionGraphicsItemCount() const;
    size_t getConnectionIndex(const DialogConnectionGraphicsItem* item) const;
    void removeConnectionGraphicsItem(DialogConnectionGraphicsItem*);
    const std::vector<DialogConnectionGraphicsItem*>& getConnectionGraphicsItems() const;
    std::vector<LinkDialogPropertyGraphicsItem*> getSubPropertyItemList(
        bool recursive = false) const;

    virtual void updatePositions() override;
    void showToolTip(QGraphicsSceneHelpEvent* event);

protected:
    QPointF calculateArrowCenterLocal(size_t curPort) const;
    QRectF calculateArrowRect(size_t curPort) const;
    QRectF calculateArrowRect(DialogConnectionGraphicsItem* cItem) const;

    virtual void paint(QPainter* p, const QStyleOptionGraphicsItem* options, QWidget* widget) override;
    virtual QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;

    virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* e) override;

private:
    bool isExpanded_;
    bool animateEnabled_;

    LinkDialogParent* parent_;

    std::vector<DialogConnectionGraphicsItem*> connections_;
    std::vector<LinkDialogPropertyGraphicsItem*> subProperties_;
};

}  // namespace

#endif  // IVW_LINKDIALOG_PROPERTYGRAPHICSITEMS_H
