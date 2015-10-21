/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2015 Inviwo Foundation
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

#include <inviwo/qt/widgets/properties/transferfunctioneditor.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/datastructures/transferfunction.h>
#include <inviwo/qt/widgets/properties/transferfunctioneditorcontrolpoint.h>
#include <inviwo/qt/widgets/properties/transferfunctioncontrolpointconnection.h>
#include <inviwo/core/network/networklock.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QBrush>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsView>
#include <QKeyEvent>
#include <QLineF>
#include <QPainter>
#include <QPen>
#include <warn/pop>

namespace inviwo {

class ControlPointEquals {
public:
    ControlPointEquals(const TransferFunctionDataPoint* p) : p_(p) {}
    bool operator()(TransferFunctionEditorControlPoint* editorPoint) {
        return editorPoint->getPoint() == p_;
    }
    bool operator<(TransferFunctionEditorControlPoint* editorPoint) {
        return editorPoint->getPoint()->getPos().x < p_->getPos().x;
    }

private:
    const TransferFunctionDataPoint* p_;
};

TransferFunctionEditor::TransferFunctionEditor(TransferFunction* transferFunction,
                                               QGraphicsView* view)
    : QGraphicsScene()
    , zoomRangeXMin_(0.0)
    , zoomRangeXMax_(1.0)
    , zoomRangeYMin_(0.0)
    , zoomRangeYMax_(1.0)
    , view_(view)
    , transferFunction_(transferFunction)
    , groups_()
    , moveMode_(0) {

    setSceneRect(0.0, 0.0, 512.0, 512.0);
    mouseDrag_ = false;
    // initialize editor with current tf
    
    // The defalt bsp tends to crash...  
    setItemIndexMethod(QGraphicsScene::NoIndex);
    
    for (int i = 0; i < transferFunction_->getNumPoints(); ++i){
        onControlPointAdded(transferFunction_->getPoint(i));
    }

    for (int i = 0; i<10; ++i) {
        groups_.push_back(std::vector<TransferFunctionEditorControlPoint*>());
    }
}

TransferFunctionEditor::~TransferFunctionEditor() {
    for (auto& elem : points_) delete elem;
    points_.clear();

    for (auto& elem : connections_) delete elem;
    connections_.clear();
}

void TransferFunctionEditor::resetTransferFunction() {
    NetworkLock lock;
    transferFunction_->clearPoints();
    addControlPoint(QPointF(0.0 * (width() - 1), 0.0 * (height() - 1)), vec4(0.0f));
    addControlPoint(QPointF(1.0 * (width() - 1), 1.0 * (height() - 1)), vec4(1.0f));
}

void TransferFunctionEditor::mousePressEvent(QGraphicsSceneMouseEvent* e) {
    TransferFunctionEditorControlPoint* controlPointGraphicsItem =
        getControlPointGraphicsItemAt(e->scenePos());
    // Need to store these since they are deselected in mousePressEvent.
    selectedItemsAtPress_ = QGraphicsScene::selectedItems();
    
    #include <warn/push>
    #include <warn/ignore/switch-enum>
    switch (e->button()) {
        case Qt::LeftButton:
            if (controlPointGraphicsItem) {
                mouseDrag_ = true;
            } else {
                views().front()->setDragMode(QGraphicsView::RubberBandDrag);
            }

            break;
        default:
            break;
    }
    #include <warn/pop>
    mouseMovedSincePress_ = false;

    QGraphicsScene::mousePressEvent(e);
}

void TransferFunctionEditor::mouseMoveEvent(QGraphicsSceneMouseEvent* e) {
    mouseMovedSincePress_ = true;
    if (mouseDrag_) {
        // Prevent network evaluations while moving control point
        NetworkLock lock;
        QGraphicsScene::mouseMoveEvent(e);
    } else {
        QGraphicsScene::mouseMoveEvent(e);    
    }
}

void TransferFunctionEditor::mouseReleaseEvent(QGraphicsSceneMouseEvent* e) {
    // left mouse button and no movement -> add new point if there is no selection
    #include <warn/push>
    #include <warn/ignore/switch-enum>
    switch (e->button()) {
    case Qt::LeftButton:
        if (!mouseMovedSincePress_ && !mouseDrag_) {
            // Add a new point if a group of points are not selected
            // Simply deselect if more than one point is selected
            if (selectedItemsAtPress_.size() <= 1) {
                addControlPoint(e->scenePos());
                this->clearSelection();
                auto controlPointGraphicsItem =
                    getControlPointGraphicsItemAt(e->scenePos());
                if (controlPointGraphicsItem) {
                    controlPointGraphicsItem->setSelected(true);
                }
            } else {
                this->clearSelection();
            }
            
            e->accept();
        }
        break;
    case Qt::RightButton:
        if (!mouseDrag_) {
            auto controlPointGraphicsItem =
                getControlPointGraphicsItemAt(e->scenePos());
            if (selectedItemsAtPress_.size() <= 1) {
                // One or no selection, check whether there is a TF point under the mouse
                
                if (controlPointGraphicsItem) {
                    removeControlPoint(controlPointGraphicsItem);
                }
            } else {
                auto pressedOnSelectedItem = std::find(std::begin(selectedItemsAtPress_), std::end(selectedItemsAtPress_), controlPointGraphicsItem) != selectedItemsAtPress_.end();
                if (pressedOnSelectedItem) {
                    for (auto& elem : selectedItemsAtPress_) {
                        auto point =
                            qgraphicsitem_cast<TransferFunctionEditorControlPoint*>(elem);
                        if (point) {
                            removeControlPoint(point);
                        }
                    }
                } else {
                    this->clearSelection();
                }

            }
            e->accept();
        }
        break;
    }
    #include <warn/pop>
    mouseDrag_ = false;
    if (!e->isAccepted()) {
        QGraphicsScene::mouseReleaseEvent(e);
    }
}

void TransferFunctionEditor::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* e) {
    emit doubleClick();
    QGraphicsScene::mouseDoubleClickEvent(e);
}

void TransferFunctionEditor::keyPressEvent(QKeyEvent* keyEvent) {
    int k = keyEvent->key();
    keyEvent->accept();

    NetworkLock lock;

    if (k == 'A' && keyEvent->modifiers() == Qt::ControlModifier) {                // Select all
        QList<QGraphicsItem*> itemList = items();
        for (auto& elem : itemList) {
            elem->setSelected(true);
        }

    }
    else if (k == 'D' && keyEvent->modifiers() == Qt::ControlModifier) {         // Select none
        QList<QGraphicsItem*> itemList = selectedItems();
        for (auto& elem : itemList) {
            elem->setSelected(false);
        }

    } else if (k == Qt::Key_Delete) {  // Delete selected
        QList<QGraphicsItem*> itemList = selectedItems();
        for (auto& elem : itemList) {
            TransferFunctionEditorControlPoint* p =
                qgraphicsitem_cast<TransferFunctionEditorControlPoint*>(elem);
            if (p) {
                removeControlPoint(p);
            }
        }

    }
    else if (!(keyEvent->modifiers() & Qt::ControlModifier) &&                     // Move points
               (k == Qt::Key_Left || k == Qt::Key_Right || k == Qt::Key_Up || k == Qt::Key_Down || 
                k == 'I' || k == 'J' || k == 'K' || k == 'L')) {
        QPointF delta;
        float x = sceneRect().width() / 1000.0;
        float y = sceneRect().height() / 1000.0;
        switch (k) {
            case Qt::Key_Left:
            case 'J':
                delta = QPointF(-x, 0.0f);
                break;
            case Qt::Key_Right:
            case 'L':
                delta = QPointF(x, 0.0f);
                break;
            case Qt::Key_Up:
            case 'I':
                delta = QPointF(0.0f, y);
                break;
            case Qt::Key_Down:
            case 'K':
                delta = QPointF(0.0f, -y);
                break;
        }

        if (keyEvent->modifiers() & Qt::ShiftModifier || keyEvent->modifiers() & Qt::AltModifier) {
            delta *= 10.0;
        }

        QList<QGraphicsItem*> selitems = selectedItems();
        for (auto& selitem : selitems) {
            selitem->setPos(selitem->pos() + delta);
        }

    }
    else if (keyEvent->modifiers() & Qt::ControlModifier &&                     // Modify selection
               (k == Qt::Key_Left || k == Qt::Key_Right || k == Qt::Key_Up || k == Qt::Key_Down ||
                k == 'I' || k == 'J' || k == 'K' || k == 'L')) {
        QList<QGraphicsItem*> selitems = selectedItems();

        std::vector<TransferFunctionEditorControlPoint*> points;
        for (auto& selitem : selitems) {
            TransferFunctionEditorControlPoint* p =
                qgraphicsitem_cast<TransferFunctionEditorControlPoint*>(selitem);
            if (p) points.push_back(p);
        }
        std::stable_sort(points.begin(), points.end(),
                         comparePtr<TransferFunctionEditorControlPoint>);

        switch (k) {
            case Qt::Key_Left:
            case 'J':
                if (points.size() > 0) {
                    if (points.front()->left_ && points.front()->left_->left_) {
                        points.front()->left_->left_->setSelected(true);
                        if (!(keyEvent->modifiers() & Qt::ShiftModifier)) {
                            points.back()->setSelected(false);
                        }
                    }
                } else if (points_.size() > 0) {
                    points_.back()->setSelected(true);
                }

                break;
            case Qt::Key_Right:
            case 'L':
                if (points.size() > 0) {
                    if (points.back()->right_ && points.back()->right_->right_) {
                        points.back()->right_->right_->setSelected(true);
                        if (!(keyEvent->modifiers() & Qt::ShiftModifier)) {
                            points.front()->setSelected(false);
                        }
                    }
                } else if (points_.size() > 0) {
                    points_.front()->setSelected(true);
                }
                break;
            case Qt::Key_Up:
            case 'I':
                if (points.size() > 0) {
                    points.front()->setSelected(false);
                }
                break;
            case Qt::Key_Down:
            case 'K':
                if (points.size() > 0) {
                    points.back()->setSelected(false);
                }
                break;
        }
    
    } else if ((k >= '0' && k <= '9') ||                               // Groups selection
               k == '!' || k =='"' || k =='#' || k ==Qt::Key_paragraph || k =='%' || k =='&' || k =='(' ||
               k ==')' || k =='=') {                                                                
        int group = 0;
        switch(k) {
            case '0':
            case '=':
                group = 0;
                break;
            case '1':
            case '!':
                group = 1;
                break;
            case '2':
            case '"':
                group = 2;
                break;
            case '3':
            case '#':
                group = 3;
                break;
            case '4':
            case Qt::Key_paragraph:
                group = 4;
                break;
            case '5':
            case '%':
                group = 5;
                break;
            case '6':
            case '&':
                group = 6;
                break;
            case '7':
            case '/':
                group = 7;
                break;
            case '8':
            case '(':
                group = 8;
                break;
            case '9':
            case ')':
                group = 9;
                break;
        }

        QList<QGraphicsItem*> selitems = selectedItems();
        if (keyEvent->modifiers() & Qt::ControlModifier) { // Create group
            groups_[group].clear();
            for (auto& selitem : selitems) {
                TransferFunctionEditorControlPoint* p =
                    qgraphicsitem_cast<TransferFunctionEditorControlPoint*>(selitem);
                if (p) groups_[group].push_back(p);
            }
        } else {
            if (!(keyEvent->modifiers() & Qt::ShiftModifier)) {
                            for (auto& selitem : selitems) {
                                selitem->setSelected(false);
                 }
            }
            for (auto& elem : groups_[group]) {
                elem->setSelected(true);
            }     
        }
    } else {
        keyEvent->ignore();
        QGraphicsScene::keyPressEvent(keyEvent);
    }
}

void TransferFunctionEditor::addControlPoint(QPointF pos) {
    if (pos.x() < 0.0) {
        pos.setX(0.0);
    } else if (pos.x() > width()) {
        pos.setX(width());
    }
    if (pos.y() < 0.0) {
        pos.setY(0.0);
    } else if (pos.y() > height()) {
        pos.setY(height());
    }
    transferFunction_->addPoint(vec2(pos.x() / width(), pos.y() / height()));
}

void TransferFunctionEditor::addControlPoint(QPointF pos, vec4 color) {
    // add control point to transfer function
    if (pos.x() < 0.0) {
        pos.setX(0.0);
    } else if (pos.x() > width()) {
        pos.setX(width());
    }
    if (pos.y() < 0.0) {
        pos.setY(0.0);
    } else if (pos.y() > height()) {
        pos.setY(height());
    }
    transferFunction_->addPoint(vec2(pos.x() / width(), pos.y() / height()), color);
}

void TransferFunctionEditor::removeControlPoint(TransferFunctionEditorControlPoint* controlPoint) {
    if (transferFunction_->getNumPoints() > 1) {
        transferFunction_->removePoint(controlPoint->getPoint());
    }
}

TransferFunctionEditorControlPoint* TransferFunctionEditor::getControlPointGraphicsItemAt(
    const QPointF pos) const {
    QList<QGraphicsItem*> graphicsItems = items(pos);

    for (auto& graphicsItem : graphicsItems) {
        TransferFunctionEditorControlPoint* controlPointGraphicsItem =
            qgraphicsitem_cast<TransferFunctionEditorControlPoint*>(graphicsItem);

        if (controlPointGraphicsItem) return controlPointGraphicsItem;
    }

    return nullptr;
}

void TransferFunctionEditor::onControlPointAdded(TransferFunctionDataPoint* p) {
    QPointF pos(p->getPos().x * width(), p->getPos().y * height());

    TransferFunctionEditorControlPoint* newpoint =
        new TransferFunctionEditorControlPoint(p, dataMap_);
    newpoint->setSize(controlPointSize_);
    newpoint->setPos(pos);

    PointVec::iterator it = std::lower_bound(points_.begin(), points_.end(), newpoint,
                                             comparePtr<TransferFunctionEditorControlPoint>);
    it = points_.insert(it, newpoint);

    updateConnections();

    addItem(newpoint);
}

void TransferFunctionEditor::onControlPointRemoved(TransferFunctionDataPoint* p) {
    std::vector<TransferFunctionEditorControlPoint*>::iterator it;
    
    // remove point from all groups
    for (auto& elem : groups_) {
        it = std::find_if(elem.begin(), elem.end(), ControlPointEquals(p));
        if (it != elem.end()) elem.erase(it);
    }
    
    // remove item.
    it = std::find_if(points_.begin(), points_.end(), ControlPointEquals(p));
    if (it != points_.end()) {
        delete *it;
        points_.erase(it);
        updateConnections();
    }
}

void TransferFunctionEditor::updateConnections() {
    std::stable_sort(points_.begin(), points_.end(), comparePtr<TransferFunctionEditorControlPoint>);
    
    while (connections_.size() < points_.size() + 1){
        TransferFunctionControlPointConnection* c = new TransferFunctionControlPointConnection();
        connections_.push_back(c);
        addItem(c);
    }
    while (connections_.size() > points_.size() + 1){
        TransferFunctionControlPointConnection* c = connections_.back();
        removeItem(c);
        delete c;
        connections_.pop_back();
    }

    connections_[0]->left_ = nullptr;
    connections_[connections_.size() - 1]->right_ = nullptr;

    for (int i = 0; i < static_cast<int>(points_.size()); ++i){
        points_[i]->left_ = connections_[i];
        points_[i]->right_ = connections_[i+1];
        connections_[i]->right_ = points_[i];
        connections_[i+1]->left_ = points_[i];
    }

    for (auto& elem : connections_) {
        elem->updateShape();
    }
}

void TransferFunctionEditor::onControlPointChanged(const TransferFunctionDataPoint* p) {
}

void TransferFunctionEditor::setControlPointSize(float val) {
    controlPointSize_ = val;
    for (auto e : points_) {
        e->setSize(getControlPointSize());
    }
}

void TransferFunctionEditor::setDataMap(const DataMapper& dataMap) {
   dataMap_ = dataMap;
}

inviwo::DataMapper TransferFunctionEditor::getDataMap() const {
    return dataMap_;
}

float TransferFunctionEditor::getZoomRangeXMin() const {
    return zoomRangeXMin_;
}

void TransferFunctionEditor::setZoomRangeXMin(float min) {
    zoomRangeXMin_ = min;
}

float TransferFunctionEditor::getZoomRangeXMax() const {
    return zoomRangeXMax_;
}

void TransferFunctionEditor::setZoomRangeXMax(float max) {
    zoomRangeXMax_ = max;
}

float TransferFunctionEditor::getZoomRangeYMin() const {
    return zoomRangeYMin_;
}

void TransferFunctionEditor::setZoomRangeYMin(float min) {
    zoomRangeYMin_ = min;
}

float TransferFunctionEditor::getZoomRangeYMax() const {
    return zoomRangeYMax_;
}

void TransferFunctionEditor::setZoomRangeYMax(float max) {
    zoomRangeYMax_ = max;
}

QGraphicsView* TransferFunctionEditor::getView() {
    return view_;
}

void TransferFunctionEditor::setMoveMode(int i) {
   moveMode_ = i; 
}

int TransferFunctionEditor::getMoveMode() const {
    return moveMode_;
}

}