/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2022 Inviwo Foundation
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

#include <inviwo/core/algorithm/markdown.h>            // for operator""_unindentHelp
#include <inviwo/core/ports/volumeport.h>              // for VolumeInport, VolumeOutport
#include <inviwo/core/processors/processor.h>          // for Processor
#include <inviwo/core/processors/processorinfo.h>      // for ProcessorInfo
#include <inviwo/core/processors/processorstate.h>     // for CodeState, CodeState::Experimental
#include <inviwo/core/processors/processortags.h>      // for Tag, Tag::CPU
#include <inviwo/core/properties/boolproperty.h>       // for BoolProperty
#include <inviwo/core/properties/compositeproperty.h>  // for CompositeProperty
#include <inviwo/core/properties/minmaxproperty.h>     // for FloatMinMaxProperty
#include <inviwo/core/properties/optionproperty.h>     // for OptionPropertyOption, OptionProperty
#include <inviwo/core/properties/ordinalproperty.h>    // for IntSizeTProperty, IntProperty, Int...
#include <inviwo/core/util/glmvec.h>                   // for size3_t
#include <inviwo/core/util/staticstring.h>             // for operator+
#include <inviwo/core/util/zip.h>                      // for zipper
#include <modules/base/algorithm/randomutils.h>        // for haltonSequence, randomVolume

#include <memory>       // for shared_ptr, shared_ptr<>::element_...
#include <type_traits>  // for remove_extent_t

#include <glm/detail/qualifier.hpp>  // for tvec2
#include <glm/vec2.hpp>              // for vec<>::(anonymous)

namespace inviwo {
class Volume;

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo NoiseVolumeProcessor::processorInfo_{
    "org.inviwo.NoiseVolumeProcessor",  // Class identifier
    "Noise Generator 3D",               // Display name
    "Data Creation",                    // Category
    CodeState::Experimental,            // Code state
    Tag::CPU | Tag("Volume"),           // Tags
    R"(
    A processor to generate noise volumes.
    Using the Mersenne Twister 19937 generator to generate random numbers.
    
    ![Image Of Noise Types](file:~modulePath~/docs/images/noise_types.png)
    
    #### Available Methods
    * __Random__ Generates a uniform, random value in the range [min,max] for each voxel
    * __Halton Sequence__  Create a binary image of based on semi-random pairs (deterministic)
      constructed using two [Halton Sequence](https://en.wikipedia.org/wiki/Halton_sequence)
      of different bases (base 2 and base 3 gives good results)
      
    Example workspace:
    [base/noise_volume_generation.inv](file:~modulePath~/data/workspaces/noise_volume_generation.inv)
    )"_unindentHelp};

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
