/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2019 Inviwo Foundation
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
#include <modules/discretedata/properties/datasamplerproperty.h>

namespace inviwo {
namespace discretedata {
const std::string DataSamplerProperty::classIdentifier = "inviwo.discretedata.datasamplerproperty";

DataSamplerProperty::DataSamplerProperty(const std::string& identifier,
                                         const std::string& displayName, DataSetInport* dataInport,
                                         InvalidationLevel invalidationLevel,
                                         PropertySemantics semantics)
    : CompositeProperty(identifier, displayName, invalidationLevel, semantics)
    , datasetInput_(dataInport)
    , samplerName_("samplerName", "Sampler name", {{"NONE", "NONE"}})
    , ongoingChange_(false) {
    auto updateCallback = [&]() { updateSamplerList(); };
    if (datasetInput_) datasetInput_->onChange(updateCallback);

    addProperty(samplerName_);
}

void DataSamplerProperty::setDatasetInput(DataSetInport* port) {
    datasetInput_ = port;
    if (datasetInput_) datasetInput_->onChange([&]() { updateSamplerList(); });
}

DataSamplerProperty::DataSamplerProperty(const DataSamplerProperty& prop)
    : CompositeProperty(prop)
    , datasetInput_(prop.datasetInput_)
    , samplerName_("samplerName", "Sampler name", {{"NONE", "NONE"}})
    , ongoingChange_(false) {
    auto updateCallback = [&]() { updateSamplerList(); };
    if (datasetInput_) datasetInput_->onChange(updateCallback);

    addProperty(samplerName_);
    updateSamplerList();
}

DataSamplerProperty& DataSamplerProperty::operator=(const DataSamplerProperty& prop) {

    setIdentifier(prop.getIdentifier());
    setDisplayName(prop.getDisplayName());
    if (prop.datasetInput_) datasetInput_ = prop.datasetInput_;
    samplerName_.replaceOptions({{"NONE", "NONE"}});
    ongoingChange_ = false;

    auto updateCallback = [&]() { updateSamplerList(); };
    if (datasetInput_) datasetInput_->onChange(updateCallback);

    updateSamplerList();
}

std::shared_ptr<const DataSetSamplerBase> DataSamplerProperty::getCurrentSampler() const {
    if (!datasetInput_) return nullptr;

    auto pInDataSet = datasetInput_->getData();
    if (!pInDataSet || samplerName_.size() == 0 || samplerName_.get().compare("NONE") == 0)
        return nullptr;

    const auto& samplerMap = pInDataSet->getSamplers();
    auto samplerIt = samplerMap.find(samplerName_.get());
    if (samplerIt == samplerMap.cend()) return nullptr;
    return samplerIt->second;
}

void DataSamplerProperty::updateSamplerList() {
    if (ongoingChange_) return;

    // If no dataset is given, keep names for now.
    if (!datasetInput_->hasData()) return;

    // Get the current name to select same name if possible.
    std::string lastName = samplerName_.get();

    samplerName_.clearOptions();
    samplerName_.addOption("NONE", "NONE");

    if (!datasetInput_) return;
    auto dataset = datasetInput_->getData();
    if (!dataset) return;

    ongoingChange_ = true;

    const auto& samplerMap = dataset->getSamplers();

    for (auto& sampler : samplerMap) {
        samplerName_.addOption(sampler.first, sampler.first);
    }

    samplerName_.setSelectedIdentifier(lastName);

    ongoingChange_ = false;
}

}  // namespace discretedata
}  // namespace inviwo