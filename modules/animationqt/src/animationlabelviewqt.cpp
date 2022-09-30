/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2022 Inviwo Foundation
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

#include <modules/animationqt/animationlabelviewqt.h>

#include <inviwo/core/common/inviwoapplication.h>            // for InviwoApplication
#include <inviwo/core/network/networkutils.h>                // for setSelected
#include <inviwo/core/network/processornetwork.h>            // for ProcessorNetwork
#include <inviwo/core/processors/processorutils.h>           // for setSelected
#include <inviwo/core/properties/property.h>                 // for Property
#include <inviwo/core/properties/propertyowner.h>            // for PropertyOwner
#include <inviwo/core/util/indirectiterator.h>               // for IndirectIterator
#include <modules/animation/animationcontroller.h>           // for AnimationController
#include <modules/animation/animationcontrollerobserver.h>   // for AnimationControllerObservable
#include <modules/animation/datastructures/animation.h>      // for Animation
#include <modules/animation/datastructures/propertytrack.h>  // for BasePropertyTrack
#include <modules/animation/datastructures/track.h>          // for Track
#include <modules/animationqt/trackcontrolswidgetqt.h>       // for TrackControlsWidgetQt
#include <modules/animationqt/widgets/editorconstants.h>     // for timelineHeight

#include <warn/push>
#include <warn/ignore/all>
#include <QAbstractItemView>    // for QAbstractItemView, QAbstract...
#include <QAction>              // for QAction
#include <QFlags>               // for QFlags
#include <QIcon>                // for QIcon
#include <QItemSelection>       // for QItemSelection
#include <QItemSelectionModel>  // for QItemSelectionModel
#include <QKeySequence>         // for QKeySequence, QKeySequence::...
#include <QList>                // for QList, QList<>::iterator
#include <QModelIndex>          // for QModelIndex
#include <QModelIndexList>      // for QModelIndexList
#include <QSize>                // for QSize
#include <QStandardItem>        // for QStandardItem
#include <QStandardItemModel>   // for QStandardItemModel
#include <QString>              // for QString
#include <QVariant>             // for QVariant
#include <Qt>                   // for operator|, DisplayRole, Item...

class QModelIndex;
class QObject;
class QWidget;

namespace inviwo {
class Processor;
}  // namespace inviwo

#include <warn/pop>

namespace inviwo {

namespace animation {

// Simply stores the pointer to the item's Track
class TrackItem : public QStandardItem {
public:
    TrackItem(Track* track)
        : QStandardItem(QString::fromStdString(track->getName())), track_(track) {}

    Track* track_;
};

class AnimationLabelModelQt : public QStandardItemModel {
public:
    AnimationLabelModelQt(QObject* parent) : QStandardItemModel(parent) { setColumnCount(1); }

    virtual Qt::ItemFlags flags(const QModelIndex&) const override {
        return Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsEnabled;
    }
    virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override {
        if (role == Qt::SizeHintRole) {
            return QSize(200, 31);
        }
        if (role == Qt::DisplayRole) {
            return QVariant{};
        }
        return QStandardItemModel::data(index, role);
    }
};

AnimationLabelViewQt::AnimationLabelViewQt(AnimationController& controller)
    : QListView(), controller_(controller) {
    setMouseTracking(true);
    setSelectionBehavior(SelectRows);
    setMovement(Snap);
    setDragDropMode(InternalMove);
    setDragDropOverwriteMode(false);
    setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);

    setViewportMargins(0, timelineHeight, 0, 0);

    model_ = new AnimationLabelModelQt(this);
    setModel(model_);

    controller_.AnimationControllerObservable::addObserver(this);
    Animation& animation = controller_.getAnimation();
    for (auto& track : animation) {
        onTrackAdded(&track);
    }
    animation.addObserver(this);

    connect(selectionModel(), &QItemSelectionModel::selectionChanged, this,
            [this](const QItemSelection& selected, const QItemSelection& /*deselected*/) {
                for (auto& index : selected.indexes()) {
                    if (auto tcw = static_cast<TrackControlsWidgetQt*>(indexWidget(index))) {
                        if (auto propertytrack = dynamic_cast<BasePropertyTrack*>(&tcw->track())) {
                            // Deselect all processors first
                            util::setSelected(controller_.getInviwoApplication()
                                                  ->getProcessorNetwork()
                                                  ->getProcessors(),
                                              false);
                            auto property = propertytrack->getProperty();
                            // Select the processor the selected property belongs to
                            Processor* processor = property->getOwner()->getProcessor();
                            util::setSelected(processor, true);
                        }
                    }
                }
            });

    auto deleteAction = new QAction(QIcon(":/svgicons/edit-delete.svg"), tr("&Delete"), this);
    deleteAction->setShortcuts(QKeySequence::Delete);
    deleteAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    addAction(deleteAction);
    connect(deleteAction, &QAction::triggered, this, [this]() {
        auto& animation = controller_.getAnimation();
        for (auto& index : selectionModel()->selection().indexes()) {
            animation.remove(static_cast<TrackItem*>(model_->itemFromIndex(index))->track_);
        }
    });
}

void AnimationLabelViewQt::onTrackAdded(Track* track) {
    QList<QStandardItem*> row;
    auto item = new TrackItem(track);
    row.append(item);
    QWidget* widget = new TrackControlsWidgetQt(item, *track, controller_);
    model_->appendRow(row);
    auto index = model_->indexFromItem(item);
    setIndexWidget(index, widget);
}

void AnimationLabelViewQt::onTrackRemoved(Track* track) {
    QModelIndex parent = QModelIndex();
    for (int r = 0; r < model_->rowCount(parent); ++r) {
        QModelIndex index = model_->index(r, 0, parent);
        if (static_cast<TrackItem*>(model_->itemFromIndex(index))->track_ == track) {
            model_->removeRow(r, parent);
            break;
        }
    }
}

void AnimationLabelViewQt::onAnimationChanged(AnimationController*, Animation* oldAnim,
                                              Animation* newAnim) {
    oldAnim->removeObserver(this);
    model_->clear();

    for (auto& track : *newAnim) {
        onTrackAdded(&track);
    }

    newAnim->addObserver(this);
}

}  // namespace animation

}  // namespace inviwo
