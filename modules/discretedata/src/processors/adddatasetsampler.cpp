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

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo AddDataSetSampler::processorInfo_{
    "org.inviwo.AddDataSetSampler",  // Class identifier
    "Add DataSet Sampler",           // Display name
    "Undefined",                     // Category
    CodeState::Experimental,         // Code state
    Tags::None,                      // Tags
};
const ProcessorInfo AddDataSetSampler::getProcessorInfo() const { return processorInfo_; }

// namespace dd_detail {
// // struct CelltreeDispatcher {
// //     template <typename Result, ind N>
// //     Result operator()(std::shared_ptr<const Connectivity> grid,
// //                       std::shared_ptr<const Channel> coordinates,
// //                       const InterpolantBase* interpolant) {
// //         auto* usableInterpolant = dynamic_cast<const Interpolant<N>*>(interpolant);
// //         auto usableCoords = std::dynamic_pointer_cast<const DataChannel<double,
// N>>(coordinates);
// //         if (!usableInterpolant || !usableCoords) return nullptr;
// //         // return std::make_shared<CellTree<N>>(grid, usableCoords, *usableInterpolant);
// //         return nullptr;
// //     }
// // };

// // struct SkewedBoxDispatcher {
// //     template <typename Result, ind N>
// //     Result operator()() {
// //         return new SkewedBoxInterpolant<N>();
// //     }
// // };
// }  // namespace dd_detail

AddDataSetSampler::AddDataSetSampler()
    : Processor()
    , dataIn_("addDataSetSampler")
    , positionChannel_(
          dataIn_, "positionChannel", "Position Channel",
          [&](const std::shared_ptr<const Channel> channel) {
              return dataIn_.hasData() &&
                     channel->getGridPrimitiveType() == GridPrimitive::Vertex &&
                     channel->getNumComponents() ==
                         static_cast<ind>(dataIn_.getData()->getGrid()->getDimension());
          })
    , samplerCreator_("sampler", "Sampler Type")
    , interpolantCreator_("interpolation", "Cell Interpolant")
    , interpolationType_("interpolationType", "Interpolation Type",
                         {{"nearest", "Nearest Neighbor", InterpolationType::Nearest},
                          {"squared", "Squared Distance", InterpolationType::SquaredDistance},
                          {"linear", "Linear", InterpolationType::Linear}},
                         2) {

    addPort(dataIn_);
    addProperties(positionChannel_, samplerCreator_, interpolantCreator_, interpolationType_);
    interpolationType_.setCurrentStateAsDefault();

    // samplerCreator_.addOption(
    //     "celltree", "Cell Tree",
    //     [baseDim](std::shared_ptr<const Connectivity> grid,
    //               std::shared_ptr<const Channel> coordinates, const InterpolantBase* interpolant)
    //               {
    //         dd_detail::CelltreeDispatcher dispatcher;
    //         return channeldispatching::dispatchNumber<std::shared_ptr<DataSetSamplerBase>, 1,
    //                                                   DISCRETEDATA_MAX_NUM_DIMENSIONS>(
    //             baseDim, dispatcher, grid, coordinates, interpolant);
    //     });

    // interpolantCreator_.addOption("skewedCube", "Skewed Cube", [baseDim]() {
    //     dd_detail::SkewedBoxDispatcher dispatcher;
    //     return channeldispatching::dispatchNumber<const InterpolantBase*, 1,
    //                                               DISCRETEDATA_MAX_NUM_DIMENSIONS>(baseDim,
    //                                                                                dispatcher);
    // });

    for (auto& creator : samplerCreatorList_) {
        samplerCreator_.addOption(creator.first, creator.second.first, samplerCreator_.size());
    }

    for (auto& creator : interpolantCreatorList_) {
        interpolantCreator_.addOption(creator.first, creator.second.first,
                                      interpolantCreator_.size());
    }

    interpolantCreator_.onChange([this]() { interpolantChanged_ = true; });
    interpolationType_.onChange([this]() { interpolationChanged_ = true; });
    samplerCreator_.onChange([this]() { samplerChanged_ = true; });
}

void AddDataSetSampler::process() {

    if (!dataIn_.hasData() || positionChannel_.size() < 1) return;

    ind baseDim = static_cast<ind>(dataIn_.getData()->getGrid()->getDimension());
    if (interpolantChanged_) {
        delete interpolant_;

        interpolant_ =
            interpolantCreatorList_[interpolantCreator_.getSelectedIdentifier()].second(baseDim);
        fillInterpolationTypes();
        if (!samplerChanged_ && sampler_) sampler_->setInterpolant(*interpolant_);
    }

    if (samplerChanged_) {
        sampler_ = samplerCreatorList_[samplerCreator_.getSelectedIdentifier()].second(
            baseDim, dataIn_.getData()->getGrid(), positionChannel_.getCurrentChannel(),
            interpolant_);
    }
    // outport_.setData(myImage);
}

void AddDataSetSampler::fillInterpolationTypes() {
    if (!interpolant_) {
        interpolationType_.clearOptions();
        return;
    }
    if (interpolationType_.size() < 3)
        interpolationType_.replaceOptions(
            {{"nearest", "Nearest Neighbor", InterpolationType::Nearest},
             {"squared", "Squared Distance", InterpolationType::SquaredDistance},
             {"linear", "Linear", InterpolationType::Linear}});

    // Remove interpolation options that are not supported by the selected interpolant.
    for (auto& opt : interpolationType_.getOptions()) {
        if (!interpolant_->supportsInterpolationType(opt.value_))
            interpolationType_.removeOption(opt.id_);
    }
}

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
