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

#include <modules/discretedata/processors/exampledataset.h>
#include <modules/discretedata/connectivity/structuredgrid.h>
#include <modules/discretedata/channels/analyticchannel.h>
#include <modules/discretedata/dataset.h>

namespace inviwo {
namespace discretedata {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo ExampleDataset::processorInfo_{
    "org.inviwo.ExampleDataset",  // Class identifier
    "Example Dataset",            // Display name
    "Data Set",                   // Category
    CodeState::Stable,            // Code state
    Tags::None,                   // Tags
};

const ProcessorInfo ExampleDataset::getProcessorInfo() const { return processorInfo_; }

ExampleDataset::ExampleDataset() : Processor(), dataOutport("ExampleData") { addPort(dataOutport); }

void ExampleDataset::process() {

    const std::array<ind, 3> dataSize = {9, 6, 11};
    auto dataset = std::make_shared<DataSet>(dataSize);
    auto wave = std::make_shared<AnalyticChannel<float, 3, glm::vec3>>(
        [dataSize](auto& vec, ind idx) {
            glm::vec3 pos = {
                (float)(idx % dataSize[0]) / dataSize[0],
                (float)((idx % (dataSize[0] * dataSize[1])) / dataSize[0]) / dataSize[1],
                (float)(idx / (dataSize[0] * dataSize[1])) / dataSize[2]};

            vec.x = pos.z * 10;  // Check
            vec.y = std::cos(pos.z * 6);
            vec.z = (pos.x - 0.5f) * (pos.z * 3 + 1);  // Check

            float slope = std::sin(pos.z * 6);
            float rad = pos.y - 0.5f;
            vec.x += slope * rad;
            vec.y += rad;
        },
        dataset->getGrid()->getNumElements(), "Wave Pos");

    auto donut = std::make_shared<AnalyticChannel<float, 3, glm::vec3>>(
        [dataSize](auto& vec, ind idx) {
            glm::vec3 pos = {
                (float)(idx % dataSize[0]) / (dataSize[0] - 1),
                (float)((idx % (dataSize[0] * dataSize[1])) / dataSize[0]) / dataSize[1],
                (float)(idx / (dataSize[0] * dataSize[1])) / (dataSize[2] - 1)};

            float angle = pos.x * 2 * M_PI;
            float angleMax = pos.z * 2 * M_PI;
            float rad = pos.y * 2;  //(1.0f + pos.z * 2);
            float bigRad = 8;

            vec.x = std::cos(angleMax) * bigRad;
            vec.y = std::sin(angleMax) * bigRad;
            vec.z = std::sin(angle) * rad;

            float norm = std::sqrt(vec.x * vec.x + vec.y * vec.y);

            vec.x += std::cos(angle) * rad * vec.x / norm;
            vec.y += std::cos(angle) * rad * vec.y / norm;
        },
        dataset->getGrid()->getNumElements(), "Torus Pos");

    auto color = std::make_shared<AnalyticChannel<float, 3, glm::vec3>>(
        [dataSize](auto& vec, ind idx) {
            glm::vec3 pos = {
                (float)(idx % dataSize[0]) / (dataSize[0] - 1),
                (float)((idx % (dataSize[0] * dataSize[1])) / dataSize[0]) / dataSize[1],
                (float)(idx / (dataSize[0] * dataSize[1])) / (dataSize[2] - 1)};

            vec.x = std::sin(pos.x * 2 * M_PI) * 0.5f + 0.5f;
            vec.y = pos.y;
            vec.z = std::cos(pos.z * 2 * M_PI) * 0.5f + 0.5f;
        },
        dataset->getGrid()->getNumElements(), "Colors");

    dataset->addChannel(wave);
    dataset->addChannel(donut);
    dataset->addChannel(color);
    dataOutport.setData(dataset);
}

}  // namespace discretedata
}  // namespace inviwo
