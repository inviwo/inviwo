/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2025 Inviwo Foundation
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

#include <inviwo/core/metadata/processorwidgetmetadata.h>  // for ProcessorWidgetMet...
#include <inviwo/core/processors/processor.h>              // for Processor
#include <inviwo/core/processors/processorinfo.h>          // for ProcessorInfo
#include <inviwo/core/properties/boolproperty.h>           // for BoolProperty
#include <inviwo/core/properties/stringproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>                    // for IntSize2Property
#include <inviwo/core/util/glmvec.h>                                   // for size2_t
#include <inviwo/dataframe/datastructures/dataframe.h>                 // for DataFrameInport
#include <modules/brushingandlinking/ports/brushingandlinkingports.h>  // for BrushingAndLinking...
#include <inviwo/core/processors/processorwidget.h>
#include <modules/qtwidgets/processors/processordockwidgetqt.h>

namespace inviwo {

class DataFrameTableView;

class IVW_MODULE_DATAFRAMEQT_API DataFrameDockTableWidget : public ProcessorDockWidgetQt {
public:
    explicit DataFrameDockTableWidget(Processor* p);
    DataFrameDockTableWidget(const DataFrameDockTableWidget&) = delete;
    DataFrameDockTableWidget(DataFrameDockTableWidget&&) = delete;
    DataFrameDockTableWidget& operator=(const DataFrameDockTableWidget&) = delete;
    DataFrameDockTableWidget& operator=(DataFrameDockTableWidget&&) = delete;
    virtual ~DataFrameDockTableWidget();

    void setManager(BrushingAndLinkingManager& manager);
    void setDataFrame(std::shared_ptr<const DataFrame> dataframe, bool categoryIndices = false);
    void setIndexColumnVisible(bool visible);
    void setFilteredRowsVisible(bool visible);
    void brushingUpdate();

private:
    DataFrameTableView* tableview_;
};

class IVW_MODULE_DATAFRAMEQT_API DataFrameDockTable : public Processor,
                                                      public ProcessorWidgetMetaDataObserver {
public:
    DataFrameDockTable();
    DataFrameDockTable(const DataFrameDockTable&) = delete;
    DataFrameDockTable(DataFrameDockTable&&) = delete;
    DataFrameDockTable& operator=(const DataFrameDockTable&) = delete;
    DataFrameDockTable& operator=(DataFrameDockTable&&) = delete;

    virtual ~DataFrameDockTable();

    virtual void process() override;

    virtual const ProcessorInfo& getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

    void setWidgetSize(size2_t);
    size2_t getWidgetSize() const;

    virtual void setProcessorWidget(std::unique_ptr<ProcessorWidget> processorWidget) override;

protected:
    virtual void onProcessorWidgetPositionChange(ProcessorWidgetMetaData*) override;
    virtual void onProcessorWidgetDimensionChange(ProcessorWidgetMetaData*) override;
    virtual void onProcessorWidgetVisibilityChange(ProcessorWidgetMetaData*) override;

    DataFrameDockTableWidget* getWidget() const;

private:
    DataFrameInport inport_;
    BrushingAndLinkingInport brushLinkPort_;

    IntSize2Property dimensions_;
    IntVec2Property position_;
    BoolProperty visible_;
    StringProperty parent_;

    BoolProperty showIndexColumn_;
    BoolProperty showCategoryIndices_;
    BoolProperty showFilteredRowCols_;

    ProcessorWidgetMetaData* widgetMetaData_;
};

}  // namespace inviwo
