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
 * 1. Redistributions of source code must retain the above copyright notice,
 *this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
 *FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 *DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 *SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 *CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 *OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *********************************************************************************/

#include <inviwo/core/datastructures/volume/volume.h>
#include <inviwo/core/datastructures/volume/volumeramprecision.h>
#include <modules/discretedata/channels/formatconversionchannel.h>
#include <modules/discretedata/connectivity/structuredgrid.h>
#include <modules/discretedata/processors/imagefromdataset.h>

namespace inviwo {
namespace discretedata {

// The Class Identifier has to be globally unique. Use a reverse DNS naming
// scheme
const ProcessorInfo ImageFromDataSet::processorInfo_{
    "org.inviwo.ImageFromDataSet",  // Class identifier
    "Image From DataSet",           // Display name
    "DataSet",                      // Category
    CodeState::Experimental,        // Code state
    Tags::None,                     // Tags
};

const ProcessorInfo ImageFromDataSet::getProcessorInfo() const { return processorInfo_; }

ImageFromDataSet::ImageFromDataSet()
    : dataIn_("InDataSet")
    , imageOut_("OutImage", DataFormat<vec4>::get(), false)
    , datasetSamplerName_("datasetSampler", "DataSet Sampler")
    , interpolationType_("interpolationType", "Interpolation Type",
                         {{"nearest", "Nearest Neighbor", InterpolationType::Nearest},
                          {"squared", "Squared Distance", InterpolationType::SquaredDistance},
                          {"linear", "Linear", InterpolationType::Linear}},
                         2)
    , dataChannel_("channelName", "Data Channel", &dataIn_,
                   [&](const std::shared_ptr<const Channel> channel) -> bool {
                       return channel->getGridPrimitiveType() == GridPrimitive::Vertex;
                   })
    //, useVoxelData("useVoxelData", "Use Voxel Data", false)
    // , floatImageOutput("floatImageOutput", "Convert Data to Float 32")
    , imageSize_("imageSize", "Image Size", {200, 100}, {2, 2}, {2048, 2048}) {
    addPort(dataIn_);
    addPort(imageOut_);
    interpolationType_.setCurrentStateAsDefault();

    addProperties(datasetSamplerName_, interpolationType_, dataChannel_, imageSize_);
    // addProperty(useVoxelData);
}

void ImageFromDataSet::process() {
    // Get data
    if (!dataIn_.isChanged() && !dataIn_.hasData()) return;

    // No samplers in the dataset??
    if (dataIn_.isChanged() &&
        (!dataIn_.hasData() || dataIn_.getData()->getSamplers().size() == 0)) {
        datasetSamplerName_.clearOptions();
        imageOut_.clear();
        return;
    }

    const auto& samplerMap = dataIn_.getData()->getSamplers();

    // Refresh the sampler options.
    if (dataIn_.isChanged() && dataIn_.hasData()) {
        std::vector<std::string> samplerOptions;
        for (auto& sampler : samplerMap) {
            samplerOptions.push_back(sampler.first);
        }
        datasetSamplerName_.replaceOptions(samplerOptions);
    }

    // Something missing to create a SpatialSampler?
    if (!datasetSamplerName_.size() || !dataChannel_.getCurrentChannel()) {
        std::cout << "We didn't get a channel!" << std::endl;
        imageOut_.clear();
        return;
    }

    auto samplerIt = samplerMap.find(datasetSamplerName_.getSelectedValue());
    if (samplerIt == samplerMap.end()) {
        return;
    }

    auto image = sampleImage(dataChannel_.getCurrentChannel(), samplerIt->second);
    if (!image) {
        LogWarn("Invalid sampled image.");
        std::cout << "Invalid image" << std::endl;
        return;
    }

    std::cout << "! Data format II: " << image->getDataFormat()->getString() << std::endl;
    imageOut_.setData(image);
}

std::shared_ptr<inviwo::Image> ImageFromDataSet::sampleImage(
    const std::shared_ptr<const Channel>& channel,
    const std::shared_ptr<const DataSetSamplerBase>& samplerBase) const {
    std::cout << "! Made it to function" << std::endl;

    auto sampler = std::dynamic_pointer_cast<const DataSetSampler<2>>(samplerBase);
    if (!sampler) {
        LogError("Sampler of wrong type (expected 2D).");
        return nullptr;
    }

    size2_t imageSize = imageSize_.get();

    auto image = std::make_shared<Image>(imageSize, DataFormat<vec4>::get());

    auto layer = image->getColorLayer();
    if (!layer) return nullptr;

    auto layerRam =
        dynamic_cast<LayerRAMPrecision<vec4>*>(layer->getEditableRepresentation<LayerRAM>());
    if (!layerRam) return nullptr;

    auto data = layerRam->getDataTyped();
    if (!data) return nullptr;
    std::fill_n(data, imageSize.x * imageSize.y, vec4(0));

    Channel::dispatchSharedPointer<void>(channel, [&](auto dataChannel) -> void {
        using DataChannelNT = typename std::remove_pointer_t<decltype(dataChannel.get())>;
        using T = typename DataChannelNT::value_type;
        constexpr ind N = DataChannelNT::num_comp;

        auto spatialSampler =
            DataSetSpatialSampler<2, N, T>(sampler, interpolationType_.get(), dataChannel);

        for (size_t y = 0; y < imageSize.y; ++y)
            for (size_t x = 0; x < imageSize.x; ++x) {
                dvec2 samplePos = {double(x) / (imageSize.x - 1), double(y) / (imageSize.y - 1)};
                auto sampled = spatialSampler.sampleDataSpace(samplePos);
                data[x + y * imageSize.x] = util::glm_convert<vec4>(sampled);
                // for (size_t n = 0; n < std::min<size_t>(4, N); ++n) {
                //     data[x + y * imageSize.x][n] = double(sampled[n]);
                // }
            }
    });

    LogWarn(image->getDataFormat()->getString());

    std::cout << "! Data format: " << image->getDataFormat()->getString() << std::endl;

    // dvec2 min, max;
    // dataChannel->getMinMax(min, max);
    // // image->dataMap_.dataRange = dvec2(min, max);
    // // image->dataMap_.valueRange = dvec2(min, max);
    // mat2 baseMat(extent.x / imageSize.x, 0, 0, extent.y / imageSize.y);
    // // baseMat /= size.x;
    // // image->setBasis(baseMat);
    return image;
}

}  // namespace discretedata
}  // namespace inviwo
