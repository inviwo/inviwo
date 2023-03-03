/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2021-2023 Inviwo Foundation
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

#include <inviwo/dataframeqt/dataframeqtmoduledefine.h>  // for IVW_MODULE_DATAFRAMEQT_API

#include <warn/push>
#include <warn/ignore/all>
#include <QAbstractTableModel>  // for QAbstractTableModel
#include <QModelIndex>          // for QModelIndex
#include <QModelIndexList>      // for QModelIndexList
#include <QObject>              // for Q_OBJECT
#include <QVariant>             // for QVariant
#include <Qt>                   // for DisplayRole, Orientation, UserRole

class QModelIndex;

#include <warn/pop>

#include <functional>  // for function
#include <memory>      // for shared_ptr
#include <vector>      // for vector

namespace inviwo {

class BrushingAndLinkingManager;
class DataFrame;

class IVW_MODULE_DATAFRAMEQT_API DataFrameModel : public QAbstractTableModel {
#include <warn/push>
#include <warn/ignore/all>
    Q_OBJECT
#include <warn/pop>
public:
    enum Roles { Data = Qt::UserRole, Filter };

    DataFrameModel(QObject* parent = nullptr);
    virtual ~DataFrameModel();

    void setManager(BrushingAndLinkingManager& manager);
    void setDataFrame(std::shared_ptr<const DataFrame> dataframe, bool categoryIndices = false);

    void brushingUpdate();

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;

    void highlightRow(const QModelIndex& index);
    void selectRows(const QModelIndexList& indices);
    std::vector<int> getSelectedRows() const;

private:
    BrushingAndLinkingManager* manager_;
    std::shared_ptr<const DataFrame> data_;

    using ValueFunc = std::function<QVariant(int)>;
    // functions for accessing row data of each column
    std::vector<ValueFunc> valueFuncs_;
    std::vector<ValueFunc> tooltipFuncs_;
};

}  // namespace inviwo
