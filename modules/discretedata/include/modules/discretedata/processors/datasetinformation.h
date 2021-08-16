/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2021 Inviwo Foundation
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
#include <inviwo/core/properties/stringproperty.h>
#include <inviwo/core/properties/compositeproperty.h>
#include <modules/discretedata/ports/datasetport.h>
#include <modules/discretedata/dataset.h>

namespace inviwo {
namespace discretedata {

class IVW_MODULE_DISCRETEDATA_API GridInformationProperty : public CompositeProperty {
public:
    GridInformationProperty() = delete;
    GridInformationProperty(const std::string& identifier, const std::string& name,
                            const Connectivity& grid);

    StringProperty gridType_;
    StringProperty numDimensions_;
    CompositeProperty numElements_;
};

class IVW_MODULE_DISCRETEDATA_API ChannelInformationProperty : public CompositeProperty {
public:
    ChannelInformationProperty() = delete;
    ChannelInformationProperty(const std::string& identifier, const std::string& name,
                               const Channel& channel);
    StringProperty channelName_, channelPrimitive_, dataType_, dataRange_;
    // using DoubleVec2Property = OrdinalProperty<dvec2>;
};

/** \docpage{org.inviwo.DataSetInformation, Data Set Info}
 * ![](org.inviwo.DataSetInformation.png?classIdentifier=org.inviwo.DataSetInformation)
 * Displays information about a single DataSet.
 */
class IVW_MODULE_DISCRETEDATA_API DataSetInformation : public Processor {
public:
    DataSetInformation();
    virtual ~DataSetInformation() = default;

    virtual void process() override;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

private:
    DataSetInport dataIn_;

    StringProperty overview_;
    // GridInformationProperty gridInformation_;
    CompositeProperty channelInformation_;
    CompositeProperty samplerInformation_;
};

}  // namespace discretedata
}  // namespace inviwo
