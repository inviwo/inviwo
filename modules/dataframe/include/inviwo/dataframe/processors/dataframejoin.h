/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2020-2024 Inviwo Foundation
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

#include <inviwo/dataframe/dataframemoduledefine.h>  // for IVW_MODULE_DATAFRAME_API

#include <inviwo/core/processors/processor.h>                  // for Processor
#include <inviwo/core/processors/processorinfo.h>              // for ProcessorInfo
#include <inviwo/core/properties/boolproperty.h>               // for BoolProperty
#include <inviwo/core/properties/listproperty.h>               // for ListProperty
#include <inviwo/core/properties/optionproperty.h>             // for OptionProperty
#include <inviwo/core/properties/propertyownerobserver.h>      // for PropertyOwnerObserver
#include <inviwo/core/util/staticstring.h>                     // for operator+
#include <inviwo/dataframe/datastructures/dataframe.h>         // for DataFrameInport, DataFrame...
#include <inviwo/dataframe/properties/columnoptionproperty.h>  // for ColumnOptionProperty

#include <cstddef>      // for size_t
#include <functional>   // for __base
#include <string>       // for operator==, operator+, string
#include <string_view>  // for operator==
#include <vector>       // for operator!=, vector, operat...

namespace inviwo {
class Property;
class PropertyOwner;

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

    virtual const ProcessorInfo& getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

private:
    virtual void onDidAddProperty(Property* property, size_t index) override;
    virtual void onDidRemoveProperty(PropertyOwner* owner, Property* property,
                                     size_t index) override;

    DataFrameInport inportLeft_;
    DataFrameInport inportRight_;
    DataFrameOutport outport_;

    OptionProperty<JoinType> join_;
    BoolProperty ignoreDuplicateCols_;
    BoolProperty fillMissingRows_;
    OptionProperty<ColumnMatch> columnMatching_;
    ColumnOptionProperty leftKey_;
    ColumnOptionProperty rightKey_;
    ListProperty secondaryLeftKeys_;
    ListProperty secondaryRightKeys_;
};

}  // namespace inviwo
