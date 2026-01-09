/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2026 Inviwo Foundation
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
#include <ranges>

#include <glm/common.hpp>              // for mix, max, abs
#include <glm/gtx/component_wise.hpp>  // for compMax
#include <glm/mat4x4.hpp>              // for operator*, mat
#include <glm/vec3.hpp>                // for operator-, operator/
#include <glm/vec4.hpp>                // for operator*, operator+

namespace inviwo::util {

std::shared_ptr<Volume> gradientVolume(
    const Volume& srcVolume, std::size_t channel,
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
         .valueAxis =
             Axis{.name = srcVolume.dataMap.valueAxis.name.empty()
                              ? "Gradient"
                              : fmt::format("{} Gradient", srcVolume.dataMap.valueAxis.name),
                  .unit = srcVolume.dataMap.valueAxis.unit / srcVolume.axes[0].unit}});
    auto dstVolume = getVolume(config);
    auto* dstRep =
        dynamic_cast<VolumeRAMPrecision<vec3>*>(dstVolume->getEditableRepresentation<VolumeRAM>());
    IVW_ASSERT(dstRep, "should exist");

    const auto dims = srcVolume.getDimensions();
    const auto delta = static_cast<dvec3>(dims);
    const auto wrapping = srcVolume.getWrapping();
    const auto gInv = dmat3{srcVolume.getCoordinateTransformer().getInverseMetricTensor()};
    const auto basis = dmat3{srcVolume.getCoordinateTransformer().getDataToWorldMatrix()};
    const auto dm = srcVolume.dataMap;
    const auto im = util::IndexMapper3D(dims);
    const auto* const srcRep = srcVolume.getRepresentation<VolumeRAM>();
    const auto srcFmt = srcRep->getDataFormatId();
    const auto dst = dstRep->getView();
    double max = 0.0;

    grid::centralDifferences(
        dims, wrapping,
        [&](size3_t voxel, const std::array<size3_t, 6>& positions, const dvec3& invStep) {
            std::array<double, 6> samples{};
            for (auto&& [pos, dest] : std::views::zip(positions, samples)) {
                grid::dispatch<dispatching::filter::All>(srcFmt, [&]<typename T> {
                    auto src = static_cast<const VolumeRAMPrecision<T>*>(srcRep)->getView();
                    const auto val = static_cast<double>(util::glmcomp(src[im(pos)], channel));
                    dest = dm.mapFromDataTo<DataMapper::Space::Value>(val);
                });
            }

            const auto Gx = (samples[0] - samples[1]) * delta.x * invStep.x;
            const auto Gy = (samples[2] - samples[3]) * delta.y * invStep.y;
            const auto Gz = (samples[4] - samples[5]) * delta.z * invStep.z;

            const auto grad = basis * gInv * dvec3{Gx, Gy, Gz};
            max = std::max(max, std::abs(glm::compMax(grad)));
            dst[im(voxel)] = static_cast<vec3>(grad);
        },
        progress, stop);

    dstVolume->dataMap.dataRange = dvec2(-max, max);
    dstVolume->dataMap.valueRange = dvec2(-max, max);
    dstVolume->discardHistograms();

    if (progress) progress(1.0);

    return dstVolume;
}

}  // namespace inviwo::util
