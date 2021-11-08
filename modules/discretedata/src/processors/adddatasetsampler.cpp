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

#include <modules/discretedata/processors/adddatasetsampler.h>
#include <modules/discretedata/channels/channeldispatching.h>
#include <modules/discretedata/sampling/celltree.h>

namespace inviwo {
namespace discretedata {

std::map<std::string, std::pair<std::string, AddDataSetSampler::CreateSampler>>
    AddDataSetSampler::samplerCreatorList_;
std::map<std::string, std::pair<std::string, AddDataSetSampler::CreateInterpolant>>
    AddDataSetSampler::interpolantCreatorList_;

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo AddDataSetSampler::processorInfo_{
    "org.inviwo.AddDataSetSampler",  // Class identifier
    "Add DataSet Sampler",           // Display name
    "Undefined",                     // Category
    CodeState::Experimental,         // Code state
    Tags::None,                      // Tags
};
const ProcessorInfo AddDataSetSampler::getProcessorInfo() const { return processorInfo_; }

AddDataSetSampler::AddDataSetSampler()
    : Processor()
    , dataIn_("datasetIn")
    , dataOut_("datasetWithSamplerOut")
    , meshOut_("debugMeshOut")
    , positionChannel_(
          "positionChannel", "Position Channel (double)", &dataIn_,
          [&](const std::shared_ptr<const Channel> channel) {
              return channel->getGridPrimitiveType() == GridPrimitive::Vertex &&
                     channel->getNumComponents() ==
                         static_cast<ind>(dataIn_.getData()->getGrid()->getDimension());
          })
    , samplerCreator_("sampler", "Sampler Type")
    , interpolantCreator_("interpolation", "Cell Interpolant")
    // , interpolationType_("interpolationType", "Interpolation Type",
    //                      {{"nearest", "Nearest Neighbor", InterpolationType::Nearest},
    //                       {"squared", "Squared Distance", InterpolationType::SquaredDistance},
    //                       {"linear", "Linear", InterpolationType::Linear}},
    //                      2)
    , interpolant_(nullptr)
    , sampler_(nullptr)
    , interpolantChanged_(true)
    , interpolationChanged_(true)
    , samplerChanged_(true) {

    addPort(dataIn_);
    addPort(dataOut_);
    addPort(meshOut_);
    addProperties(positionChannel_, samplerCreator_, interpolantCreator_);
    positionChannel_.gridPrimitive_.setVisible(false);
    // interpolationType_.setCurrentStateAsDefault();

    for (auto& creator : samplerCreatorList_) {
        samplerCreator_.addOption(creator.first, creator.second.first, samplerCreator_.size());
    }

    for (auto& creator : interpolantCreatorList_) {
        interpolantCreator_.addOption(creator.first, creator.second.first,
                                      interpolantCreator_.size());
    }

    interpolantCreator_.onChange([this]() {
        interpolantChanged_ = true;
        invalidate(InvalidationLevel::InvalidOutput);
        std::cout << "Changed interpolant" << std::endl;
    });
    // interpolationType_.onChange([this]() {
    //     interpolationChanged_ = true;
    //     invalidate(InvalidationLevel::InvalidOutput);
    //     std::cout << "Changed interpolation" << std::endl;
    // });
    samplerCreator_.onChange([this]() {
        samplerChanged_ = true;
        invalidate(InvalidationLevel::InvalidOutput);
        std::cout << "Changed sampler" << std::endl;
    });
}

void AddDataSetSampler::process() {
    // static bool firstTime = true;
    // // if (!firstTime) return;
    // firstTime = false;

    LogWarn("= Process!");
    std::cout << "X== Proccess! ==" << std::endl;
    auto removeChangedFlags = [this]() {
        interpolantChanged_ = false;
        interpolationChanged_ = false;
        samplerChanged_ = false;
    };

    if (!dataIn_.hasData() || !positionChannel_.hasSelectableChannels() ||
        !positionChannel_.getCurrentChannel()) {
        std::cout << "ASDf" << std::endl;
        dataOut_.setData(std::shared_ptr<DataSet>(nullptr));
        meshOut_.setData(nullptr);
        removeChangedFlags();
        return;
    }
    std::cout << "# Okay, channel options:" << std::endl;
    for (auto opt : positionChannel_.channelName_.getOptions())
        std::cout << " #   " << opt.value_ << std::endl;

    ind baseDim = static_cast<ind>(dataIn_.getData()->getGrid()->getDimension());
    if (interpolantChanged_) {
        std::cout << "GHJK" << std::endl;
        delete interpolant_;

        interpolant_ =
            interpolantCreatorList_[interpolantCreator_.getSelectedIdentifier()].second(baseDim);
        // fillInterpolationTypes();
        if (!samplerChanged_ && sampler_) sampler_->setInterpolant(*interpolant_);
    }

    if (samplerChanged_ || dataIn_.isChanged()) {
        std::cout << "X Getting new sampler of type " << samplerCreator_.getSelectedIdentifier()
                  << std::endl;
        removeChangedFlags();
        LogWarn("Getting new sampler of type " << samplerCreator_.getSelectedIdentifier());
        sampler_ = samplerCreatorList_[samplerCreator_.getSelectedIdentifier()].second(
            baseDim, dataIn_.getData()->getGrid(), positionChannel_.getCurrentChannel(),
            interpolant_);
        LogWarn("Sampler address: " << sampler_);
        std::cout << "X Sampler address: " << sampler_ << std::endl;
    }

    removeChangedFlags();
    // sampler_->interpolationType_ = interpolationType_.get();

    if (!sampler_) {
        LogWarn("Sampler does not exist :(");
        return;
    }

    std::cout << "New dataset" << std::endl;
    auto datasetWithSampler = std::make_shared<DataSet>(*dataIn_.getData());
    std::cout << "Made dataset" << std::endl;

    datasetWithSampler->addChannel(sampler_->coordinates_);
    datasetWithSampler->addSampler(sampler_);
    dataOut_.setData(datasetWithSampler);

    meshOut_.setData(sampler_->getDebugMesh());
    std::cout << "Ended process AddDataSetSampler" << std::endl;
}

// void AddDataSetSampler::fillInterpolationTypes() {
//     if (!interpolant_) {
//         interpolationType_.clearOptions();
//         return;
//     }
//     if (interpolationType_.size() < 3)
//         interpolationType_.replaceOptions(
//             {{"nearest", "Nearest Neighbor", InterpolationType::Nearest},
//              {"squared", "Squared Distance", InterpolationType::SquaredDistance},
//              {"linear", "Linear", InterpolationType::Linear}});

//     // Remove interpolation options that are not supported by the selected interpolant.
//     for (auto& opt : interpolationType_.getOptions()) {
//         if (!interpolant_->supportsInterpolationType(opt.value_))
//             interpolationType_.removeOption(opt.id_);
//     }
// }

void AddDataSetSampler::addSamplerType(std::string identifier, std::string displayName,
                                       CreateSampler creator) {
    samplerCreatorList_.insert({identifier, {displayName, creator}});
}

void AddDataSetSampler::addInterpolantType(std::string identifier, std::string displayName,
                                           CreateInterpolant creator) {
    interpolantCreatorList_.insert({identifier, {displayName, creator}});
}

}  // namespace discretedata
}  // namespace inviwo
