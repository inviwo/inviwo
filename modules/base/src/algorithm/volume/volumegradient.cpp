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

#include <modules/base/algorithm/volume/volumegradient.h>

#include <inviwo/core/algorithm/buildarray.h>
#include <inviwo/core/algorithm/gridtools.h>
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
#include <inviwo/core/util/glmcomp.h>
#include <inviwo/core/util/indexmapper.h>     // for IndexMapper3D
#include <inviwo/core/util/spatialsampler.h>  // for SpatialSampler<>:...
#include <inviwo/core/util/volumeramutils.h>  // for forEachVoxelParallel
#include <inviwo/core/util/volumesampler.h>   // for VolumeDoubleSampler
#include <inviwo/core/util/assertion.h>

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

namespace {

template <typename T, Wrapping Wx, Wrapping Wy, Wrapping Wz>
double calcGradientVolume(size3_t dims, std::span<const T> src, std::span<vec3> dst,
                          const DataMapper& dm, dmat3 gInv, dmat3 basis, size_t channel,
                          const std::function<void(double)>& progress,
                          const std::function<bool()>& stop) {

    const auto im = util::IndexMapper3D(dims);
    double max = 0.0;
    const auto delta = static_cast<dvec3>(dims);

    using grid::next;
    using grid::prev;

    grid::loop(
        dims,
        [&]<grid::Part Px, grid::Part Py, grid::Part Pz>(size_t x, size_t y, size_t z) {
            std::array<double, 6> samples{
                static_cast<double>(util::glmcomp(src[im(next<Px, Wx>(x, dims.x), y, z)], channel)),
                static_cast<double>(util::glmcomp(src[im(prev<Px, Wx>(x, dims.x), y, z)], channel)),
                static_cast<double>(util::glmcomp(src[im(x, next<Py, Wy>(y, dims.y), z)], channel)),
                static_cast<double>(util::glmcomp(src[im(x, prev<Py, Wy>(y, dims.y), z)], channel)),
                static_cast<double>(util::glmcomp(src[im(x, y, next<Pz, Wz>(z, dims.z))], channel)),
                static_cast<double>(
                    util::glmcomp(src[im(x, y, prev<Pz, Wz>(z, dims.z))], channel))};

            for (auto& item : samples) {
                item = dm.mapFromDataTo<DataMapper::Space::Value>(item);
            }

            const auto Gx = (samples[0] - samples[1]) * delta.x * grid::invStep<Px, Wx>();
            const auto Gy = (samples[2] - samples[3]) * delta.y * grid::invStep<Py, Wy>();
            const auto Gz = (samples[4] - samples[5]) * delta.z * grid::invStep<Pz, Wz>();

            const auto grad = basis * gInv * dvec3{Gx, Gy, Gz};

            max = std::max(max, std::abs(glm::compMax(grad)));

            dst[im(x, y, z)] = static_cast<vec3>(grad);
        },
        progress, stop);

    return max;
}

template <size_t I>
using index = std::integral_constant<size_t, I>;

}  // namespace

std::shared_ptr<Volume> gradientVolume(
    const Volume& srcVolume, size_t channel,
    const std::function<std::shared_ptr<Volume>(const VolumeConfig&)>& getVolume,
    const std::function<void(double)>& progress, const std::function<bool()>& stop) {

    if (const auto comps = srcVolume.getDataFormat()->getComponents(); channel >= comps) {
        throw Exception{SourceContext{},
                        "Requested channel ({}) does not exist in volume ({} channels)", channel,
                        comps};
    }

    if (progress) progress(0.0);

    const auto config = VolumeConfig{srcVolume.config()}.updateFrom(
        {.format = DataVec3Float32::get(),
         .swizzleMask = swizzlemasks::defaultData(3),
         .interpolation = InterpolationType::Linear,
         .valueAxis = Axis{.name = "Gradient",
                           .unit = srcVolume.dataMap.valueAxis.unit / srcVolume.axes[0].unit}});
    auto dstVolume = getVolume(config);

    auto* dstVolumeRep =
        dynamic_cast<VolumeRAMPrecision<vec3>*>(dstVolume->getEditableRepresentation<VolumeRAM>());

    IVW_ASSERT(dstVolumeRep, "should exist");

    const auto gInv = dstVolume->getCoordinateTransformer().getInverseMetricTensor();

    const auto basis = dstVolume->getCoordinateTransformer().getDataToWorldMatrix();

    const auto dims = dstVolumeRep->getDimensions();
    const auto wrapping = srcVolume.getWrapping();
    const auto* const srcRep = srcVolume.getRepresentation<VolumeRAM>();

    const auto max = srcRep->dispatch<double, dispatching::filter::All>(
        [&]<typename T>(const VolumeRAMPrecision<T>* srcTRep) {
            static constexpr auto table =
                build_array_t_nd<3uz, 3uz, 3uz>([]<size_t x, size_t y, size_t z>() {
                    return +[](size3_t dims, std::span<const T> src, std::span<vec3> dst,
                               const DataMapper& dm, dmat3 gInv, dmat3 basis, size_t c,
                               const std::function<void(double)>& p,
                               const std::function<bool()>& s) -> double {
                        constexpr auto Wx = static_cast<Wrapping>(x);
                        constexpr auto Wy = static_cast<Wrapping>(y);
                        constexpr auto Wz = static_cast<Wrapping>(z);
                        return calcGradientVolume<T, Wx, Wy, Wz>(dims, src, dst, dm, gInv, basis, c,
                                                                 p, s);
                    };
                });

            const auto& func =
                table[std::to_underlying(wrapping[0])][std::to_underlying(wrapping[1])]
                     [std::to_underlying(wrapping[2])];

            return func(dims, srcTRep->getView(), dstVolumeRep->getView(), srcVolume.dataMap,
                        dmat3{gInv}, dmat3{basis}, channel, progress, stop);
        });

    dstVolume->dataMap.dataRange = dvec2(-max, max);
    dstVolume->dataMap.valueRange = dvec2(-max, max);
    if (progress) progress(1.0);

    return dstVolume;
}

std::shared_ptr<Volume> gradientVolume2(
    const Volume& srcVolume, int channel,
    const std::function<std::shared_ptr<Volume>(const VolumeConfig&)>& getVolume,
    const std::function<void(double)>& progress, const std::function<bool()>& stop) {

    auto newVolume = std::make_unique<Volume>(
        srcVolume, noData,
        srcVolume.config().updateFrom({
            .valueAxis =
                Axis{"gradient", srcVolume.dataMap.valueAxis.unit / srcVolume.axes[0].unit},
        }));

    auto newVolumeRep = std::make_shared<VolumeRAMPrecision<vec3>>(srcVolume.getDimensions());
    newVolume->addRepresentation(newVolumeRep);

    auto m = newVolume->getCoordinateTransformer().getDataToWorldMatrix();

    const auto a = m * vec4(0, 0, 0, 1);
    const auto b = m * vec4(1.0f / vec3(srcVolume.getDimensions() - size3_t(1)), 1);
    const auto spacing = b - a;

    const vec3 ox(spacing.x, 0, 0);
    const vec3 oy(0, spacing.y, 0);
    const vec3 oz(0, 0, spacing.z);

    VolumeSampler sampler{srcVolume, CoordinateSpace::World};

    util::IndexMapper3D index(srcVolume.getDimensions());
    auto data = newVolumeRep->getDataTyped();

    const dvec3 spacing2 = spacing * 2.0;
    float max = std::numeric_limits<float>::lowest();
    auto func = [&](const size3_t& pos) {
        const vec3 world{m * vec4(vec3(pos) / vec3(srcVolume.getDimensions() - size3_t(1)), 1)};

        const auto g = static_cast<vec3>(
            dvec3{(sampler.sample(world + ox) - sampler.sample(world - ox))[channel],
                  (sampler.sample(world + oy) - sampler.sample(world - oy))[channel],
                  (sampler.sample(world + oz) - sampler.sample(world - oz))[channel]} /
            spacing2);

        data[index(pos)] = g;
        max = glm::max(max, glm::compMax(glm::abs(g)));
    };

    util::forEachVoxelParallel(*srcVolume.getRepresentation<VolumeRAM>(), func);

    newVolume->dataMap.dataRange = dvec2(-max, max);
    newVolume->dataMap.valueRange = dvec2(-max, max);

    return newVolume;
}

}  // namespace util
}  // namespace inviwo
