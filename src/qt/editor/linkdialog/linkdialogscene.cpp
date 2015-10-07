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

#include <inviwo/qt/editor/linkdialog/linkdialogscene.h>
#include <inviwo/qt/editor/linkdialog/linkdialoggraphicsitems.h>
#include <inviwo/qt/editor/linkdialog/linkdialogprocessorgraphicsitems.h>
#include <inviwo/qt/editor/linkdialog/linkdialogpropertygraphicsitems.h>
#include <inviwo/qt/editor/linkdialog/linkdialogcurvegraphicsitems.h>

#include <inviwo/core/network/processornetwork.h>
#include <inviwo/core/network/processornetworkobserver.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/properties/propertyowner.h>
#include <inviwo/core/properties/property.h>
#include <inviwo/core/links/propertylink.h>

#include <QTimeLine>

namespace inviwo {

LinkDialogGraphicsScene::LinkDialogGraphicsScene(QWidget* parent)
    : QGraphicsScene(parent)
    , currentScrollSteps_(0)
    , linkCurve_(nullptr)
    , startProperty_(nullptr)
    , endProperty_(nullptr)
    , processorNetwork_(nullptr)
    , expandProperties_(false)
    , mouseOnLeftSide_(false) {

    // The defalt bsp tends to crash...  
    setItemIndexMethod(QGraphicsScene::NoIndex);
}

LinkDialogGraphicsScene::~LinkDialogGraphicsScene() {
    // We need to make sure to delete the links first. since they referer to the properties
    for (auto& elem : connectionGraphicsItems_) {
        delete elem;
    }
    currentConnectionGraphicsItems_.clear();
    connectionGraphicsItems_.clear();

    delete srcProcessorGraphicsItem_;
    delete dstProcessorGraphicsItem_;
}

QGraphicsItem* LinkDialogGraphicsScene::getPropertyGraphicsItemOf(Property* property) {
    std::map<Property*, LinkDialogPropertyGraphicsItem*>::iterator it =
        propertyGraphicsItemCache_.find(property);
    if (it != propertyGraphicsItemCache_.end()) return it->second;

    LinkDialogPropertyGraphicsItem* graphicsItem = nullptr;
    std::vector<LinkDialogProcessorGraphicsItem*> processorGraphicsItems;
    processorGraphicsItems.push_back(srcProcessorGraphicsItem_);
    processorGraphicsItems.push_back(dstProcessorGraphicsItem_);

    for (auto propertyItems : processorGraphicsItems) {
        for (auto& propertyItem : propertyItems->getPropertyItemList()) {
            if (propertyItem->getGraphicsItemData() == property) {
                graphicsItem = propertyItem;
                break;
            }
        }
        if (graphicsItem) break;
    }

    if (!graphicsItem) {
        //This is slightly expensive search. So do it only if you don't find required item in immediate children
        for (auto propertyItems : processorGraphicsItems) {
            for (auto& propertyItem : propertyItems->getPropertyItemList()) {
                std::vector<LinkDialogPropertyGraphicsItem*> subPropertyItems =
                    propertyItem->getSubPropertyItemList(true);
                for (auto& subPropertyItem : subPropertyItems) {
                    if (subPropertyItem->getGraphicsItemData() == property) {
                        graphicsItem = subPropertyItem;
                        break;
                    }
                }
                if (graphicsItem) break;
            }
            if (graphicsItem) break;
        }
    }

    if (graphicsItem) {
        propertyGraphicsItemCache_[property] = graphicsItem;
    }

    return graphicsItem;
}

void LinkDialogGraphicsScene::setNetwork(ProcessorNetwork* network) {
    if (processorNetwork_) processorNetwork_->removeObserver(this);
    processorNetwork_ = network;
    if (processorNetwork_) processorNetwork_->addObserver(this);
}

ProcessorNetwork* LinkDialogGraphicsScene::getNetwork() { return processorNetwork_; }

void LinkDialogGraphicsScene::mousePressEvent(QGraphicsSceneMouseEvent* e) {

    LinkDialogPropertyGraphicsItem* tempProperty =
        getSceneGraphicsItemAt<LinkDialogPropertyGraphicsItem>(e->scenePos());

    if (!startProperty_ && tempProperty) {
        startProperty_ = tempProperty;

        if (linkCurve_) {
            linkCurve_->hide();
            removeItem(linkCurve_);
            delete linkCurve_;
            linkCurve_ = 0;
        }

        QPointF center = startProperty_->getShortestBoundaryPointTo(e->scenePos());
        linkCurve_ = new DialogCurveGraphicsItem(center, e->scenePos());
        linkCurve_->setZValue(2.0);
        addItem(linkCurve_);
        linkCurve_->show();
        e->accept();
    } else
        QGraphicsScene::mousePressEvent(e);

    return;
}

void LinkDialogGraphicsScene::mouseMoveEvent(QGraphicsSceneMouseEvent* e) {

    QPointF pos = e->scenePos();
    if (pos.x() > linkDialogWidth/2.0 ) {
        mouseOnLeftSide_ = false;
    }
    else {
        mouseOnLeftSide_ = true;
    }

    if (linkCurve_) {
        QPointF center = startProperty_->getShortestBoundaryPointTo(e->scenePos());
        linkCurve_->setStartPoint(center) ;
        linkCurve_->setEndPoint(e->scenePos());
        e->accept();
    } else
        QGraphicsScene::mouseMoveEvent(e);
}

void LinkDialogGraphicsScene::mouseReleaseEvent(QGraphicsSceneMouseEvent* e) {
    if (linkCurve_) {
        delete linkCurve_;
        linkCurve_ = nullptr;
        endProperty_ = getSceneGraphicsItemAt<LinkDialogPropertyGraphicsItem>(e->scenePos());

        if (endProperty_ && (endProperty_!=startProperty_)) {
            Property* sProp = startProperty_->getGraphicsItemData();
            Property* eProp = endProperty_->getGraphicsItemData();

            if (sProp->getOwner()->getProcessor()!=eProp->getOwner()->getProcessor()) {
                bool src2dst = SimpleCondition::canLink(sProp, eProp);
                bool dst2src = SimpleCondition::canLink(eProp, sProp);
                if (src2dst &&dst2src)
                    addPropertyLink(sProp, eProp, true);
                else if (src2dst)
                    addPropertyLink(sProp, eProp, false);
                else if (dst2src)
                    addPropertyLink(eProp, sProp, false);
            }
        }

        startProperty_ = nullptr;
        endProperty_ = nullptr;
        e->accept();
    } else {
        QGraphicsScene::mouseReleaseEvent(e);
    }

    startProperty_ = nullptr;
    endProperty_ = nullptr;
}

void LinkDialogGraphicsScene::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* e) {
    LinkDialogPropertyGraphicsItem* propertyItem =
        getSceneGraphicsItemAt<LinkDialogPropertyGraphicsItem>(e->scenePos());

    if (propertyItem) {
        DialogConnectionGraphicsItem* propertyLink =
            propertyItem->getArrowConnectionAt(e->scenePos());

        if (propertyLink) {
            if (isPropertyLinkBidirectional(propertyLink)) {
                makePropertyLinkBidirectional(propertyLink, false);

                if (propertyLink->getStartProperty() == propertyItem)
                    switchPropertyLinkDirection(propertyLink);
            } else {
                if (propertyLink->getStartProperty() == propertyItem)
                    makePropertyLinkBidirectional(propertyLink, true);
            }
        }
        else {

            bool expand = false;
            if (propertyItem->isExpanded()) expand = true;

            expandOrCollapseLinkedPropertyItems(propertyItem, !expand);
        }

        e->accept();
    } else
        QGraphicsScene::mouseDoubleClickEvent(e);
}


void LinkDialogGraphicsScene::keyPressEvent(QKeyEvent* keyEvent) {
    if (keyEvent->key() == Qt::Key_Delete) {
        QList<QGraphicsItem*> selectedGraphicsItems = selectedItems();

        for (auto& selectedGraphicsItem : selectedGraphicsItems) {
            DialogConnectionGraphicsItem* connectionGraphicsItem =
                qgraphicsitem_cast<DialogConnectionGraphicsItem*>(selectedGraphicsItem);

            if (connectionGraphicsItem) {
                removeConnectionFromCurrentList(connectionGraphicsItem);
                removePropertyLink(connectionGraphicsItem);
            }
        }
    }

    QGraphicsScene::keyPressEvent(keyEvent);
}

void LinkDialogGraphicsScene::contextMenuEvent(QGraphicsSceneContextMenuEvent* e) {
    DialogConnectionGraphicsItem* linkGraphicsItem =
        getSceneGraphicsItemAt<DialogConnectionGraphicsItem>(e->scenePos());

    if (linkGraphicsItem && linkGraphicsItem->getPropertyLink()) {
        QMenu menu;
        QAction* deleteAction = menu.addAction("Delete");
        QAction* biDirectionAction = menu.addAction("BiDirectional");
        biDirectionAction->setCheckable(true);
        QAction* switchDirection = menu.addAction("Switch Direction");

        if (isPropertyLinkBidirectional(linkGraphicsItem))
            biDirectionAction->setChecked(true);
        else
            biDirectionAction->setChecked(false);

        QAction* result = menu.exec(QCursor::pos());

        if (result == deleteAction) {
            removeConnectionFromCurrentList(linkGraphicsItem);
            removePropertyLink(linkGraphicsItem);
        } else if (result == biDirectionAction) {
            if (biDirectionAction->isChecked())
                makePropertyLinkBidirectional(linkGraphicsItem, true);
            else
                makePropertyLinkBidirectional(linkGraphicsItem, false);
        } else if (result == switchDirection)
            switchPropertyLinkDirection(linkGraphicsItem);
    }
}

void LinkDialogGraphicsScene::wheelEvent(QGraphicsSceneWheelEvent* e) {
    //note:delta can be positive or negative (wheel rotated away from user or towards user)
    int numDegrees = e->delta() / 8;
    int numSteps = numDegrees / 15;

    currentScrollSteps_ = numSteps;

    QTimeLine *anim = new QTimeLine(750, this);
    anim->setUpdateInterval(20);
    connect(anim, SIGNAL(valueChanged(qreal)), SLOT(executeTimeLine(qreal)));
    connect(anim, SIGNAL(finished()), SLOT(terminateTimeLine()));
    anim->start();
    e->accept();
    //QGraphicsScene::wheelEvent(e);
}

void LinkDialogGraphicsScene::offsetItems(float yIncrement, bool scrollLeft) {
    QPointF scrollOffset(0.0f, yIncrement);

    std::vector<LinkDialogProcessorGraphicsItem*> processorGraphicsItems;
    processorGraphicsItems.push_back(srcProcessorGraphicsItem_);
    processorGraphicsItems.push_back(dstProcessorGraphicsItem_);

    for(auto procGraphicsItem : processorGraphicsItems) {
        QPointF pos = procGraphicsItem->scenePos();
        if (scrollLeft && pos.x()>=linkDialogWidth/2.0) continue;
        if (!scrollLeft && pos.x()<linkDialogWidth/2.0) continue;
        procGraphicsItem->setPos(pos.x()+scrollOffset.x(), pos.y()+scrollOffset.y());
        std::vector<LinkDialogPropertyGraphicsItem*> propItems = procGraphicsItem->getPropertyItemList();
        for (auto& propItem : propItems) {
            propItem->updatePositionBasedOnIndex();
            const std::vector<DialogConnectionGraphicsItem*> connections =
                propItem->getConnectionGraphicsItems();
            for (auto& connection : connections) {
                connection->updateConnectionDrawing();
            }
        }
    }

    update();
}

void LinkDialogGraphicsScene::executeTimeLine(qreal x) {
    float yIncrement = processorItemHeight*(0.09f)*(currentScrollSteps_);
    offsetItems(yIncrement, mouseOnLeftSide_);
}

void LinkDialogGraphicsScene::terminateTimeLine() {
    delete sender();
}


void LinkDialogGraphicsScene::addPropertyLink(Property* sProp, Property* eProp,
                                              bool bidirectional) {
    
    processorNetwork_->addLink(sProp, eProp);
    processorNetwork_->evaluatePropertyLinks(sProp);
    
    if (bidirectional) processorNetwork_->addLink(eProp, sProp);
}

int LinkDialogGraphicsScene::currentLinkItemsCount() {
    return static_cast<int>(currentConnectionGraphicsItems_.size());
}

void LinkDialogGraphicsScene::setExpandProperties(bool expand) { 
    if (expand!=expandProperties_) {
        expandProperties_ = expand;
    }
}

void LinkDialogGraphicsScene::expandOrCollapseLinkedProcessorItems(
    LinkDialogProcessorGraphicsItem* procGraphicsItem, bool expand) {
    for(auto propertyItem : procGraphicsItem->getPropertyItemList()) {
        expandOrCollapseLinkedPropertyItems(propertyItem, expand);
    }
}

void LinkDialogGraphicsScene::expandOrCollapseLinkedPropertyItems(
    LinkDialogPropertyGraphicsItem* propertyItem, bool expand) {
    if (propertyItem->hasSubProperties()) {
        propertyItem->setAnimate(true);
        if (expand)
            propertyItem->expand(true);
        else
            propertyItem->collapse(true);

        std::vector<LinkDialogPropertyGraphicsItem*> subProps =
            propertyItem->getSubPropertyItemList(true);
        for (auto& subProp : subProps) {
            std::vector<Property*> linkedSubProps =
                processorNetwork_->getLinkedProperties(subProp->getGraphicsItemData());
            for (auto& linkedSubProp : linkedSubProps) {
                Property* parentProperty = dynamic_cast<Property*>(linkedSubProp->getOwner());
                if (parentProperty && (linkedSubProp->getOwner()->getProcessor() ==
                                           dstProcessorGraphicsItem_->getProcessor() ||
                                       linkedSubProp->getOwner()->getProcessor() ==
                                           srcProcessorGraphicsItem_->getProcessor())) {
                    LinkDialogPropertyGraphicsItem* endP =
                        qgraphicsitem_cast<LinkDialogPropertyGraphicsItem*>(
                            getPropertyGraphicsItemOf(parentProperty));
                    if (endP) {
                        endP->setAnimate(true);
                        if (expand)
                            endP->expand(true);
                        else
                            endP->collapse(true);
                    } else
                        LogWarn("Required property graphics item not found.")
                }
            }
        }

        std::vector<LinkDialogProcessorGraphicsItem*> processorGraphicsItems;
        processorGraphicsItems.push_back(srcProcessorGraphicsItem_);
        processorGraphicsItems.push_back(dstProcessorGraphicsItem_);

        for(auto procGraphicsItem : processorGraphicsItems) {
            procGraphicsItem->updatePropertyItemPositions(true);
        }
    }
}

bool LinkDialogGraphicsScene::isPropertyExpanded(Property* property) {
    LinkDialogPropertyGraphicsItem* propItem =
        qgraphicsitem_cast<LinkDialogPropertyGraphicsItem*>(getPropertyGraphicsItemOf(property));
    if(propItem) return propItem->isExpanded();
    return false;
}
void LinkDialogGraphicsScene::removeCurrentPropertyLinks() {
    std::vector<DialogConnectionGraphicsItem*> tempList = currentConnectionGraphicsItems_;
    for (auto propertyLink : currentConnectionGraphicsItems_) {
        removeConnectionFromCurrentList(propertyLink);
    }
    for (auto propertyLink : tempList) {
        removePropertyLink(propertyLink);
    }
}

void LinkDialogGraphicsScene::removeAllPropertyLinks() {
    std::vector<DialogConnectionGraphicsItem*> tempList = connectionGraphicsItems_;
    for (auto propertyLink : connectionGraphicsItems_) {
        removeConnectionFromCurrentList(propertyLink);
    }
    for (auto propertyLink : tempList) {
        removePropertyLink(propertyLink);
    }
}

void LinkDialogGraphicsScene::removePropertyLink(DialogConnectionGraphicsItem* propertyLink) {
    Property* start = propertyLink->getStartProperty()->getGraphicsItemData();
    Property* end = propertyLink->getEndProperty()->getGraphicsItemData();

    processorNetwork_->removeLink(start, end);
    processorNetwork_->removeLink(end, start);
}

void LinkDialogGraphicsScene::cleanupAfterRemoveLink(DialogConnectionGraphicsItem* propertyLink) {
    LinkDialogPropertyGraphicsItem* startProperty = propertyLink->getStartProperty();
    LinkDialogPropertyGraphicsItem* endProperty = propertyLink->getEndProperty();

    //re-assign connection ids
    for (auto& elem : connectionGraphicsItems_) {
        if (elem == propertyLink)
            continue;
        else if (elem->getStartProperty() == startProperty) {
            size_t index = elem->getStartArrowHeadIndex();

            if (index > propertyLink->getStartArrowHeadIndex())
                elem->setStartArrowHeadIndex(index - 1);
        } else if (elem->getEndProperty() == startProperty) {
            size_t index = elem->getEndArrowHeadIndex();

            if (index > propertyLink->getStartArrowHeadIndex())
                elem->setEndArrowHeadIndex(index - 1);
        }
    }

    for (auto& elem : connectionGraphicsItems_) {
        if (elem == propertyLink)
            continue;
        else if (elem->getEndProperty() == endProperty) {
            size_t index = elem->getEndArrowHeadIndex();

            if (index > propertyLink->getEndArrowHeadIndex()) elem->setEndArrowHeadIndex(index - 1);
        } else if (elem->getStartProperty() == endProperty) {
            size_t index = elem->getStartArrowHeadIndex();

            if (index > propertyLink->getEndArrowHeadIndex())
                elem->setStartArrowHeadIndex(index - 1);
        }
    }

    for (auto& elem : connectionGraphicsItems_) elem->updateConnectionDrawing();

    update();
}

bool LinkDialogGraphicsScene::isPropertyLinkBidirectional(
    DialogConnectionGraphicsItem* propertyLink) {
    LinkDialogPropertyGraphicsItem* startProperty = propertyLink->getStartProperty();
    LinkDialogPropertyGraphicsItem* endProperty = propertyLink->getEndProperty();

    return processorNetwork_->isLinkedBidirectional(startProperty->getGraphicsItemData(),
                                                    endProperty->getGraphicsItemData());
}

void LinkDialogGraphicsScene::makePropertyLinkBidirectional(
    DialogConnectionGraphicsItem* propertyLink, bool isBidirectional) {
    LinkDialogPropertyGraphicsItem* startProperty = propertyLink->getStartProperty();
    LinkDialogPropertyGraphicsItem* endProperty = propertyLink->getEndProperty();

    PropertyLink* propLink = processorNetwork_->getLink(endProperty->getGraphicsItemData(),
                                                        startProperty->getGraphicsItemData());
    if (isBidirectional) {
        if (!propLink)
            processorNetwork_->addLink(endProperty->getGraphicsItemData(),
                                       startProperty->getGraphicsItemData());
    } else {
        if (propLink)
            processorNetwork_->removeLink(endProperty->getGraphicsItemData(),
                                          startProperty->getGraphicsItemData());
    }

    propertyLink->updateConnectionDrawing();
    update();
}

void LinkDialogGraphicsScene::switchPropertyLinkDirection(
    DialogConnectionGraphicsItem* propertyLink) {
    
    if (!propertyLink->isBidirectional()) {
        PropertyLink* link = propertyLink->getPropertyLink();
        Property* source = link->getSourceProperty();
        Property* destination = link->getDestinationProperty();
        processorNetwork_->removeLink(source, destination);
        processorNetwork_->addLink(destination, source);
    }
}

void LinkDialogGraphicsScene::addConnectionToCurrentList(
    DialogConnectionGraphicsItem* propertyLink) {
    if (std::find(currentConnectionGraphicsItems_.begin(), currentConnectionGraphicsItems_.end(),
                  propertyLink) == currentConnectionGraphicsItems_.end())
        currentConnectionGraphicsItems_.push_back(propertyLink);
}

void LinkDialogGraphicsScene::removeConnectionFromCurrentList(
    DialogConnectionGraphicsItem* propertyLink) {
    currentConnectionGraphicsItems_.erase(
        std::remove(currentConnectionGraphicsItems_.begin(), currentConnectionGraphicsItems_.end(),
                    propertyLink),
        currentConnectionGraphicsItems_.end());
}

DialogConnectionGraphicsItem* LinkDialogGraphicsScene::getConnectionGraphicsItem(
    LinkDialogPropertyGraphicsItem* outProperty, LinkDialogPropertyGraphicsItem* inProperty) {
    for (auto& elem : connectionGraphicsItems_) {
        if ((elem->getStartProperty() == outProperty && elem->getEndProperty() == inProperty) ||
            (elem->getStartProperty() == inProperty && elem->getEndProperty() == outProperty))
            return elem;
    }

    return nullptr;
}

DialogConnectionGraphicsItem* LinkDialogGraphicsScene::initializePropertyLinkRepresentation(
    PropertyLink* propertyLink) {
    
    LinkDialogPropertyGraphicsItem* start = qgraphicsitem_cast<LinkDialogPropertyGraphicsItem*>(
        getPropertyGraphicsItemOf(propertyLink->getSourceProperty()));
    LinkDialogPropertyGraphicsItem* end = qgraphicsitem_cast<LinkDialogPropertyGraphicsItem*>(
        getPropertyGraphicsItemOf(propertyLink->getDestinationProperty()));

    if (start == nullptr || end == nullptr) return nullptr;

    DialogConnectionGraphicsItem* cItem = nullptr;
    cItem = getConnectionGraphicsItem(start, end);

    if (!cItem) {
        cItem = new DialogConnectionGraphicsItem(start, end, propertyLink);
        addItem(cItem);
        connectionGraphicsItems_.push_back(cItem);
    }

    cItem->show();

    for (auto& elem : connectionGraphicsItems_) elem->updateConnectionDrawing();

    update();

    return cItem;
}

void LinkDialogGraphicsScene::removePropertyLinkRepresentation(PropertyLink* propertyLink) {
    DialogConnectionGraphicsItem* cItem = nullptr;
    std::vector<DialogConnectionGraphicsItem*>::iterator it;
    for (it = connectionGraphicsItems_.begin(); it != connectionGraphicsItems_.end(); ++it) {
        if((*it)->getPropertyLink() == propertyLink) {
            cItem = *it;
            break;
        }
    }
    
    if (cItem) {
        DialogConnectionGraphicsItem* cItem = *it;

        cItem->hide();
        connectionGraphicsItems_.erase(std::remove(connectionGraphicsItems_.begin(),
                                                   connectionGraphicsItems_.end(), cItem),
                                       connectionGraphicsItems_.end());

        currentConnectionGraphicsItems_.erase(
            std::remove(currentConnectionGraphicsItems_.begin(),
                        currentConnectionGraphicsItems_.end(), cItem),
            currentConnectionGraphicsItems_.end());

        LinkDialogPropertyGraphicsItem* start = cItem->getStartProperty();
        LinkDialogPropertyGraphicsItem* end = cItem->getEndProperty();

        removeItem(cItem);
        cleanupAfterRemoveLink(cItem);
        delete cItem;

        start->prepareGeometryChange();
        end->prepareGeometryChange();
    }
}

void LinkDialogGraphicsScene::initScene(Processor* srcProcessor,
                                        Processor* dstProcessor) {

    clearSceneRepresentations();

    int xPosition = linkDialogWidth / 4;
    int yPosition = processorItemHeight;
    int xIncrement = linkDialogWidth / 2;

    srcProcessorGraphicsItem_ = addProcessorsItemsToScene(srcProcessor, xPosition, yPosition);
    xPosition += xIncrement;
    dstProcessorGraphicsItem_ = addProcessorsItemsToScene(dstProcessor, xPosition, yPosition);

    //add links
    std::vector<PropertyLink*> propertyLinks =
        processorNetwork_->getLinksBetweenProcessors(srcProcessor, dstProcessor);
    for (auto& propertyLink : propertyLinks) {
        initializePropertyLinkRepresentation(propertyLink);
    }

    std::vector<LinkDialogProcessorGraphicsItem*> processorGraphicsItems;
    processorGraphicsItems.push_back(srcProcessorGraphicsItem_);
    processorGraphicsItems.push_back(dstProcessorGraphicsItem_);

    for(auto procGraphicsItem : processorGraphicsItems){
        expandOrCollapseLinkedProcessorItems(procGraphicsItem, expandProperties_);
    }

    currentConnectionGraphicsItems_.clear();
}

void LinkDialogGraphicsScene::clearSceneRepresentations() {
    srcProcessorGraphicsItem_ = nullptr;
    dstProcessorGraphicsItem_ = nullptr;

    for (auto& elem : connectionGraphicsItems_) elem->cleanup();

    for (auto& elem : currentConnectionGraphicsItems_) elem->cleanup();

    connectionGraphicsItems_.clear();
    currentConnectionGraphicsItems_.clear();
    propertyGraphicsItemCache_.clear();
    clear();
}

LinkDialogProcessorGraphicsItem* LinkDialogGraphicsScene::addProcessorsItemsToScene(
    Processor* processor, int xPosition, int yPosition) {
    LinkDialogProcessorGraphicsItem* procGraphicsItem = new LinkDialogProcessorGraphicsItem();

    procGraphicsItem->setPos(views().first()->mapToScene(xPosition, yPosition));
    procGraphicsItem->setProcessor(processor, expandProperties_);
    addItem(procGraphicsItem);
    procGraphicsItem->show();

    std::vector<LinkDialogPropertyGraphicsItem*> propItems =
        procGraphicsItem->getPropertyItemList();
    for (auto& propItem : propItems) addItem(propItem);

    return procGraphicsItem;
}

void LinkDialogGraphicsScene::onProcessorNetworkDidAddLink(PropertyLink* propertyLink) {
    DialogConnectionGraphicsItem* cItem = initializePropertyLinkRepresentation(propertyLink);
    if (cItem) addConnectionToCurrentList(cItem);
}

void LinkDialogGraphicsScene::onProcessorNetworkDidRemoveLink(PropertyLink* propertyLink) {
    removePropertyLinkRepresentation(propertyLink);
}

void LinkDialogGraphicsScene::onProcessorNetworkWillRemoveProcessor(Processor* processor) {
    if (processor == srcProcessorGraphicsItem_->getProcessor() || processor == dstProcessorGraphicsItem_->getProcessor()) {
        emit closeDialog();
    }
}

} //namespace