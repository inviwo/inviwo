/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019-2021 Inviwo Foundation
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

#include <inviwo/dataframeqt/dataframetableview.h>
#include <inviwo/dataframeqt/dataframemodel.h>
#include <inviwo/dataframeqt/dataframesortfilterproxy.h>
#include <inviwo/dataframe/datastructures/dataframe.h>
#include <inviwo/dataframe/datastructures/column.h>
#include <inviwo/dataframe/util/dataframeutil.h>

#include <inviwo/core/datastructures/buffer/buffer.h>
#include <inviwo/core/datastructures/buffer/bufferramprecision.h>
#include <inviwo/core/util/raiiutils.h>
#include <inviwo/core/util/zip.h>
#include <modules/qtwidgets/inviwoqtutils.h>

#include <type_traits>

#include <warn/push>
#include <warn/ignore/all>
#include <QHeaderView>
#include <QSortFilterProxyModel>
#include <QAbstractButton>
#include <warn/pop>

namespace inviwo {

DataFrameTableView::DataFrameTableView(QWidget* parent)
    : QTableView(parent)
    , model_(new DataFrameModel(this))
    , sortProxy_(new DataFrameSortFilterProxy(this)) {
    horizontalHeader()->setStretchLastSection(true);

    sortProxy_->setSourceModel(model_);
    setModel(sortProxy_);
    setSortingEnabled(true);

    // need mouse tracking for issuing highlight events when entering a cell
    setMouseTracking(true);

    // make it read-only
    setEditTriggers(QAbstractItemView::NoEditTriggers);
    setAlternatingRowColors(true);

    // change corner button to disable sorting instead of selecting all
    if (auto btn = findChild<QAbstractButton*>()) {
        btn->disconnect();
        QObject::connect(btn, &QAbstractButton::clicked, this, [&]() {
            horizontalHeader()->setSortIndicator(-1, Qt::AscendingOrder);
            sortProxy_->sort(-1);
        });
    }

    QObject::connect(this, &QAbstractItemView::entered, this, [this](const QModelIndex& index) {
        if (ignoreEvents_) return;
        model_->highlightRow(sortProxy_->mapToSource(index));
    });

    QObject::connect(
        selectionModel(), &QItemSelectionModel::selectionChanged, this,
        [this](const QItemSelection&, const QItemSelection&) {
            if (ignoreEvents_) return;
            util::KeepTrueWhileInScope ignore(&ignoreUpdate_);

            model_->selectRows(
                sortProxy_->mapSelectionToSource(selectionModel()->selection()).indexes());
        });
}

void DataFrameTableView::setManager(BrushingAndLinkingManager& manager) {
    model_->setManager(manager);
    sortProxy_->setManager(manager);
}

void DataFrameTableView::setDataFrame(std::shared_ptr<const DataFrame> dataframe,
                                      bool categoryIndices) {
    model_->setDataFrame(dataframe, categoryIndices);

    if (model_->columnCount() > 0) {
        horizontalHeader()->setSectionHidden(0, !indexVisible_);
    }
}

void DataFrameTableView::brushingUpdate() {
    model_->brushingUpdate();
    sortProxy_->brushingUpdate();

    if (ignoreUpdate_) return;

    util::KeepTrueWhileInScope ignore(&ignoreEvents_);
    selectionModel()->clearSelection();

    QItemSelection s;
    for (auto row : model_->getSelectedRows()) {
        QModelIndex start{model()->index(row, 0)};
        QModelIndex end{model()->index(row, model_->columnCount() - 1)};
        s.select(start, end);
    }
    selectionModel()->select(s, QItemSelectionModel::Select);
}

void DataFrameTableView::setIndexColumnVisible(bool visible) {
    indexVisible_ = visible;
    if (model_->columnCount() > 0) {
        horizontalHeader()->setSectionHidden(0, !indexVisible_);
    }
}

bool DataFrameTableView::isIndexColumnVisible() const { return indexVisible_; }

void DataFrameTableView::setFilteredRowsVisible(bool visible) {
    sortProxy_->setFiltering(!visible);
}

bool DataFrameTableView::getFilteredRowsVisible() const { return !sortProxy_->getFiltering(); }

}  // namespace inviwo
