/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2019 Inviwo Foundation
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

#include <inviwo/dataframe/processors/volumesequencetodataframe.h>

#include <inviwo/core/util/volumeramutils.h>
#include <inviwo/core/datastructures/buffer/buffer.h>
#include <inviwo/core/datastructures/buffer/bufferram.h>
#include <inviwo/core/datastructures/volume/volumeram.h>
#include <inviwo/core/util/imageramutils.h>
#include <inviwo/core/util/indexmapper.h>

#include <algorithm>
#include <random>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo VolumeSequenceToDataFrame::processorInfo_{
    "org.inviwo.VolumeSequenceToDataFrame",  // Class identifier
    "Volume Sequence To DataFrame",          // Display name
    "Data Creation",                         // Category
    CodeState::Experimental,                 // Code state
    "CPU, DataFrame, Volume",                // Tags
};

const ProcessorInfo VolumeSequenceToDataFrame::getProcessorInfo() const { return processorInfo_; }

VolumeSequenceToDataFrame::VolumeSequenceToDataFrame()
    : Processor()
    , inport_("volume")
    , outport_("dataframe")
    , reduce_("reduce", "Reduce", true)
    , probability_("probability", "Probability", 0.01f, 0.0f, 1.0f, 0.01f)
    , omitOutliers_("omitOutliers", "Omit outliers", true)
    , threshold_("threshold", "Threshold", 1000000.0f, 0.0f, std::numeric_limits<float>::max(),
                 0.01f) {
    addPort(inport_);
    addPort(outport_);

    addProperty(reduce_);
    addProperty(probability_);

    addProperty(omitOutliers_);
    addProperty(threshold_);

    reduce_.onChange([this]() { probability_.setVisible(reduce_.get()); });

    probability_.onChange([this]() {
        if (reduce_.get()) {
            invalidate(InvalidationLevel::InvalidResources);
        }
    });

    omitOutliers_.onChange([this]() { threshold_.setVisible(omitOutliers_.get()); });

    threshold_.onChange([this]() {
        if (omitOutliers_.get()) {
            invalidate(InvalidationLevel::InvalidResources);
        }
    });
}

void VolumeSequenceToDataFrame::recomputeReduceBuffer() {
    if (!inport_.hasData()) return;

    filteredIDs_.clear();

    auto volumeSequence = inport_.getVectorData();
    auto first = (volumeSequence[0])->getRepresentation<VolumeRAM>();

    auto dims = first->getDimensions();
    size_t size = dims.x * dims.y * dims.z;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(0.0f, 1.0f);

    std::vector<const glm::f32 *> volumeData;

    const auto probability = probability_.get();
    const auto omitOutliers = omitOutliers_.get();
    const auto threshold = threshold_.get();

    if (omitOutliers) {
        for (auto vol : volumeSequence) {
            if (vol->getDataFormat()->getNumericType() != NumericType::Float) continue;
            if (vol->getDataFormat()->getComponents() != 1)
                LogWarn("This volume is omitted because it has more than one channel.");
            volumeData.push_back(
                static_cast<const glm::f32 *>(vol->getRepresentation<VolumeRAM>()->getData()));
        }
    }

    std::vector<vec2> minMaxes;
    minMaxes.resize(volumeData.size(),
                    vec2(std::numeric_limits<float>::max(), std::numeric_limits<float>::lowest()));

    for (size_t i = 0; i < size; i++) {
        for (size_t j = 0; j < volumeData.size(); j++) {
            if (volumeData[j][i] < threshold)
                minMaxes[j] = vec2(std::min(volumeData[j][i], minMaxes[j].x),
                                   std::max(volumeData[j][i], minMaxes[j].y));
        }
        if (dis(gen) <= probability) {
            if (omitOutliers) {
                bool add = true;
                for (size_t j = 0; j < volumeData.size(); j++) {
                    if (volumeData[j][i] > threshold) {
                        add = false;
                        break;
                    }
                }
                if (add) {
                    filteredIDs_.insert(i);
                }
            } else {
                filteredIDs_.insert(i);
            }
        }
    }

    for (size_t j = 0; j < volumeData.size(); j++) {
        const auto &minMax = minMaxes[j];
        bool foundMin = false;
        bool foundMax = false;

        for (size_t i = 0; i < size; i++) {
            if (foundMin && foundMax) break;
            if (minMax.y == volumeData[j][i] && !foundMax) {
                filteredIDs_.insert(i);
                foundMax = true;
            }
            if (minMax.x == volumeData[j][i] && !foundMin) {
                filteredIDs_.insert(i);
                foundMin = true;
            }
        }
    }
}

void VolumeSequenceToDataFrame::initializeResources() { recomputeReduceBuffer(); }

void VolumeSequenceToDataFrame::process() {
    if (!inport_.hasData()) return;
    auto volumeSequence = inport_.getVectorData();
    auto outports = inport_.getConnectedOutports();
    auto first = (volumeSequence[0])->getRepresentation<VolumeRAM>();

    auto dims = first->getDimensions();
    size_t size = dims.x * dims.y * dims.z;

    auto dataFrame = std::make_shared<DataFrame>(static_cast<std::uint32_t>(
        reduce_.get() || omitOutliers_.get() ? filteredIDs_.size() : size));

    auto indexMapper = util::IndexMapper3D(dims);

    size_t volumeNumber = 1;
    for (const auto volume : volumeSequence) {
        const auto numericType = volume->getDataFormat()->getNumericType();
        if (numericType != NumericType::Float) continue;
        std::vector<std::vector<float> *> channelBuffer_;
        auto volumeRAM = volume->getRepresentation<VolumeRAM>();
        const auto numCh = volume->getDataFormat()->getComponents();
        for (size_t c = 0; c < numCh; c++) {
            auto identifier = outports[volumeNumber - 1]->getProcessor()->getIdentifier();
            auto col = dataFrame->addColumn<float>(
                identifier, reduce_.get() || omitOutliers_.get() ? filteredIDs_.size() : size);
            channelBuffer_.push_back(
                &col->getTypedBuffer()->getEditableRAMRepresentation()->getDataContainer());
        }
        volumeNumber++;

        util::forEachVoxelParallel(*volumeRAM, [&, this](const size3_t &pos) {
            const auto idx = indexMapper(pos);
            if (filteredIDs_.find(idx) == filteredIDs_.end()) return;
            const auto v = volumeRAM->getAsDVec4(pos);
            for (size_t c = 0; c < numCh; c++) {
                channelBuffer_[c]->at(idx) = float(v[c]);
            }
        });
    }
    outport_.setData(dataFrame);
}

}  // namespace inviwo
