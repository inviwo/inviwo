/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2019 Inviwo Foundation
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

#ifndef IVW_TFSELECTOR_H
#define IVW_TFSELECTOR_H

#include <modules/base/basemoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/properties/transferfunctionproperty.h>
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/properties/eventproperty.h>
#include <inviwo/core/properties/compositeproperty.h>
#include <inviwo/core/properties/listproperty.h>
#include <inviwo/core/properties/propertyownerobserver.h>
#include <inviwo/core/ports/imageport.h>

namespace inviwo {

/** \docpage{org.inviwo.TFSelector, TFSelector}
 * ![](org.inviwo.TFSelector.png?classIdentifier=org.inviwo.TFSelector)
 * allows to select a transfer function from a number of presets which can be added and modified by
 * the user. The image is passed through without any modifications.
 *
 * ### Inports
 *   * __inport__   Input image
 *
 * ### Outports
 *   * __outport__  Unchanged input image
 *
 * ### Properties
 *   * __TF out__   selected transfer function (read-only)
 *   * __Selected__ option property to select the TF
 *   * __Cycle__    if true, the first TF will be the successor of the last available TF
 *   * __Presets__  list of TF presets
 */

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

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

private:
    virtual void deserialize(Deserializer& d) override;

    virtual void onSetDisplayName(Property* property, const std::string& displayName) override;
    virtual void onDidAddProperty(Property* property, size_t index) override;
    virtual void onDidRemoveProperty(Property* property, size_t index) override;

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

#endif  // IVW_TFSELECTOR_H
