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
#pragma once

#include <inviwo/dataframeqt/dataframeqtmoduledefine.h>
#include <modules/qtwidgets/processors/processorwidgetqt.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/datastructures/bitset.h>
#include <inviwo/core/util/dispatcher.h>

namespace inviwo {

class DataFrame;
class DataFrameTableView;
class BrushingAndLinkingManager;

/**
 * \brief A processor widget showing a DataFrame in a table view.
 */
class IVW_MODULE_DATAFRAMEQT_API DataFrameTableProcessorWidget : public ProcessorWidgetQt {
#include <warn/push>
#include <warn/ignore/all>
    Q_OBJECT
#include <warn/pop>
public:
    DataFrameTableProcessorWidget(Processor* p);
    virtual ~DataFrameTableProcessorWidget() = default;

    virtual void setVisible(bool visible) override;

    void setManager(BrushingAndLinkingManager& manager);
    void setDataFrame(std::shared_ptr<const DataFrame> dataframe, bool categoryIndices = false);
    void setIndexColumnVisible(bool visible);
    void setFilteredRowsVisible(bool visible);

    /**
     * update the selection, filtering, and highlight state of the widget using the brushing and
     * linking manager. Call this function in Processor::process(). This function performs the
     * checks itself whether there have been any changes.
     *
     * \see setManager
     */
    void brushingUpdate();

private:
    using tableview_ptr =
        std::unique_ptr<DataFrameTableView, std::function<void(DataFrameTableView*)>>;
    tableview_ptr tableview_;

    Processor::NameDispatcherHandle nameChange_;
};

}  // namespace inviwo
