/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2025 Inviwo Foundation
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

#include <modules/base/algorithm/volume/volumecurl.h>

#include <inviwo/core/algorithm/samplevolume.h>
#include <inviwo/core/datastructures/coordinatetransformer.h>           // for StructuredCoordin...
#include <inviwo/core/datastructures/data.h>                            // for noData
#include <inviwo/core/datastructures/datamapper.h>                      // for DataMapper
#include <inviwo/core/datastructures/representationconverter.h>         // for RepresentationCon...
#include <inviwo/core/datastructures/representationconverterfactory.h>  // for RepresentationCon...
#include <inviwo/core/datastructures/volume/volume.h>                   // for Volume
#include <inviwo/core/datastructures/volume/volumeram.h>                // for VolumeRAMPrecision
#include <inviwo/core/util/formatdispatching.h>                         // for PrecisionValueType
#include <inviwo/core/util/glmutils.h>                                  // for Vector
#include <inviwo/core/util/glmvec.h>                                    // for vec3, size3_t, dvec2
#include <inviwo/core/util/indexmapper.h>                               // for IndexMapper, Inde...
#include <inviwo/core/util/templatesampler.h>                           // for TemplateVolumeSam...
#include <inviwo/core/util/volumeramutils.h>                            // for forEachVoxel
#include <inviwo/core/util/rendercontext.h>

#include <stdlib.h>       // for abs
#include <algorithm>      // for max, min
#include <cmath>          // for abs
#include <limits>         // for numeric_limits
#include <type_traits>    // for conditional_t
#include <unordered_set>  // for unordered_set

#include <glm/common.hpp>  // for mix
#include <glm/mat4x4.hpp>  // for operator*, mat
#include <glm/vec3.hpp>    // for operator/, operator*
#include <glm/vec4.hpp>    // for operator*, operator+

namespace inviwo {
namespace util {

std::unique_ptr<Volume> curlVolume(std::shared_ptr<const Volume> volume) {
    return curlVolume(*volume);
}

std::unique_ptr<Volume> curlVolume(const Volume& volume) {
    auto newVolume = std::make_unique<Volume>(volume, noData);
    auto newVolumeRep = std::make_shared<VolumeRAMPrecision<vec3>>(volume.getDimensions());
    newVolume->addRepresentation(newVolumeRep);

    const auto m = newVolume->getCoordinateTransformer().getDataToWorldMatrix();

    const auto a = m * vec4(0, 0, 0, 1);
    const auto b = m * vec4(1.0f / vec3(volume.getDimensions() - size3_t(1)), 1);
    const auto spacing = b - a;

    const vec3 ox(spacing.x, 0, 0);
    const vec3 oy(0, spacing.y, 0);
    const vec3 oz(0, 0, spacing.z);

    volume.getRepresentation<VolumeRAM>()->dispatch<void, dispatching::filter::Vec3s>(
        [&](auto vol) {
            using DataType = util::PrecisionValueType<decltype(vol)>;
            using ComponentType = util::value_type_t<DataType>;
            using SampleType =
                typename std::conditional_t<std::is_same<float, ComponentType>::value, vec3, dvec3>;
            using Sampler = TemplateVolumeSampler<SampleType, DataType>;

            util::IndexMapper3D index(volume.getDimensions());
            auto data = newVolumeRep->getDataTyped();
            float minV = std::numeric_limits<float>::max();
            float maxV = std::numeric_limits<float>::lowest();

            const Sampler sampler{volume, CoordinateSpace::World};

            util::forEachVoxel(*vol, [&](const size3_t& pos) {
                const vec3 world{m *
                                 vec4(vec3(pos) / vec3(volume.getDimensions() - size3_t(1)), 1)};

                const auto Fxp = static_cast<vec3>(sampler.sample(world + ox));
                const auto Fxm = static_cast<vec3>(sampler.sample(world - ox));
                const auto Fyp = static_cast<vec3>(sampler.sample(world + oy));
                const auto Fym = static_cast<vec3>(sampler.sample(world - oy));
                const auto Fzp = static_cast<vec3>(sampler.sample(world + oz));
                const auto Fzm = static_cast<vec3>(sampler.sample(world - oz));

                const vec3 Fx = (Fxp - Fxm) / (2.0f * spacing.x);
                const vec3 Fy = (Fyp - Fym) / (2.0f * spacing.y);
                const vec3 Fz = (Fzp - Fzm) / (2.0f * spacing.z);

                const vec3 c{Fy.z - Fz.y, Fz.x - Fx.z, Fx.y - Fy.x};

                minV = std::min({minV, c.x, c.y, c.z});
                maxV = std::max({maxV, c.x, c.y, c.z});

                data[index(pos)] = c;
            });

            auto range = std::max(std::abs(minV), std::abs(maxV));
            newVolume->dataMap.dataRange = dvec2(-range, range);
            newVolume->dataMap.valueRange = dvec2(minV, maxV);
        });

    return newVolume;
}

template <typename C, typename Init, typename BinaryOp>
auto voxelReduceParallel(const size3_t dims, Init init, BinaryOp op, C callback, size_t jobs = 0)
    -> Init {

    const size_t poolSize = util::getPoolSize();
    if (jobs == 0) {
        jobs = 4 * poolSize;
    }
    // if ((jobs == 0) || (poolSize == 0)) {
    //     // fallback to serial version
    //     forEachVoxel(dims, callback);
    //     return;
    // }

    std::vector<std::future<Init>> futures;

    for (size_t job = 0; job < jobs; ++job) {
        size3_t start = size3_t(0, 0, job * dims.z / jobs);
        size3_t stop = size3_t(dims.x, dims.y, std::min(dims.z, (job + 1) * dims.z / jobs));

        futures.push_back(util::dispatchPool([&callback, init, op, start, stop]() {
            size3_t pos{0};

            rendercontext::activateLocal();

            Init val = init;
            for (pos.z = start.z; pos.z < stop.z; ++pos.z) {
                for (pos.y = start.y; pos.y < stop.y; ++pos.y) {
                    for (pos.x = start.x; pos.x < stop.x; ++pos.x) {
                        val = std::invoke(op, val, callback(pos));
                    }
                }
            }
            return val;
        }));
    }

    Init val = init;
    for (auto& e : futures) {
        val = std::invoke(op, val, e.get());
    }
    return val;
}

std::unique_ptr<Volume> curlVolume2(const Volume& volume) {
    auto newVolume = std::make_unique<Volume>(volume, noData);
    auto newVolumeRep = std::make_shared<VolumeRAMPrecision<vec3>>(volume.getDimensions());
    newVolume->addRepresentation(newVolumeRep);

    const auto m = dmat4{newVolume->getCoordinateTransformer().getDataToWorldMatrix()};

    const auto a = m * dvec4(0, 0, 0, 1);
    const auto b = m * dvec4(1.0 / dvec3(volume.getDimensions() - size3_t(1)), 1);
    const auto spacing = b - a;

    const dvec3 ox(spacing.x, 0, 0);
    const dvec3 oy(0, spacing.y, 0);
    const dvec3 oz(0, 0, spacing.z);

    auto dst = newVolumeRep->getView();
    const auto im = util::IndexMapper3D(volume.getDimensions());

    auto r = volume.getRepresentation<VolumeRAM>();
    if (auto* p = dynamic_cast<const VolumeRAMPrecision<glm::vec3>*>(r)) {
        const auto data = p->getView();
        for (size_t i = 0; i < volume.getDimensions().z; ++i) {
            log::info("{} -> {}", i, data[im(size3_t(50, 50, i))]);
        }
    }

    const auto max = util::voxelReduceParallel(
        newVolumeRep->getDimensions(), std::numeric_limits<double>::lowest(), std::ranges::max,
        [&](const size3_t& pos) {
            const dvec3 world{m *
                              dvec4(dvec3(pos) / dvec3(volume.getDimensions() - size3_t(1)), 1)};

            const auto samplePos = std::array<dvec3, 6>{world + ox, world - ox, world + oy,
                                                        world - oy, world + oz, world - oz};

            auto samples = std::array<dvec3, 6>{};
            sample::sample(volume, samplePos, samples, CoordinateSpace::World,
                           DataMapper::Space::Value);

            const dvec3 Fx = (samples[0] - samples[1]) / (2.0 * spacing.x);
            const dvec3 Fy = (samples[2] - samples[3]) / (2.0 * spacing.y);
            const dvec3 Fz = (samples[4] - samples[5]) / (2.0 * spacing.z);
            const dvec3 curl{Fy.z - Fz.y, Fz.x - Fx.z, Fx.y - Fy.x};
            dst[im(pos)] = static_cast<vec3>(curl);

            return glm::compMax(glm::abs(curl));
        });

    newVolume->dataMap.dataRange = dvec2(-max, max);
    newVolume->dataMap.valueRange = dvec2(-max, max);

    return newVolume;
}

}  // namespace util
}  // namespace inviwo
