/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2019 Inviwo Foundation
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

#include <inviwo/core/network/processornetwork.h>
#include <inviwo/core/network/networkutils.h>
#include <inviwo/core/processors/processorutils.h>

#include <modules/qtwidgets/inviwoqtutils.h>

#include <modules/animation/datastructures/animation.h>
#include <modules/animation/datastructures/propertytrack.h>

#include <modules/animationqt/trackcontrolswidgetqt.h>
#include <modules/animationqt/widgets/editorconstants.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QWheelEvent>
#include <QPainter>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QStyledItemDelegate>
#include <QPushButton>
#include <QGridLayout>
#include <QAction>
#include <QIcon>
#include <warn/pop>

namespace inviwo {

namespace animation {

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
            animation.remove(utilqt::fromQString(model_->data(index, Qt::UserRole + 1).toString()));
        }
    });
}

void AnimationLabelViewQt::onTrackAdded(Track* track) {
    QList<QStandardItem*> row;
    auto item = new QStandardItem(QString::fromStdString(track->getName()));
    item->setData(utilqt::toQString(track->getIdentifier()), Qt::UserRole + 1);
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
        auto id = utilqt::toQString(track->getIdentifier());
        if (model_->data(index, Qt::UserRole + 1).toString() == id) {
            model_->removeRow(r, parent);
            break;
        }
    }
}

}  // namespace animation

}  // namespace inviwo
