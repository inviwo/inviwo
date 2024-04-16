/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2024 Inviwo Foundation
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

#include <inviwo/core/datastructures/datamapper.h>          // for DataMapper
#include <inviwo/core/datastructures/tfprimitive.h>         // for TFPrimitive, operator==, TFPr...
#include <inviwo/core/datastructures/tfprimitiveset.h>      // for TFPrimitiveSet, alignAlphaToB...
#include <inviwo/core/network/networklock.h>                // for NetworkLock
#include <inviwo/core/ports/volumeport.h>                   // for VolumeInport
#include <inviwo/core/properties/property.h>                // for Property
#include <inviwo/core/util/glmvec.h>                        // for dvec2, vec4
#include <inviwo/core/util/raiiutils.h>                     // for KeepTrueWhileInScope
#include <inviwo/core/util/stdextensions.h>                 // for contains
#include <inviwo/core/util/transformiterator.h>             // for TransformIterator
#include <inviwo/core/util/typetraits.h>                    // for identity
#include <inviwo/core/util/vectoroperations.h>              // for comparePtr
#include <inviwo/core/util/zip.h>                           // for make_sequence, sequence, sequ...
#include <modules/qtwidgets/inviwoqtutils.h>                // for clamp, toQString
#include <modules/qtwidgets/tf/tfcontrolpointconnection.h>  // for TFControlPointConnection
#include <modules/qtwidgets/tf/tfeditorcontrolpoint.h>      // for TFEditorControlPoint, operator<
#include <modules/qtwidgets/tf/tfeditorisovalue.h>          // for TFEditorIsovalue, operator<
#include <modules/qtwidgets/tf/tfeditorprimitive.h>         // for TFEditorPrimitive, TFEditorPr...
#include <modules/qtwidgets/tf/tfpropertyconcept.h>         // for TFPropertyConcept
#include <modules/qtwidgets/tf/tfutils.h>                   // for addTFColorbrewerPresetsMenu
#include <modules/qtwidgets/tf/tfeditormask.h>

#include <algorithm>         // for stable_sort, find_if, max
#include <array>             // for array
#include <cmath>             // for abs
#include <cstdlib>           // for abs, size_t
#include <initializer_list>  // for initializer_list
#include <iterator>          // for back_insert_iterator, back_in...
#include <type_traits>       // for remove_extent_t
#include <utility>           // for pair, forward

#include <QAction>                         // for QAction
#include <QFlags>                          // for QFlags, operator==
#include <QGraphicsItem>                   // for qgraphicsitem_cast, QGraphics...
#include <QGraphicsSceneContextMenuEvent>  // for QGraphicsSceneContextMenuEvent
#include <QGraphicsSceneMouseEvent>        // for QGraphicsSceneMouseEvent
#include <QGraphicsView>                   // for QGraphicsView, QGraphicsView:...
#include <QGuiApplication>                 // for QGuiApplication
#include <QKeyEvent>                       // for QKeyEvent
#include <QList>                           // for QList, QList<>::iterator
#include <QMenu>                           // for QMenu
#include <QPoint>                          // for operator-, operator+
#include <QRectF>                          // for QRectF
#include <QSizeF>                          // for QSizeF
#include <QString>                         // for QString
#include <QTransform>                      // for QTransform
#include <QWidget>                         // for QWidget
#include <Qt>                              // for ControlModifier, ShiftModifier
#include <fmt/core.h>                      // for format
#include <glm/common.hpp>                  // for clamp
#include <glm/vec2.hpp>                    // for vec<>::(anonymous), vec

class QGraphicsSceneMouseEvent;
class QKeyEvent;

namespace inviwo {

namespace {

template <typename Operator, typename Proj = util::identity>
constexpr auto derefOperator(Operator&& op, Proj&& proj = {}) {
    return [o = std::forward<Operator>(op), p = std::forward<Proj>(proj)](auto* a, auto* b) {
        return std::invoke(o, std::invoke(p, *a), std::invoke(p, *b));
    };
}

template <typename T>
void createAndInsertPrimitive(std::vector<std::unique_ptr<TFEditorPrimitive>>& list, TFPrimitive& p,
                              TFEditor* editor, bool selected) {

    auto newPoint = std::make_unique<T>(p);
    newPoint->setSelected(selected);
    editor->addItem(newPoint.get());
    auto it = std::upper_bound(list.begin(), list.end(), newPoint,
                               [](const auto& a, const auto& b) { return *a < *b; });
    list.insert(it, std::move(newPoint));
}

}  // namespace

TFEditor::TFEditor(TFPropertyConcept* tfProperty, QWidget* parent)
    : QGraphicsScene(parent)
    , concept_(tfProperty)
    , primitives_{}
    , activeSet_{nullptr}
    , mouse_{}
    , groups_(10)
    , moveMode_{TFMoveMode::Free}
    , maskMin_{std::make_unique<TFEditorMaskMin>(concept_)}
    , maskMax_{std::make_unique<TFEditorMaskMax>(concept_)}
    , selectNewPrimitives_{false} {

    setItemIndexMethod(QGraphicsScene::NoIndex);

    setSceneRect(0.0, 0.0, 1.0, 1.0);

    for (auto* set : concept_->sets()) {
        auto& items = primitives_[set];
        if (auto* tf = dynamic_cast<TransferFunction*>(set)) {
            items.connected = true;
            for (auto& p : *tf) {
                createAndInsertPrimitive<TFEditorControlPoint>(items.points, p, this,
                                                               selectNewPrimitives_);
            }
        } else if (auto* iso = dynamic_cast<IsoValueCollection*>(set)) {
            items.connected = false;
            for (auto& p : *iso) {
                createAndInsertPrimitive<TFEditorIsovalue>(items.points, p, this,
                                                           selectNewPrimitives_);
            }
        }
        set->addObserver(this);
    }
    activeSet_ = primitives_.begin()->first;
    updateConnections();

    addItem(maskMin_.get());
    addItem(maskMax_.get());
    maskMin_->setPos(QPointF{concept_->getMask().x, 0.5});
    maskMax_->setPos(QPointF{concept_->getMask().y, 0.5});
}

TFEditor::~TFEditor() = default;

void TFEditor::onTFPrimitiveAdded(const TFPrimitiveSet& set, TFPrimitive& p) {
    auto it = primitives_.find(&set);
    if (it == primitives_.end()) return;

    auto& items = it->second;
    if (dynamic_cast<const TransferFunction*>(&set)) {
        items.connected = true;
        createAndInsertPrimitive<TFEditorControlPoint>(items.points, p, this, selectNewPrimitives_);
        updateConnections();
    } else if (dynamic_cast<const IsoValueCollection*>(&set)) {
        items.connected = false;
        createAndInsertPrimitive<TFEditorIsovalue>(items.points, p, this, selectNewPrimitives_);
    }
}
void TFEditor::onTFPrimitiveRemoved(const TFPrimitiveSet& set, TFPrimitive& p) {
    // remove point from all groups
    for (auto& group : groups_) {
        std::erase_if(group, [&](auto& e) { return &e->getPrimitive() == &p; });
    }

    auto it = primitives_.find(&set);
    if (it == primitives_.end()) return;
    auto& items = it->second;
    std::erase_if(items.points, [&](auto& e) { return &e->getPrimitive() == &p; });
    if (items.connected) {
        updateConnections();
    }
}
void TFEditor::onTFPrimitiveChanged(const TFPrimitiveSet& set, const TFPrimitive& p) {}
void TFEditor::onTFTypeChanged(const TFPrimitiveSet& set, TFPrimitiveSetType type) {}

void TFEditor::mousePressEvent(QGraphicsSceneMouseEvent* e) {
    if (e->button() == Qt::LeftButton) {
        if (auto* start = getTFPrimitiveItemAt(e->scenePos())) {
            auto selected = getSelectedPrimitiveItems();
            selected.push_back(start);  // start might not have been selected yet

            activeSet_ = findSet(&start->getPrimitive());

            // inform all selected TF primitives about imminent move (need to cache position)
            std::ranges::for_each(selected, [](auto* item) { item->beginMouseDrag(); });

            mouse_.dragItem = start;
            mouse_.rigid = calcTransformRef(selected, start);
        } else {
            mouse_.dragItem = nullptr;
            views().front()->setDragMode(QGraphicsView::RubberBandDrag);
        }
    }
    QGraphicsScene::mousePressEvent(e);
}

void TFEditor::mouseMoveEvent(QGraphicsSceneMouseEvent* e) {
    if (mouse_.dragItem && e->buttons() == Qt::LeftButton) {
        e->accept();
        emit updateBegin();
        // Prevent network evaluations while moving control point
        const NetworkLock lock{concept_->getProperty()};

        auto selection = getSelectedPrimitiveItems();
        if (!selection.empty()) {
            activeSet_ = findSet(&selection.front()->getPrimitive());
        }
        move(selection, calcTransform(e->scenePos(), e->lastScenePos()), sceneRect());
        emit updateEnd();
    } else {
        QGraphicsScene::mouseMoveEvent(e);
    }
}

void TFEditor::mouseReleaseEvent(QGraphicsSceneMouseEvent* e) {
    if (e->button() == Qt::LeftButton) {
        // left mouse button and no movement -> add new point if there is no selection
        // add new TF primitive when control is pressed and nothing is below the cursor
        if (e->modifiers() == Qt::ControlModifier &&
            e->scenePos() == e->buttonDownScenePos(Qt::LeftButton)) {
            clearSelection();
            addPoint(e->scenePos());
        } else {
            // disable rubber band selection
            views().front()->setDragMode(QGraphicsView::NoDrag);
        }
    }
    mouse_.dragItem = nullptr;

    if (!e->isAccepted()) {
        QGraphicsScene::mouseReleaseEvent(e);
    }
}

void TFEditor::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* e) {
    if (e->modifiers() == Qt::NoModifier) {
        if (getTFPrimitiveItemAt(e->scenePos()) != nullptr) {
            e->accept();
            emit showColorDialog();
        } else {
            e->accept();
            clearSelection();
            addPoint(e->scenePos());
        }
    }
    QGraphicsScene::mouseDoubleClickEvent(e);
}

void TFEditor::keyPressEvent(QKeyEvent* keyEvent) {
    const NetworkLock lock(concept_->getProperty());

    const auto k = keyEvent->key();
    const auto m = keyEvent->modifiers();

    if (k == Qt::Key_A && m == Qt::ControlModifier) {  // Select all
        std::ranges::for_each(items(), [](auto* item) { item->setSelected(true); });
        keyEvent->accept();
    } else if (k == Qt::Key_D && m == Qt::ControlModifier) {  // Select none
        std::ranges::for_each(selectedItems(), [](auto* item) { item->setSelected(false); });
        keyEvent->accept();
    } else if (k == Qt::Key_Delete || k == Qt::Key_Backspace) {  // Delete selected
        auto selection = getSelectedPrimitiveItems();
        std::ranges::for_each(selection, [](auto* item) { item->setSelected(false); });
        std::ranges::for_each(selection, [this](auto* item) { removeControlPoint(item); });
        keyEvent->accept();
    } else if (handleMoveSelection(keyEvent)) {
        keyEvent->accept();
    } else if (handleModifySelection(keyEvent)) {
        keyEvent->accept();
    } else if (handleGroupSelection(keyEvent)) {
        keyEvent->accept();
    } else if (k == Qt::Key_F) {
        moveMode_ = TFMoveMode::Free;
        emit moveModeChange(moveMode_);
        keyEvent->accept();
    } else if (k == Qt::Key_R) {
        moveMode_ = TFMoveMode::Restrict;
        emit moveModeChange(moveMode_);
        keyEvent->accept();
    } else if (k == Qt::Key_P) {
        moveMode_ = TFMoveMode::Push;
        emit moveModeChange(moveMode_);
        keyEvent->accept();
    } else if (k == Qt::Key_H) {
        const auto mode = concept_->getHistogramMode();
        const auto newMode =
            static_cast<HistogramMode>(static_cast<int>(mode) + 1 % numberOfHistogramModes);
        concept_->setHistogramMode(newMode);
        keyEvent->accept();
    } else {
        QGraphicsScene::keyPressEvent(keyEvent);
    }
}

void TFEditor::contextMenuEvent(QGraphicsSceneContextMenuEvent* e) {
    const auto pos(e->scenePos());
    auto* const primitiveUnderMouse = getTFPrimitiveItemAt(pos);

    // change selection if primitive under the mouse is not yet in selection
    if (primitiveUnderMouse && !primitiveUnderMouse->isSelected()) {
        clearSelection();
        primitiveUnderMouse->setSelected(true);
    }

    auto selection = getSelectedPrimitiveItems();

    QMenu menu{};

    for (auto&& [setTmp, item] : primitives_) {
        auto* set = setTmp;
        if (item.connected) {
            auto* addTFPoint = menu.addAction("Add TF &Point");
            auto* addTFPeak = menu.addAction("Add TF P&eak");
            connect(addTFPoint, &QAction::triggered, this,
                    [this, pos, set]() { addPoint(pos, set); });
            connect(addTFPeak, &QAction::triggered, this,
                    [this, pos, set]() { addPeak(pos, set); });
        } else {
            auto* addIsovalue = menu.addAction("Add &Isovalue");
            connect(addIsovalue, &QAction::triggered, this,
                    [this, pos, set]() { addPoint(pos, set); });
        }
    }

    menu.addSeparator();

    {
        auto* editColor = menu.addAction("Edit &Color");
        auto* duplicatePrimitive = menu.addAction("D&uplicate");
        auto* deletePrimitive = menu.addAction("&Delete");
        auto* clearTF = menu.addAction("&Clear");
        auto* resetTF = menu.addAction("&Reset");

        editColor->setEnabled(!selection.empty());
        connect(editColor, &QAction::triggered, this, &TFEditor::showColorDialog);

        duplicatePrimitive->setEnabled(!selection.empty());
        connect(duplicatePrimitive, &QAction::triggered, this, [this, selection]() mutable {
            setSelected(selection, false);
            NetworkLock const lock(concept_->getProperty());
            util::KeepTrueWhileInScope const k(&selectNewPrimitives_);
            duplicate(selection);
        });

        deletePrimitive->setEnabled(!selection.empty());
        connect(deletePrimitive, &QAction::triggered, this, [this, selection]() mutable {
            NetworkLock const lock(concept_->getProperty());
            setSelected(selection, false);
            std::ranges::for_each(selection, [this](auto* item) { removeControlPoint(item); });
        });

        connect(clearTF, &QAction::triggered, this, [this]() {
            NetworkLock const lock(concept_->getProperty());
            std::ranges::for_each(concept_->sets(), [](auto* set) { set->clear(); });
        });
        connect(resetTF, &QAction::triggered, this, [this]() {
            NetworkLock const lock(concept_->getProperty());
            concept_->getProperty()->resetToDefaultState();
        });
    }

    menu.addSeparator();

    {
        auto* transformMenu = menu.addMenu("Trans&form");

        auto* distributeAlphaEvenly = transformMenu->addAction("&Distribute Alpha Evenly");
        auto* distributePositionEvenly = transformMenu->addAction("&Distribute Position Evenly");
        transformMenu->addSeparator();
        auto* alignAlphaToMean = transformMenu->addAction("&Align Alpha to Mean");
        auto* alignAlphaToTop = transformMenu->addAction("&Align Alpha to Top");
        auto* alignAlphaToBottom = transformMenu->addAction("&Align Alpha to Bottom");
        transformMenu->addSeparator();
        auto* alignPositionToMean = transformMenu->addAction("&Align Position to Mean");
        auto* alignPositionToLeft = transformMenu->addAction("&Align Position to Left");
        auto* alignPositionToRight = transformMenu->addAction("&Align Position to Right");
        transformMenu->addSeparator();
        auto* flipPositions = transformMenu->addAction("&Horizontal Flip");
        auto* interpolateAlpha = transformMenu->addAction("&Interpolate Alpha");

        connect(distributeAlphaEvenly, &QAction::triggered, this, [this]() {
            NetworkLock const lock(concept_->getProperty());
            util::distributeAlphaEvenly(getAllOrSelectedPrimitives());
        });

        connect(distributePositionEvenly, &QAction::triggered, this, [this]() {
            NetworkLock const lock(concept_->getProperty());
            util::distributePositionEvenly(getAllOrSelectedPrimitives());
        });

        connect(alignAlphaToMean, &QAction::triggered, this, [this]() {
            NetworkLock const lock(concept_->getProperty());
            util::alignAlphaToMean(getAllOrSelectedPrimitives());
        });

        connect(alignAlphaToTop, &QAction::triggered, this, [this]() {
            NetworkLock const lock(concept_->getProperty());
            util::alignAlphaToTop(getAllOrSelectedPrimitives());
        });

        connect(alignAlphaToBottom, &QAction::triggered, this, [this]() {
            NetworkLock const lock(concept_->getProperty());
            util::alignAlphaToBottom(getAllOrSelectedPrimitives());
        });

        connect(alignPositionToMean, &QAction::triggered, this, [this]() {
            NetworkLock const lock(concept_->getProperty());
            util::alignPositionToMean(getAllOrSelectedPrimitives());
        });

        connect(alignPositionToLeft, &QAction::triggered, this, [this]() {
            NetworkLock const lock(concept_->getProperty());
            util::alignPositionToLeft(getAllOrSelectedPrimitives());
        });

        connect(alignPositionToRight, &QAction::triggered, this, [this]() {
            NetworkLock const lock(concept_->getProperty());
            util::alignPositionToRight(getAllOrSelectedPrimitives());
        });

        connect(flipPositions, &QAction::triggered, this, [this]() {
            NetworkLock const lock(concept_->getProperty());
            util::flipPositions(getAllOrSelectedPrimitives());
        });

        connect(interpolateAlpha, &QAction::triggered, this, [this]() {
            NetworkLock const lock(concept_->getProperty());
            util::interpolateAlpha(getAllOrSelectedPrimitives());
        });
    }

    menu.addSeparator();

    if (concept_->hasTF()) {

        auto* simplify = menu.addMenu("Simplify");
        menu.addSeparator();

        auto makeSimple = [this](double delta) {
            return [this, delta]() {
                NetworkLock const lock(concept_->getProperty());
                auto* tf = concept_->getTransferFunction();
                auto simple = TransferFunction::simplify(tf->get(), delta);
                tf->clear();
                tf->add(simple);
            };
        };

        for (double delta : {0.001, 0.002, 0.005, 0.01, 0.02, 0.05, 0.10, 0.20}) {
            auto* action = simplify->addAction(utilqt::toQString(fmt::format("{:05.3f}", delta)));
            connect(action, &QAction::triggered, this, makeSimple(delta));
        }
    }

    if (concept_->supportsMask()) {
        auto* maskMenu = menu.addMenu("&Mask");
        // TF masking
        auto* maskBegin = maskMenu->addAction("Set &Begin");
        auto* maskEnd = maskMenu->addAction("Set &End");
        maskMenu->addSeparator();
        auto* clearAction = maskMenu->addAction("&Clear");

        connect(maskBegin, &QAction::triggered, this,
                [this, pos]() { concept_->setMask(pos.x() / width(), concept_->getMask().y); });
        connect(maskEnd, &QAction::triggered, this,
                [this, pos]() { concept_->setMask(concept_->getMask().x, pos.x() / width()); });

        connect(clearAction, &QAction::triggered, this, [this]() { concept_->clearMask(); });
    }

    menu.addSeparator();

    {
        // group selection / assignment
        auto* groupSelectMenu = menu.addMenu("&Select Group");
        auto* groupAssignMenu = menu.addMenu("Assign &Group");

        // select TF primitives which were stored as group
        for (auto&& i : util::make_sequence(1, 11)) {
            const QString str = (i < 10 ? QString("Group &%1").arg(i) : "Group 1&0");
            auto* action = groupSelectMenu->addAction(str);
            const auto group = i % 10;
            action->setEnabled(!groups_[group].empty());
            connect(action, &QAction::triggered, this, [this, group, selection]() mutable {
                const bool addToSelection = ((QGuiApplication::queryKeyboardModifiers() &
                                              Qt::ShiftModifier) == Qt::ShiftModifier);
                if (!addToSelection) {
                    setSelected(selection, false);
                }
                setSelected(groups_[group], true);
            });
        }

        // assign current selection to a group
        for (auto&& i : util::make_sequence(1, 11, 1)) {
            const QString str = (i < 10 ? QString("Group &%1").arg(i) : "Group 1&0");
            auto* action = groupAssignMenu->addAction(str);
            const auto group = i % 10;
            connect(action, &QAction::triggered, this,
                    [this, group, selection]() { groups_[group] = selection; });
        }
    }

    menu.addSeparator();
    {
        if (concept_->hasTF()) {
            util::addTFPresetsMenu(e->widget(), &menu, concept_->getTFProperty());
            util::addTFColorbrewerPresetsMenu(e->widget(), &menu, concept_->getTFProperty());
            menu.addSeparator();
        }

        if (concept_->hasTF()) {
            auto* importTF = menu.addAction("&Import TF...");
            auto* exportTF = menu.addAction("&Export TF...");
            connect(importTF, &QAction::triggered, this,
                    [this]() { concept_->showImportDialog(); });
            connect(exportTF, &QAction::triggered, this,
                    [this]() { concept_->showExportDialog(); });
            menu.addSeparator();
        }
        if (concept_->hasIsovalues()) {
            auto* importTF = menu.addAction("&Import Isovalues...");
            auto* exportTF = menu.addAction("&Export Isovalues...");
            connect(importTF, &QAction::triggered, this,
                    [this]() { concept_->showImportDialog(); });
            connect(exportTF, &QAction::triggered, this,
                    [this]() { concept_->showExportDialog(); });
        }
    }

    if (menu.exec(e->screenPos())) {
        e->accept();
    }
}

double TFEditor::sceneToPos(const QPointF& pos) const {
    return glm::clamp(pos.x() / width(), 0.0, 1.0);
}
double TFEditor::sceneToAlpha(const QPointF& pos) const {
    return glm::clamp(pos.y() / height(), 0.0, 1.0);
}

void TFEditor::addPoint(double pos, const vec4& color, TFPrimitiveSet* set) {
    if (!set) return;
    const NetworkLock lock(concept_->getProperty());
    set->add(pos, color);
}
void TFEditor::addPoint(double pos, double alpha, TFPrimitiveSet* set) {
    if (!set) return;
    const NetworkLock lock(concept_->getProperty());
    set->add(pos, alpha);
}

void TFEditor::addPoint(const QPointF& scenePos, TFPrimitiveSet* set) {
    const util::KeepTrueWhileInScope k(&selectNewPrimitives_);
    addPoint(sceneToPos(scenePos), sceneToAlpha(scenePos), set);
}
void TFEditor::addPoint(const QPointF& scenePos) { addPoint(scenePos, activeSet()); }

void TFEditor::addPeak(const QPointF& scenePos, TFPrimitiveSet* set) {
    if (!set) return;

    const util::KeepTrueWhileInScope k(&selectNewPrimitives_);
    const NetworkLock lock(concept_->getProperty());

    const double pos = sceneToPos(scenePos);
    const double alpha = sceneToAlpha(scenePos);

    set->add(pos, alpha);

    const double normalizedOffset = viewDependentOffset().x * 5.0 / width();

    // add point to the left
    if (pos > 0.0) {
        // compute intercept on alpha by using alpha - alpha / offset * pos
        const double leftAlpha = std::max(0.0, alpha * (1.0 - pos / normalizedOffset));
        set->add(std::max(pos - normalizedOffset, 0.0), leftAlpha);
    }

    // add point to the right
    if (pos < 1.0) {
        // compute intercept on alpha by using alpha + alpha / offset * (pos - 1.0)
        const double rightAlpha = std::max(0.0, alpha * (1.0 + (pos - 1.0) / normalizedOffset));
        set->add(std::min(pos + normalizedOffset, 1.0), rightAlpha);
    }
}

TFPrimitiveSet* TFEditor::activeSet() {
    if (!activeSet_ && !primitives_.empty()) {
        activeSet_ = primitives_.begin()->first;
    }
    return activeSet_;
}

auto TFEditor::activeItem() -> Items* {
    if (auto it = primitives_.find(activeSet()); it != primitives_.end()) {
        return &it->second;
    }
    return nullptr;
}

TFPrimitiveSet* TFEditor::findSet(TFPrimitive* primitive) const {
    for (auto* set : concept_->sets()) {
        if (set->contains(primitive)) {
            return set;
        }
    }
    return nullptr;
}

void TFEditor::removeControlPoint(TFEditorPrimitive* item) {
    const NetworkLock lock(concept_->getProperty());
    for (auto* set : concept_->sets()) {
        if (set->remove(item->getPrimitive())) break;
    }
}

TFEditorPrimitive* TFEditor::getTFPrimitiveItemAt(const QPointF& pos) const {
    auto* view = views().front();
    const auto deviceTransform = view->viewportTransform();

    for (auto& graphicsItem :
         items(pos, Qt::IntersectsItemShape, Qt::DescendingOrder, deviceTransform)) {
        if (auto* item = dynamic_cast<TFEditorPrimitive*>(graphicsItem)) {
            return item;
        }
    }
    return nullptr;
}

void TFEditor::updateConnections() {
    for (auto&& [set, items] : primitives_) {
        if (!items.connected) continue;

        std::stable_sort(items.points.begin(), items.points.end(), comparePtr{});
        while (items.connections.size() < items.points.size() + 1) {
            auto c = std::make_unique<TFControlPointConnection>();
            addItem(c.get());
            items.connections.push_back(std::move(c));
        }
        while (items.connections.size() > items.points.size() + 1) {
            removeItem(items.connections.back().get());
            items.connections.pop_back();
        }

        items.connections.front()->left = nullptr;
        items.connections.back()->right = nullptr;

        for (size_t i = 0; i < items.points.size(); ++i) {
            items.points[i]->setLeft(items.connections[i].get());
            items.points[i]->setRight(items.connections[i + 1].get());
            items.connections[i]->right = items.points[i].get();
            items.connections[i + 1]->left = items.points[i].get();
        }

        for (auto& elem : items.connections) {
            elem->updateShape();
        }
    }
}

void TFEditor::setMoveMode(TFMoveMode i) { moveMode_ = i; }

TFMoveMode TFEditor::getMoveMode() const { return moveMode_; }

const DataMapper& TFEditor::getDataMapper() const {
    if (const auto* map = concept_->getDataMap()) {
        return *map;
    } else {
        static const DataMapper dataMap{};
        return dataMap;
    }
}

std::vector<TFPrimitive*> TFEditor::getSelectedPrimitives() const {
    std::vector<TFPrimitive*> selection;
    for (auto& item : selectedItems()) {
        if (auto* p = dynamic_cast<TFEditorPrimitive*>(item)) {
            selection.push_back(&p->getPrimitive());
        }
    }
    return selection;
}

std::vector<TFPrimitive*> TFEditor::getAllPrimitives() const {
    std::vector<TFPrimitive*> res;

    for (auto&& [set, items] : primitives_) {
        std::transform(items.points.begin(), items.points.end(), std::back_inserter(res),
                       [](auto& p) { return &p->getPrimitive(); });
    }

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
        if (auto* p = dynamic_cast<TFEditorPrimitive*>(elem)) {
            selection.push_back(p);
        }
    }
    return selection;
}

void TFEditor::setSelected(std::span<TFEditorPrimitive*> primitives, bool selected) {
    std::ranges::for_each(primitives, [&](TFEditorPrimitive* p) { p->setSelected(selected); });
}

QTransform TFEditor::calcTransform(QPointF scenePos, QPointF lastScenePos) const {
    const bool altPressed =
        ((QGuiApplication::queryKeyboardModifiers() & Qt::AltModifier) == Qt::AltModifier);
    if (altPressed) {
        const auto org = mouse_.dragItem->pos() - mouse_.rigid;
        const auto pos = scenePos - mouse_.rigid;
        const auto xScale = std::abs(org.x()) > 0.0001 ? pos.x() / org.x() : 1.0;
        const auto yScale = std::abs(org.y()) > 0.0001 ? pos.y() / org.y() : 1.0;
        const auto scale = QTransform::fromScale(xScale, yScale);
        const auto translate = QTransform::fromTranslate(-mouse_.rigid.x(), -mouse_.rigid.y());
        return translate * scale * translate.inverted();
    } else {
        const auto delta = scenePos - lastScenePos;
        return QTransform::fromTranslate(delta.x(), delta.y());
    }
}

QPointF TFEditor::calcTransformRef(std::span<TFEditorPrimitive*> primitives,
                                   TFEditorPrimitive* start) {
    const auto xRange = std::ranges::minmax_element(primitives, std::less<>{},
                                                    [](auto* p) { return p->pos().x(); });
    const auto yRange = std::ranges::minmax_element(primitives, std::less<>{},
                                                    [](auto* p) { return p->pos().y(); });

    const auto selRect = QRectF{QPointF{(*xRange.min)->pos().x(), (*yRange.min)->pos().y()},
                                QSizeF{(*xRange.max)->pos().x() - (*xRange.min)->pos().x(),
                                       (*yRange.max)->pos().y() - (*yRange.min)->pos().y()}};

    const std::array corners = {selRect.bottomLeft(), selRect.bottomRight(), selRect.topLeft(),
                                selRect.topRight()};

    return *std::max_element(corners.begin(), corners.end(),
                             [&](const QPointF& a, const QPointF& b) {
                                 const auto da = a - start->pos();
                                 const auto db = b - start->pos();
                                 return QPointF::dotProduct(da, da) < QPointF::dotProduct(db, db);
                             });
}

void TFEditor::move(std::span<TFEditorPrimitive*> primitives, const QTransform& transform,
                    const QRectF& rect) {
    constexpr auto less = derefOperator(std::less<>{}, &TFEditorPrimitive::getPosition);
    constexpr auto greater = derefOperator(std::greater<>{}, &TFEditorPrimitive::getPosition);

    if (transform.map(QPointF{0, 0}).x() > 0) {
        std::ranges::stable_sort(primitives, greater);
    } else {
        std::ranges::stable_sort(primitives, less);
    }
    std::ranges::for_each(primitives, [&](TFEditorPrimitive* p) {
        p->setPos(utilqt::clamp(transform.map(p->pos()), rect));
    });
}

void TFEditor::duplicate(std::span<TFEditorPrimitive*> primitives) {
    auto offset = viewDependentOffset().x;
    std::ranges::for_each(primitives, [&](auto* item) {
        if (auto* set = findSet(&item->getPrimitive())) {
            const double newPos = item->pos().x() + offset * 5.0;
            const auto newColor = item->getPrimitive().getColor();
            addPoint(newPos, newColor, set);
        }
    });
}

bool TFEditor::handleGroupSelection(QKeyEvent* event) {
#if defined(_WIN32)
    static constexpr std::array<std::pair<qint32, int>, 10> nativeKeyMap{
        {{2, 1}, {3, 2}, {4, 3}, {5, 4}, {6, 5}, {7, 6}, {8, 7}, {9, 8}, {10, 9}, {11, 0}}};
    const quint32 nativeKey = event->nativeScanCode();
#elif defined(__APPLE__)
    static constexpr std::array<std::pair<quint32, int>, 10> nativeKeyMap{
        {{18, 1}, {19, 2}, {20, 3}, {21, 4}, {23, 5}, {22, 6}, {26, 7}, {28, 8}, {25, 9}, {29, 0}}};
    const quint32 nativeKey = event->nativeVirtualKey();
#else
    // TODO update...
    static constexpr std::array<std::pair<quint32, int>, 10> nativeKeyMap{
        {{18, 1}, {19, 2}, {20, 3}, {21, 4}, {23, 5}, {22, 6}, {26, 7}, {28, 8}, {25, 9}, {29, 0}}};
    const quint32 nativeKey = event->nativeVirtualKey();
#endif

    const auto* const it =
        std::ranges::find_if(nativeKeyMap, [&](auto& p) { return p.first == nativeKey; });
    if (it == nativeKeyMap.end()) {
        return false;
    }
    const int group = it->second;

    if (event->modifiers() & Qt::ControlModifier != 0u) {  // Create group
        groups_[group] = getSelectedPrimitiveItems();
    } else if (event->modifiers() & Qt::ShiftModifier != 0u) {
        setSelected(groups_[group], true);
    } else {
        auto selection = getSelectedPrimitiveItems();
        setSelected(selection, false);
        setSelected(groups_[group], true);
    }
    return true;
}

bool TFEditor::handleMoveSelection(QKeyEvent* event) {
    if (event->modifiers() & Qt::ControlModifier != 0u) {
        return false;
    }
    const auto k = event->key();

    if (k != Qt::Key_Left && k != Qt::Key_Right && k != Qt::Key_Up && k != Qt::Key_Down &&
        k != 'I' && k != 'J' && k != 'K' && k != 'L') {
        return false;
    }

    QPointF delta;
    switch (k) {
        case Qt::Key_Left:
        case 'J':
            delta = QPointF(-viewDependentOffset().x, 0.0f);
            break;
        case Qt::Key_Right:
        case 'L':
            delta = QPointF(viewDependentOffset().x, 0.0f);
            break;
        case Qt::Key_Up:
        case 'I':
            delta = QPointF(0.0f, viewDependentOffset().y);
            break;
        case Qt::Key_Down:
        case 'K':
            delta = QPointF(0.0f, -viewDependentOffset().y);
            break;
    }

    constexpr double stepUpScalingFactor = 5.0;
    constexpr double stepDownScalingFactor = 0.2;
    if (event->modifiers() & Qt::ShiftModifier != 0u) {
        delta *= stepUpScalingFactor;
    } else if (event->modifiers() & Qt::AltModifier != 0u) {
        delta *= stepDownScalingFactor;
    }

    emit updateBegin();
    QList<QGraphicsItem*> items = selectedItems();
    for (auto& item : items) {
        item->setPos(item->pos() + delta);
    }
    emit updateEnd();

    return true;
}

bool TFEditor::handleModifySelection(QKeyEvent* event) {
    if (!(event->modifiers() & Qt::ControlModifier)) {
        return false;
    }

    const auto k = event->key();
    if (k != Qt::Key_Left && k != Qt::Key_Right && k != Qt::Key_Up && k != Qt::Key_Down &&
        k != 'I' && k != 'J' && k != 'K' && k != 'L') {
        return false;
    }

    auto points = getSelectedPrimitiveItems();
    std::stable_sort(points.begin(), points.end(), comparePtr{});

    switch (k) {
        case Qt::Key_Left:
        case 'J':
            if (!points.empty()) {
                if (points.front()->left() && points.front()->left()->left) {
                    points.front()->left()->left->setSelected(true);
                    if (!(event->modifiers() & Qt::ShiftModifier)) {
                        points.back()->setSelected(false);
                    }
                }
            } else if (auto* item = activeItem()) {
                if (!item->points.empty()) {
                    item->points.back()->setSelected(true);
                }
            }

            break;
        case Qt::Key_Right:
        case 'L':
            if (!points.empty()) {
                if (points.back()->right() && points.back()->right()->right) {
                    points.back()->right()->right->setSelected(true);
                    if (!(event->modifiers() & Qt::ShiftModifier)) {
                        points.front()->setSelected(false);
                    }
                }
            } else if (auto* item = activeItem()) {
                if (!item->points.empty()) {
                    item->points.front()->setSelected(true);
                }
            }
            break;
        case Qt::Key_Up:
        case 'I':
            if (!points.empty()) {
                points.front()->setSelected(false);
            }
            break;
        case Qt::Key_Down:
        case 'K':
            if (!points.empty()) {
                points.back()->setSelected(false);
            }
            break;
    }
    return true;
}

dvec2 TFEditor::viewDependentOffset() const {
    const int defaultOffset = 5;  //!< offset in pixel
    if (views().empty()) {
        return {0.01, 0.01};
    }

    // to determine the offset in scene coords, map a square where each side has length
    // defaultOffset to the scene. We assume that there is no rotation or non-linear
    // view transformation.
    auto rect = views()
                    .front()
                    ->mapToScene(QRect(QPoint(0, 0), QSize(defaultOffset, defaultOffset)))
                    .boundingRect();

    return {rect.width(), rect.height()};
}

}  // namespace inviwo
