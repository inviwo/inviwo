/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2019 Inviwo Foundation
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
#include <modules/discretedata/processors/datasetsource.h>

namespace inviwo {
namespace discretedata {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo DataSetSource::processorInfo_{
    "org.inviwo.DataSetSource",  // Class identifier
    "DataSet Source",            // Display name
    "Data Set",                  // Category
    CodeState::Experimental,     // Code state
    Tags::None,                  // Tags
};

const ProcessorInfo DataSetSource::getProcessorInfo() const { return processorInfo_; }

DataSetSource::DataSetSource()
    : DataSource<DataSetInitializer, DataSetOutport>()
    , renameChannels_("renameChannels", "Rename Channels") {
    DataSource<DataSetInitializer, DataSetOutport>::file_.setContentType("dataset");
    DataSource<DataSetInitializer, DataSetOutport>::file_.setDisplayName("Data Set File");

    addProperty(renameChannels_);
}
void DataSetSource::updateChannelNames(std::shared_ptr<DataSetInitializer> data) {
    ind numChannels = (ind)data->channels_.size();
    while ((ind)renameChannels_.size() > numChannels) {
        renameChannels_.removeProperty(renameChannels_.size() - 1);
    }
    auto stringProperties = renameChannels_.getProperties();

    ind c = 0;
    for (; c < (ind)stringProperties.size(); ++c) {
        auto propName = stringProperties[c]->getDisplayName();
        auto currentName = data->channels_[c]->getName();

        // No the same original channel name as in the property.
        if (currentName.length() > propName.length() ||
            propName.compare(0, currentName.length(), currentName)) {

            auto stringProp = dynamic_cast<StringProperty*>(stringProperties[c]);
            stringProp->set(currentName);
            stringProp->setDisplayName(
                currentName + " (" +
                std::to_string((int)data->channels_[c]->getGridPrimitiveType()) + "D)");
        }
    }

    for (; c < numChannels; ++c) {
        auto name = data->channels_[c]->getName();
        auto newProp = new StringProperty(
            "channel" + std::to_string(renameChannels_.size()),
            name + " (" + std::to_string((int)data->channels_[c]->getGridPrimitiveType()) + "D)",
            name);
        renameChannels_.addProperty(newProp, true);
    }
}

void DataSetSource::process() {
    DataSource<DataSetInitializer, DataSetOutport>::process();

    if (!port_.hasData()) return;

    bool dataChanged = false;
    for (ind c = 0; c < (ind)loadedData_->channels_.size(); ++c) {
        auto& nameProp = dynamic_cast<StringProperty*>(renameChannels_.getProperties()[c])->get();
        if (loadedData_->channels_[c]->getName().compare(nameProp)) {
            loadedData_->channels_[c]->setName(nameProp);
            dataChanged = true;
        }
    }
    if (dataChanged) port_.setData(loadedData_);
}

}  // namespace discretedata
}  // namespace inviwo
