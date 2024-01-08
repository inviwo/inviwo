/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2021-2024 Inviwo Foundation
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

#include <inviwo/dataframeqt/dataframesortfilterproxy.h>

#include <modules/brushingandlinking/brushingandlinkingmanager.h>      // for BrushingAndLinking...
#include <modules/brushingandlinking/datastructures/brushingaction.h>  // for BrushingTarget

class QModelIndex;

namespace inviwo {

DataFrameSortFilterProxy::DataFrameSortFilterProxy(QObject* parent)
    : QSortFilterProxyModel(parent) {
    setFilterRole(Roles::Filter);
    setSortRole(Roles::Data);
}

void DataFrameSortFilterProxy::setManager(BrushingAndLinkingManager& manager) {
    manager_ = &manager;
}

void DataFrameSortFilterProxy::brushingUpdate() { invalidateFilter(); }

void DataFrameSortFilterProxy::setFiltering(bool enable) {
    if (filtering_ == enable) return;

    filtering_ = enable;
    invalidateFilter();
}

bool DataFrameSortFilterProxy::getFiltering() const { return filtering_; }

bool DataFrameSortFilterProxy::filterAcceptsRow(int sourceRow, const QModelIndex&) const {
    if (!filtering_ || !manager_) return true;

    return !manager_->isFiltered(sourceRow, BrushingTarget::Row);
}

bool DataFrameSortFilterProxy::filterAcceptsColumn(int sourceColumn, const QModelIndex&) const {
    if (!filtering_ || !manager_) return true;

    return !manager_->isFiltered(sourceColumn, BrushingTarget::Column);
}

}  // namespace inviwo
