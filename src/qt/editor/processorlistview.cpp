/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2026 Inviwo Foundation
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

#include <inviwo/qt/editor/processorlistview.h>

#include <inviwo/core/util/logcentral.h>

#include <inviwo/qt/editor/processorlistmodel.h>
#include <inviwo/qt/editor/processorlistwidget.h>
#include <inviwo/qt/editor/processormimedata.h>
#include <inviwo/qt/editor/processorpreview.h>

#include <modules/qtwidgets/inviwoqtutils.h>

#include <QMouseEvent>
#include <QApplication>
#include <QMenu>
#include <QHeaderView>

namespace inviwo {

ProcessorListView::ProcessorListView(QAbstractItemModel* model, ProcessorListWidget* parent)
    : QTreeView(parent), processorTreeWidget_{parent} {
    setContextMenuPolicy(Qt::CustomContextMenu);

    setModel(model);

    setDragDropMode(QAbstractItemView::DragOnly);

    setSelectionMode(QAbstractItemView::SingleSelection);
    setSelectionBehavior(QAbstractItemView::SelectRows);

    setHeaderHidden(true);
    setIndentation(utilqt::emToPx(this, 1.0));
    setAnimated(true);
    header()->setStretchLastSection(false);
    header()->setSectionResizeMode(0, QHeaderView::Stretch);
    header()->setSectionResizeMode(1, QHeaderView::Fixed);
    header()->setDefaultSectionSize(utilqt::emToPx(this, 4.0));
    expandAll();
    setUniformRowHeights(true);

    QObject::connect(this, &QTreeView::customContextMenuRequested, this,
                     &ProcessorListView::showContextMenu);
}

void ProcessorListView::mousePressEvent(QMouseEvent* e) {
    if (e->buttons() & Qt::LeftButton) dragStartPosition_ = e->pos();

    QTreeView::mousePressEvent(e);
}

void ProcessorListView::mouseMoveEvent(QMouseEvent* e) {
    if ((e->buttons() & Qt::LeftButton) && dragStartPosition_) {
        if ((e->pos() - *dragStartPosition_).manhattanLength() <
            QApplication::startDragDistance()) {
            return;
        }
        using Role = ProcessorListModel::Role;
        using Type = ProcessorListModel::Node::Type;

        auto index = indexAt(*dragStartPosition_);
        if (index.isValid() &&
            utilqt::getData(index, Role::Type).toInt() == static_cast<int>(Type::Item)) {
            dragStartPosition_.reset();
            auto id = utilqt::getData(index, Role::ClassIdentifier).toString();
            try {
                if (auto p = processorTreeWidget_->createProcessor(id)) {
                    auto* drag = new ProcessorDragObject(this, std::move(p));
                    drag->exec(Qt::MoveAction);
                }
            } catch (const std::exception& e) {
                log::error("Error trying to create processor: {} Message:\n{}",
                           utilqt::fromQString(id), e.what());
            }
        }
    }
}

void ProcessorListView::mouseReleaseEvent(QMouseEvent* e) {
    dragStartPosition_.reset();
    QTreeView::mouseReleaseEvent(e);
}

QModelIndex ProcessorListView::findFirstLeaf(QAbstractItemModel* model, QModelIndex parent) {
    if (!model->hasChildren(parent)) return parent;
    return findFirstLeaf(model, model->index(0, 0, parent));
}

void ProcessorListView::focusInEvent(QFocusEvent* e) {
    QTreeView::focusInEvent(e);
    auto firstLeaf = findFirstLeaf(model());
    if (firstLeaf.isValid()) {
        selectionModel()->setCurrentIndex(firstLeaf, QItemSelectionModel::ClearAndSelect);
    }
}

void ProcessorListView::focusOutEvent(QFocusEvent* e) { QTreeView::focusOutEvent(e); }

void ProcessorListView::showContextMenu(const QPoint& p) {
    using Role = ProcessorListModel::Role;

    auto index = indexAt(p);
    auto id = utilqt::getData(index, Role::ClassIdentifier).toString();

    const bool enableExpandCollapse =
        ((processorTreeWidget_->getGrouping() != ProcessorListModel::Grouping::LastUsed) &&
         (processorTreeWidget_->getGrouping() != ProcessorListModel::Grouping::MostUsed));

    QMenu menu(this);
    auto* addItem = menu.addAction("&Add Processor");
    addItem->setEnabled(!id.isEmpty());
    QObject::connect(addItem, &QAction::triggered, this,
                     [&, id]() { processorTreeWidget_->addProcessor(id); });

    menu.addSeparator();

    auto* expandAction = menu.addAction("&Expand All Categories");
    expandAction->setEnabled(enableExpandCollapse);
    QObject::connect(expandAction, &QAction::triggered, this, [&]() { expandAll(); });

    auto* collapseOtherAction = menu.addAction("C&ollapse Others");
    collapseOtherAction->setEnabled(index.isValid() && enableExpandCollapse);
    // figure out top-level tree item, i.e. category
    while (index.isValid() && index.parent().isValid()) {
        index = index.parent();
    }
    QObject::connect(collapseOtherAction, &QAction::triggered, this, [&, index]() {
        collapseAll();
        expand(index);
    });

    auto* collapse = menu.addAction("&Collapse All Categories");
    collapse->setEnabled(enableExpandCollapse);
    QObject::connect(collapse, &QAction::triggered, this, [&]() { collapseAll(); });

    menu.exec(mapToGlobal(p));
}

ProcessorDragObject::ProcessorDragObject(QWidget* source, std::shared_ptr<Processor> processor)
    : QDrag(source) {
    auto img = QPixmap::fromImage(utilqt::generateProcessorPreview(*processor, 1.0));
    setPixmap(img);
    auto* mime = new ProcessorMimeData(std::move(processor));
    setMimeData(mime);
    setHotSpot(QPoint(img.width() / 2, img.height() / 2));
}

}  // namespace inviwo
