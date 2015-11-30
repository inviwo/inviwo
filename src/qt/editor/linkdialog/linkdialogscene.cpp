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
#include <inviwo/core/util/stdextensions.h>
#include <inviwo/qt/editor/linkdialog/linkdialoggraphicsitems.h>
#include <warn/push>
#include <warn/ignore/all>
#include <QTimeLine>
#include <warn/pop>

namespace inviwo {

LinkDialogGraphicsScene::LinkDialogGraphicsScene(QWidget* parent, ProcessorNetwork* network,
                                                 Processor* srcProcessor, Processor* dstProcessor)
    : QGraphicsScene(parent)
    , currentScrollSteps_(0)
    , linkCurve_(nullptr)
    , startProperty_(nullptr)
    , endProperty_(nullptr)
    , srcProcessor_(nullptr)
    , dstProcessor_(nullptr)
    , network_(network)
    , expandProperties_(false)
    , mouseOnLeftSide_(false) {
    // The default bsp tends to crash...
    setItemIndexMethod(QGraphicsScene::NoIndex);

    network_->addObserver(this);

    srcProcessor_ =
        new LinkDialogProcessorGraphicsItem(LinkDialogParent::Side::Left, srcProcessor);
    dstProcessor_ =
        new LinkDialogProcessorGraphicsItem(LinkDialogParent::Side::Right, dstProcessor);

    float xPos1 = linkdialog::dialogWidth / 10.0 + linkdialog::processorWidth/2;
    addItem(srcProcessor_);
    srcProcessor_->setPos(QPointF(xPos1, 2*linkdialog::processorHeight));
    srcProcessor_->show();

    float xPos2 = linkdialog::dialogWidth * 9.0 / 10.0 - linkdialog::processorWidth/2;
    addItem(dstProcessor_);
    dstProcessor_->setPos(QPointF(xPos2, 2*linkdialog::processorHeight));
    dstProcessor_->show();


    std::function<void(LinkDialogPropertyGraphicsItem*)> buildCache = [this, &buildCache](
        LinkDialogPropertyGraphicsItem* item) {
        propertyMap_[item->getItem()] = item;
        for (auto i : item->getSubPropertyItemList()) {
            buildCache(i);
        }
    };

    for (auto item : srcProcessor_->getPropertyItemList()) buildCache(item);
    for (auto item : dstProcessor_->getPropertyItemList()) buildCache(item);

    // add links
    auto links = network_->getLinksBetweenProcessors(srcProcessor, dstProcessor);
    for (auto& link : links) {
        initializePropertyLinkRepresentation(link);
    }
    update();

}

LinkDialogGraphicsScene::~LinkDialogGraphicsScene() {
    network_->removeObserver(this);

    // We need to make sure to delete the links first. since they refer to the properties
    for (auto& elem : connections_) {
        removeItem(elem);
        delete elem;
    }
    connections_.clear();
}

LinkDialogPropertyGraphicsItem* LinkDialogGraphicsScene::getPropertyGraphicsItemOf(Property* property) const {
    return util::map_find_or_null(propertyMap_, property);
}

ProcessorNetwork* LinkDialogGraphicsScene::getNetwork() const { return network_; }

void LinkDialogGraphicsScene::mousePressEvent(QGraphicsSceneMouseEvent* e) {
    auto tempProperty = getSceneGraphicsItemAt<LinkDialogPropertyGraphicsItem>(e->scenePos());

    if (!startProperty_ && tempProperty) {
        startProperty_ = tempProperty;

        if (linkCurve_) {
            linkCurve_->hide();
            removeItem(linkCurve_);
            delete linkCurve_;
            linkCurve_ = nullptr;
        }

        QPointF center = startProperty_->getConnectionPoint();
        linkCurve_ = new DialogCurveGraphicsItem(center, e->scenePos());
        linkCurve_->setZValue(linkdialog::connectionDragDepth);
        addItem(linkCurve_);
        linkCurve_->show();
        e->accept();
    } else {
        QGraphicsScene::mousePressEvent(e);
    }
}

void LinkDialogGraphicsScene::mouseMoveEvent(QGraphicsSceneMouseEvent* e) {
    QPointF pos = e->scenePos();

    

    if (pos.x() > linkdialog::dialogWidth / 2.0) {
        mouseOnLeftSide_ = false;
    } else {
        mouseOnLeftSide_ = true;
    }

    if (linkCurve_) {
        QPointF center = startProperty_->getConnectionPoint();
        linkCurve_->setStartPoint(center);
        linkCurve_->setEndPoint(pos);
        e->accept();
    } else {
        QGraphicsScene::mouseMoveEvent(e);
    }
}

void LinkDialogGraphicsScene::mouseReleaseEvent(QGraphicsSceneMouseEvent* e) {
    if (linkCurve_) {
        delete linkCurve_;
        linkCurve_ = nullptr;
        endProperty_ = getSceneGraphicsItemAt<LinkDialogPropertyGraphicsItem>(e->scenePos());

        if (endProperty_ && (endProperty_ != startProperty_)) {
            Property* sProp = startProperty_->getItem();
            Property* eProp = endProperty_->getItem();

            if (sProp->getOwner()->getProcessor() != eProp->getOwner()->getProcessor()) {
                bool src2dst = SimpleCondition::canLink(sProp, eProp);
                bool dst2src = SimpleCondition::canLink(eProp, sProp);
                if (src2dst && dst2src) {
                    addPropertyLink(sProp, eProp, true);
                } else if (src2dst) {
                    addPropertyLink(sProp, eProp, false);
                } else if (dst2src) {
                    addPropertyLink(eProp, sProp, false);
                }
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

void LinkDialogGraphicsScene::keyPressEvent(QKeyEvent* keyEvent) {
    if (keyEvent->key() == Qt::Key_Delete) {
        keyEvent->accept();
        QList<QGraphicsItem*> selectedGraphicsItems = selectedItems();

        for (auto& selectedGraphicsItem : selectedGraphicsItems) {
            if (auto item =
                    qgraphicsitem_cast<DialogConnectionGraphicsItem*>(selectedGraphicsItem)) {
                removePropertyLink(item);
                removePropertyLinkRepresentation(item->getPropertyLink());
            }
        }
    } else if (keyEvent->key() == Qt::Key_Escape) {
        emit closeDialog();    
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

        if (isPropertyLinkBidirectional(linkGraphicsItem)) {
            biDirectionAction->setChecked(true);
        } else {
            biDirectionAction->setChecked(false);
        }

        QAction* result = menu.exec(QCursor::pos());

        if (result == deleteAction) {
            removePropertyLink(linkGraphicsItem);
            removePropertyLinkRepresentation(linkGraphicsItem->getPropertyLink());
        } else if (result == biDirectionAction) {
            if (biDirectionAction->isChecked()) {
                makePropertyLinkBidirectional(linkGraphicsItem, true);
            } else {
                makePropertyLinkBidirectional(linkGraphicsItem, false);
            }
        } else if (result == switchDirection) {
            switchPropertyLinkDirection(linkGraphicsItem);
        }
    }
}

void LinkDialogGraphicsScene::wheelAction(float offset) {
    currentScrollSteps_ = offset;

    QTimeLine* anim = new QTimeLine(750, this);
    anim->setUpdateInterval(20);
    connect(anim, SIGNAL(valueChanged(qreal)), SLOT(executeTimeLine(qreal)));
    connect(anim, SIGNAL(finished()), SLOT(terminateTimeLine()));
    anim->start();
}

void LinkDialogGraphicsScene::offsetItems(float yIncrement, bool scrollLeft) {
    auto proc = scrollLeft ? srcProcessor_ : dstProcessor_;

    auto items = proc->getPropertyItemList();
    while (!items.empty() && items.back()->hasSubProperties() && items.back()->isExpanded()) {
        items = items.back()->getSubPropertyItemList();
    }
    qreal miny = std::numeric_limits<double>::lowest();
    if (!items.empty()) {
        miny = items.back()->scenePos().y();
    }

    auto view = views().front();
    auto top = view->mapToScene(QPoint(0, 0)).y();

    QPointF pos = proc->scenePos();
    qreal newy = std::min(pos.y() + yIncrement,
                          top + static_cast<qreal>(linkdialog::processorHeight));

    qreal listHeight = miny - pos.y();
    newy = std::max(newy, top - (listHeight - linkdialog::processorHeight));
    proc->setPos(pos.x(), newy);

    update();
}

void LinkDialogGraphicsScene::executeTimeLine(qreal x) {
    float yIncrement = linkdialog::processorHeight * (0.09f) * (currentScrollSteps_);
    offsetItems(yIncrement, mouseOnLeftSide_);
}

void LinkDialogGraphicsScene::terminateTimeLine() { delete sender(); }

void LinkDialogGraphicsScene::addPropertyLink(Property* sProp, Property* eProp,
                                              bool bidirectional) {
    network_->addLink(sProp, eProp);
    network_->evaluateLinksFromProperty(sProp);

    if (bidirectional) network_->addLink(eProp, sProp);
}

void LinkDialogGraphicsScene::toggleExpand() {
    expandProperties_ = !expandProperties_;

    std::function<void(LinkDialogPropertyGraphicsItem*)> expand = [this, &expand](
        LinkDialogPropertyGraphicsItem* item) {
        item->setExpanded(expandProperties_);
        for (auto i : item->getSubPropertyItemList()) {
            expand(i);
        }
    };

    for (auto item : srcProcessor_->getPropertyItemList()) expand(item);
    for (auto item : dstProcessor_->getPropertyItemList()) expand(item);
}

bool LinkDialogGraphicsScene::isPropertyExpanded(Property* property) const {
    if (auto propItem = getPropertyGraphicsItemOf(property)) return propItem->isExpanded();
    return false;
}

void LinkDialogGraphicsScene::removeAllPropertyLinks() {
    std::vector<DialogConnectionGraphicsItem*> tempList = connections_;

    for (auto propertyLink : tempList) {
        removePropertyLink(propertyLink);
    }
}

void LinkDialogGraphicsScene::removePropertyLink(DialogConnectionGraphicsItem* propertyLink) {
    Property* start = propertyLink->getStartProperty()->getItem();
    Property* end = propertyLink->getEndProperty()->getItem();

    network_->removeLink(start, end);
    network_->removeLink(end, start);
}

void LinkDialogGraphicsScene::cleanupAfterRemoveLink(DialogConnectionGraphicsItem* propertyLink) {
    for (auto& elem : connections_) elem->updateConnectionDrawing();
    update();
}

bool LinkDialogGraphicsScene::isPropertyLinkBidirectional(
    DialogConnectionGraphicsItem* propertyLink) const {
    LinkDialogPropertyGraphicsItem* startProperty = propertyLink->getStartProperty();
    LinkDialogPropertyGraphicsItem* endProperty = propertyLink->getEndProperty();

    return network_->isLinkedBidirectional(startProperty->getItem(), endProperty->getItem());
}

void LinkDialogGraphicsScene::makePropertyLinkBidirectional(
    DialogConnectionGraphicsItem* propertyLink, bool isBidirectional) {
    LinkDialogPropertyGraphicsItem* startProperty = propertyLink->getStartProperty();
    LinkDialogPropertyGraphicsItem* endProperty = propertyLink->getEndProperty();

    PropertyLink* propLink = network_->getLink(endProperty->getItem(), startProperty->getItem());
    if (isBidirectional) {
        if (!propLink) network_->addLink(endProperty->getItem(), startProperty->getItem());
    } else {
        if (propLink) network_->removeLink(endProperty->getItem(), startProperty->getItem());
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
        network_->removeLink(source, destination);
        network_->addLink(destination, source);
    }
}

DialogConnectionGraphicsItem* LinkDialogGraphicsScene::getConnectionGraphicsItem(
    LinkDialogPropertyGraphicsItem* outProperty, LinkDialogPropertyGraphicsItem* inProperty) const {
    for (auto& elem : connections_) {
        if ((elem->getStartProperty() == outProperty && elem->getEndProperty() == inProperty) ||
            (elem->getStartProperty() == inProperty && elem->getEndProperty() == outProperty))
            return elem;
    }

    return nullptr;
}

DialogConnectionGraphicsItem* LinkDialogGraphicsScene::initializePropertyLinkRepresentation(
    PropertyLink* propertyLink) {
    auto start = qgraphicsitem_cast<LinkDialogPropertyGraphicsItem*>(
        getPropertyGraphicsItemOf(propertyLink->getSourceProperty()));
    auto end = qgraphicsitem_cast<LinkDialogPropertyGraphicsItem*>(
        getPropertyGraphicsItemOf(propertyLink->getDestinationProperty()));

    if (start == nullptr || end == nullptr) return nullptr;

    DialogConnectionGraphicsItem* cItem = getConnectionGraphicsItem(start, end);
    if (!cItem) {
        cItem = new DialogConnectionGraphicsItem(start, end, propertyLink);
        addItem(cItem);
        connections_.push_back(cItem);
    }

    cItem->show();

    for (auto& elem : connections_) elem->updateConnectionDrawing();
    update();

    return cItem;
}

void LinkDialogGraphicsScene::removePropertyLinkRepresentation(PropertyLink* propertyLink) {
    auto it = util::find_if(connections_, [propertyLink](DialogConnectionGraphicsItem* i){
        return i->getPropertyLink() == propertyLink;
    });
    
    if (it != connections_.end()) {
        DialogConnectionGraphicsItem* cItem = *it;

        cItem->hide();
        util::erase_remove(connections_, cItem);

        LinkDialogPropertyGraphicsItem* start = cItem->getStartProperty();
        LinkDialogPropertyGraphicsItem* end = cItem->getEndProperty();

        removeItem(cItem);
        cleanupAfterRemoveLink(cItem);
        delete cItem;

        start->prepareGeometryChange();
        end->prepareGeometryChange();
    }
}

void LinkDialogGraphicsScene::onProcessorNetworkDidAddLink(PropertyLink* propertyLink) {
    initializePropertyLinkRepresentation(propertyLink);
}

void LinkDialogGraphicsScene::onProcessorNetworkDidRemoveLink(PropertyLink* propertyLink) {
    removePropertyLinkRepresentation(propertyLink);
}

void LinkDialogGraphicsScene::onProcessorNetworkWillRemoveProcessor(Processor* processor) {
    if (processor == srcProcessor_->getItem() || processor == dstProcessor_->getItem()) {
        emit closeDialog();
    }
}

// Manage various tooltips.
void LinkDialogGraphicsScene::helpEvent(QGraphicsSceneHelpEvent* e) {
    QList<QGraphicsItem*> graphicsItems = items(e->scenePos());
    for (auto item : graphicsItems) {
        switch (item->type()) {
            case LinkDialogPropertyGraphicsItem::Type:
                qgraphicsitem_cast<LinkDialogPropertyGraphicsItem*>(item)->showToolTip(e);
                return;

        };
    }
    QGraphicsScene::helpEvent(e);
}


}  // namespace