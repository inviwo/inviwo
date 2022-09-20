/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2022 Inviwo Foundation
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

#include <modules/base/processors/pointgenerationprocessor.h>

#include <inviwo/core/datastructures/camera/camera.h>      // for mat4
#include <inviwo/core/ports/dataoutport.h>                 // for DataOutport
#include <inviwo/core/ports/outportiterable.h>             // for OutportIterableImpl<>::const_i...
#include <inviwo/core/processors/processor.h>              // for Processor
#include <inviwo/core/processors/processorinfo.h>          // for ProcessorInfo
#include <inviwo/core/processors/processorstate.h>         // for CodeState, CodeState::Stable
#include <inviwo/core/processors/processortags.h>          // for Tags, Tags::CPU
#include <inviwo/core/properties/boolcompositeproperty.h>  // for BoolCompositeProperty
#include <inviwo/core/properties/ordinalproperty.h>        // for ordinalSymmetricVector, ordina...
#include <inviwo/core/util/glmmat.h>                       // for dmat4
#include <inviwo/core/util/glmvec.h>                       // for vec3, vec4, vec2, dvec3, size3_t
#include <inviwo/core/util/staticstring.h>                 // for operator+
#include <modules/base/algorithm/pointgeneration.h>        // for generatePoints, Grid3DPointGen...

#include <cstddef>      // for size_t
#include <functional>   // for __base
#include <iterator>     // for back_insert_iterator, back_ins...
#include <memory>       // for shared_ptr, make_shared
#include <optional>     // for optional, nullopt
#include <string_view>  // for string_view

#include <fmt/core.h>                         // for format, basic_string_view
#include <fmt/format.h>                       // for formatbuf<>::int_type, formatb...
#include <glm/gtx/scalar_multiplication.hpp>  // for operator*
#include <glm/vec3.hpp>                       // for operator+, operator*, vec, ope...

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo Point3DGenerationProcessor::processorInfo_{
    "org.inviwo.PointGenerationProcessor",  // Class identifier
    "Point Generation",                     // Display name
    "Data Creation",                        // Category
    CodeState::Stable,                      // Code state
    Tags::CPU,                              // Tags
};
const ProcessorInfo Point3DGenerationProcessor::getProcessorInfo() const { return processorInfo_; }

Point3DGenerationProcessor::Point3DGenerationProcessor()
    : Processor()
    , outport_("outport")
    , grid_{{"basis", "Basis", util::ordinalSymmetricVector(vec3{1.0f, 0.0f, 0.0f}, 100.f)},
            {"axis2", "Axis 2", util::ordinalSymmetricVector(vec3{0.0f, 1.0f, 0.0f}, 100.f)},
            {"axis3", "Axis 3", util::ordinalSymmetricVector(vec3{0.0f, 0.0f, 1.0f}, 100.f)},
            {"autoCenter", "Auto Center", true},
            {"offset", "Offset", util::ordinalSymmetricVector(vec3{0.0f, 0.0f, 0.0f}, 100.f)},
            {"nPoints", "Number of Points", util::ordinalCount(size3_t{10}, 100u)},
            {"jitter", "jitter", util::ordinalSymmetricVector(vec3{0.0f, 0.0f, 0.0f}, 10.f)},
            {"seed", "Seed", util::ordinalCount(size_t{0}, 1000u)}}
    , box_{{"axis1", "Axis 1", util::ordinalSymmetricVector(vec3{1.0f, 0.0f, 0.0f}, 100.f)},
           {"axis2", "Axis 2", util::ordinalSymmetricVector(vec3{0.0f, 1.0f, 0.0f}, 100.f)},
           {"axis3", "Axis 3", util::ordinalSymmetricVector(vec3{0.0f, 0.0f, 1.0f}, 100.f)},
           {"autoCenter", "Auto Center", true},
           {"offset", "Offset", util::ordinalSymmetricVector(vec3{0.0f, 0.0f, 0.0f}, 100.f)},
           {"nPoints", "Number of Points", util::ordinalCount(size_t{10}, 100u)},
           {"seed", "Seed", util::ordinalCount(size_t{0}, 1000u)}}
    , sphere_{{"center", "Center", util::ordinalSymmetricVector(vec3{0.0f, 0.0f, 0.0f}, 100.f)},
              {"radius", "Radius", util::ordinalLength(vec2{0.0f, 1.0f}, 100.f)},
              {"nPoints", "Number of Points", util::ordinalCount(size_t{10}, 100u)},
              {"seed", "Seed", util::ordinalCount(size_t{0}, 1000u)}}
    , gridProps_{"grid", "Grid", true}
    , boxProps_{"box", "Random box", false}
    , sphereProps_{"sphere", "Random Sphere", false} {

    gridProps_.addProperties(grid_.a1, grid_.a2, grid_.a3, grid_.autoCenter, grid_.offset,
                             grid_.nPoints, grid_.jitter, grid_.seed);
    boxProps_.addProperties(box_.a1, box_.a2, box_.a3, box_.autoCenter, box_.offset, box_.nPoints,
                            box_.seed);
    sphereProps_.addProperties(sphere_.center, sphere_.radius, sphere_.nPoints, sphere_.seed);
    addProperties(gridProps_, boxProps_, sphereProps_);

    addPort(outport_);

    auto center = [](auto& item) {
        auto callback = [&item]() {
            if (item.autoCenter) {
                item.offset.set(-0.5 * (item.a1.get() + item.a2.get() + item.a3.get()));
            }
        };
        item.a1.onChange(callback);
        item.a2.onChange(callback);
        item.a3.onChange(callback);
        item.autoCenter.onChange([&item, callback]() {
            item.autoCenter.setReadOnly(item.autoCenter);
            if (item.autoCenter) callback();
        });
    };

    center(grid_);
    center(box_);
}

void Point3DGenerationProcessor::process() {
    std::shared_ptr<std::vector<vec3>> points = std::make_shared<std::vector<vec3>>();

    if (gridProps_) {
        util::Grid3DPointGeneration opts{
            mat4{vec4{grid_.a1.get(), 0.0f}, vec4{grid_.a2.get(), 0.0f}, vec4{grid_.a3.get(), 0.0f},
                 vec4{grid_.offset.get(), 1.0f}},
            grid_.nPoints.get(),
            grid_.jitter.get() == vec3(0) ? std::nullopt : std::optional<vec3>(grid_.jitter.get()),
            grid_.seed.get()};
        util::generatePoints<float>(std::back_inserter(*points), opts);
    }

    if (boxProps_) {
        util::RandomCubicalPointGeneration opts{
            mat4{vec4{box_.a1.get(), 0.0f}, vec4{box_.a2.get(), 0.0f}, vec4{box_.a3.get(), 0.0f},
                 vec4{box_.offset.get(), 1.0f}},
            box_.nPoints.get(), box_.seed.get()};
        util::generatePoints<float>(std::back_inserter(*points), opts);
    }

    if (sphereProps_) {
        util::RandomSphericalPointGeneration opts{sphere_.center.get(), sphere_.radius.get(),
                                                  sphere_.nPoints.get(), sphere_.seed.get()};
        util::generatePoints<float>(std::back_inserter(*points), opts);
    }

    outport_.setData(points);
}

}  // namespace inviwo
