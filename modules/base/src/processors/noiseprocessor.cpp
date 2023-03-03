/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015-2023 Inviwo Foundation
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

#include <inviwo/core/algorithm/markdown.h>               // for operator""_help, operator""_uni...
#include <inviwo/core/datastructures/image/imagetypes.h>  // for luminance
#include <inviwo/core/datastructures/image/layer.h>       // for Layer
#include <inviwo/core/ports/imageport.h>                  // for ImageOutport, HandleResizeEvents
#include <inviwo/core/processors/processor.h>             // for Processor
#include <inviwo/core/processors/processorinfo.h>         // for ProcessorInfo
#include <inviwo/core/processors/processorstate.h>        // for CodeState, CodeState::Experimental
#include <inviwo/core/processors/processortags.h>         // for Tag, Tag::CPU
#include <inviwo/core/properties/boolproperty.h>          // for BoolProperty
#include <inviwo/core/properties/compositeproperty.h>     // for CompositeProperty
#include <inviwo/core/properties/constraintbehavior.h>    // for ConstraintBehavior, ConstraintB...
#include <inviwo/core/properties/minmaxproperty.h>        // for IntMinMaxProperty, FloatMinMaxP...
#include <inviwo/core/properties/optionproperty.h>        // for OptionPropertyOption, OptionPro...
#include <inviwo/core/properties/ordinalproperty.h>       // for IntProperty, IntSizeTProperty
#include <inviwo/core/util/formats.h>                     // for DataFloat32, DataFormat
#include <inviwo/core/util/glmvec.h>                      // for ivec2
#include <inviwo/core/util/staticstring.h>                // for operator+
#include <inviwo/core/util/zip.h>                         // for zipper
#include <modules/base/algorithm/randomutils.h>           // for haltonSequence, nextPow2, perli...

#include <algorithm>    // for max
#include <cmath>        // for log, round
#include <memory>       // for shared_ptr
#include <type_traits>  // for remove_extent_t

#include <glm/detail/qualifier.hpp>   // for tvec2
#include <glm/vec2.hpp>               // for vec, vec<>::(anonymous)
#include <glm/vector_relational.hpp>  // for any, greaterThanEqual, lessThan

namespace inviwo {
class Image;

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo NoiseProcessor::processorInfo_{"org.inviwo.NoiseProcessor",  // Class identifier
                                                   "Noise Generator 2D",         // Display name
                                                   "Data Creation",              // Category
                                                   CodeState::Experimental,      // Code state
                                                   Tag::CPU | Tag("Image"),      // Tags
                                                   R"(
    A processor to generate noise images.
    Using the Mersenne Twister 19937 generator to generate random numbers.
        
    ![Image Of Noise Types](file:~modulePath~/docs/images/noise_types.png)
    
    ### Available Methods
    * __Random__ Generates a uniform, random value in the range [min,max] for each pixel
    * __Perlin Noise__ Generates a perlin noise image
    * __PoissonDisk__ Create a binary image of points uniformly distributed over the image.
      Read more at [devmag](http://devmag.org.za/2009/05/03/poisson-disk-sampling/)
    * __Halton Sequence__ Create a binary image of based on semi-random pairs (deterministic)
      constructed using two [Halton Sequence](https://en.wikipedia.org/wiki/Halton_sequence)
      of different bases (base 2 and base 3 gives good results)
      
    )"_unindentHelp};

const ProcessorInfo NoiseProcessor::getProcessorInfo() const { return processorInfo_; }

NoiseProcessor::NoiseProcessor()
    : Processor()
    , noise_("noise", "Generated noise image"_help, DataFloat32::get(), HandleResizeEvents::No)
    , size_("size", "Size", "Size of the output image."_help, ivec2(256),
            {ivec2(32), ConstraintBehavior::Editable}, {ivec2(4096), ConstraintBehavior::Editable})
    , type_("type", "Type", "Type of noise to generate."_help,
            {{"random", "Random", NoiseType::Random},
             {"perlin", "Perlin", NoiseType::Perlin},
             {"poissonDisk", "Poisson Disk", NoiseType::PoissonDisk},
             {"haltonSequence", "Halton Sequence", NoiseType::HaltonSequence}})
    , range_("range", "Range", "The min/max values of the output values (default: [0 1])."_help,
             0.0f, 1.0f, 0.0f, 1.0f)
    , levels_("levels", "Levels",
              "Numbers of levels used in the generation of the Perlin noise"_help, 2, 8, 1, 16)
    , persistence_("persistence", "Persistence", "Controls the sharpens in Perlin noise"_help, 0.5f,
                   {0.001f, ConstraintBehavior::Editable}, {1.0f, ConstraintBehavior::Editable},
                   0.001f)
    , poissonDotsAlongX_("poissonDotsAlongX", "Dots Along X",
                         "Average number of points along the x-axis."_help, 100,
                         {1, ConstraintBehavior::Immutable}, {1024, ConstraintBehavior::Ignore})
    , poissonMaxPoints_("poissonMaxPoints", "Max Points",
                        "Maximum number of output points (total)."_help, 10000000,
                        {1, ConstraintBehavior::Immutable}, {10000000, ConstraintBehavior::Ignore})

    , haltonNumPoints_("numPoints", "Number of points", 100, 1, 1000)
    , haltonXBase_("haltonXBase", "Base for x values", 2, 2, 32)
    , haltonYBase_("haltonYBase", "Base for y values", 3, 2, 32)

    , randomness_("randomness", "Randomness", "Random number generation settings"_help)
    , useSameSeed_("useSameSeed", "Use same seed",
                   "Use the same seed for each call to process."_help, true)
    , seed_("seed", "Seed", "The seed used to initialize the random sequence"_help, 1,
            {0, ConstraintBehavior::Immutable}, {1000, ConstraintBehavior::Ignore})
    , rd_()
    , mt_(rd_()) {

    addPort(noise_);
    addProperties(size_, type_, range_, levels_, persistence_, poissonDotsAlongX_,
                  poissonMaxPoints_, haltonNumPoints_, haltonXBase_, haltonYBase_);

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
    randomness_.addProperties(useSameSeed_, seed_);
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
