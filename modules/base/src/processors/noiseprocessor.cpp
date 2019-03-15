/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015-2019 Inviwo Foundation
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

#include <modules/base/processors/noiseprocessor.h>
#include <inviwo/core/datastructures/image/imageram.h>
#include <inviwo/core/datastructures/image/image.h>
#include <inviwo/core/util/imagesampler.h>
#include <inviwo/core/util/zip.h>
#include <modules/base/algorithm/randomutils.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo NoiseProcessor::processorInfo_{
    "org.inviwo.NoiseProcessor",  // Class identifier
    "Noise Generator 2D",         // Display name
    "Data Creation",              // Category
    CodeState::Experimental,      // Code state
    Tags::CPU,                    // Tags
};
const ProcessorInfo NoiseProcessor::getProcessorInfo() const { return processorInfo_; }

NoiseProcessor::NoiseProcessor()
    : Processor()
    , noise_("noise", DataFloat32::get(), false)
    , size_("size", "Size", ivec2(256), ivec2(32), ivec2(4096))
    , type_("type", "Type",
            {{"random", "Random", NoiseType::Random},
             {"perlin", "Perlin", NoiseType::Perlin},
             {"poissonDisk", "Poisson Disk", NoiseType::PoissonDisk},
             {"haltonSequence", "Halton Sequence", NoiseType::HaltonSequence}})
    , range_("range_", "Range", 0.0f, 1.0f, 0.0f, 1.0f)
    , levels_("levels", "Levels", 2, 8, 1, 16)
    , persistence_("persistence", "Persistence", 0.5f, 0.001f, 1.0f, 0.001f)
    , poissonDotsAlongX_("poissonDotsAlongX", "Dots Along X", 100, 1, 1024)
    , poissonMaxPoints_("poissonMaxPoints", "Max Points", 1, 10000000, 10000000)

    , haltonNumPoints_("numPoints", "Number of points", 100, 1, 1000)
    , haltonXBase_("haltonXBase", "Base for x values", 2, 2, 32)
    , haltonYBase_("haltonYBase", "Base for y values", 3, 2, 32)

    , randomness_("randomness", "Randomness")
    , useSameSeed_("useSameSeed", "Use same seed", true)
    , seed_("seed", "Seed", 1, 0, 1000)
    , rd_()
    , mt_(rd_()) {

    addPort(noise_);
    addProperty(size_);

    addProperty(type_);
    addProperty(range_);
    addProperty(levels_);
    addProperty(persistence_);
    addProperty(poissonDotsAlongX_);
    addProperty(poissonMaxPoints_);
    addProperty(haltonNumPoints_);
    addProperty(haltonXBase_);
    addProperty(haltonYBase_);

    auto typeOnChange = [&]() {
        range_.setVisible(type_.getSelectedValue() == NoiseType::Random);
        levels_.setVisible(type_.getSelectedValue() == NoiseType::Perlin);
        persistence_.setVisible(type_.getSelectedValue() == NoiseType::Perlin);
        poissonDotsAlongX_.setVisible(type_.getSelectedValue() == NoiseType::PoissonDisk);
        poissonMaxPoints_.setVisible(type_.getSelectedValue() == NoiseType::PoissonDisk);

        haltonNumPoints_.setVisible(type_.getSelectedValue() == NoiseType::HaltonSequence);
        haltonXBase_.setVisible(type_.getSelectedValue() == NoiseType::HaltonSequence);
        haltonYBase_.setVisible(type_.getSelectedValue() == NoiseType::HaltonSequence);
    };

    type_.onChange(typeOnChange);

    addProperty(randomness_);
    randomness_.addProperty(useSameSeed_);
    randomness_.addProperty(seed_);
    useSameSeed_.onChange([&]() { seed_.setVisible(useSameSeed_.get()); });

    size_.onChange([&]() {
        auto s = std::max(size_.get().x, size_.get().y);
        s = util::nextPow2(s);
        auto l2 = log(s) / log(2.0f);
        levels_.setRangeMax(static_cast<int>(std::round(l2)));
    });

    typeOnChange();
}

NoiseProcessor::~NoiseProcessor() {}

void NoiseProcessor::process() {
    if (useSameSeed_.get()) {
        mt_.seed(seed_.get());
    }

    std::uniform_real_distribution<float> r(range_.get().x, range_.get().y);
    std::shared_ptr<Image> img;

    switch (type_.get()) {
        case NoiseType::Random:
            img = util::randomImage<float>(size_.get(), mt_, r);
            break;
        case NoiseType::Perlin:
            img = util::perlinNoise(size_.get(), persistence_.get(), levels_.get().x,
                                    levels_.get().y, mt_);
            break;
        case NoiseType::PoissonDisk:
            img = util::poissonDisk(size_.get(), poissonDotsAlongX_.get(), poissonMaxPoints_.get(),
                                    mt_);
            break;
        case NoiseType::HaltonSequence:
            img = util::haltonSequence<float>(size_.get(), haltonNumPoints_.get(),
                                              haltonXBase_.get(), haltonYBase_.get());
            break;
    }

    img->getColorLayer()->setSwizzleMask(swizzlemasks::luminance);
    noise_.setData(img);
}

}  // namespace inviwo
