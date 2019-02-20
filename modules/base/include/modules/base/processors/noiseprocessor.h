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

#ifndef IVW_NOISEPROCESSOR_H
#define IVW_NOISEPROCESSOR_H

#include <modules/base/basemoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/ports/imageport.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/properties/compositeproperty.h>
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/properties/minmaxproperty.h>

#include <random>

namespace inviwo {

class ImageRAM;

/** \docpage{org.inviwo.NoiseProcessor, Noise Processor}
 * ![](org.inviwo.NoiseProcessor.png?classIdentifier=org.inviwo.NoiseProcessor)
 *
 * A processor to generate noise images. Using the Mersenne Twister 19937 generator to generate
 * random numbers. Supported methods are:
 * ### Available Methods
 * ![](noise_types.png)
 * #### Random
 * Generates a uniform, random value in the range [min,max] for each pixel
 * #### Perlin Noise
 * Generates a perlin noise image
 * #### PoissonDisk
 * Create a binary image of points uniformly distributed over the image. Read more at
 * [http://devmag.org.za/2009/05/03/poisson-disk-sampling/](http://devmag.org.za/2009/05/03/poisson-disk-sampling/)
 * ### Halton Sequence
 * Create a binary image of based on semi-random pairs (deterministic) constructed using two Halton
 * Sequences of different bases (base 2 and base 3 gives good results). Read more at
 * [https://en.wikipedia.org/wiki/Halton_sequence/](https://en.wikipedia.org/wiki/Halton_sequence/)
 *
 * ### Outports
 *   * __noise__ The noise image, a single channel 32-bit float image.
 *
 * ### Properties
 *   * __size__ Size of the output image.
 *   * __type__ Witch type of noise to generate.
 *   * __range__ The min/max values of the output values (default: [0 1]).
 *   * __Perlin Noise:__
 *     + __persistence__ Controls the sharpnes in Perlin noise
 *     + __levels__ Numbers of levels used in the generation.
 *   * __Poisson Disk Sampling:__
 *     + __poissonDotsAlongX__ Average number of points along the x-axis.
 *     + __poissonMaxPoints__ Maximum number of output points (total).
 *   * __Halton Sequence:__
 *     + __numPoints:__ Number of pairs to generate
 *     + __haltonXBase:__ Base for the x values
 *     + __haltonYBase:__ Base for the y values
 *   * __Random__
 *     * __Use same seed__ Use the same seed for each call to process. Seed specified by __Seed__
 *     * __Seed__ The seed used to initialize the random sequence
 *
 */

/**
 * \class NoiseProcessor
 *
 * \brief A processor to generate a noise image
 */
class IVW_MODULE_BASE_API NoiseProcessor : public Processor {
    enum class NoiseType { Random, Perlin, PoissonDisk, HaltonSequence };

public:
    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;
    NoiseProcessor();
    virtual ~NoiseProcessor();

    virtual void process() override;

protected:
    ImageOutport noise_;

    IntSize2Property size_;                   ///< Size of the output image.
    TemplateOptionProperty<NoiseType> type_;  ///< Witch type of noise to generate.
    FloatMinMaxProperty range_;  ///< The min/max values of the output values (default: [0 1]).
    IntMinMaxProperty levels_;   ///< Numbers of levels used in the generation of the Perlin noise
    FloatProperty persistence_;  ///< Controls the sharpnes in Perlin noise

    IntProperty poissonDotsAlongX_;  ///< Average number of points along the x-axis.
    IntProperty poissonMaxPoints_;   ///< Maximum number of output points (total).

    IntSizeTProperty haltonNumPoints_;
    IntSizeTProperty haltonXBase_;
    IntSizeTProperty haltonYBase_;

    CompositeProperty randomness_;
    BoolProperty useSameSeed_;  ///< Use the same seed for each call to process.
    IntProperty seed_;          ///<  The seed used to initialize the random sequence

private:
    std::random_device rd_;
    std::mt19937 mt_;
};

}  // namespace inviwo

#endif  // IVW_NOISEPROCESSOR_H
