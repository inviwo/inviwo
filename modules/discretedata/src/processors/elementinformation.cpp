/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2022 Inviwo Foundation
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

#include <modules/discretedata/processors/elementinformation.h>
#include <modules/discretedata/channels/analyticchannel.h>

namespace inviwo {
namespace discretedata {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo ElementInformation::processorInfo_{
    "org.inviwo.ElementInformation",  // Class identifier
    "Element Information",            // Display name
    "Undefined",                      // Category
    CodeState::Experimental,          // Code state
    Tags::None,                       // Tags
};
const ProcessorInfo ElementInformation::getProcessorInfo() const { return processorInfo_; }

ElementInformation::ElementInformation()
    : Processor()
    , dataIn_("dataIn")
    , dataOut_("dataOut")

    , primitive_("primitive", "Primitive")
    , elementIndex_("elementIndex", "Index", 0, {0, ConstraintBehavior::Immutable},
                    {1, ConstraintBehavior::Mutable}, 1)
    , elementInformation_("elementInfo", "Element Information", InvalidationLevel::Valid)

    , addHighlightChannel_("addHighlightChannel", "Add Highlight Channel", true)
    , baseValue_("baseValue", "Base Value", 0.1, {0, ConstraintBehavior::Ignore},
                 {1.0, ConstraintBehavior::Ignore}, 0.1, InvalidationLevel::InvalidOutput,
                 PropertySemantics::Text)
    , highlightValue_("highlightValue", "Highlight Value", 1, {0, ConstraintBehavior::Ignore},
                      {10.0, ConstraintBehavior::Ignore}, 0.1, InvalidationLevel::InvalidOutput,
                      PropertySemantics::Text) {

    addPorts(dataIn_, dataOut_);
    addProperties(primitive_, elementIndex_, elementInformation_, addHighlightChannel_);

    addHighlightChannel_.addProperties(baseValue_, highlightValue_);
    // auto highlightVisibility = [](const Property* prop) -> bool { return *dynamic_cast<const
    // BoolCompositePropertyprop; }; baseValue_.readonlyDependsOn(addHighlightChannel_,
    // highlightVisibility); highlightValue_.readonlyDependsOn(addHighlightChannel_,
    // highlightVisibility);

    isSink_.setUpdate([]() { return true; });
}

void ElementInformation::process() {
    if (!dataIn_.hasData()) {
        dataOut_.clear();
        return;
    }

    auto data = dataIn_.getData();
    if (!data) {
        dataOut_.clear();
        return;
    }

    GridPrimitive maxDim = data->getGrid()->getDimension();
    std::vector<OptionPropertyOption<GridPrimitive>> primitiveOptions;
    for (int dim = 0; dim <= int(maxDim); ++dim) {
        GridPrimitive prim = GridPrimitive(dim);
        primitiveOptions.emplace_back(primitiveName(prim), primitiveName(prim), prim);
    }
    primitive_.replaceOptions(primitiveOptions);

    size_t numElements = data->getGrid()->getNumElements(primitive_.get());
    elementIndex_.setMaxValue(numElements);

    // Update properties displaying values.
    for (Property* prop : elementInformation_) {
        prop->setReadOnly(false);
    }
    for (auto channel : data->getChannels()) {
        if (channel.second->getGridPrimitiveType() != primitive_.get()) continue;

        channel.second
            ->dispatch<void, dispatching::filter::Scalars, 1, DISCRETEDATA_MAX_NUM_DIMENSIONS>(
                detail::AddStringPropertyDispatcher(), &elementInformation_,
                ind(elementIndex_.get()));
        // try {
        //     channel.second->dispatch<void, detail::OrdinalScalars, 1, 4>(
        //         detail::AddPropertyDispatcher(), &elementInformation_, ind(elementIndex_.get()));
        // } catch (Exception& e) {
        //     // LogWarn(fmt::format("", ));
        //     try {
        //         channel.second
        //             ->dispatch<void, detail::NonOrdinalScalars, 1,
        //             DISCRETEDATA_MAX_NUM_DIMENSIONS>(
        //                 detail::AddStringPropertyDispatcher(), &elementInformation_,
        //                 ind(elementIndex_.get()));
        //     } catch (...) {
        //     }
        // }
    }
    std::vector<Property*> allProps(elementInformation_.getProperties());
    for (Property* prop : allProps) {
        if (!prop->getReadOnly()) elementInformation_.removeProperty(prop);
    }

    auto dataOut = std::make_shared<DataSet>(*data);
    if (addHighlightChannel_) {
        auto highlightChannel = std::make_shared<AnalyticChannel<float, 1, float>>(
            [highlightIdx = ind(elementIndex_.get()), baseValue = baseValue_.get(),
             highlightValue = highlightValue_.get()](float& value, ind idx) {
                value = (idx == highlightIdx) ? highlightValue : baseValue;
            },
            numElements, "ElementHighlight", primitive_.get());

        dataOut->addChannel(highlightChannel);
    }
    dataOut_.setData(dataOut);
}

}  // namespace discretedata
}  // namespace inviwo