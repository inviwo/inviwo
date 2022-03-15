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

#include <modules/discretedata/processors/datasetinformation.h>
#include <inviwo/core/util/zip.h>

namespace inviwo {
namespace discretedata {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo DataSetInformation::processorInfo_{
    "org.inviwo.DataSetInformation",  // Class identifier
    "DataSet Information",            // Display name
    "Undefined",                      // Category
    CodeState::Experimental,          // Code state
    Tags::None,                       // Tags
};
const ProcessorInfo DataSetInformation::getProcessorInfo() const { return processorInfo_; }

DataSetInformation::DataSetInformation()
    : Processor()
    , dataIn_("dataset")
    , overview_("overview", "Overview", "empty", InvalidationLevel::Valid,
                PropertySemantics::Multiline)
    , channelInformation_("channels", "Channels")
    , samplerInformation_("samplers", "Samplers") {

    addPort(dataIn_);
    overview_.setReadOnly(true);
}

void DataSetInformation::process() {

    if (!dataIn_.hasData()) return;
    const DataSet& data = *dataIn_.getData();

    overview_.set(fmt::format("{}, a {} with {} Channels and {} Samplers", data.getName(),
                              data.getGrid()->getIdentifier(), data.size(),
                              data.getSamplers().size()));

    clearProperties();
    addProperty(overview_);
    addProperty(new GridInformationProperty("grid", "Grid", *(dataIn_.getData()->getGrid())));
    addProperties(channelInformation_, samplerInformation_);
    channelInformation_.clearProperties();
    samplerInformation_.clearProperties();

    size_t channelID = 0;
    for (auto channel : data.getChannels()) {
        channelInformation_.addProperty(new ChannelInformationProperty{
            fmt::format("Channel{}", channelID++), channel.second->getName(), *channel.second});
    }

    size_t samplerID = 0;
    for (auto sampler : data.getSamplers()) {
        auto samplerName =
            new StringProperty{fmt::format("Sampler{}", samplerID++), "Sampler", sampler.first};
        samplerInformation_.addProperty(samplerName, true);
    }
}

GridInformationProperty::GridInformationProperty(const std::string& identifier,
                                                 const std::string& name, const Connectivity& grid)
    : CompositeProperty(identifier, name)
    , gridType_("grid", "Type", grid.getIdentifier())
    , numDimensions_("numDims", "Dimensions",
                     fmt::format("{}D ({})", static_cast<int>(grid.getDimension()),
                                 primitiveName(grid.getDimension(), true)))
    , numElements_("numElements", "Num Elements") {
    for (GridPrimitive prim = GridPrimitive::Vertex; prim <= grid.getDimension();
         prim = GridPrimitive((int)prim + 1)) {
        ind numElements = grid.getNumElements(prim);
        auto* prop = new IntProperty{fmt::format("numElements{}D", static_cast<int>(prim)),
                                     primitiveName(prim, numElements != 1),
                                     numElements,
                                     numElements,
                                     numElements,
                                     1,
                                     InvalidationLevel::Valid,
                                     PropertySemantics::Text};
        prop->setReadOnly(true);
        numElements_.addProperty(prop);
    }

    gridType_.setReadOnly(true);
    numDimensions_.setReadOnly(true);
    numElements_.setReadOnly(true);

    addProperties(gridType_, numDimensions_, numElements_);
}

ChannelInformationProperty::ChannelInformationProperty(const std::string& identifier,
                                                       const std::string& name,
                                                       const Channel& channel)
    : CompositeProperty(identifier, name)
    , channelName_("name", "Name", channel.getName())
    , channelPrimitive_(
          "primitive", "Primitive",
          fmt::format("{} ({})", primitiveName(channel.getGridPrimitiveType()), channel.size()))
    , dataType_("dataFormat", "Data Format")
    , dataRange_("dataRange", "Data Range") {

    dataType_ = fmt::format(
        "{} x {}",
        DataFormatBase::get(static_cast<DataFormatId>(channel.getDataFormatId()))->getString(),
        channel.getNumComponents());

    channel.template dispatch<void>([&](const auto* dataChannel) {
        typename std::remove_pointer<decltype(dataChannel)>::type::ArrayType min, max;
        dataChannel->getMinMax(min, max);

        std::stringstream str;

        str << "[ (" << min[0];
        for (size_t i = 1; i < dataChannel->getNumComponents(); ++i) {
            str << ", " << min[i];
        }

        str << "), (" << max[0];
        for (size_t i = 1; i < dataChannel->getNumComponents(); ++i) {
            str << ", " << max[i];
        }
        str << ") ]";

        dataRange_ = str.str();
    });

    channelName_.setReadOnly(true);
    channelPrimitive_.setReadOnly(true);
    dataType_.setReadOnly(true);
    dataRange_.setReadOnly(true);

    addProperties(channelName_, channelPrimitive_, dataType_, dataRange_);
    //     template <typename Result, typename Callable, typename... Args>
    // auto dispatch(Callable&& callable, Args&&... args) -> Result;
    // void DataChannel<T, N>::getMinMax(VecNT& minDest, VecNT& maxDest) const {
}

}  // namespace discretedata
}  // namespace inviwo
