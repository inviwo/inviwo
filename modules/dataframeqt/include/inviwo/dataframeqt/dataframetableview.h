/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019-2022 Inviwo Foundation
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

#include <memory>  // for shared_ptr

#include <warn/push>
#include <warn/ignore/all>
#include <QObject>     // for Q_OBJECT
#include <QTableView>  // for QTableView

class QEvent;
class QWidget;

#include <warn/pop>

namespace inviwo {

class BrushingAndLinkingManager;
class DataFrame;
class DataFrameModel;
class DataFrameSortFilterProxy;

/**
 * \brief Widget for showing a DataFrame in a QTableView
 */
class IVW_MODULE_DATAFRAMEQT_API DataFrameTableView : public QTableView {
#include <warn/push>
#include <warn/ignore/all>
    Q_OBJECT
#include <warn/pop>
public:
    DataFrameTableView(QWidget* parent = nullptr);
    virtual ~DataFrameTableView() = default;

    void setManager(BrushingAndLinkingManager& manager);
    void setDataFrame(std::shared_ptr<const DataFrame> dataframe, bool categoryIndices = false);

    void brushingUpdate();

    void setIndexColumnVisible(bool visible);
    bool isIndexColumnVisible() const;

    void setFilteredRowsVisible(bool visible);
    bool getFilteredRowsVisible() const;

    void leaveEvent(QEvent* event) override;

private:
    DataFrameModel* model_;
    DataFrameSortFilterProxy* sortProxy_;

    bool indexVisible_ = false;
    bool categoryIndices_ = false;

    bool ignoreEvents_{false};
    bool ignoreUpdate_{false};
};

}  // namespace inviwo
