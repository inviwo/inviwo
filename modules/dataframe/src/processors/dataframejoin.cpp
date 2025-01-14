/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2020-2025 Inviwo Foundation
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

#include <inviwo/dataframe/processors/dataframejoin.h>

#include <inviwo/core/processors/processor.h>                  // for Processor
#include <inviwo/core/processors/processorinfo.h>              // for ProcessorInfo
#include <inviwo/core/processors/processorstate.h>             // for CodeState, CodeState::Expe...
#include <inviwo/core/processors/processortags.h>              // for Tags
#include <inviwo/core/properties/boolproperty.h>               // for BoolProperty
#include <inviwo/core/properties/listproperty.h>               // for ListProperty
#include <inviwo/core/properties/optionproperty.h>             // for OptionProperty, OptionProp...
#include <inviwo/core/properties/property.h>                   // for Property
#include <inviwo/core/properties/propertyownerobserver.h>      // for PropertyOwnerObservable
#include <inviwo/core/util/exception.h>                        // for Exception
#include <inviwo/core/util/sourcecontext.h>                    // for SourceContext
#include <inviwo/core/util/staticstring.h>                     // for operator+
#include <inviwo/dataframe/datastructures/dataframe.h>         // for DataFrameInport, DataFrame...
#include <inviwo/dataframe/properties/columnoptionproperty.h>  // for ColumnOptionProperty
#include <inviwo/dataframe/util/dataframeutil.h>               // for appendColumns, appendRows

#include <memory>  // for shared_ptr, make_unique

namespace inviwo {
class PropertyOwner;

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo DataFrameJoin::processorInfo_{
    "org.inviwo.DataFrameJoin",  // Class identifier
    "DataFrame Join",            // Display name
    "DataFrame",                 // Category
    CodeState::Experimental,     // Code state
    "CPU, DataFrame",            // Tags
};
const ProcessorInfo& DataFrameJoin::getProcessorInfo() const { return processorInfo_; }

DataFrameJoin::DataFrameJoin()
    : Processor()
    , inportLeft_("top")
    , inportRight_("bottom")
    , outport_("outport")
    , join_("join", "Join Type",
            {{"appendColumns", "Append Columns", JoinType::AppendColumns},
             {"appendRows", "Append Rows", JoinType::AppendRows},
             {"inner", "Inner Join", JoinType::Inner},
             {"outerleft", "Outer Left Join", JoinType::OuterLeft}})
    , ignoreDuplicateCols_("ignoreDuplicateCols", "Ignore Duplicate Columns", false)
    , fillMissingRows_("fillMissingRows", "Fill Missing Rows", false)
    , columnMatching_("columnMatching", "Match Columns",
                      {{"byname", "By Name", ColumnMatch::ByName},
                       {"ordered", "By Order", ColumnMatch::Ordered}})
    , leftKey_("leftKey", "Left Key Column", inportLeft_)
    , rightKey_("rightKey", "Right Key Column", inportRight_)
    , secondaryLeftKeys_(
          "secondaryLeftKeys", "Secondary Left Key Columns",
          std::make_unique<ColumnOptionProperty>("leftKey2", "Left Key Column 2", inportLeft_))
    , secondaryRightKeys_(
          "secondaryRightKeys", "Secondary Right Key Columns",
          std::make_unique<ColumnOptionProperty>("rightKey2", "Right Key Column 2", inportRight_)) {

    addPorts(inportLeft_, inportRight_, outport_);

    ignoreDuplicateCols_.visibilityDependsOn(
        join_, [](const auto& p) { return p == JoinType::AppendColumns; });
    fillMissingRows_.visibilityDependsOn(
        join_, [](const auto& p) { return p == JoinType::AppendColumns; });
    columnMatching_.visibilityDependsOn(join_,
                                        [](const auto& p) { return p == JoinType::AppendRows; });
    auto keyVisible = [](const auto& p) {
        return (p == JoinType::Inner || p == JoinType::OuterLeft);
    };

    leftKey_.visibilityDependsOn(join_, keyVisible);
    rightKey_.visibilityDependsOn(join_, keyVisible);
    secondaryLeftKeys_.visibilityDependsOn(join_, keyVisible);
    secondaryRightKeys_.visibilityDependsOn(join_, keyVisible);

    addProperties(join_, ignoreDuplicateCols_, fillMissingRows_, columnMatching_, leftKey_,
                  rightKey_, secondaryLeftKeys_, secondaryRightKeys_);

    inportLeft_.onChange([&]() {
        for (auto p : secondaryLeftKeys_) {
            if (auto keyProp = dynamic_cast<ColumnOptionProperty*>(p)) {
                if (inportLeft_.hasData()) {
                    keyProp->setOptions(*inportLeft_.getData());
                }
            }
        }
    });

    inportRight_.onChange([&]() {
        for (auto p : secondaryRightKeys_) {
            if (auto keyProp = dynamic_cast<ColumnOptionProperty*>(p)) {
                if (inportRight_.hasData()) {
                    keyProp->setOptions(*inportRight_.getData());
                }
            }
        }
    });

    secondaryLeftKeys_.PropertyOwnerObservable::addObserver(this);
    secondaryRightKeys_.PropertyOwnerObservable::addObserver(this);
}

void DataFrameJoin::process() {
    std::vector<std::pair<std::string, std::string>> keys;
    keys.push_back({leftKey_.getSelectedColumnHeader(), rightKey_.getSelectedColumnHeader()});

    for (auto&& [left, right] : util::zip(secondaryLeftKeys_, secondaryRightKeys_)) {
        if (auto leftKeyProp = dynamic_cast<ColumnOptionProperty*>(left)) {
            if (auto rightKeyProp = dynamic_cast<ColumnOptionProperty*>(right)) {
                keys.push_back({leftKeyProp->getSelectedColumnHeader(),
                                rightKeyProp->getSelectedColumnHeader()});
            }
        }
    }

    std::shared_ptr<DataFrame> dataframe;
    switch (join_) {
        case JoinType::AppendColumns:
            dataframe = dataframe::appendColumns(*inportLeft_.getData(), *inportRight_.getData(),
                                                 ignoreDuplicateCols_, fillMissingRows_);
            break;
        case JoinType::AppendRows:
            dataframe = dataframe::appendRows(*inportLeft_.getData(), *inportRight_.getData(),
                                              columnMatching_ == ColumnMatch::ByName);
            break;
        case JoinType::Inner:
            dataframe = dataframe::innerJoin(*inportLeft_.getData(), *inportRight_.getData(), keys);
            break;
        case JoinType::OuterLeft:
            dataframe = dataframe::leftJoin(*inportLeft_.getData(), *inportRight_.getData(), keys);
            break;
        default:
            throw Exception("unsupported join operation");
    }
    outport_.setData(dataframe);
}

void DataFrameJoin::onDidAddProperty(Property*, size_t) {}

void DataFrameJoin::onDidRemoveProperty(PropertyOwner* owner, Property*, size_t) {
    owner->invalidate(InvalidationLevel::InvalidOutput, nullptr);
}

}  // namespace inviwo
