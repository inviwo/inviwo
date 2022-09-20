/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2022 Inviwo Foundation
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

#include <modules/base/processors/tfselector.h>

#include <inviwo/core/datastructures/transferfunction.h>      // for TransferFunction
#include <inviwo/core/interaction/events/event.h>             // for Event
#include <inviwo/core/interaction/events/keyboardkeys.h>      // for IvwKey, KeyState, KeyState:...
#include <inviwo/core/network/networklock.h>                  // for NetworkLock
#include <inviwo/core/ports/imageport.h>                      // for ImageInport, ImageOutport
#include <inviwo/core/processors/processor.h>                 // for Processor
#include <inviwo/core/processors/processorinfo.h>             // for ProcessorInfo
#include <inviwo/core/processors/processorstate.h>            // for CodeState, CodeState::Stable
#include <inviwo/core/processors/processortags.h>             // for Tags
#include <inviwo/core/properties/boolproperty.h>              // for BoolProperty
#include <inviwo/core/properties/compositeproperty.h>         // for CompositeProperty
#include <inviwo/core/properties/eventproperty.h>             // for EventProperty, EventPropert...
#include <inviwo/core/properties/listproperty.h>              // for ListProperty
#include <inviwo/core/properties/optionproperty.h>            // for OptionPropertyOption, Optio...
#include <inviwo/core/properties/property.h>                  // for Property
#include <inviwo/core/properties/propertyownerobserver.h>     // for PropertyOwnerObservable
#include <inviwo/core/properties/transferfunctionproperty.h>  // for TransferFunctionProperty

#include <functional>                                         // for __base
#include <memory>                                             // for make_unique, unique_ptr
#include <string_view>                                        // for string_view
#include <vector>                                             // for vector

namespace inviwo {
class Deserializer;
class PropertyOwner;

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo TFSelector::processorInfo_{
    "org.inviwo.TFSelector",               // Class identifier
    "Transfer Function Selector",          // Display name
    "Transfer Function",                   // Category
    CodeState::Stable,                     // Code state
    "Transfer Function, TF, Presets, UI",  // Tags
};

const ProcessorInfo TFSelector::getProcessorInfo() const { return processorInfo_; }

TFSelector::TFSelector()
    : Processor()
    , inport_("inport")
    , outport_("outport")
    , tfOut_("tfOut", "TF Out", TransferFunction{})
    , selectedTF_("selectedTF", "Selected")
    , cycle_("cycle", "Cycle TFs")
    , tfPresets_("presets", "Presets",
                 std::make_unique<TransferFunctionProperty>("tfPreset1", "TF 1"))
    , interactions_("interactions", "Interactions")
    , nextTF_(
          "nextTF", "Next TF",
          [this](Event* e) {
              size_t next = selectedTF_.getSelectedIndex() + 1;
              if (cycle_.get()) {
                  next = next % selectedTF_.size();
              }
              selectedTF_.setSelectedIndex(next);
              e->markAsUsed();
          },
          IvwKey::Period, KeyState::Press)
    , previousTF_(
          "previousTF", "Previous TF",
          [this](Event* e) {
              if (auto index = selectedTF_.getSelectedIndex()) {
                  selectedTF_.setSelectedIndex(index - 1);
              } else if (cycle_.get()) {
                  selectedTF_.setSelectedIndex(selectedTF_.size() - 1);
              }
              e->markAsUsed();
          },
          IvwKey::Comma, KeyState::Press) {

    addPort(inport_);
    addPort(outport_);

    tfOut_.setReadOnly(true);
    tfOut_.setCurrentStateAsDefault();
    addProperty(tfOut_);

    addProperty(selectedTF_);
    addProperty(cycle_);
    addProperty(tfPresets_);

    interactions_.addProperty(nextTF_);
    interactions_.addProperty(previousTF_);
    addProperty(interactions_);
    interactions_.setCollapsed(true);

    selectedTF_.onChange([this]() {
        for (auto p : tfPresets_.getProperties()) {
            p->onChange([]() {});
        }
        if (tfPresets_.size() > 0 && selectedTF_.size() > 0) {
            auto p = static_cast<TransferFunctionProperty*>(
                tfPresets_.getPropertyByIdentifier(selectedTF_.getSelectedIdentifier()));
            tfOut_.set(p->get());
            // ensure that if the TF is modified, the output is in sync
            p->onChange([this, p]() {
                if (p->getIdentifier() == selectedTF_.getSelectedIdentifier()) {
                    tfOut_.set(p->get());
                }
            });
        }
    });

    tfPresets_.PropertyOwnerObservable::addObserver(this);
}

void TFSelector::process() { outport_.setData(inport_.getData()); }

void TFSelector::deserialize(Deserializer& d) {
    Processor::deserialize(d);

    // ensure that option property is up to date
    updateOptions();
    // make processor aware of changes in display names of TFs
    for (auto p : tfPresets_.getProperties()) {
        p->Property::addObserver(this);
    }
}

void TFSelector::onSetDisplayName(Property*, const std::string&) { updateOptions(); }

void TFSelector::onDidAddProperty(Property* property, size_t) {
    updateOptions();
    property->Property::addObserver(this);
}

void TFSelector::onDidRemoveProperty(PropertyOwner*, Property*, size_t) { updateOptions(); }

void TFSelector::updateOptions() {
    // update option property
    NetworkLock lock(&selectedTF_);

    if (tfPresets_.size()) {
        std::vector<OptionPropertyStringOption> tfOptions;
        for (auto p : tfPresets_.getProperties()) {
            tfOptions.emplace_back(p->getIdentifier(), p->getDisplayName());
        }
        selectedTF_.replaceOptions(tfOptions);
    } else {
        selectedTF_.clearOptions();
        tfOut_.resetToDefaultState();
    }
}

}  // namespace inviwo
