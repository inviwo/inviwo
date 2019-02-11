/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2019 Inviwo Foundation
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

#include <modules/base/processors/noisevolumeprocessor.h>
#include <inviwo/core/datastructures/volume/volume.h>
#include <inviwo/core/datastructures/volume/volumeram.h>
#include <inviwo/core/datastructures/volume/volumeramprecision.h>
#include <inviwo/core/util/zip.h>
#include <modules/base/algorithm/randomutils.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo NoiseVolumeProcessor::processorInfo_{
    "org.inviwo.NoiseVolumeProcessor",  // Class identifier
    "Noise Generator 3D",               // Display name
    "Data Creation",                    // Category
    CodeState::Experimental,            // Code state
    Tags::None,                         // Tags
};

const ProcessorInfo NoiseVolumeProcessor::getProcessorInfo() const { return processorInfo_; }

NoiseVolumeProcessor::NoiseVolumeProcessor()
    : Processor()
    , basisVolume_("forBasis")
    , volume_("volume_")
    , size_("size", "Size", size3_t(100), size3_t(32), size3_t(1024))
    , type_("type", "Type",
            {{"random", "Random", NoiseType::Random},
             {"haltonSequence", "Halton Sequence", NoiseType::HaltonSequence}})
    , range_("range_", "Range", 0.0f, 1.0f, 0.0f, 1.0f)

    , haltonNumPoints_("numPoints", "Number of points", 20, 1, 1000)
    , haltonXBase_("haltonXBase", "Base for x values", 2, 2, 32)
    , haltonYBase_("haltonYBase", "Base for y values", 3, 2, 32)
    , haltonZBase_("haltonZBase", "Base for z values", 5, 2, 32)

    , randomness_("randomness", "Randomness")
    , useSameSeed_("useSameSeed", "Use same seed", true)
    , seed_("seed", "Seed", 1, 0, 1000)
    , rd_()
    , mt_(rd_()) {

    addPort(basisVolume_);
    basisVolume_.setOptional(true);
    addPort(volume_);
    addProperty(size_);

    addProperty(type_);
    addProperty(range_);
    addProperty(haltonNumPoints_);
    addProperty(haltonXBase_);
    addProperty(haltonYBase_);
    addProperty(haltonZBase_);

    auto typeOnChange = [&]() {
        range_.setVisible(type_.getSelectedValue() == NoiseType::Random);
        haltonNumPoints_.setVisible(type_.getSelectedValue() == NoiseType::HaltonSequence);
        haltonXBase_.setVisible(type_.getSelectedValue() == NoiseType::HaltonSequence);
        haltonYBase_.setVisible(type_.getSelectedValue() == NoiseType::HaltonSequence);
    };

    type_.onChange(typeOnChange);

    addProperty(randomness_);
    randomness_.addProperty(useSameSeed_);
    randomness_.addProperty(seed_);
    useSameSeed_.onChange([&]() { seed_.setVisible(useSameSeed_.get()); });

    typeOnChange();
}

void NoiseVolumeProcessor::process() {
    if (useSameSeed_.get()) {
        mt_.seed(seed_.get());
    }

    std::uniform_real_distribution<float> r(range_.get().x, range_.get().y);

    std::shared_ptr<Volume> vol;

    switch (type_.get()) {
        case NoiseType::Random:
            vol = util::randomVolume<float>(size_.get(), mt_, r);
            break;
        case NoiseType::HaltonSequence:
            vol =
                util::haltonSequence<float>(size_.get(), haltonNumPoints_.get(), haltonXBase_.get(),
                                            haltonYBase_.get(), haltonZBase_.get());
            break;
    }

    if (basisVolume_.hasData()) {
        vol->setModelMatrix(basisVolume_.getData()->getModelMatrix());
        vol->setWorldMatrix(basisVolume_.getData()->getWorldMatrix());
    }

    volume_.setData(vol);
}

}  // namespace inviwo
