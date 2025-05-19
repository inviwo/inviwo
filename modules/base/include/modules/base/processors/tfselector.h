/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2025 Inviwo Foundation
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

#include <modules/base/basemoduledefine.h>  // for IVW_MODULE_BASE_API

#include <inviwo/core/ports/imageport.h>                      // for ImageInport, ImageOutport
#include <inviwo/core/processors/processor.h>                 // for Processor
#include <inviwo/core/processors/processorinfo.h>             // for ProcessorInfo
#include <inviwo/core/properties/boolproperty.h>              // for BoolProperty
#include <inviwo/core/properties/compositeproperty.h>         // for CompositeProperty
#include <inviwo/core/properties/eventproperty.h>             // for EventProperty
#include <inviwo/core/properties/listproperty.h>              // for ListProperty
#include <inviwo/core/properties/optionproperty.h>            // for OptionPropertyString
#include <inviwo/core/properties/propertyobserver.h>          // for PropertyObserver
#include <inviwo/core/properties/propertyownerobserver.h>     // for PropertyOwnerObserver
#include <inviwo/core/properties/transferfunctionproperty.h>  // for TransferFunctionProperty

#include <cstddef>  // for size_t
#include <string>   // for string

namespace inviwo {
class Deserializer;
class Property;
class PropertyOwner;

/**
 * \class TFSelector
 * \brief processor for selecting a transfer function from a number of customizable presets
 */
class IVW_MODULE_BASE_API TFSelector : public Processor,
                                       public PropertyObserver,
                                       public PropertyOwnerObserver {
public:
    TFSelector();
    virtual ~TFSelector() = default;

    virtual void process() override;

    virtual const ProcessorInfo& getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

private:
    virtual void deserialize(Deserializer& d) override;

    virtual void onSetDisplayName(Property* property, const std::string& displayName) override;
    virtual void onDidAddProperty(Property* property, size_t index) override;
    virtual void onDidRemoveProperty(PropertyOwner* owner, Property* property,
                                     size_t index) override;

    void updateOptions();

    ImageInport inport_;
    ImageOutport outport_;

    TransferFunctionProperty tfOut_;
    OptionPropertyString selectedTF_;
    BoolProperty cycle_;

    ListProperty tfPresets_;

    CompositeProperty interactions_;
    EventProperty nextTF_;
    EventProperty previousTF_;
};

}  // namespace inviwo
