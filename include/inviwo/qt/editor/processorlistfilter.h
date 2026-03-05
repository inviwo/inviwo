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
#pragma once

#include <inviwo/qt/editor/inviwoqteditordefine.h>

#include <inviwo/core/algorithm/searchdsl.h>
#include <inviwo/core/util/document.h>
#include <inviwo/core/processors/processortags.h>

#include <inviwo/qt/editor/processorlistmodel.h>

#include <QSortFilterProxyModel>
#include <QModelIndex>
#include <QString>

namespace inviwo {

class ProcessorNetwork;

class IVW_QTEDITOR_API ProcessorListFilter : public QSortFilterProxyModel {
public:
    using Item = ProcessorListModel::Item;
    using Role = ProcessorListModel::Role;
    using Type = ProcessorListModel::Node::Type;
    using Grouping = ProcessorListModel::Grouping;

    explicit ProcessorListFilter(QAbstractItemModel* model, ProcessorNetwork* net,
                                 QObject* parent = nullptr);

    virtual bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const override;

    void setCustomFilter(const QString& filter);

    Document description() const;

    std::optional<std::string_view> currentStr(std::string_view name);
    std::any currentData(std::string_view name);

    void setGrouping(Grouping grouping);

    virtual bool lessThan(const QModelIndex& left, const QModelIndex& right) const override;

    void setCheckedTags(const Tags& tags);

private:
    Grouping grouping_;
    Tags tags_;
    SearchDSL<Item> dsl_;
};

}  // namespace inviwo
