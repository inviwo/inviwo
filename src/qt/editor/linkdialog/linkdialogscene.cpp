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
#include <QMenu>
#include <QAction>
#include <QKeyEvent>
#include <QGraphicsSceneWheelEvent>
#include <QGraphicsSceneMouseEvent>
#include <warn/pop>

namespace inviwo {

LinkDialogGraphicsScene::LinkDialogGraphicsScene(QWidget* parent, ProcessorNetwork* network,
                                                 Processor* srcProcessor, Processor* dstProcessor)
    : QGraphicsScene(parent)
    , curve_(this)
    , startProperty_(nullptr)
    , srcProcessor_(nullptr)
    , dstProcessor_(nullptr)
    , network_(network)
    , expandProperties_(false) {
    // The default bsp tends to crash...
    setItemIndexMethod(QGraphicsScene::NoIndex);

    network_->addObserver(this);

    QColor bgColor;
    bgColor.setNamedColor("#4d4d4d");
    setBackgroundBrush(bgColor);

    srcProcessor_ =
        new LinkDialogProcessorGraphicsItem(LinkDialogTreeItem::Side::Left, srcProcessor);
    dstProcessor_ =
        new LinkDialogProcessorGraphicsItem(LinkDialogTreeItem::Side::Right, dstProcessor);

    auto yPos = linkdialog::processorHeight;
    float xPos1 = linkdialog::dialogWidth / 10.0 + linkdialog::processorWidth / 2;
    addItem(srcProcessor_);
    srcProcessor_->setPos(QPointF(xPos1, yPos));
    srcProcessor_->show();

    float xPos2 = linkdialog::dialogWidth * 9.0 / 10.0 - linkdialog::processorWidth / 2;
    addItem(dstProcessor_);
    dstProcessor_->setPos(QPointF(xPos2, yPos));
    dstProcessor_->show();

    std::function<void(LinkDialogPropertyGraphicsItem*)> buildCache =
        [this, &buildCache](LinkDialogPropertyGraphicsItem* item) {
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
    curve_.clear();
    network_->removeObserver(this);

    // We need to make sure to delete the links first. since they refer to the properties
    for (auto& elem : connections_) {
        removeItem(elem);
        delete elem;
    }
    connections_.clear();
}

LinkDialogPropertyGraphicsItem* LinkDialogGraphicsScene::getPropertyGraphicsItemOf(
    Property* property) const {
    return util::map_find_or_null(propertyMap_, property);
}

ProcessorNetwork* LinkDialogGraphicsScene::getNetwork() const { return network_; }

LinkDialogGraphicsScene::Curve::Curve(LinkDialogGraphicsScene* scene) : scene_{scene} {}

void LinkDialogGraphicsScene::Curve::clear() {
    if (linkCurve_) {
        linkCurve_->hide();
        scene_->removeItem(linkCurve_.get());
        linkCurve_.reset();
    }
}

void LinkDialogGraphicsScene::Curve::start(QPointF start, QPointF end) {
    linkCurve_ = std::make_unique<DialogCurveGraphicsItem>(start, end);
    linkCurve_->setZValue(linkdialog::connectionDragDepth);
    scene_->addItem(linkCurve_.get());
    linkCurve_->show();
}
void LinkDialogGraphicsScene::Curve::update(QPointF start, QPointF end) {
    linkCurve_->setStartPoint(start);
    linkCurve_->setEndPoint(end);
}

void LinkDialogGraphicsScene::mousePressEvent(QGraphicsSceneMouseEvent* e) {
    auto tempProperty = getItemAt<LinkDialogPropertyGraphicsItem>(e->scenePos());

    if (e->button() == Qt::LeftButton && !startProperty_ && tempProperty) {
        startProperty_ = tempProperty;

        curve_.clear();
        curve_.start(startProperty_->getConnectionPoint(), e->scenePos());

        e->accept();
    } else {
        QGraphicsScene::mousePressEvent(e);
    }
}

void LinkDialogGraphicsScene::mouseMoveEvent(QGraphicsSceneMouseEvent* e) {
    if ((e->buttons() & Qt::LeftButton) && curve_) {
        curve_.update(startProperty_->getConnectionPoint(), e->scenePos());
        if (auto endProperty = getItemAt<LinkDialogPropertyGraphicsItem>(e->scenePos())) {
            bool canlink = false;
            if (endProperty != startProperty_) {
                auto sProp = startProperty_->getItem();
                auto eProp = endProperty->getItem();

                if (sProp->getOwner()->getProcessor() != eProp->getOwner()->getProcessor()) {
                    canlink |= network_->canLink(sProp, eProp);
                    if (!(e->modifiers() & Qt::ShiftModifier)) {
                        canlink |= network_->canLink(eProp, sProp);
                    }
                }
            }
            if (canlink) {
                curve_.linkCurve_->setBorderColor(QColor(0, 255, 0));
            } else {
                curve_.linkCurve_->setBorderColor(QColor(255, 0, 0));
            }
        } else {
            curve_.linkCurve_->resetBorderColors();
        }

        e->accept();
    } else {
        QGraphicsScene::mouseMoveEvent(e);
    }
}

void LinkDialogGraphicsScene::mouseReleaseEvent(QGraphicsSceneMouseEvent* e) {
    if (curve_) {
        curve_.clear();

        auto endProperty = getItemAt<LinkDialogPropertyGraphicsItem>(e->scenePos());
        if (endProperty && (endProperty != startProperty_)) {
            auto sProp = startProperty_->getItem();
            auto eProp = endProperty->getItem();

            if (sProp->getOwner()->getProcessor() != eProp->getOwner()->getProcessor()) {
                bool src2dst = network_->canLink(sProp, eProp);
                bool dst2src = network_->canLink(eProp, sProp);
                if (src2dst && dst2src && !(e->modifiers() & Qt::ShiftModifier)) {
                    addPropertyLink(sProp, eProp, true);
                } else if (src2dst) {
                    addPropertyLink(sProp, eProp, false);
                } else if (dst2src && !(e->modifiers() & Qt::ShiftModifier)) {
                    addPropertyLink(eProp, sProp, false);
                }
            }
        }
        e->accept();
    } else {
        QGraphicsScene::mouseReleaseEvent(e);
    }
    startProperty_ = nullptr;
}

void LinkDialogGraphicsScene::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* e) {
    if (auto property = getItemAt<LinkDialogPropertyGraphicsItem>(e->scenePos())) {
        property->setExpanded(!property->isExpanded());
        auto item = property->next();
        while (item) {
            item->updatePositions();
            item = item->next();
        }
        e->accept();
    } else if (auto connection = getItemAt<DialogConnectionGraphicsItem>(e->scenePos())) {
        if (e->modifiers() == Qt::ShiftModifier || e->modifiers() == Qt::ControlModifier) {
            makePropertyLinkBidirectional(connection, !isPropertyLinkBidirectional(connection));
        } else if (isPropertyLinkBidirectional(connection)) {
            makePropertyLinkBidirectional(connection, false);
        } else {
            switchPropertyLinkDirection(connection);
        }
        e->accept();
    } else {
        QGraphicsScene::mouseReleaseEvent(e);
    }
}

void LinkDialogGraphicsScene::wheelEvent(QGraphicsSceneWheelEvent* e) {
    offsetItems(e->delta() * 0.25f, e->scenePos().x() < linkdialog::dialogWidth / 2.0);
    e->accept();
}

void LinkDialogGraphicsScene::keyPressEvent(QKeyEvent* keyEvent) {
    if (keyEvent->key() == Qt::Key_Delete) {
        keyEvent->accept();
        QList<QGraphicsItem*> selectedGraphicsItems = selectedItems();

        for (auto& selectedGraphicsItem : selectedGraphicsItems) {
            if (auto item =
                    qgraphicsitem_cast<DialogConnectionGraphicsItem*>(selectedGraphicsItem)) {
                removePropertyLink(item);
            }
        }
    } else if (keyEvent->key() == Qt::Key_Escape) {
        keyEvent->accept();
        emit closeDialog();
    }

    QGraphicsScene::keyPressEvent(keyEvent);
}

void LinkDialogGraphicsScene::contextMenuEvent(QGraphicsSceneContextMenuEvent* e) {
    if (auto linkGraphicsItem = getItemAt<DialogConnectionGraphicsItem>(e->scenePos())) {
        QMenu menu;
        auto deleteAction = menu.addAction("Delete");
        auto biDirectionAction = menu.addAction("Bidirectional");
        biDirectionAction->setCheckable(true);
        auto switchDirection = menu.addAction("Switch Direction");

        if (isPropertyLinkBidirectional(linkGraphicsItem)) {
            biDirectionAction->setChecked(true);
            switchDirection->setDisabled(true);
        } else {
            biDirectionAction->setChecked(false);
        }

        connect(deleteAction, &QAction::triggered, this,
                [&]() { removePropertyLink(linkGraphicsItem); });
        connect(biDirectionAction, &QAction::triggered, this, [&]() {
            makePropertyLinkBidirectional(linkGraphicsItem, biDirectionAction->isChecked());
        });
        connect(switchDirection, &QAction::triggered, this,
                [&]() { switchPropertyLinkDirection(linkGraphicsItem); });

        menu.exec(QCursor::pos());
        e->accept();
    }
}

void LinkDialogGraphicsScene::offsetItems(double yIncrement, bool scrollLeft) {
    auto proc = scrollLeft ? srcProcessor_ : dstProcessor_;

    auto tree =
        proc->mapToScene(proc->boundingRect() | proc->childrenBoundingRect()).boundingRect();

    auto view = views().front();
    auto vr = view->mapToScene(view->rect()).boundingRect();

    if (tree.top() + yIncrement > vr.center().y()) yIncrement = vr.center().y() - tree.top();
    if (tree.bottom() + yIncrement < vr.center().y()) yIncrement = vr.center().y() - tree.bottom();

    proc->setPos(proc->scenePos().x(), proc->scenePos().y() + yIncrement);
}

void LinkDialogGraphicsScene::addPropertyLink(Property* sProp, Property* eProp,
                                              bool bidirectional) {
    network_->addLink(sProp, eProp);
    network_->evaluateLinksFromProperty(sProp);

    if (bidirectional) network_->addLink(eProp, sProp);
}

void LinkDialogGraphicsScene::showHidden(bool val) {
    showHidden_ = val;

    LinkDialogTreeItem* item = srcProcessor_;
    while (item) {
        item->updatePositions();
        item = item->next();
    }

    item = dstProcessor_;
    while (item) {
        item->updatePositions();
        item = item->next();
    }
}

bool LinkDialogGraphicsScene::isShowingHidden() const { return showHidden_; }

void LinkDialogGraphicsScene::toggleExpand() {
    expandProperties_ = !expandProperties_;

    LinkDialogTreeItem* item = srcProcessor_;
    while (item) {
        item->setExpanded(expandProperties_);
        item->updatePositions();
        item = item->next();
    }

    item = dstProcessor_;
    while (item) {
        item->setExpanded(expandProperties_);
        item->updatePositions();
        item = item->next();
    }
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

void LinkDialogGraphicsScene::updateAll() {
    for (auto& elem : connections_) elem->updateShape();
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

    if (isBidirectional) {
        if (!network_->isLinked(endProperty->getItem(), startProperty->getItem())) {
            network_->addLink(endProperty->getItem(), startProperty->getItem());
        }
    } else {
        if (network_->isLinked(endProperty->getItem(), startProperty->getItem())) {
            network_->removeLink(endProperty->getItem(), startProperty->getItem());
        }
    }

    propertyLink->updateShape();
    update();
}

void LinkDialogGraphicsScene::switchPropertyLinkDirection(
    DialogConnectionGraphicsItem* propertyLink) {

    if (!isPropertyLinkBidirectional(propertyLink)) {
        const auto link = propertyLink->getPropertyLink();
        network_->removeLink(link.getSource(), link.getDestination());
        network_->addLink(link.getDestination(), link.getSource());
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
    const PropertyLink& propertyLink) {
    auto start = qgraphicsitem_cast<LinkDialogPropertyGraphicsItem*>(
        getPropertyGraphicsItemOf(propertyLink.getSource()));
    auto end = qgraphicsitem_cast<LinkDialogPropertyGraphicsItem*>(
        getPropertyGraphicsItemOf(propertyLink.getDestination()));

    if (start == nullptr || end == nullptr) return nullptr;

    DialogConnectionGraphicsItem* cItem = getConnectionGraphicsItem(start, end);
    if (!cItem) {
        cItem = new DialogConnectionGraphicsItem(start, end, propertyLink);
        addItem(cItem);
        connections_.push_back(cItem);
    }

    cItem->show();

    updateAll();

    return cItem;
}

void LinkDialogGraphicsScene::removePropertyLinkRepresentation(const PropertyLink& propertyLink) {
    auto it = util::find_if(connections_, [&propertyLink](DialogConnectionGraphicsItem* i) {
        return i->getPropertyLink() == propertyLink;
    });

    if (it != connections_.end()) {
        DialogConnectionGraphicsItem* cItem = *it;

        cItem->hide();
        util::erase_remove(connections_, cItem);

        LinkDialogPropertyGraphicsItem* start = cItem->getStartProperty();
        LinkDialogPropertyGraphicsItem* end = cItem->getEndProperty();

        removeItem(cItem);
        updateAll();
        delete cItem;

        start->prepareGeometryChange();
        end->prepareGeometryChange();
    }
}

void LinkDialogGraphicsScene::onProcessorNetworkDidAddLink(const PropertyLink& propertyLink) {
    initializePropertyLinkRepresentation(propertyLink);
}

void LinkDialogGraphicsScene::onProcessorNetworkDidRemoveLink(const PropertyLink& propertyLink) {
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

}  // namespace inviwo
