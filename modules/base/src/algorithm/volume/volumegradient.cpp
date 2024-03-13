/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2024 Inviwo Foundation
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

#include <modules/base/algorithm/volume/volumegradient.h>

#include <inviwo/core/datastructures/coordinatetransformer.h>           // for StructuredCoordin...
#include <inviwo/core/datastructures/data.h>                            // for noData
#include <inviwo/core/datastructures/datamapper.h>                      // for DataMapper
#include <inviwo/core/datastructures/representationconverter.h>         // for RepresentationCon...
#include <inviwo/core/datastructures/representationconverterfactory.h>  // for RepresentationCon...
#include <inviwo/core/datastructures/unitsystem.h>                      // for Axis, Unit
#include <inviwo/core/datastructures/volume/volume.h>                   // for Volume
#include <inviwo/core/datastructures/volume/volumeram.h>                // for VolumeRAMPrecision
#include <inviwo/core/util/glmutils.h>                                  // for Vector
#include <inviwo/core/util/glmvec.h>                                    // for vec3, size3_t, dvec2
#include <inviwo/core/util/indexmapper.h>                               // for IndexMapper3D
#include <inviwo/core/util/spatialsampler.h>                            // for SpatialSampler<>:...
#include <inviwo/core/util/volumeramutils.h>                            // for forEachVoxelParallel
#include <inviwo/core/util/volumesampler.h>                             // for VolumeDoubleSampler

#include <array>          // for array
#include <functional>     // for __base
#include <limits>         // for numeric_limits
#include <string>         // for string
#include <type_traits>    // for remove_extent_t
#include <unordered_set>  // for unordered_set
#include <vector>         // for vector

#include <glm/common.hpp>              // for mix, max, abs
#include <glm/gtx/component_wise.hpp>  // for compMax
#include <glm/mat4x4.hpp>              // for operator*, mat
#include <glm/vec3.hpp>                // for operator-, operator/
#include <glm/vec4.hpp>                // for operator*, operator+

namespace inviwo {
namespace util {

std::shared_ptr<Volume> gradientVolume(std::shared_ptr<const Volume> volume, int channel) {
    auto newVolume = std::make_unique<Volume>(
        *volume, noData,
        volume->config().updateFrom({
            .valueAxis = Axis{"gradient", volume->dataMap.valueAxis.unit / volume->axes[0].unit},
        }));

    auto newVolumeRep = std::make_shared<VolumeRAMPrecision<vec3>>(volume->getDimensions());
    newVolume->addRepresentation(newVolumeRep);

    auto m = newVolume->getCoordinateTransformer().getDataToWorldMatrix();

    const auto a = m * vec4(0, 0, 0, 1);
    const auto b = m * vec4(1.0f / vec3(volume->getDimensions() - size3_t(1)), 1);
    const auto spacing = b - a;

    const vec3 ox(spacing.x, 0, 0);
    const vec3 oy(0, spacing.y, 0);
    const vec3 oz(0, 0, spacing.z);

    VolumeSampler sampler{volume, CoordinateSpace::World};

    util::IndexMapper3D index(volume->getDimensions());
    auto data = newVolumeRep->getDataTyped();

    const dvec3 spacing2 = spacing * 2.0;
    float max = std::numeric_limits<float>::lowest();
    auto func = [&](const size3_t& pos) {
        const vec3 world{m * vec4(vec3(pos) / vec3(volume->getDimensions() - size3_t(1)), 1)};

        const auto g = static_cast<vec3>(
            dvec3{(sampler.sample(world + ox) - sampler.sample(world - ox))[channel],
                  (sampler.sample(world + oy) - sampler.sample(world - oy))[channel],
                  (sampler.sample(world + oz) - sampler.sample(world - oz))[channel]} /
            spacing2);

        data[index(pos)] = g;
        max = glm::max(max, glm::compMax(glm::abs(g)));
    };

    util::forEachVoxelParallel(*volume->getRepresentation<VolumeRAM>(), func);

    newVolume->dataMap.dataRange = dvec2(-max, max);
    newVolume->dataMap.valueRange = dvec2(-max, max);

    return newVolume;
}

}  // namespace util
}  // namespace inviwo
