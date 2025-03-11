/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015-2024 Inviwo Foundation
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

#include <modules/base/processors/mynoisegeneratorcolor.h>

#include <inviwo/core/datastructures/image/layer.h>
#include <inviwo/core/ports/imageport.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/network/processornetwork.h>
#include <inviwo/core/util/formats.h>
#include <inviwo/core/util/glmvec.h>
#include <inviwo/core/datastructures/geometry/mesh.h>
#include <modules/base/algorithm/randomutils.h>

#include <bit>

namespace inviwo {
class Image;

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo MyNoiseGeneratorColor::processorInfo_{
    "org.inviwo.MyNoiseGeneratorColor",       // Class identifier
    "My Noise Generator Color",               // Display name
    "Data Creation",                          // Category
    CodeState::Stable,                        // Code state
    Tags::CPU | Tag("Layer") | Tag("Noise"),  // Tags
    R"(
    A processor to generate a noise layer.
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
      
    )"_unindentHelp,
};

const ProcessorInfo MyNoiseGeneratorColor::getProcessorInfo() const { return processorInfo_; }

MyNoiseGeneratorColor::MyNoiseGeneratorColor()
    : Processor()
    , points_{"points", "Generated points"_help}    
    , orbitals_("orbitals", "Generated orbitals"_help)
    , pointsLayer_("pointsLayer", "Generated points image"_help)
    , mesh_("spheremesh", "Positions and radii"_help)
    , size_("size", "Size", "Size of the output image."_help, size_t(256),
            {size_t(1), ConstraintBehavior::Editable}, {size_t(1096), ConstraintBehavior::Editable})
    , radii_{"radii", "radii scale factor"}
    , seed_{"seed",
            "Seed",
            "Random seed."_help,
            size_t(256),
            {size_t(1), ConstraintBehavior::Editable},
            {size_t(1096), ConstraintBehavior::Editable}} {

    addPorts(points_ ,orbitals_,pointsLayer_, mesh_);
    addProperties(size_, radii_, seed_);
}

void MyNoiseGeneratorColor::process() {
    
    
    std::vector<vec4> data(size_.get(), vec4{0, 0, 0, 0});
    std::vector<GaussianOrbital> data2(size_.get(), GaussianOrbital{});
    std::random_device rd;          // Will be used to obtain a seed for the random number engine
    std::mt19937 gen(static_cast<unsigned int>(seed_.get()));  // Standard mersenne_twister_engine seeded with rd()
    std::uniform_real_distribution<double> posDis(0.0, 1.0);
    std::uniform_real_distribution<double> radiiDis(0.1, 0.3);
    std::uniform_int_distribution<int> quantDis(0, 3);

    std::transform(std::begin(data), std::end(data), std::begin(data), [&](const vec4& point) {
        return vec4{posDis(gen), posDis(gen), posDis(gen), radiiDis(gen) * radii_};
    });

    std::transform(std::begin(data), std::end(data), std::begin(data2),
                        [&](const vec4& point) {
        vec3 coefs{quantDis(gen), quantDis(gen), quantDis(gen)};
        GaussianOrbital orbital{point, coefs};
        return orbital;
    });

    std::vector<vec4> positions(data.size());
    std::transform(data.begin(), data.end(), positions.begin(),
                   [](auto& v) { return static_cast<vec4>(v); });

    auto mesh = std::make_shared<Mesh>(DrawType::Points, ConnectivityType::None);
    mesh->addBuffer(BufferType::PositionAttrib, util::makeBuffer(std::move(positions)));

    points_.setData(std::move(data));
    orbitals_.setData(std::move(data2));
    auto ram = std::make_shared<LayerRAMPrecision<vec4>>(
        LayerReprConfig{.dimensions = size2_t{size_.get(), 1},
                        .type = LayerType::Color,
                        .interpolation = InterpolationType::Nearest});

    std::transform(std::begin(data), std::end(data), std::begin(ram->getView()),
                   [](const dvec4& point) { return point; });
    /* std::ranges::for_each(ram->getView(), [&](vec4& point) {
        point = vec4{posDis(gen), posDis(gen), posDis(gen), radiiDis(gen) * radii_};

    });*/
    // fixa outport inport med vector tänk på tincr så att den är oberoende av antalet voxlar osv
    pointsLayer_.setData(std::make_shared<Layer>(ram));

    mesh_.setData(mesh);
}

}  // namespace inviwo
