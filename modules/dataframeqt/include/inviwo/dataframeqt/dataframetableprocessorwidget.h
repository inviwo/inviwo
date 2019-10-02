/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019 Inviwo Foundation
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

#include <inviwo/dataframeqt/dataframeqtmoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <modules/qtwidgets/processors/processorwidgetqt.h>
#include <inviwo/core/processors/processorobserver.h>
#include <inviwo/core/util/dispatcher.h>

#include <unordered_set>

namespace inviwo {

class DataFrame;
class DataFrameTableView;

/**
 * \brief A processor widget showing a DataFrame in a table view.
 */
class IVW_MODULE_DATAFRAMEQT_API DataFrameTableProcessorWidget : public ProcessorWidgetQt,
                                                                 public ProcessorObserver {
#include <warn/push>
#include <warn/ignore/all>
    Q_OBJECT
#include <warn/pop>
public:
    using SelectionChangedFunc = void(const std::unordered_set<size_t>&);
    using CallbackHandle = std::shared_ptr<std::function<SelectionChangedFunc>>;

    DataFrameTableProcessorWidget(Processor* p);
    virtual ~DataFrameTableProcessorWidget() = default;

    void setDataFrame(std::shared_ptr<const DataFrame> dataframe, bool vectorsIntoColumns = false);
    void setIndexColumnVisible(bool visible);

    void updateSelection(const std::unordered_set<size_t>& columns,
                         const std::unordered_set<size_t>& rows);

    CallbackHandle setColumnSelectionChangedCallback(std::function<SelectionChangedFunc> callback);
    CallbackHandle setRowSelectionChangedCallback(std::function<SelectionChangedFunc> callback);

    // Override ProcessorObserver
    virtual void onProcessorDisplayNameChanged(Processor*, const std::string&) override;

private:
    using tableview_ptr =
        std::unique_ptr<DataFrameTableView, std::function<void(DataFrameTableView*)>>;
    tableview_ptr tableview_;

    Dispatcher<SelectionChangedFunc> columnSelectionChanged_;
    Dispatcher<SelectionChangedFunc> rowSelectionChanged_;
};

}  // namespace inviwo
