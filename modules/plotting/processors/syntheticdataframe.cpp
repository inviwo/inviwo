/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2017 Inviwo Foundation
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

#include <modules/plotting/processors/syntheticdataframe.h>

#include <random>

namespace inviwo {

namespace plot {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo SyntheticDataFrame::processorInfo_{
    "org.inviwo.SyntheticDataFrame",  // Class identifier
    "Synthetic DataFrame",            // Display name
    "Data Creation",                  // Category
    CodeState::Stable,                // Code state
    "CPU, Plotting, DataFrame"        // Tags
};
const ProcessorInfo SyntheticDataFrame::getProcessorInfo() const { return processorInfo_; }

SyntheticDataFrame::SyntheticDataFrame()
    : Processor(), dataFrame_("dataFrame_"), numRow_("numRow", "Number of Rows", 1000, 3, 100000)
    , randomParams_("randomParams", "Random Generator Settings")
    , useSameSeed_("useSameSeed", "Use same seed", true)
    , seed_("seed", "Seed", 1, 0, 1000)
{

    addPort(dataFrame_);
    addProperty(numRow_);

    addProperty(randomParams_);
    randomParams_.addProperty(useSameSeed_);
    randomParams_.addProperty(seed_);
    useSameSeed_.onChange([&]() { seed_.setVisible(useSameSeed_.get()); });
}

void SyntheticDataFrame::process() {
    auto dataframe = std::make_shared<DataFrame>();

    using rdist = std::uniform_real_distribution<float>;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(-1, 1);
    std::vector<rdist> rdists = {rdist{0.0f, 0.0f},  rdist{-0.1f, 0.1f}, rdist{-0.2f, 0.2f},
                                 rdist{-0.3f, 0.3f}, rdist{-0.4f, 0.4f}, rdist{-0.5f, 0.5f},
                                 rdist{-0.6f, 0.6f}, rdist{-0.7f, 0.7f}, rdist{-0.8f, 0.8f},
                                 rdist{-0.9f, 0.9f}, rdist{-1.0f, 1.0f}

    };

    if (useSameSeed_.get()) {
        gen.seed(seed_.get());
    }

    std::vector<std::vector<float> *> cols;
    for (size_t j = 0; j < rdists.size(); j++) {
        std::ostringstream oss;
        oss << "Column " << (j + 1);
        auto col = dataframe->addColumn<float>(oss.str());
        cols.push_back(&col->getTypedBuffer()->getEditableRAMRepresentation()->getDataContainer());
    }

    for (size_t i = 0; i < numRow_.get(); i++) {
        float p = dis(gen);
        for (size_t j = 0; j < cols.size(); j++) {
            float v = j % 2 == 0 ? p : -p;
            cols[j]->push_back(v + rdists[j](gen));
        }
    }

    dataframe->updateIndexBuffer();

    dataFrame_.setData(dataframe);
}

}  // namespace plot

}  // namespace inviwo
