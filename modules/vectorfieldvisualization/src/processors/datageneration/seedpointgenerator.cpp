/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015-2025 Inviwo Foundation
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

#include <modules/vectorfieldvisualization/processors/datageneration/seedpointgenerator.h>

#include <inviwo/core/processors/processor.h>                       // for Processor
#include <inviwo/core/processors/processorinfo.h>                   // for ProcessorInfo
#include <inviwo/core/processors/processorstate.h>                  // for CodeState, CodeState:...
#include <inviwo/core/processors/processortags.h>                   // for Tags
#include <inviwo/core/properties/boolproperty.h>                    // for BoolProperty
#include <inviwo/core/properties/compositeproperty.h>               // for CompositeProperty
#include <inviwo/core/properties/minmaxproperty.h>                  // for FloatMinMaxProperty
#include <inviwo/core/properties/optionproperty.h>                  // for OptionPropertyInt
#include <inviwo/core/properties/ordinalproperty.h>                 // for FloatVec3Property
#include <inviwo/core/util/glmvec.h>                                // for vec3, vec2
#include <inviwo/core/util/logcentral.h>                            // for LogCentral
#include <modules/base/algorithm/randomutils.h>                     // for randomNumber
#include <modules/vectorfieldvisualization/ports/seedpointsport.h>  // for SeedPoints3DOutport

#include <cmath>        // for cos, sin, acos, pow
#include <functional>   // for __base
#include <memory>       // for make_shared, shared_ptr
#include <string>       // for string
#include <string_view>  // for string_view
#include <type_traits>  // for remove_extent_t
#include <vector>       // for vector

#include <glm/gtc/constants.hpp>  // for two_pi
#include <glm/vec2.hpp>           // for vec, vec<>::(anonymous)
#include <glm/vec3.hpp>           // for operator*, vec, opera...

#define RND 1
#define PLANE 2
#define LINE 3
#define SPHERE 4

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo SeedPointGenerator::processorInfo_{
    "org.inviwo.SeedPointGenerator3D",  // Class identifier
    "Seed Point Generator 3D",          // Display name
    "Data Creation",                    // Category
    CodeState::Stable,                  // Code state
    "CPU, Seed Points, Generator",      // Tags
};
const ProcessorInfo& SeedPointGenerator::getProcessorInfo() const { return processorInfo_; }

SeedPointGenerator::SeedPointGenerator()
    : Processor()
    , seedPoints_("seedPoints")
    , lineGroup_("line", "Line")
    , planeGroup_("plane", "Plane")
    , sphereGroup_("sphere", "Sphere")
    , numberOfPoints_("numberOfPoints", "Number of Points", 10, 1, 1000)
    , planeResolution_("planeResolution", "Resolution", vec2(4, 4), vec2(2, 2), vec2(100, 100))
    , planeOrigin_("planeOrigin_", "Origin", vec3(0.0f, 0.0f, 0.5f), vec3(-1, -1, -1),
                   vec3(1, 1, 1))
    , planeE1_("planeP1_", "Offset 1", vec3(1.0f, 0.0f, 0.5f), vec3(-1, -1, -1), vec3(1, 1, 1))
    , planeE2_("planeP2_", "Offset 2", vec3(0.0f, 1.0f, 0.5f), vec3(-1, -1, -1), vec3(1, 1, 1))
    , sphereCenter_("sphereCenter", "Center", vec3(0.5f, 0.5f, 0.5f), vec3(0, 0, 0), vec3(1, 1, 1))
    , sphereRadius_("sphereRadius", "Radius")
    , lineStart_("lineStart", "Start", vec3(0.5f, 0.0f, 0.5f), vec3(-1, -1, -1), vec3(1, 1, 1))
    , lineEnd_("lineEnd_", "End", vec3(0.5f, 1.0f, 0.5f), vec3(-1, -1, -1), vec3(1, 1, 1))
    , generator_("generator", "Generator")
    , randomness_("randomness", "Randomness")
    , useSameSeed_("useSameSeed", "Use same seed", true)
    , seed_("seed", "Seed", 1, 0, 1000)
    , rd_()
    , mt_(rd_()) {
    addPort(seedPoints_);

    generator_.addOption("random", "Random", RND);
    generator_.addOption("line", "Line", LINE);
    generator_.addOption("plane", "Plane", PLANE);
    generator_.addOption("sphere", "Sphere", SPHERE);
    generator_.setCurrentStateAsDefault();
    generator_.onChange([this]() { onGeneratorChange(); });
    addProperty(generator_);

    lineGroup_.addProperty(lineStart_);
    lineGroup_.addProperty(lineEnd_);
    addProperty(lineGroup_);

    planeGroup_.addProperty(planeResolution_);
    planeGroup_.addProperty(planeOrigin_);
    planeGroup_.addProperty(planeE1_);
    planeGroup_.addProperty(planeE2_);
    addProperty(planeGroup_);

    sphereGroup_.addProperty(sphereCenter_);
    sphereGroup_.addProperty(sphereRadius_);
    sphereRadius_.set(vec2(0.45, 0.55));
    sphereRadius_.setCurrentStateAsDefault();
    addProperty(sphereGroup_);

    addProperty(numberOfPoints_);

    addProperty(randomness_);
    randomness_.addProperty(useSameSeed_);
    randomness_.addProperty(seed_);
    useSameSeed_.onChange([&]() { seed_.setVisible(useSameSeed_.get()); });

    onGeneratorChange();
}

void SeedPointGenerator::process() {
    if (useSameSeed_.get()) {
        mt_.seed(seed_.get());
    }

    switch (generator_.get()) {
        case RND:
            randomPoints();
            break;
        case PLANE:
            planePoints();
            break;
        case LINE:
            linePoints();
            break;
        case SPHERE:
            spherePoints();
            break;
        default:
            log::warn("No points generated since given type is not yet implemented");
            break;
    }
}

void SeedPointGenerator::onGeneratorChange() {
    bool rnd = generator_.get() == RND;
    bool plane = generator_.get() == PLANE;
    bool line = generator_.get() == LINE;
    bool sphere = generator_.get() == SPHERE;

    numberOfPoints_.setVisible(rnd || line || sphere);

    planeResolution_.setVisible(plane);
    planeOrigin_.setVisible(plane);
    planeE1_.setVisible(plane);
    planeE2_.setVisible(plane);

    sphereCenter_.setVisible(sphere);
    sphereRadius_.setVisible(sphere);

    lineStart_.setVisible(line);
    lineEnd_.setVisible(line);
}

void SeedPointGenerator::spherePoints() {
    auto T = [](auto&& r) { return util::randomNumber<float>(r, 0, glm::two_pi<float>()); };
    auto cos_phi = [](auto&& r) { return util::randomNumber<float>(r, -1, 1); };
    auto R = [](auto&& r) { return util::randomNumber<float>(r, 0, 1); };

    auto points = std::make_shared<std::vector<vec3>>();

    for (int i = 0; i < numberOfPoints_.get(); i++) {
        float theta = T(mt_);
        float phi = std::acos(cos_phi(mt_));

        vec2 range = sphereRadius_.get();
        float r = std::pow(R(mt_), 1.0f / 3.0f);
        r = range.x + r * (range.y - range.x);

        float ct = std::cos(theta);
        float st = std::sin(theta);
        float sp = std::sin(phi);
        float cp = std::cos(phi);

        vec3 g = vec3(ct * sp, st * sp, cp);
        vec3 p = g * r + sphereCenter_.get();

        points->push_back(p);
    }

    seedPoints_.setData(points);
}

void SeedPointGenerator::linePoints() {
    auto points = std::make_shared<std::vector<vec3>>();
    float dt = 1.0f / std::max((numberOfPoints_.get() - 1), 1);
    for (int i = 0; i < numberOfPoints_.get(); i++) {
        auto p = lineStart_.get() + (lineEnd_.get() - lineStart_.get()) * (i * dt);
        points->push_back(p);
    }
    seedPoints_.setData(points);
}

void SeedPointGenerator::planePoints() {
    auto points = std::make_shared<std::vector<vec3>>();
    float dx = 1.0f / (planeResolution_.get().x - 1);
    float dy = 1.0f / (planeResolution_.get().y - 1);

    vec3 ox = planeE1_.get();
    vec3 oy = planeE2_.get();

    for (int i = 0; i < planeResolution_.get().x; i++) {
        for (int j = 0; j < planeResolution_.get().y; j++) {
            vec3 p = planeOrigin_.get();
            p += ox * (i * dx);
            p += oy * (j * dy);
            points->push_back(p);
        }
    }
    seedPoints_.setData(points);
}

void SeedPointGenerator::randomPoints() {
    auto points = std::make_shared<std::vector<vec3>>();
    for (int i = 0; i < numberOfPoints_.get(); i++) {
        const float x = util::randomNumber<float>(mt_);
        const float y = util::randomNumber<float>(mt_);
        const float z = util::randomNumber<float>(mt_);
        points->emplace_back(x, y, z);
    }
    seedPoints_.setData(points);
}

}  // namespace inviwo
