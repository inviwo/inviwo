/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2024 Inviwo Foundation
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

#include <inviwo/pathtracing/processors/uniformgridopacity.h>
#include <inviwo/core/datastructures/volume/volumeram.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo UniformGridOpacity::processorInfo_{
    "org.inviwo.uniformgridopacity",  // Class identifier
    "Uniform Grid Opacity MinMax",    // Display name
    "Volume",                         // Category
    CodeState::Experimental,          // Code state
    Tags::CPU,                        // Tags
};

const ProcessorInfo UniformGridOpacity::getProcessorInfo() const { return processorInfo_; }

UniformGridOpacity::UniformGridOpacity()
    : Processor{}
    , inport_{"data"}
    //, outport_{"MinMaxOpacity"}
    , outportVolume_{"MinMaxOpacityVolume"}
    , transferFunction_("transferFunction", "Transfer function")
    , volumeRegionSize_("region", "Region size", 8, 1, 100) {

    addPort(inport_);
    //addPort(outport_);
    addPort(outportVolume_);

    addProperty(transferFunction_);
    addProperty(volumeRegionSize_);
}

void UniformGridOpacity::process() {
    auto curVolume = inport_.getData();

    const VolumeRAM* curRAMVolume = curVolume->getRepresentation<VolumeRAM>();
    auto dim = curVolume->getDimensions();
    auto region = size3_t(volumeRegionSize_.get());
    const size3_t outDim{glm::ceil(vec3(dim) / static_cast<float>(volumeRegionSize_.get()))};

    auto statsRep = std::make_shared<VolumeRAMPrecision<vec4>>(
        outDim, swizzlemasks::rgba, InterpolationType::Nearest, curVolume->getWrapping());
    auto stats = std::make_shared<Volume>(statsRep);

    //stats->dataMap_.dataRange = curVolume->dataMap_.dataRange; // should map from 0,1
    stats->dataMap_.dataRange.x = 0; // should map from 0,1
    stats->dataMap_.dataRange.y = 1; // should map from 0,1
    stats->dataMap_.valueRange = curVolume->dataMap_.valueRange; // makes sense to match operated volume
    //stats->dataMap_.valueRange = std::make_shared<inviwo::dvec2>({0, 1});

    vec4* data = statsRep->getDataTyped();

    curRAMVolume->dispatch<void, dispatching::filter::Scalars>([&] <typename VR>(VR* vr) {
        using T = typename VR::type;
        using P = typename util::same_extent<T, double>::type;

        const VolumeRAMPrecision<T>* volume = dynamic_cast<const VolumeRAMPrecision<T>*>(vr);

        if (!volume) return;

        // determine parameters
        const size3_t dataDims{volume->getDimensions()};

        const auto& dataMapper = vr->getOwner()->dataMap_;
        const T* src = static_cast<const T*>(volume->getData());
        
        

        for (size_t z = 0; z < outDim.z; ++z) {
            for (size_t y = 0; y < outDim.y; ++y) {
                for (size_t x = 0; x < outDim.x; ++x) {

                    auto offset = size3_t(x, y, z) * region;
                    auto startCoord = offset;
                    auto endCoord = glm::min(startCoord + region, dataDims);

                    double minOpacity(1);
                    double maxOpacity(0);
                    double avgOpacity(0);

                    for (auto z = startCoord.z; z < endCoord.z; ++z) {
                        for (auto y = startCoord.y; y < endCoord.y; ++y) {
                            for (auto x = startCoord.x; x < endCoord.x; ++x) {
                                size_t volumePos = x + (y * dataDims.x) + (z * dataDims.x * dataDims.y);
                                T valueA = src[volumePos];
                                auto v = static_cast<double>(dataMapper.mapFromDataToValue(valueA));
                                auto vn = dataMapper.mapFromValueToNormalized(v);
                                auto opacity = P{transferFunction_.get().sample(vn).w};
                                minOpacity = glm::min(minOpacity, opacity);
                                maxOpacity = glm::max(maxOpacity, opacity);
                                avgOpacity += opacity;
                                 
                            }
                        }
                    }
                    avgOpacity /= static_cast<double>(region.x * region.y * region.z);

                    vec4 minMaxVal(static_cast<float>(minOpacity),
                                              static_cast<float>(maxOpacity),
                                              static_cast<float>(avgOpacity), 0.f);

                    data[VolumeRAM::posToIndex(size3_t(x, y, z), outDim)] = minMaxVal;
                }
            }
        }
    });

    outportVolume_.setData(stats);

};

}  // namespace inviwo