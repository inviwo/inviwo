/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2020-2022 Inviwo Foundation
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

#include <inviwo/dataframe/dataframemoduledefine.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/properties/listproperty.h>

#include <inviwo/dataframe/datastructures/dataframe.h>
#include <inviwo/dataframe/properties/columnoptionproperty.h>

namespace inviwo {

/** \docpage{org.inviwo.DataFrameJoin, Data Frame Join}
 * ![](org.inviwo.DataFrameJoin.png?classIdentifier=org.inviwo.DataFrameJoin)
 * Merges two DataFrames according to selected join type. This processor supports appending either
 * rows or columns as well as standard database joins, i.e. inner, left outer, right outer, full
 * outer, and cross joins.
 *
 * For row joins, the column count, column headers, and types must match. Similarly, when joining
 * columns the number of rows must match (unless the fill missing rows option is checked).
 *
 * ### Inports
 *   * __left__   DataFrame used as left table in join
 *   * __right__  DataFrame used as right table in join
 *
 * ### Outports
 *   * __outport__  joined DataFrame
 *
 * ### Properties
 *   * __join__   type of join
 */
class IVW_MODULE_DATAFRAME_API DataFrameJoin : public Processor, public PropertyOwnerObserver {
public:
    enum class JoinType {
        AppendColumns,
        AppendRows,
        Inner,
        OuterLeft,
    };
    enum class ColumnMatch { ByName, Ordered };

    DataFrameJoin();
    virtual ~DataFrameJoin() = default;

    virtual void process() override;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

private:
    virtual void onDidAddProperty(Property* property, size_t index) override;
    virtual void onDidRemoveProperty(Property* property, size_t index) override;

    DataFrameInport inportLeft_;
    DataFrameInport inportRight_;
    DataFrameOutport outport_;

    OptionProperty<JoinType> join_;
    BoolProperty ignoreDuplicateCols_;
    BoolProperty fillMissingRows_;
    OptionProperty<ColumnMatch> columnMatching_;
    ColumnOptionProperty key_;
    ListProperty secondaryKeys_;
};

}  // namespace inviwo
