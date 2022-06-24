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
#include <inviwo/core/properties/boolcompositeproperty.h>
#include <inviwo/core/ports/imageport.h>
#include <modules/discretedata/ports/datasetport.h>
#include <inviwo/core/util/zip.h>
#include <map>

namespace inviwo {
namespace discretedata {

class IVW_MODULE_DISCRETEDATA_API ChannelPickingListProperty : public BoolProperty {
    using GridPrimitiveProperty = TemplateOptionProperty<GridPrimitive>;

public:
    ChannelPickingListProperty() = delete;
    ChannelPickingListProperty(const ChannelPickingListProperty&) = delete;
    ChannelPickingListProperty& operator=(const ChannelPickingListProperty&) = delete;

    ChannelPickingListProperty(const std::shared_ptr<const Channel>& channel,
                               const Connectivity& grid);

    std::shared_ptr<const Channel> channel_;
};

class IVW_MODULE_DISCRETEDATA_API DataSetChannelsProperty : public CompositeProperty {

public:
    DataSetChannelsProperty() = delete;
    DataSetChannelsProperty(const std::string& identifier,  // const std::string& name,
                            std::shared_ptr<const DataSet>& dataSet, const Connectivity& grid);

    void updateDataSet(std::shared_ptr<const DataSet>& dataSet, const Connectivity& grid);

    std::shared_ptr<const DataSet> dataSet_ = nullptr;
};

/** \docpage{org.inviwo.CombineDataSets, Combine Data Sets}
 * ![](org.inviwo.CombineDataSets.png?classIdentifier=org.inviwo.CombineDataSets)
 * Combine two or more DataSets
 * Take the grid and samplers of the first dataset, and a number of channels from the others.
 */
class IVW_MODULE_DISCRETEDATA_API CombineDataSets : public Processor {
public:
    CombineDataSets();
    virtual ~CombineDataSets() = default;

    virtual void process() override;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

    void deserializeSelection();
    virtual void deserialize(Deserializer&) override;
    virtual void serialize(Serializer& s) const override;

private:
    DataSetInport baseDataSetIn_;
    DataInport<DataSet, 0> additionalDataSetsIn_;
    DataSetOutport dataOut_;

    std::vector<std::string> deserializedChannels_;
};

}  // namespace discretedata
}  // namespace inviwo
