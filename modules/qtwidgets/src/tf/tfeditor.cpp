/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2021 Inviwo Foundation
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

#include <modules/qtwidgets/tf/tfeditor.h>

#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/datastructures/transferfunction.h>
#include <inviwo/core/datastructures/tfprimitive.h>
#include <inviwo/core/datastructures/tfprimitiveset.h>

#include <modules/qtwidgets/tf/tfeditorcontrolpoint.h>
#include <modules/qtwidgets/tf/tfeditorisovalue.h>
#include <modules/qtwidgets/tf/tfeditorprimitive.h>
#include <modules/qtwidgets/tf/tfcontrolpointconnection.h>
#include <modules/qtwidgets/tf/tfutils.h>
#include <modules/qtwidgets/tf/tfpropertyconcept.h>
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
#include <QRectF>
#include <QTransform>
#include <warn/pop>

#include <fmt/format.h>

namespace inviwo {

class ControlPointEquals {
public:
    ControlPointEquals(const TFPrimitive& p) : p_(p) {}
    bool operator()(TFEditorPrimitive* editorPoint) { return editorPoint->getPrimitive() == p_; }
    bool operator<(TFEditorPrimitive* editorPoint) {
        return editorPoint->getPrimitive().getPosition() < p_.getPosition();
    }

private:
    const TFPrimitive& p_;
};

template <typename Operator, typename Proj = util::identity>
constexpr auto derefOperator(Operator&& op, Proj&& proj = {}) {
    return [o = std::forward<Operator>(op), p = std::forward<Proj>(proj)](auto* a, auto* b) {
        return std::invoke(o, std::invoke(p, *a), std::invoke(p, *b));
    };
}

TFEditor::TFEditor(util::TFPropertyConcept* tfProperty,
                   const std::vector<TFPrimitiveSet*>& primitiveSets, QWidget* parent)
    : QGraphicsScene(parent)
    , tfPropertyPtr_(tfProperty)
    , tfSets_(primitiveSets)
    , dataMap_()
    , groups_()
    , moveMode_(0)
    , selectNewPrimitives_(false) {

    setSceneRect(0.0, 0.0, 512.0, 512.0);
    mouseDrag_ = false;

    // The default BSP tree tends to crash...
    setItemIndexMethod(QGraphicsScene::NoIndex);

    if (auto port = tfProperty->getVolumeInport()) {
        const auto portChange = [this, port]() {
            dataMap_ = port->hasData() ? port->getData()->dataMap_ : DataMapper{};
        };
        portCallBacks_.push_back(port->onChangeScoped(portChange));
        portCallBacks_.push_back(port->onConnectScoped(portChange));
        portCallBacks_.push_back(port->onDisconnectScoped(portChange));
        portChange();
    }

    // initialize editor with current tf
    if (tfPropertyPtr_->hasTF()) {
        for (auto& p : *tfPropertyPtr_->getTransferFunction()) {
            createControlPointItem(p);
        }
        // the next primitive inserted with Control+left click should be a control point
        lastInsertedPrimitiveType_ = TFEditorPrimitive::TFEditorControlPointType;
    }
    // and isovalues
    if (tfPropertyPtr_->hasIsovalues()) {
        for (auto& p : *tfPropertyPtr_->getIsovalues()) {
            createIsovalueItem(p);
        }
        // the next primitive inserted with Control+left click should be an isovalue,
        // but only if there is not TF
        if (!tfPropertyPtr_->hasTF()) {
            lastInsertedPrimitiveType_ = TFEditorPrimitive::TFEditorIsovalueType;
        }
    }

    for (int i = 0; i < 10; ++i) {
        groups_.push_back(std::vector<TFEditorPrimitive*>());
    }
}

TFEditor::~TFEditor() {
    for (auto& elem : points_) delete elem;
    points_.clear();

    for (auto& elem : connections_) delete elem;
    connections_.clear();

    for (auto& item : isovalueItems_) delete item;
    isovalueItems_.clear();
}

void TFEditor::mousePressEvent(QGraphicsSceneMouseEvent* e) {
#include <warn/push>
#include <warn/ignore/switch-enum>
    switch (e->button()) {
        case Qt::LeftButton:
            if (auto start = getTFPrimitiveItemAt(e->scenePos())) {
                auto selected = getSelectedPrimitiveItems();
                mouseDrag_ = true;

                // inform all selected TF primitives about imminent move (need to cache position)
                for (auto& item : selected) item->beginMouseDrag();

                selected.push_back(start);  // start might not have been selected yet

                constexpr auto lessx =
                    derefOperator(std::less<>{}, [](const auto& p) { return p.pos().x(); });
                constexpr auto lessy =
                    derefOperator(std::less<>{}, [](const auto& p) { return p.pos().y(); });

                const auto xrange = std::minmax_element(selected.begin(), selected.end(), lessx);
                const auto yrange = std::minmax_element(selected.begin(), selected.end(), lessy);

                const auto selRect =
                    QRectF{QPointF{(*xrange.first)->pos().x(), (*yrange.first)->pos().y()},
                           QSizeF{(*xrange.second)->pos().x() - (*xrange.first)->pos().x(),
                                  (*yrange.second)->pos().y() - (*yrange.first)->pos().y()}};

                const std::array corners = {selRect.bottomLeft(), selRect.bottomRight(),
                                            selRect.topLeft(), selRect.topRight()};

                rigidTransRef_ = *std::max_element(
                    corners.begin(), corners.end(), [&](const QPointF& a, const QPointF& b) {
                        const auto da = a - start->pos();
                        const auto db = b - start->pos();
                        return QPointF::dotProduct(da, da) < QPointF::dotProduct(db, db);
                    });
                dragItem_ = start;
                dragPos_ = e->scenePos();

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

void TFEditor::mouseMoveEvent(QGraphicsSceneMouseEvent* e) {
    mouseMovedSincePress_ = true;
    if (mouseDrag_ && ((e->buttons() & Qt::LeftButton) == Qt::LeftButton)) {
        e->accept();
        emit updateBegin();
        // Prevent network evaluations while moving control point
        NetworkLock lock;

        const bool altPressed =
            ((QGuiApplication::queryKeyboardModifiers() & Qt::AltModifier) == Qt::AltModifier);

        constexpr auto less = derefOperator(std::less<>{}, &TFEditorPrimitive::getPosition);
        constexpr auto greater = derefOperator(std::greater<>{}, &TFEditorPrimitive::getPosition);

        auto selection = getSelectedPrimitiveItems();
        if (altPressed) {
            const auto org = dragItem_->pos() - rigidTransRef_;
            const auto pos = e->scenePos() - rigidTransRef_;

            if (selection.size() > 1) {
                const auto translate =
                    QTransform::fromTranslate(-rigidTransRef_.x(), -rigidTransRef_.y());

                const auto xscale = std::abs(org.x()) > 0.0001 ? pos.x() / org.x() : 1.0;
                const auto yscale = std::abs(org.y()) > 0.0001 ? pos.y() / org.y() : 1.0;

                const auto scale = QTransform::fromScale(xscale, yscale);
                const auto trans = translate * scale * translate.inverted();

                if (xscale > 1.0) {
                    std::stable_sort(selection.begin(), selection.end(), greater);
                } else {
                    std::stable_sort(selection.begin(), selection.end(), less);
                }

                for (auto& item : selection) {
                    auto oldPos = item->pos();
                    auto newPos = utilqt::clamp(trans.map(oldPos), sceneRect());
                    item->setPos(newPos);
                }
            }
        } else {
            const auto delta = e->scenePos() - dragPos_;
            if (delta.x() > 0) {
                std::stable_sort(selection.begin(), selection.end(), greater);
            } else {
                std::stable_sort(selection.begin(), selection.end(), less);
            }

            for (auto& item : selection) {
                auto oldPos = item->pos();
                auto newPos = utilqt::clamp(oldPos + delta, sceneRect());
                item->setPos(newPos);
            }
        }
        dragPos_ = e->scenePos();
        emit updateEnd();
    } else {
        QGraphicsScene::mouseMoveEvent(e);
    }
}

void TFEditor::mouseReleaseEvent(QGraphicsSceneMouseEvent* e) {
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
                    switch (lastInsertedPrimitiveType_) {
                        case TFEditorPrimitive::TFEditorControlPointType:
                            addControlPoint(e->scenePos());
                            e->accept();
                            break;
                        case TFEditorPrimitive::TFEditorIsovalueType:
                            addIsovalue(e->scenePos());
                            e->accept();
                            break;
                        case TFEditorPrimitive::TFEditorUnknownPrimitiveType:
                        default:
                            break;
                    }
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

void TFEditor::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* e) {
    mouseDoubleClick_ = true;
    QGraphicsScene::mouseDoubleClickEvent(e);
}

void TFEditor::keyPressEvent(QKeyEvent* keyEvent) {
    // these factors are applied when holding shift (increasing step size) or alt (decreasing step
    // size) when moving control points with the keyboard
    const double stepUpScalingFactor = 5.0;
    const double stepDownScalingFactor = 0.2;

    int k = keyEvent->key();
    keyEvent->accept();

    NetworkLock lock(tfPropertyPtr_->getProperty());

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

        emit updateBegin();
        QList<QGraphicsItem*> selitems = selectedItems();
        for (auto& selitem : selitems) {
            selitem->setPos(selitem->pos() + delta);
        }
        emit updateEnd();

    } else if (keyEvent->modifiers() & Qt::ControlModifier &&  // Modify selection
               (k == Qt::Key_Left || k == Qt::Key_Right || k == Qt::Key_Up || k == Qt::Key_Down ||
                k == 'I' || k == 'J' || k == 'K' || k == 'L')) {
        QList<QGraphicsItem*> selitems = selectedItems();

        std::vector<TFEditorControlPoint*> points;
        for (auto& selitem : selitems) {
            if (auto p = qgraphicsitem_cast<TFEditorControlPoint*>(selitem)) points.push_back(p);
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
            for (auto& item : getSelectedPrimitiveItems()) {
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

void TFEditor::contextMenuEvent(QGraphicsSceneContextMenuEvent* e) {
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
        if (tfPropertyPtr_->hasTF()) {
            auto addTFpoint = menu.addAction("Add TF &Point");
            auto addTFpeak = menu.addAction("Add TF P&eak");

            connect(addTFpoint, &QAction::triggered, this, [this, pos]() {
                util::KeepTrueWhileInScope k(&selectNewPrimitives_);
                addControlPoint(pos);
            });
            connect(addTFpeak, &QAction::triggered, this, [this, pos]() {
                util::KeepTrueWhileInScope k(&selectNewPrimitives_);
                addControlPointPeak(pos);
            });
        }
        if (tfPropertyPtr_->hasIsovalues()) {
            auto addIsovalue = menu.addAction("Add &Isovalue");
            connect(addIsovalue, &QAction::triggered, this, [this, pos]() {
                util::KeepTrueWhileInScope k(&selectNewPrimitives_);
                TFEditor::addIsovalue(pos);
            });
        }

        menu.addSeparator();
    }

    auto editColor = menu.addAction("Edit &Color");
    auto duplicatePrimitive = menu.addAction("D&uplicate");
    auto deletePrimitive = menu.addAction("&Delete");
    auto clearTF = menu.addAction("&Clear");
    auto resetTF = menu.addAction("&Reset");

    menu.addSeparator();

    {
        editColor->setEnabled(!selectionEmpty);
        connect(editColor, &QAction::triggered, this, &TFEditor::showColorDialog);

        duplicatePrimitive->setEnabled(!selectionEmpty);
        connect(duplicatePrimitive, &QAction::triggered, this, [this]() {
            NetworkLock lock(tfPropertyPtr_->getProperty());
            util::KeepTrueWhileInScope k(&selectNewPrimitives_);
            auto selection = getSelectedPrimitiveItems();
            for (auto& elem : selection) {
                addControlPoint(elem->pos().x() + relativeSceneOffset_.x * 5.0,
                                elem->getPrimitive().getColor());
                // deselect source primitives
                elem->setSelected(false);
            }
        });

        deletePrimitive->setEnabled(!selectionEmpty);
        connect(deletePrimitive, &QAction::triggered, this, [this]() {
            NetworkLock lock(tfPropertyPtr_->getProperty());
            auto selection = getSelectedPrimitiveItems();
            clearSelection();
            for (auto& elem : selection) {
                removeControlPoint(elem);
            }
        });

        connect(clearTF, &QAction::triggered, this, [this]() {
            NetworkLock lock(tfPropertyPtr_->getProperty());
            for (auto& elem : tfSets_) {
                elem->clear();
            }
        });
        connect(resetTF, &QAction::triggered, this, [this]() {
            NetworkLock lock(tfPropertyPtr_->getProperty());
            tfPropertyPtr_->getProperty()->resetToDefaultState();
        });
    }

    auto transformMenu = menu.addMenu("Trans&form");
    menu.addSeparator();

    {
        auto distributeAlphaEvenly = transformMenu->addAction("&Distribute Alpha Evenly");
        auto distributePositionEvenly = transformMenu->addAction("&Distribute Position Evenly");
        transformMenu->addSeparator();
        auto alignAlphaToMean = transformMenu->addAction("&Align Alpha to Mean");
        auto alignAlphaToTop = transformMenu->addAction("&Align Alpha to Top");
        auto alignAlphaToBottom = transformMenu->addAction("&Align Alpha to Bottom");
        transformMenu->addSeparator();
        auto alignPositionToMean = transformMenu->addAction("&Align Position to Mean");
        auto alignPositionToLeft = transformMenu->addAction("&Align Position to Left");
        auto alignPositionToRight = transformMenu->addAction("&Align Position to Right");
        transformMenu->addSeparator();
        auto flipPositions = transformMenu->addAction("&Horizontal Flip");
        auto interpolateAlpha = transformMenu->addAction("&Interpolate Alpha");

        connect(distributeAlphaEvenly, &QAction::triggered, this, [this]() {
            NetworkLock lock(tfPropertyPtr_->getProperty());
            util::distributeAlphaEvenly(getAllOrSelectedPrimitives());
        });

        connect(distributePositionEvenly, &QAction::triggered, this, [this]() {
            NetworkLock lock(tfPropertyPtr_->getProperty());
            util::distributePositionEvenly(getAllOrSelectedPrimitives());
        });

        connect(alignAlphaToMean, &QAction::triggered, this, [this]() {
            NetworkLock lock(tfPropertyPtr_->getProperty());
            util::alignAlphaToMean(getAllOrSelectedPrimitives());
        });

        connect(alignAlphaToTop, &QAction::triggered, this, [this]() {
            NetworkLock lock(tfPropertyPtr_->getProperty());
            util::alignAlphaToTop(getAllOrSelectedPrimitives());
        });

        connect(alignAlphaToBottom, &QAction::triggered, this, [this]() {
            NetworkLock lock(tfPropertyPtr_->getProperty());
            util::alignAlphaToBottom(getAllOrSelectedPrimitives());
        });

        connect(alignPositionToMean, &QAction::triggered, this, [this]() {
            NetworkLock lock(tfPropertyPtr_->getProperty());
            util::alignPositionToMean(getAllOrSelectedPrimitives());
        });

        connect(alignPositionToLeft, &QAction::triggered, this, [this]() {
            NetworkLock lock(tfPropertyPtr_->getProperty());
            util::alignPositionToLeft(getAllOrSelectedPrimitives());
        });

        connect(alignPositionToRight, &QAction::triggered, this, [this]() {
            NetworkLock lock(tfPropertyPtr_->getProperty());
            util::alignPositionToRight(getAllOrSelectedPrimitives());
        });

        connect(flipPositions, &QAction::triggered, this, [this]() {
            NetworkLock lock(tfPropertyPtr_->getProperty());
            util::flipPositions(getAllOrSelectedPrimitives());
        });

        connect(interpolateAlpha, &QAction::triggered, this, [this]() {
            NetworkLock lock(tfPropertyPtr_->getProperty());
            util::interpolateAlpha(getAllOrSelectedPrimitives());
        });
    }

    if (tfPropertyPtr_->hasTF()) {

        auto simplify = menu.addMenu("Simplify");
        menu.addSeparator();

        auto makeSimple = [this](double delta) {
            return [this, delta]() {
                NetworkLock lock(tfPropertyPtr_->getProperty());
                auto tf = tfPropertyPtr_->getTransferFunction();
                auto simple = TransferFunction::simplify(tf->get(), delta);
                tf->clear();
                tf->add(simple);
            };
        };

        for (double delta : {0.001, 0.002, 0.005, 0.01, 0.02, 0.05, 0.10, 0.20}) {
            auto action = simplify->addAction(utilqt::toQString(fmt::format("{:05.3f}", delta)));
            connect(action, &QAction::triggered, this, makeSimple(delta));
        }
    }

    if (tfPropertyPtr_->supportsMask()) {
        auto maskMenu = menu.addMenu("&Mask");
        // TF masking
        auto maskBegin = maskMenu->addAction("Set &Begin");
        auto maskEnd = maskMenu->addAction("Set &End");
        maskMenu->addSeparator();
        auto clearAction = maskMenu->addAction("&Clear");

        connect(maskBegin, &QAction::triggered, this, [this, pos]() {
            tfPropertyPtr_->setMask(pos.x() / width(), tfPropertyPtr_->getMask().y);
        });
        connect(maskEnd, &QAction::triggered, this, [this, pos]() {
            tfPropertyPtr_->setMask(tfPropertyPtr_->getMask().x, pos.x() / width());
        });

        connect(clearAction, &QAction::triggered, this, [this]() { tfPropertyPtr_->clearMask(); });
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
            connect(action, &QAction::triggered, [this, group = i % 10]() {
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
            connect(action, &QAction::triggered, [this, group = i % 10]() {
                groups_[group].clear();
                for (auto& item : getSelectedPrimitiveItems()) {
                    groups_[group].push_back(item);
                }
            });
        }
    }

    menu.addSeparator();
    {
        if (tfPropertyPtr_->hasTF()) {
            util::addTFPresetsMenu(e->widget(), &menu, tfPropertyPtr_->getTFProperty());
            util::addTFColorbrewerPresetsMenu(e->widget(), &menu, tfPropertyPtr_->getTFProperty());
            menu.addSeparator();
        }

        if (tfPropertyPtr_->hasTF()) {
            auto importTF = menu.addAction("&Import TF...");
            auto exportTF = menu.addAction("&Export TF...");
            connect(importTF, &QAction::triggered, this,
                    [this]() { emit TFEditor::importTF(*tfPropertyPtr_->getTransferFunction()); });
            connect(exportTF, &QAction::triggered, this,
                    [this]() { emit TFEditor::exportTF(*tfPropertyPtr_->getTransferFunction()); });
            menu.addSeparator();
        }
        if (tfPropertyPtr_->hasIsovalues()) {
            auto importTF = menu.addAction("&Import Isovalues...");
            auto exportTF = menu.addAction("&Export Isovalues...");
            connect(importTF, &QAction::triggered, this,
                    [this]() { emit TFEditor::importTF(*tfPropertyPtr_->getIsovalues()); });
            connect(exportTF, &QAction::triggered, this,
                    [this]() { emit TFEditor::exportTF(*tfPropertyPtr_->getIsovalues()); });
        }
    }

    if (menu.exec(e->screenPos())) {
        e->accept();
    }
}

void TFEditor::addControlPoint(const QPointF& pos) {
    if (!tfPropertyPtr_->hasTF()) return;
    dvec2 p(glm::clamp(pos.x() / width(), 0.0, 1.0), glm::clamp(pos.y() / height(), 0.0, 1.0));

    NetworkLock lock(tfPropertyPtr_->getProperty());
    tfPropertyPtr_->getTransferFunction()->add(p);
}

void TFEditor::addControlPoint(double pos, const vec4& color) {
    if (!tfPropertyPtr_->hasTF()) return;
    pos = glm::clamp(pos / width(), 0.0, 1.0);

    NetworkLock lock(tfPropertyPtr_->getProperty());
    tfPropertyPtr_->getTransferFunction()->add(TFPrimitiveData{pos, color});
}

void TFEditor::addControlPointPeak(const QPointF& pos) {
    if (!tfPropertyPtr_->hasTF()) return;
    dvec2 p(glm::clamp(pos.x() / width(), 0.0, 1.0), glm::clamp(pos.y() / height(), 0.0, 1.0));

    NetworkLock lock(tfPropertyPtr_->getProperty());
    tfPropertyPtr_->getTransferFunction()->add(p);

    double normalizedOffset = relativeSceneOffset_.x * 5.0 / width();

    // add point to the left
    if (p.x > 0.0) {
        // compute intercept on y by using p.y - p.y / offset * p.x
        double y = std::max(0.0, p.y * (1.0 - p.x / normalizedOffset));
        tfPropertyPtr_->getTransferFunction()->add(dvec2(std::max(p.x - normalizedOffset, 0.0), y));
    }

    // add point to the right
    if (p.y < 1.0) {
        // compute intercept on y by using p.y + p.y / offset * (p.x - 1.0)
        double y = std::max(0.0, p.y * (1.0 + (p.x - 1.0) / normalizedOffset));
        tfPropertyPtr_->getTransferFunction()->add(dvec2(std::min(p.x + normalizedOffset, 1.0), y));
    }
}

void TFEditor::addIsovalue(const QPointF& pos) {
    if (!tfPropertyPtr_->hasIsovalues()) return;
    dvec2 p(glm::clamp(pos.x() / width(), 0.0, 1.0), glm::clamp(pos.y() / height(), 0.0, 1.0));

    NetworkLock lock(tfPropertyPtr_->getProperty());
    tfPropertyPtr_->getIsovalues()->add(p);
}

void TFEditor::removeControlPoint(TFEditorPrimitive* controlPoint) {
    NetworkLock lock(tfPropertyPtr_->getProperty());
    for (auto& elem : tfSets_) {
        if (elem->remove(controlPoint->getPrimitive())) break;
    }
}

TFEditorPrimitive* TFEditor::getTFPrimitiveItemAt(const QPointF& pos) const {
    QList<QGraphicsItem*> graphicsItems = items(pos);

    for (auto& graphicsItem : graphicsItems) {
        if (auto item = qgraphicsitem_cast<TFEditorPrimitive*>(graphicsItem)) {
            return item;
        }
    }
    return nullptr;
}

void TFEditor::onTFPrimitiveDoubleClicked(const TFEditorPrimitive*) { emit showColorDialog(); }

void TFEditor::onControlPointAdded(TFPrimitive& p) {
    const bool isIsovalue =
        tfPropertyPtr_->hasIsovalues() && util::contains(*tfPropertyPtr_->getIsovalues(), p);
    const bool isTFpoint =
        tfPropertyPtr_->hasTF() && util::contains(*tfPropertyPtr_->getTransferFunction(), p);

    if (isIsovalue) {
        createIsovalueItem(p);
        lastInsertedPrimitiveType_ = TFEditorPrimitive::TFEditorIsovalueType;
    } else if (isTFpoint) {
        createControlPointItem(p);
        lastInsertedPrimitiveType_ = TFEditorPrimitive::TFEditorControlPointType;
    }
}

void TFEditor::createControlPointItem(TFPrimitive& p) {
    auto newpoint = new TFEditorControlPoint(p, this, controlPointSize_);
    if (selectNewPrimitives_) {
        newpoint->setSelected(true);
    }
    auto it = std::upper_bound(points_.begin(), points_.end(), newpoint, comparePtr{});
    it = points_.insert(it, newpoint);
    updateConnections();
}

void TFEditor::createIsovalueItem(TFPrimitive& p) {
    auto newpoint = new TFEditorIsovalue(p, this, controlPointSize_);
    if (selectNewPrimitives_) {
        newpoint->setSelected(true);
    }
    auto it =
        std::upper_bound(isovalueItems_.begin(), isovalueItems_.end(), newpoint, comparePtr{});
    it = isovalueItems_.insert(it, newpoint);
}

void TFEditor::onControlPointRemoved(TFPrimitive& p) {
    // remove point from all groups
    for (auto& elem : groups_) {
        auto it = std::find_if(elem.begin(), elem.end(), ControlPointEquals(p));
        if (it != elem.end()) elem.erase(it);
    }

    // remove item from list of control points
    if (tfPropertyPtr_->hasTF()) {
        auto it = std::find_if(points_.begin(), points_.end(), ControlPointEquals(p));
        if (it != points_.end()) {
            delete *it;
            points_.erase(it);
            updateConnections();
        }
    }
    if (tfPropertyPtr_->hasIsovalues()) {
        auto it = std::find_if(isovalueItems_.begin(), isovalueItems_.end(), ControlPointEquals(p));
        if (it != isovalueItems_.end()) {
            delete *it;
            isovalueItems_.erase(it);
        }
    }
}

void TFEditor::onControlPointChanged(const TFPrimitive&) {}

void TFEditor::updateConnections() {
    std::stable_sort(points_.begin(), points_.end(), comparePtr{});

    while (connections_.size() < points_.size() + 1) {
        auto c = new TFControlPointConnection();
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

    for (size_t i = 0; i < points_.size(); ++i) {
        points_[i]->left_ = connections_[i];
        points_[i]->right_ = connections_[i + 1];
        connections_[i]->right_ = points_[i];
        connections_[i + 1]->left_ = points_[i];
    }

    for (auto& elem : connections_) {
        elem->updateShape();
    }
}

void TFEditor::setMoveMode(int i) { moveMode_ = i; }

int TFEditor::getMoveMode() const { return moveMode_; }

void TFEditor::setControlPointSize(double val) {
    controlPointSize_ = val;
    for (auto e : points_) {
        e->setSize(controlPointSize_);
    }
}

void TFEditor::setRelativeSceneOffset(const dvec2& offset) { relativeSceneOffset_ = offset; }

const DataMapper& TFEditor::getDataMapper() const { return dataMap_; }

std::vector<TFPrimitive*> TFEditor::getSelectedPrimitives() const {
    std::vector<TFPrimitive*> selection;
    for (auto& elem : selectedItems()) {
        if (auto p = qgraphicsitem_cast<TFEditorPrimitive*>(elem)) {
            selection.push_back(&p->getPrimitive());
        }
    }

    return selection;
}

std::vector<TFPrimitive*> TFEditor::getAllPrimitives() const {
    std::vector<TFPrimitive*> res;

    std::transform(points_.begin(), points_.end(), std::back_inserter(res),
                   [](TFEditorControlPoint* p) { return &p->getPrimitive(); });

    std::transform(isovalueItems_.begin(), isovalueItems_.end(), std::back_inserter(res),
                   [](TFEditorIsovalue* p) { return &p->getPrimitive(); });

    return res;
}

std::vector<TFPrimitive*> TFEditor::getAllOrSelectedPrimitives() const {
    if (auto sel = getSelectedPrimitives(); !sel.empty()) {
        return sel;
    } else {
        return getAllPrimitives();
    }
}

std::vector<TFEditorPrimitive*> TFEditor::getSelectedPrimitiveItems() const {
    std::vector<TFEditorPrimitive*> selection;
    for (auto& elem : selectedItems()) {
        if (auto p = qgraphicsitem_cast<TFEditorPrimitive*>(elem)) {
            selection.push_back(p);
        }
    }

    return selection;
}

dvec2 TFEditor::getZoom() const {
    const auto zoomH = tfPropertyPtr_->getZoomH();
    const auto zoomV = tfPropertyPtr_->getZoomV();
    return {zoomH.y - zoomH.x, zoomV.y - zoomV.x};
}

}  // namespace inviwo
