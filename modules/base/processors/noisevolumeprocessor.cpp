/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017 Inviwo Foundation
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
#include <modules/base/algorithm/random.h>


namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo NoiseVolumeProcessor::processorInfo_{
    "org.inviwo.NoiseVolumeProcessor",  // Class identifier
    "Noise Volume Processor",           // Display name
    "Data Creation",                    // Category
    CodeState::Experimental,            // Code state
    Tags::None,                         // Tags
};


const ProcessorInfo NoiseVolumeProcessor::getProcessorInfo() const {
    return processorInfo_;
}

NoiseVolumeProcessor::NoiseVolumeProcessor()
    : Processor()
    , basisVolume_("forBasis")
    , volume_("volume_")
    , size_("size", "Size", size3_t(256), size3_t(32), size3_t(4096))
    , type_("type", "Type",
    { { "random", "Random", NoiseType::Random },
  /*  { "perlin", "Perlin", NoiseType::Perlin },
    { "poissonDisk", "Poisson Disk", NoiseType::PoissonDisk },*/
    { "haltonSequence", "Halton Sequence", NoiseType::HaltonSequence } })
    , range_("range_", "Range", 0.0f, 1.0f, 0.0f, 1.0f)
    /*, levels_("levels", "Levels", 2, 8, 1, 16)
    , persistence_("persistence", "Persistence", 0.5f, 0.001f, 1.0f, 0.001f)
    , poissonDotsAlongX_("poissonDotsAlongX", "Dots Along X", 100, 1, 1024)
    , poissonMaxPoints_("poissonMaxPoints", "Max Points", 1, 10000000, 10000000)*/

    , haltonNumPoints_("numPoints", "Number of points", 100, 1, 1000)
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
  /*  addProperty(levels_);
    addProperty(persistence_);
    addProperty(poissonDotsAlongX_);
    addProperty(poissonMaxPoints_);*/
    addProperty(haltonNumPoints_);
    addProperty(haltonXBase_);
    addProperty(haltonYBase_);
    addProperty(haltonZBase_);

    auto typeOnChange = [&]() {
        range_.setVisible(type_.getSelectedValue() == NoiseType::Random);
        //levels_.setVisible(type_.getSelectedValue() == NoiseType::Perlin);
        //persistence_.setVisible(type_.getSelectedValue() == NoiseType::Perlin);
        //poissonDotsAlongX_.setVisible(type_.getSelectedValue() == NoiseType::PoissonDisk);
        //poissonMaxPoints_.setVisible(type_.getSelectedValue() == NoiseType::PoissonDisk);

        haltonNumPoints_.setVisible(type_.getSelectedValue() == NoiseType::HaltonSequence);
        haltonXBase_.setVisible(type_.getSelectedValue() == NoiseType::HaltonSequence);
        haltonYBase_.setVisible(type_.getSelectedValue() == NoiseType::HaltonSequence);
    };

    type_.onChange(typeOnChange);

    addProperty(randomness_);
    randomness_.addProperty(useSameSeed_);
    randomness_.addProperty(seed_);
    useSameSeed_.onChange([&]() { seed_.setVisible(useSameSeed_.get()); });

    //size_.onChange([&]() {
    //    auto s = std::max(size_.get().x, size_.get().y);
    //    s = nextPow2(s);
    //    auto l2 = log(s) / log(2.0f);
    //    levels_.setRangeMax(static_cast<int>(std::round(l2)));
    //});

    typeOnChange();
}
    
void NoiseVolumeProcessor::process() {
    if (useSameSeed_.get()) {
        mt_.seed(seed_.get());
    }


    auto vol = std::make_shared<Volume>(size_.get(), DataFloat32::get());
    vol->dataMap_.dataRange = dvec2(0, 1);
    vol->dataMap_.valueRange = dvec2(0, 1);
    auto ram = static_cast<VolumeRAMPrecision<float>*>(vol->getEditableRepresentation<VolumeRAM>());
    switch (type_.get()) {
    case NoiseType::Random:
        randomNoise(*ram, range_.get().x, range_.get().y);
        break;
    //case NoiseType::Perlin:
    //    perlinNoise(ram);
    //    break;
    //case NoiseType::PoissonDisk:
    //    poissonDisk(ram);
    //    break;
    case NoiseType::HaltonSequence:
        haltonSequence(*ram);
        break;
    }

    if (basisVolume_.hasData()) {
        vol->setModelMatrix(basisVolume_.getData()->getModelMatrix());
        vol->setWorldMatrix(basisVolume_.getData()->getWorldMatrix());
    }

    volume_.setData(vol);

    
}


void NoiseVolumeProcessor::randomNoise(VolumeRAMPrecision<float> &vol, float minv, float maxv) {
    std::uniform_real_distribution<float> r(minv, maxv);
    size_t voxels = vol.getDimensions().x * vol.getDimensions().y* vol.getDimensions().z;
    auto data = vol.getDataTyped();
    std::generate(data, data + voxels, [&]() {return r(mt_); });
}

void NoiseVolumeProcessor::haltonSequence(VolumeRAMPrecision<float> &vol) {
    auto x = util::haltonSequence<float>(haltonXBase_.get(), haltonNumPoints_.get());
    auto y = util::haltonSequence<float>(haltonYBase_.get(), haltonNumPoints_.get());
    auto z = util::haltonSequence<float>(haltonZBase_.get(), haltonNumPoints_.get());

    auto dims = vol.getDimensions();
    auto dimsf = vec3(dims - size3_t(1));

    util::IndexMapper3D index(dims);
    auto data = vol.getDataTyped();

    for (auto &&pair : util::zip(x, y,z)) {
        auto coord = dimsf * vec3(get<0>(pair), get<1>(pair), get<2>(pair));
        data[index(coord)] = 1;
    }
}

} // namespace

