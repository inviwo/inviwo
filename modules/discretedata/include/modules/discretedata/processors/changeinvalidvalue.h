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

#pragma once

#include <modules/discretedata/discretedatamoduledefine.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/properties/stringproperty.h>
#include <inviwo/core/properties/compositeproperty.h>
#include <inviwo/core/properties/boolcompositeproperty.h>
#include <modules/discretedata/ports/datasetport.h>
#include <modules/discretedata/dataset.h>

namespace inviwo {
namespace discretedata {

/** \docpage{org.inviwo.ElementInformation, Element Information}
 * ![](org.inviwo.ElementInformation.png?classIdentifier=org.inviwo.ElementInformation)
 * Get data for a single element in a grid. Only up to 4 dimensions will be displayed.
 */
class IVW_MODULE_DISCRETEDATA_API ChangeInvalidValue : public Processor {
public:
    ChangeInvalidValue();
    virtual ~ChangeInvalidValue() = default;

    virtual void process() override;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

private:
    DataSetInport dataIn_;
    DataSetOutport dataOut_;
};

namespace detail {
struct UpdatePropertyAndChannelDispatcher {
    template <typename T, ind N>
    void operator()(std::shared_ptr<const DataChannel<T, N>>&& channel, PropertyOwner* owner,
                    DataSet* dataset) {

        std::string name =
            fmt::format("{}_{}", channel->getName(), (int)channel->getGridPrimitiveType());
        Property* baseProp = owner->getPropertyByIdentifier(name);
        OrdinalProperty<T>* prop = dynamic_cast<OrdinalProperty<T>*>(baseProp);
        // Wrong type? Remove.
        if (baseProp && !prop) {
            owner->removeProperty(prop);
        }
        T invalidValue = channel->getInvalidValue();
        if (std::isnan(invalidValue)) {
            prop->setReadOnly(true);
            return;
        }

        // Make new property.
        if (!prop) {
            // if (std::isnan(invalidValue))
            //     invalidValue
            prop = new OrdinalProperty<T>(name, channel->getName(), invalidValue,
                                          {T{0}, ConstraintBehavior::Ignore},
                                          {T{10}, ConstraintBehavior::Ignore});
            owner->addProperties(prop);
            dataset->addChannel(channel);

            std::cout << fmt::format("Making new property for for {}", channel->getName())
                      << std::endl;
            return;
        }

        // Nothing to do?
        prop->setReadOnly(false);
        if (prop->get() == invalidValue) {
            dataset->addChannel(channel);
            std::cout << fmt::format("Invalid value the same for {}: {} == {}", channel->getName(),
                                     prop->get(), invalidValue)
                      << std::endl;
            return;
        }

        std::cout << fmt::format("Invalid value NOT the same for {}: {} == {}", channel->getName(),
                                 prop->get(), invalidValue)
                  << std::endl;

        // Make new channel with different invalid value.
        auto newChannel = dynamic_cast<DataChannel<T, N>*>(channel->clone());
        newChannel->setInvalidValue(prop->get());
        std::cout << fmt::format("New invalid value: {}", newChannel->getInvalidValue())
                  << std::endl;
        dataset->addChannel(newChannel);
    }
};

}  // namespace detail

}  // namespace discretedata
}  // namespace inviwo
