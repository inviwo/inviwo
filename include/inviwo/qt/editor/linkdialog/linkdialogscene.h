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

#ifndef IVW_LINKDIALOG_SCENE_H
#define IVW_LINKDIALOG_SCENE_H

#include <inviwo/qt/editor/inviwoqteditordefine.h>
#include <inviwo/core/network/processornetworkobserver.h>
#include <QMouseEvent>
#include <QGraphicsSceneWheelEvent>
#include <QGraphicsScene>
#include <QGraphicsItem>
#include <QGraphicsSceneMouseEvent>

namespace inviwo {

class DialogConnectionGraphicsItem;
class DialogCurveGraphicsItem;
class LinkDialogPropertyGraphicsItem;
class LinkDialogProcessorGraphicsItem;

class ProcessorNetwork;
class Processor;
class Property;
class PropertyOwner;
class PropertyLink;

class IVW_QTEDITOR_API LinkDialogGraphicsScene : public QGraphicsScene,
                                                 public ProcessorNetworkObserver {
    Q_OBJECT
public:
    LinkDialogGraphicsScene(QWidget* parent);
    virtual ~LinkDialogGraphicsScene();

    template <typename T>
    T* getSceneGraphicsItemAt(const QPointF pos,
                              const Qt::ItemSelectionMode mode = Qt::IntersectsItemShape,
                              Qt::SortOrder order = Qt::DescendingOrder) const;

    void setNetwork(ProcessorNetwork* network);
    ProcessorNetwork* getNetwork();

    void initScene(Processor* srcProcessor, Processor* dstProcessor);
    void clearSceneRepresentations();

    void removeCurrentPropertyLinks();
    void removeAllPropertyLinks();

    void addPropertyLink(Property* srcProperty, Property* dstProperty, bool bidirectional);
    int currentLinkItemsCount();
    void setExpandProperties(bool expand);
    void expandOrCollapseLinkedProcessorItems(
        LinkDialogProcessorGraphicsItem* processorGraphicsItem, bool expand);

    void expandOrCollapseLinkedPropertyItems(LinkDialogPropertyGraphicsItem* propertyItem,
                                             bool expand);

    void updatePropertyItemsOfAllProcessors();
    bool isPropertyExpanded(Property* property);

    virtual void onProcessorNetworkDidAddLink(PropertyLink* propertyLink);
    virtual void onProcessorNetworkDidRemoveLink(PropertyLink* propertyLink);
    virtual void onProcessorNetworkWillRemoveProcessor(Processor* processor);

signals:
    void closeDialog();

protected:
    // Overload qt events
    virtual void mousePressEvent(QGraphicsSceneMouseEvent* e);
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent* e);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent* e);
    virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* e);
    virtual void keyPressEvent(QKeyEvent* keyEvent);
    virtual void wheelEvent(QGraphicsSceneWheelEvent* e);
    virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent* e);


    void removePropertyLink(DialogConnectionGraphicsItem* propertyLink);

    void cleanupAfterRemoveLink(DialogConnectionGraphicsItem* propertyLink);

    bool isPropertyLinkBidirectional(DialogConnectionGraphicsItem* propertyLink);
    void makePropertyLinkBidirectional(DialogConnectionGraphicsItem* propertyLink,
                                       bool isBidirectional);
    void switchPropertyLinkDirection(DialogConnectionGraphicsItem* propertyLink);

    DialogConnectionGraphicsItem* initializePropertyLinkRepresentation(PropertyLink* propLink);
    void removePropertyLinkRepresentation(PropertyLink* propLink);

    LinkDialogProcessorGraphicsItem* addProcessorsItemsToScene(Processor* prcoessor, int xPosition,
                                                               int yPosition);
    DialogConnectionGraphicsItem* getConnectionGraphicsItem(LinkDialogPropertyGraphicsItem*,
                                                            LinkDialogPropertyGraphicsItem*);

    // smooth scroll effect support
    void offsetItems(float yIncrement, bool scrollLeft);
    int currentScrollSteps_;
private slots:
    void executeTimeLine(qreal);
    void terminateTimeLine();

private:
    QGraphicsItem* getPropertyGraphicsItemOf(Property* property);
    void addConnectionToCurrentList(DialogConnectionGraphicsItem*);
    void removeConnectionFromCurrentList(DialogConnectionGraphicsItem*);

    DialogCurveGraphicsItem* linkCurve_;
    LinkDialogPropertyGraphicsItem* startProperty_;
    LinkDialogPropertyGraphicsItem* endProperty_;

    LinkDialogProcessorGraphicsItem* srcProcessorGraphicsItem_;
    LinkDialogProcessorGraphicsItem* dstProcessorGraphicsItem_;
    std::vector<DialogConnectionGraphicsItem*> connectionGraphicsItems_;
    std::vector<DialogConnectionGraphicsItem*> currentConnectionGraphicsItems_;

    ProcessorNetwork* processorNetwork_;

    bool expandProperties_;
    bool mouseOnLeftSide_;

    std::map<Property*, LinkDialogPropertyGraphicsItem*> propertyGraphicsItemCache_;
};

template <typename T>
T* LinkDialogGraphicsScene::getSceneGraphicsItemAt(const QPointF pos,
                                                   const Qt::ItemSelectionMode mode,
                                                   Qt::SortOrder order) const {
    QList<QGraphicsItem*> graphicsItems = items(pos, mode, order);

    if (graphicsItems.size() > 0) {
        for (int i = 0; i < graphicsItems.size(); i++) {
            T* graphicsItem = qgraphicsitem_cast<T*>(graphicsItems[i]);
            if (graphicsItem) return graphicsItem;
        }
    }
    return nullptr;
}

}  // namespace

#endif  // IVW_LINKDIALOG_SCENE_H
