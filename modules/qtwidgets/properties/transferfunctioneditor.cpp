/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2018 Inviwo Foundation
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

#include <modules/qtwidgets/properties/transferfunctioneditor.h>

#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/datastructures/transferfunction.h>
#include <inviwo/core/datastructures/tfprimitive.h>

#include <modules/qtwidgets/properties/transferfunctioneditorcontrolpoint.h>
#include <modules/qtwidgets/properties/transferfunctioneditorprimitive.h>
#include <modules/qtwidgets/properties/transferfunctioncontrolpointconnection.h>
#include <modules/qtwidgets/inviwoqtutils.h>
#include <inviwo/core/properties/transferfunctionproperty.h>
#include <inviwo/core/network/networklock.h>
#include <inviwo/core/util/zip.h>
#include <inviwo/core/util/raiiutils.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QApplication>
#include <QGuiApplication>
#include <QBrush>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsView>
#include <QKeyEvent>
#include <QLineF>
#include <QPainter>
#include <QPen>
#include <QMenu>
#include <QAction>
#include <QGraphicsPixmapItem>
#include <warn/pop>

namespace inviwo {

class ControlPointEquals {
public:
    ControlPointEquals(const TFPrimitive* p) : p_(p) {}
    bool operator()(TransferFunctionEditorPrimitive* editorPoint) {
        return editorPoint->getPrimitive() == p_;
    }
    bool operator<(TransferFunctionEditorPrimitive* editorPoint) {
        return editorPoint->getPrimitive()->getPosition() < p_->getPosition();
    }

private:
    const TFPrimitive* p_;
};

TransferFunctionEditor::TransferFunctionEditor(TransferFunctionProperty* tfProperty,
                                               QWidget* parent)
    : QGraphicsScene(parent)
    , tfProperty_(tfProperty)
    , transferFunction_(&tfProperty->get())
    , dataMap_()
    , groups_()
    , moveMode_(0)
    , selectNewPrimitives_(false) {

    setSceneRect(0.0, 0.0, 512.0, 512.0);
    mouseDrag_ = false;

    // The default bsp tends to crash...
    setItemIndexMethod(QGraphicsScene::NoIndex);

    if (auto port = tfProperty->getVolumeInport()) {

        const auto portChange = [this, port]() {
            dataMap_ = port->hasData() ? port->getData()->dataMap_ : DataMapper{};
        };

        port->onChange(portChange);
        port->onConnect(portChange);
        port->onDisconnect(portChange);

        if (port->hasData()) {
            dataMap_ = port->getData()->dataMap_;
        }
    }

    // initialize editor with current tf
    for (auto p : *transferFunction_) {
        onControlPointAdded(p);
    }

    for (int i = 0; i < 10; ++i) {
        groups_.push_back(std::vector<TransferFunctionEditorPrimitive*>());
    }
}

TransferFunctionEditor::~TransferFunctionEditor() {
    for (auto& elem : points_) delete elem;
    points_.clear();

    for (auto& elem : connections_) delete elem;
    connections_.clear();
}

void TransferFunctionEditor::mousePressEvent(QGraphicsSceneMouseEvent* e) {
#include <warn/push>
#include <warn/ignore/switch-enum>
    switch (e->button()) {
        case Qt::LeftButton:
            if (getTFPrimitiveItemAt(e->scenePos())) {
                mouseDrag_ = true;
                // inform all selected TF primitives about imminent move (need to cache position)
                for (auto& item : getSelectedPrimitiveItems()) {
                    item->beginMouseDrag();
                }

            } else {
                views().front()->setDragMode(QGraphicsView::RubberBandDrag);
            }

            break;
        case Qt::RightButton:
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
    if (mouseDrag_ && ((e->buttons() & Qt::LeftButton) == Qt::LeftButton)) {
        // Prevent network evaluations while moving control point
        NetworkLock lock;
        QGraphicsScene::mouseMoveEvent(e);
    } else {
        QGraphicsScene::mouseMoveEvent(e);
    }
}

void TransferFunctionEditor::mouseReleaseEvent(QGraphicsSceneMouseEvent* e) {
    // left mouse button and no movement -> add new point if there is no selection

    const bool controlPressed =
        ((QGuiApplication::queryKeyboardModifiers() & Qt::ControlModifier) == Qt::ControlModifier);
#include <warn/push>
#include <warn/ignore/switch-enum>
    switch (e->button()) {
        case Qt::LeftButton:
            if (!mouseMovedSincePress_) {
                // add new TF primitive when control is pressed and nothing is below the cursor
                if (controlPressed && !mouseDoubleClick_ && !mouseDrag_) {
                    this->clearSelection();

                    util::KeepTrueWhileInScope k(&selectNewPrimitives_);
                    addControlPoint(e->scenePos());
                    e->accept();
                }
            }
            if (mouseDrag_) {
                for (auto& item : getSelectedPrimitiveItems()) {
                    item->stopMouseDrag();
                }
            } else {
                // disable rubber band selection
                views().front()->setDragMode(QGraphicsView::NoDrag);
            }
            break;
        case Qt::RightButton:
            break;
        default:
            break;
    }
#include <warn/pop>
    mouseDrag_ = false;
    mouseDoubleClick_ = false;
    mouseMovedSincePress_ = false;

    if (!e->isAccepted()) {
        QGraphicsScene::mouseReleaseEvent(e);
    }
}

void TransferFunctionEditor::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* e) {
    mouseDoubleClick_ = true;
    QGraphicsScene::mouseDoubleClickEvent(e);
}

void TransferFunctionEditor::keyPressEvent(QKeyEvent* keyEvent) {
    // these factors are applied when holding shift (increasing step size) or alt (decreasing step
    // size) when moving control points with the keyboard
    const double stepUpScalingFactor = 5.0;
    const double stepDownScalingFactor = 0.2;

    int k = keyEvent->key();
    keyEvent->accept();

    NetworkLock lock(tfProperty_);

    if (k == Qt::Key_A && keyEvent->modifiers() == Qt::ControlModifier) {  // Select all
        QList<QGraphicsItem*> itemList = items();
        for (auto& elem : itemList) {
            elem->setSelected(true);
        }

    } else if (k == Qt::Key_D && keyEvent->modifiers() == Qt::ControlModifier) {  // Select none
        QList<QGraphicsItem*> itemList = selectedItems();
        for (auto& elem : itemList) {
            elem->setSelected(false);
        }

    } else if (k == Qt::Key_Delete || k == Qt::Key_Backspace) {  // Delete selected
        auto selection = getSelectedPrimitiveItems();
        clearSelection();
        for (auto& item : selection) {
            removeControlPoint(item);
        }

    } else if (!(keyEvent->modifiers() & Qt::ControlModifier) &&  // Move points
               (k == Qt::Key_Left || k == Qt::Key_Right || k == Qt::Key_Up || k == Qt::Key_Down ||
                k == 'I' || k == 'J' || k == 'K' || k == 'L')) {
        QPointF delta;
        switch (k) {
            case Qt::Key_Left:
            case 'J':
                delta = QPointF(-relativeSceneOffset_.x, 0.0f);
                break;
            case Qt::Key_Right:
            case 'L':
                delta = QPointF(relativeSceneOffset_.x, 0.0f);
                break;
            case Qt::Key_Up:
            case 'I':
                delta = QPointF(0.0f, relativeSceneOffset_.y);
                break;
            case Qt::Key_Down:
            case 'K':
                delta = QPointF(0.0f, -relativeSceneOffset_.y);
                break;
        }

        if (keyEvent->modifiers() & Qt::ShiftModifier) {
            delta *= stepUpScalingFactor;
        } else if (keyEvent->modifiers() & Qt::AltModifier) {
            delta *= stepDownScalingFactor;
        }

        QList<QGraphicsItem*> selitems = selectedItems();
        for (auto& selitem : selitems) {
            selitem->setPos(selitem->pos() + delta);
        }

    } else if (keyEvent->modifiers() & Qt::ControlModifier &&  // Modify selection
               (k == Qt::Key_Left || k == Qt::Key_Right || k == Qt::Key_Up || k == Qt::Key_Down ||
                k == 'I' || k == 'J' || k == 'K' || k == 'L')) {
        QList<QGraphicsItem*> selitems = selectedItems();

        std::vector<TransferFunctionEditorControlPoint*> points;
        for (auto& selitem : selitems) {
            if (auto p = qgraphicsitem_cast<TransferFunctionEditorControlPoint*>(selitem))
                points.push_back(p);
        }
        std::stable_sort(points.begin(), points.end(), comparePtr{});

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

    } else if ((k >= '0' && k <= '9') ||  // Groups selection
               k == '!' || k == '"' || k == '#' || k == Qt::Key_paragraph || k == '%' || k == '&' ||
               k == '(' || k == ')' || k == '=') {
        int group = 0;
        switch (k) {
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

        if (keyEvent->modifiers() & Qt::ControlModifier) {  // Create group
            groups_[group].clear();
            for (auto& item: getSelectedPrimitiveItems()) {
                groups_[group].push_back(item);
            }
        } else {
            if (!(keyEvent->modifiers() & Qt::ShiftModifier)) {
                for (auto& item : getSelectedPrimitiveItems()) {
                    item->setSelected(false);
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

void TransferFunctionEditor::contextMenuEvent(QGraphicsSceneContextMenuEvent* e) {
    const QPointF pos(e->scenePos());

    auto primitiveUnderMouse = getTFPrimitiveItemAt(pos);

    // change selection if primitive under the mouse is not yet in selection
    if (primitiveUnderMouse && !primitiveUnderMouse->isSelected()) {
        clearSelection();
        primitiveUnderMouse->setSelected(true);
    }
    const bool selectionEmpty = getSelectedPrimitiveItems().empty();

    QMenu menu;
    if (!primitiveUnderMouse) {
        // auto addIsovalue = menu.addAction("Add &Isovalue");
        auto addTFpoint = menu.addAction("Add TF &Point");
        auto addTFpeak = menu.addAction("Add TF P&eak");
        menu.addSeparator();

        // connect(addIsovalue, &QAction::triggered, this, [this, pos]() {});
        connect(addTFpoint, &QAction::triggered, this, [this, pos]() {
            util::KeepTrueWhileInScope k(&selectNewPrimitives_);
            addControlPoint(pos);
        });
        connect(addTFpeak, &QAction::triggered, this, [this, pos]() {
            util::KeepTrueWhileInScope k(&selectNewPrimitives_);
            addControlPointPeak(pos);
        });
    }
    auto editColor = menu.addAction("Edit &Color");
    auto duplicatePrimitive = menu.addAction("D&uplicate");
    auto deletePrimitive = menu.addAction("&Delete");
    menu.addSeparator();

    {
        editColor->setEnabled(!selectionEmpty);
        connect(editColor, &QAction::triggered, this, &TransferFunctionEditor::showColorDialog);

        duplicatePrimitive->setEnabled(!selectionEmpty);
        connect(duplicatePrimitive, &QAction::triggered, this, [this, pos]() {
            NetworkLock lock(tfProperty_);
            util::KeepTrueWhileInScope k(&selectNewPrimitives_);
            auto selection = getSelectedPrimitiveItems();
            for (auto& elem : selection) {
                addControlPoint(elem->pos().x() + relativeSceneOffset_.x * 5.0,
                                elem->getPrimitive()->getColor());
                // deselect source primitives
                elem->setSelected(false);
            }
        });

        deletePrimitive->setEnabled(!selectionEmpty);
        connect(deletePrimitive, &QAction::triggered, this, [this, pos]() {
            if (transferFunction_->empty()) return;
            NetworkLock lock(tfProperty_);
            auto selection = getSelectedPrimitiveItems();
            clearSelection();
            for (auto& elem : selection) {
                transferFunction_->removePoint(elem->getPrimitive());
            }
        });
    }

    auto maskMenu = menu.addMenu("&Mask");
    {
        // TF masking
        auto maskBegin = maskMenu->addAction("Set &Begin");
        auto maskEnd = maskMenu->addAction("Set &End");
        maskMenu->addSeparator();
        auto clearAction = maskMenu->addAction("&Clear");

        connect(maskBegin, &QAction::triggered, this, [this, pos]() {
            tfProperty_->setMask(pos.x() / width(), tfProperty_->getMask().y);
        });
        connect(maskEnd, &QAction::triggered, this, [this, pos]() {
            tfProperty_->setMask(tfProperty_->getMask().x, pos.x() / width());
        });

        connect(clearAction, &QAction::triggered, this, [this]() { tfProperty_->clearMask(); });
    }

    menu.addSeparator();

    // group selection / assignment
    auto groupSelectMenu = menu.addMenu("&Select Group");
    auto groupAssignMenu = menu.addMenu("Assign &Group");
    {
        // select TF primitives which were stored as group
        for (auto&& i : util::make_sequence(1, 11, 1)) {
            QString str = (i < 10 ? QString("Group &%1").arg(i) : "Group 1&0");
            auto action = groupSelectMenu->addAction(str);
            action->setEnabled(!groups_[i % 10].empty());
            connect(action, &QAction::triggered, [ this, group = i % 10 ]() {
                auto selection = getSelectedPrimitiveItems();

                const bool addToSelection = ((QGuiApplication::queryKeyboardModifiers() &
                                              Qt::ShiftModifier) == Qt::ShiftModifier);
                if (!addToSelection) {
                    for (auto& item : selection) {
                        item->setSelected(false);
                    }
                }
                for (auto& item : groups_[group]) {
                    item->setSelected(true);
                }
            });
        }

        // assign current selection to a group
        for (auto&& i : util::make_sequence(1, 11, 1)) {
            QString str = (i < 10 ? QString("Group &%1").arg(i) : "Group 1&0");
            auto action = groupAssignMenu->addAction(str);
            connect(action, &QAction::triggered, [ this, group = i % 10 ]() {
                groups_[group].clear();
                for (auto& item : getSelectedPrimitiveItems()) {
                    groups_[group].push_back(item);
                }
            });
        }
    }

    menu.addSeparator();
    auto tfMenu = menu.addMenu("&Transfer Function");
    {
        auto clearTF = tfMenu->addAction("&Clear");
        auto resetTF = tfMenu->addAction("&Reset");
        auto importTF = tfMenu->addAction("&Import TF...");
        auto exportTF = tfMenu->addAction("&Export TF...");

        connect(clearTF, &QAction::triggered, this, [this]() {
            NetworkLock lock(tfProperty_);
            transferFunction_->clear();
        });
        connect(resetTF, &QAction::triggered, this, [this]() {
            NetworkLock lock(tfProperty_);
            tfProperty_->resetToDefaultState();
        });
        connect(importTF, &QAction::triggered, this, &TransferFunctionEditor::importTF);
        connect(exportTF, &QAction::triggered, this, &TransferFunctionEditor::exportTF);
    }

    if (menu.exec(e->screenPos())) {
        e->accept();
    }
}

void TransferFunctionEditor::addControlPoint(const QPointF& pos) {
    dvec2 p(glm::clamp(pos.x() / width(), 0.0, 1.0), glm::clamp(pos.y() / height(), 0.0, 1.0));

    NetworkLock lock(tfProperty_);
    transferFunction_->add(p);
}

void TransferFunctionEditor::addControlPoint(double pos, const vec4& color) {
    // add control point to transfer function
    pos = glm::clamp(pos / width(), 0.0, 1.0);

    NetworkLock lock(tfProperty_);
    transferFunction_->add(TFPrimitiveData{pos, color});
}

void TransferFunctionEditor::addControlPointPeak(const QPointF& pos) {
    dvec2 p(glm::clamp(pos.x() / width(), 0.0, 1.0), glm::clamp(pos.y() / height(), 0.0, 1.0));

    NetworkLock lock(tfProperty_);
    transferFunction_->add(p);

    double normalizedOffset = relativeSceneOffset_.x * 5.0 / width();

    // add point to the left
    if (p.x > 0.0) {
        // compute intercept on y by using p.y - p.y / offset * p.x
        double y = std::max(0.0, p.y * (1.0 - p.x / normalizedOffset));
        transferFunction_->add(dvec2(std::max(p.x - normalizedOffset, 0.0), y));
    }

    // add point to the right
    if (p.y < 1.0) {
        // compute intercept on y by using p.y + p.y / offset * (p.x - 1.0)
        double y = std::max(0.0, p.y * (1.0 + (p.x - 1.0) / normalizedOffset));
        transferFunction_->add(dvec2(std::min(p.x + normalizedOffset, 1.0), y));
    }
}

void TransferFunctionEditor::removeControlPoint(TransferFunctionEditorPrimitive* controlPoint) {
    NetworkLock lock(tfProperty_);
    transferFunction_->removePoint(controlPoint->getPrimitive());
}

TransferFunctionEditorPrimitive* TransferFunctionEditor::getTFPrimitiveItemAt(
    const QPointF& pos) const {
    QList<QGraphicsItem*> graphicsItems = items(pos);

    for (auto& graphicsItem : graphicsItems) {
        if (auto item = qgraphicsitem_cast<TransferFunctionEditorPrimitive*>(graphicsItem)) {
            return item;
        }
    }

    return nullptr;
}

void TransferFunctionEditor::onTFPrimitiveDoubleClicked(const TransferFunctionEditorPrimitive*) {
    emit showColorDialog();
}

void TransferFunctionEditor::onControlPointAdded(TFPrimitive* p) {
    auto newpoint = new TransferFunctionEditorControlPoint(p, this, controlPointSize_);
    if (selectNewPrimitives_) {
        newpoint->setSelected(true);
    }
    auto it = std::upper_bound(points_.begin(), points_.end(), newpoint, comparePtr{});
    it = points_.insert(it, newpoint);
    updateConnections();
}

void TransferFunctionEditor::onControlPointRemoved(TFPrimitive* p) {
    // remove point from all groups
    for (auto& elem : groups_) {
        auto it = std::find_if(elem.begin(), elem.end(), ControlPointEquals(p));
        if (it != elem.end()) elem.erase(it);
    }

    // remove item from list of control points
    auto it = std::find_if(points_.begin(), points_.end(), ControlPointEquals(p));
    if (it != points_.end()) {
        delete *it;
        points_.erase(it);
        updateConnections();
    }
}

void TransferFunctionEditor::onControlPointChanged(const TFPrimitive*) {}

void TransferFunctionEditor::updateConnections() {
    std::stable_sort(points_.begin(), points_.end(), comparePtr{});

    while (connections_.size() < points_.size() + 1) {
        auto c = new TransferFunctionControlPointConnection();
        connections_.push_back(c);
        addItem(c);
    }
    while (connections_.size() > points_.size() + 1) {
        auto c = connections_.back();
        removeItem(c);
        delete c;
        connections_.pop_back();
    }

    connections_[0]->left_ = nullptr;
    connections_[connections_.size() - 1]->right_ = nullptr;

    for (int i = 0; i < static_cast<int>(points_.size()); ++i) {
        points_[i]->left_ = connections_[i];
        points_[i]->right_ = connections_[i + 1];
        connections_[i]->right_ = points_[i];
        connections_[i + 1]->left_ = points_[i];
    }

    for (auto& elem : connections_) {
        elem->updateShape();
    }
}

void TransferFunctionEditor::setMoveMode(int i) { moveMode_ = i; }

int TransferFunctionEditor::getMoveMode() const { return moveMode_; }

void TransferFunctionEditor::setControlPointSize(double val) {
    controlPointSize_ = val;
    for (auto e : points_) {
        e->setSize(controlPointSize_);
    }
}

void TransferFunctionEditor::setRelativeSceneOffset(const dvec2& offset) {
    relativeSceneOffset_ = offset;
}

const DataMapper& TransferFunctionEditor::getDataMapper() const { return dataMap_; }

TransferFunctionProperty* TransferFunctionEditor::getTransferFunctionProperty() {
    return tfProperty_;
}

std::vector<TFPrimitive*> TransferFunctionEditor::getSelectedPrimitives() const {
    std::vector<TFPrimitive*> selection;
    for (auto& elem : selectedItems()) {
        if (auto p = qgraphicsitem_cast<TransferFunctionEditorPrimitive*>(elem)) {
            selection.push_back(p->getPrimitive());
        }
    }

    return selection;
}

std::vector<TransferFunctionEditorPrimitive*> TransferFunctionEditor::getSelectedPrimitiveItems()
    const {
    std::vector<TransferFunctionEditorPrimitive*> selection;
    for (auto& elem : selectedItems()) {
        if (auto p = qgraphicsitem_cast<TransferFunctionEditorPrimitive*>(elem)) {
            selection.push_back(p);
        }
    }

    return selection;
}

void TransferFunctionEditor::showHelpText(bool show) {

}

}  // namespace inviwo
