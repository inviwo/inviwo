/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017 Inviwo Foundation
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

#ifndef IVW_INPUTSELECTOR_H
#define IVW_INPUTSELECTOR_H

#include <modules/base/basemoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/processors/processortraits.h>
#include <inviwo/core/ports/datainport.h>
#include <inviwo/core/ports/dataoutport.h>
#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/properties/stringproperty.h>
#include <inviwo/core/datastructures/image/image.h>
#include <inviwo/core/datastructures/volume/volume.h>
#include <inviwo/core/datastructures/geometry/mesh.h>

namespace inviwo {

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
 * \class InputSelector
 * \brief processor for selecting one of n connected inputs
 */
template <typename Inport, typename Outport>
class InputSelector : public Processor {
public:
    InputSelector()
        : Processor()
        , inport_("inport")
        , outport_("outport")
        , selectedPort_("selectedPort", "Select Inport") {

        addPort(inport_);
        addPort(outport_);

        selectedPort_.setSerializationMode(PropertySerializationMode::All);

        inport_.onChange([&]() {
            std::string selectedID;
            if (selectedPort_.size() != 0) {
                selectedID = selectedPort_.getSelectedIdentifier();
                selectedPort_.clearOptions();
            }
            int idx = 0;
            for (auto port : inport_.getConnectedOutports()) {
                auto p = port->getProcessor();
                auto displayName = p->getIdentifier();
                auto id = p->getDisplayName();
                selectedPort_.addOption(displayName, displayName, idx++);
            }

            if (!selectedID.empty()) {
                selectedPort_.setSelectedIdentifier(selectedID);
            }

            selectedPort_.setCurrentStateAsDefault();
        });

        addProperty(selectedPort_);

        selectedPort_.setSerializationMode(PropertySerializationMode::All);

        setAllPropertiesCurrentStateAsDefault();
    }
    virtual ~InputSelector() = default;

    virtual void process() override {
        outport_.setData(inport_.getVectorData().at(selectedPort_.get()));
    }

    virtual const ProcessorInfo getProcessorInfo() const override {
        return ProcessorTraits<InputSelector<Inport, Outport>>::getProcessorInfo();
    }

private:
    Inport inport_;
    Outport outport_;

    OptionPropertyInt selectedPort_;
};



template <typename T>
struct DataNamer {
    static std::string getName() {
        using DataType = typename T::type;
        return port_traits<DataType>::class_identifier();
    }
};

template <>
struct DataNamer<Volume> {
    static std::string getName() { return "Volume"; }
};

template <>
struct DataNamer<Image> {
    static std::string getName() { return "Image"; }
};

template <>
struct DataNamer<Mesh> {
    static std::string getName() { return "Mesh"; }
};

template <typename Inport, typename Outport>
struct ProcessorTraits<InputSelector<Inport, Outport>> {
    static ProcessorInfo getProcessorInfo() {
        using DataType = typename Inport::type;
        return {
            port_traits<DataType>::class_identifier() + ".InputSelector",  // Class identifier
            DataNamer<DataType>::getName() + " Input Selector",            // Display name
            "Data Selector",                                               // Category
            CodeState::Stable,                                             // Code state
            Tags::CPU,                                                     // Tags
        };
    }
};

}  // namespace inviwo

#endif  // IVW_INPUTSELECTOR_H
