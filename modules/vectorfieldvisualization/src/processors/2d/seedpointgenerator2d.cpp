/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2024 Inviwo Foundation
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

#include <modules/vectorfieldvisualization/processors/2d/seedpointgenerator2d.h>

#include <inviwo/core/processors/processor.h>                       // for Processor
#include <inviwo/core/processors/processorinfo.h>                   // for ProcessorInfo
#include <inviwo/core/processors/processorstate.h>                  // for CodeState, CodeState:...
#include <inviwo/core/processors/processortags.h>                   // for Tags
#include <inviwo/core/properties/boolproperty.h>                    // for BoolProperty
#include <inviwo/core/properties/compositeproperty.h>               // for CompositeProperty
#include <inviwo/core/properties/optionproperty.h>                  // for OptionPropertyOption
#include <inviwo/core/properties/ordinalproperty.h>                 // for IntSizeTProperty, Int...
#include <inviwo/core/util/glmvec.h>                                // for vec2
#include <inviwo/core/util/staticstring.h>                          // for operator+
#include <inviwo/core/util/zip.h>                                   // for get, zip, zipIterator
#include <modules/base/algorithm/randomutils.h>                     // for haltonSequence, rando...
#include <modules/vectorfieldvisualization/ports/seedpointsport.h>  // for SeedPoints2DOutport

#include <memory>       // for shared_ptr, make_shared
#include <type_traits>  // for remove_extent_t
#include <algorithm>
#include <ranges>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo SeedPointGenerator2D::processorInfo_{
    "org.inviwo.SeedPointGenerator2D",  // Class identifier
    "Seed Point Generator 2D",          // Display name
    "Data Creation",                    // Category
    CodeState::Stable,                  // Code state
    "CPU, Seed Points, Generator",      // Tags
};
const ProcessorInfo SeedPointGenerator2D::getProcessorInfo() const { return processorInfo_; }

SeedPointGenerator2D::SeedPointGenerator2D()
    : Processor()
    , seeds_("seeds")

    , samplingDomain_{"samplingDomain", "Sampling Domain"}
    , domain_{"domain",
              "Domain",
              {{"fullDomain", "Entire Domain", SamplingDomain::FullDomain},
               {"line", "Line", SamplingDomain::Line},
               {"rectangle", "Rectangle", SamplingDomain::Rectangle},
               {"disk", "Disk", SamplingDomain::Disk}}}
    , position_{"position",
                "Position",
                {vec2{0.0f, 0.0f}, vec2{0.0f}, ConstraintBehavior::Ignore, vec2{1.0f},
                 ConstraintBehavior::Ignore, vec2{0.01f}, InvalidationLevel::InvalidOutput,
                 PropertySemantics::SpinBox}}
    , extent_{"extent", "Extent", util::ordinalLength(vec2{0.5f}, vec2{1.0f})}
    , endpoint_{"endpoint",
                "End Point",
                {vec2{0.0f, 0.0f}, vec2{0.0f}, ConstraintBehavior::Ignore, vec2{1.0f},
                 ConstraintBehavior::Ignore, vec2{0.01f}, InvalidationLevel::InvalidOutput,
                 PropertySemantics::SpinBox}}
    , radius_{"radius", "Radius", util::ordinalLength(0.5f)}

    , generator_{"generator",
                 "Generator",
                 {{"random", "Random", Generator::Random},
                  {"haltonSequence", "Halton Sequence", Generator::HaltonSequence}}}

    , numPoints_{"numPoints", "Number of points", util::ordinalCount<size_t>(100u, 1000u)}
    , haltonXBase_{"haltonXBase", "Base for x values", 2, 2, 32}
    , haltonYBase_{"haltonYBase", "Base for y values", 3, 2, 32}

    , randomness_{"randomness", "Randomness"}
    , useSameSeed_{"useSameSeed", "Use same seed", true}
    , seed_{"seed", "Seed", util::ordinalCount(1, 1000)}

    , rd_()
    , mt_(rd_()) {
    addPort(seeds_);

    addProperties(samplingDomain_, generator_, numPoints_, haltonXBase_, haltonYBase_, randomness_);
    samplingDomain_.addProperties(domain_, position_, extent_, endpoint_, radius_);
    randomness_.addProperties(useSameSeed_, seed_);

    extent_.readonlyDependsOn(
        domain_, [](auto& p) { return p.getSelectedValue() != SamplingDomain::Rectangle; });
    endpoint_.readonlyDependsOn(
        domain_, [](auto& p) { return p.getSelectedValue() != SamplingDomain::Line; });
    radius_.readonlyDependsOn(domain_,
                              [](auto& p) { return p.getSelectedValue() != SamplingDomain::Disk; });

    haltonXBase_.visibilityDependsOn(generator_, [](const auto& p) {
        return p.getSelectedValue() == Generator::HaltonSequence;
    });
    haltonYBase_.visibilityDependsOn(generator_, [](const auto& p) {
        return p.getSelectedValue() == Generator::HaltonSequence;
    });
    randomness_.visibilityDependsOn(
        generator_, [](const auto& p) { return p.getSelectedValue() == Generator::Random; });
}

void SeedPointGenerator2D::process() {
    auto seeds = std::make_shared<std::vector<vec3>>();
    seeds->resize(numPoints_.get());

    auto createSpatialSeeds = [&]() {
        switch (generator_.get()) {
            case Generator::Random: {
                std::uniform_real_distribution<float> dis(0, 1);
                std::ranges::generate(*seeds, [&]() { return vec3{dis(mt_), dis(mt_), 0.0f}; });
                break;
            }
            case Generator::HaltonSequence: {
                std::ranges::copy(std::views::iota(size_t{1}, seeds->size()) |
                                      std::views::transform([baseX = haltonXBase_.get(),
                                                             baseY = haltonYBase_.get()](int i) {
                                          return vec3{util::haltonSequence<float>(i, baseX),
                                                      util::haltonSequence<float>(i, baseY), 0.0f};
                                      }),
                                  seeds->begin());
                break;
            }

            default:
                break;
        }
    };
    auto createLinearSeeds = [&]() {
        switch (generator_.get()) {
            case Generator::Random: {
                std::uniform_real_distribution<float> dis(0, 1);
                std::ranges::generate(*seeds, [&]() { return vec3{dis(mt_), 0.0f, 0.0f}; });
                break;
            }
            case Generator::HaltonSequence: {
                std::ranges::generate_n(
                    seeds->begin(), numPoints_,
                    [i = 1, baseX = haltonXBase_.get(),
                     baseY = haltonYBase_.get()]() mutable -> vec3 {
                        return vec3{util::haltonSequence<float>(i++, baseX), 0.0f, 0.0f};
                    });
                break;
            }

            default:
                break;
        }
    };

    switch (domain_.get()) {
        case SamplingDomain::Rectangle: {
            const vec2 lowerLeft{position_};
            const vec2 upperRight{glm::clamp(lowerLeft + extent_.get(), vec2{0.0f}, vec2{1.0f})};
            const vec2 extent{upperRight - lowerLeft};

            const mat2 m{vec2{extent.x, 0.0f}, vec2{0.0f, extent.y}};

            createSpatialSeeds();
            for (auto& pos : *seeds) {
                pos = vec3{m * vec2{pos} + lowerLeft, 0.0f};
            }
            break;
        }
        case SamplingDomain::Line: {
            // project points onto a line
            const vec2 start{position_};
            const vec2 end{endpoint_};

            createLinearSeeds();
            for (auto& pos : *seeds) {
                pos = vec3{glm::mix(start, end, pos.x), 0.0f};
            }
            break;
        }
        case SamplingDomain::Disk: {
            // transform seed positions from [0,1] to uniformly sampled disk
            // see https://mathworld.wolfram.com/DiskPointPicking.html
            const vec3 center{position_.get(), 0.0f};
            const float radiusSqrt = std::sqrt(radius_.get());

            createSpatialSeeds();
            for (auto& pos : *seeds) {
                pos = vec3{std::cos(pos.x + glm::two_pi<float>()),
                           std::sin(pos.y + glm::two_pi<float>()), 0.0f} *
                          radiusSqrt +
                      center;
            }
            break;
        }
        case SamplingDomain::FullDomain:
            createSpatialSeeds();
            break;
        default:
            break;
    }

    seeds_.setData(seeds);
}

}  // namespace inviwo
