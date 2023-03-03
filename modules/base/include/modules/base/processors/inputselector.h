/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2023 Inviwo Foundation
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

#include <inviwo/core/datastructures/datatraits.h>   // for DataTraits
#include <inviwo/core/ports/imageport.h>             // for ImageMultiInport
#include <inviwo/core/ports/inport.h>                // for Inport
#include <inviwo/core/processors/processor.h>        // for Processor
#include <inviwo/core/processors/processorinfo.h>    // for ProcessorInfo
#include <inviwo/core/processors/processorstate.h>   // for CodeState, CodeState::Stable
#include <inviwo/core/processors/processortags.h>    // for Tags, Tags::CPU
#include <inviwo/core/processors/processortraits.h>  // for ProcessorTraits
#include <inviwo/core/properties/optionproperty.h>   // for OptionPropertySize_t, OptionProperty...
#include <inviwo/core/properties/valuewrapper.h>     // for PropertySerializationMode, PropertyS...
#include <inviwo/core/util/assertion.h>              // for IVW_ASSERT
#include <inviwo/core/network/processornetwork.h>    // IWYU pragma: keep

#include <string_view>  // for string_view
#include <vector>       // for vector

namespace inviwo {
class Outport;

/** \docpage{org.inviwo.InputSelector, Input Selector}
 * ![](org.inviwo.InputSelector.png?classIdentifier=org.inviwo.InputSelector)
 * Allows to select one particular input from all given ones.
 *
 * ### Inports
 *   * __inport__  all available inputs
 *
 * ### Outports
 *   * __outport__ selected input
 *
 * ### Properties
 *   * __selected inport__  name of selected port
 */

/**
 * \brief processor for selecting one of n connected inputs
 */
template <typename InportType, typename OutportType>
class InputSelector : public Processor {
public:
    InputSelector();
    virtual ~InputSelector() = default;

    virtual const ProcessorInfo getProcessorInfo() const override;

    virtual void process() override;

    virtual bool isConnectionActive(Inport* from, Outport* to) const override;

private:
    void portSettings();

    InportType inport_;
    OutportType outport_;

    OptionPropertySize_t selectedPort_;
};

template <typename Inport, typename Outport>
const ProcessorInfo InputSelector<Inport, Outport>::getProcessorInfo() const {
    return ProcessorTraits<InputSelector<Inport, Outport>>::getProcessorInfo();
}

template <typename Inport, typename Outport>
InputSelector<Inport, Outport>::InputSelector()
    : Processor()
    , inport_("inport")
    , outport_("outport")
    , selectedPort_("selectedPort", "Select Inport") {
    portSettings();

    addPort(inport_);
    addPort(outport_);

    auto updateOptions = [this]() {
        if (getNetwork()->isDeserializing()) return;
        std::vector<OptionPropertySize_tOption> options;

        for (auto port : inport_.getConnectedOutports()) {
            const auto id = port->getProcessor()->getIdentifier();
            const auto dispName = port->getProcessor()->getDisplayName();
            options.emplace_back(id, dispName, options.size());
        }
        selectedPort_.replaceOptions(options);
        selectedPort_.setCurrentStateAsDefault();
    };

    inport_.onConnect([updateOptions]() { updateOptions(); });
    inport_.onDisconnect([updateOptions]() { updateOptions(); });

    inport_.setIsReadyUpdater([this]() {
        return selectedPort_.size() > 0 && inport_.getConnectedOutports().size() > *selectedPort_ &&
               inport_.getConnectedOutports()[*selectedPort_]->isReady();
    });

    addProperty(selectedPort_);

    selectedPort_.setSerializationMode(PropertySerializationMode::All);
    setAllPropertiesCurrentStateAsDefault();

    selectedPort_.onChange([this]() {
        inport_.readyUpdate();
        this->notifyObserversActiveConnectionsChange(this);
    });
}

template <typename Inport, typename Outport>
void InputSelector<Inport, Outport>::process() {
    auto data = inport_.getVectorData();
    if (selectedPort_.get() < data.size()) {
        outport_.setData(data[selectedPort_.get()]);
    } else {
        outport_.setData(data.back());
    }
}

template <typename InportType, typename OutportType>
bool InputSelector<InportType, OutportType>::isConnectionActive(Inport* from, Outport* to) const {
    IVW_ASSERT(from == &inport_, "only one inport");
    return from->getConnectedOutports().size() > *selectedPort_ &&
           from->getConnectedOutports()[*selectedPort_] == to;
}

template <>
void InputSelector<ImageMultiInport, ImageOutport>::portSettings();

template <typename Inport, typename Outport>
void InputSelector<Inport, Outport>::portSettings() {}

template <typename Inport, typename Outport>
struct ProcessorTraits<InputSelector<Inport, Outport>> {
    static ProcessorInfo getProcessorInfo() {
        using DataType = typename Inport::type;
        return {
            DataTraits<DataType>::classIdentifier() + ".InputSelector",  // Class identifier
            DataTraits<DataType>::dataName() + " Input Selector",        // Display name
            "Data Selector",                                             // Category
            CodeState::Stable,                                           // Code state
            Tags::CPU,                                                   // Tags
        };
    }
};

}  // namespace inviwo
