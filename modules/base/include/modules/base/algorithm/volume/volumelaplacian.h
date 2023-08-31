/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2023 Inviwo Foundation
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

#pragma once

#include <modules/base/basemoduledefine.h>  // for IVW_MODULE_BASE_API

#include <inviwo/core/datastructures/coordinatetransformer.h>           // for CoordinateSpace
#include <inviwo/core/datastructures/datamapper.h>                      // for DataMapper
#include <inviwo/core/datastructures/representationconverter.h>         // for RepresentationCon...
#include <inviwo/core/datastructures/representationconverterfactory.h>  // for RepresentationCon...
#include <inviwo/core/datastructures/unitsystem.h>                      // for Axis, Unit
#include <inviwo/core/datastructures/volume/volumeram.h>                // for VolumeRAM
#include <inviwo/core/util/formats.h>                                   // for DataFormat
#include <inviwo/core/util/glmcomp.h>                                   // for glmcomp
#include <inviwo/core/util/glmmat.h>                                    // for dmat4
#include <inviwo/core/util/glmutils.h>                                  // for same_extent
#include <inviwo/core/util/glmvec.h>                                    // for dvec3, dvec2, siz...
#include <inviwo/core/util/indexmapper.h>                               // for IndexMapper3D
#include <inviwo/core/util/templatesampler.h>                           // for TemplateVolumeSam...
#include <inviwo/core/util/volumeramutils.h>                            // for forEachVoxelParallel

#include <algorithm>      // for max
#include <array>          // for array
#include <cstdlib>        // for abs, size_t
#include <limits>         // for numeric_limits
#include <memory>         // for shared_ptr, share...
#include <type_traits>    // for remove_extent_t
#include <unordered_set>  // for unordered_set

#include <glm/gtx/matrix_operation.hpp>  // for diagonal3x3
#include <glm/mat3x3.hpp>                // for mat
#include <glm/mat4x4.hpp>                // for operator*, mat
#include <glm/vec3.hpp>                  // for operator-, operator+
#include <glm/vec4.hpp>                  // for operator*, operator+
#include <llnl-units/units_decl.hpp>          // for precise_unit

namespace inviwo {
class Volume;

namespace util {

enum class VolumeLaplacianPostProcessing { None, Normalized, SignNormalized, Scaled };

IVW_MODULE_BASE_API std::shared_ptr<Volume> volumeLaplacian(
    std::shared_ptr<const Volume> volume, VolumeLaplacianPostProcessing postProcessing,
    double scale);

namespace detail {

struct VolumeLaplacianDispatcher {
    using type = std::shared_ptr<Volume>;
    template <typename Result, typename T>
    std::shared_ptr<Volume> operator()(std::shared_ptr<const Volume> volume,
                                       VolumeLaplacianPostProcessing postProcessing, double scale);
};

template <typename Result, typename DF>
std::shared_ptr<Volume> VolumeLaplacianDispatcher::operator()(
    std::shared_ptr<const Volume> volume, VolumeLaplacianPostProcessing postProcessing,
    double scale) {
    using T = typename DF::type;
    constexpr size_t comp = DF::comp;
    using R = typename util::same_extent<T, float>::type;
    using Sampler = TemplateVolumeSampler<T, double, double>;

    static_assert(comp > 0, "zero extent");

    auto newVolume = std::make_shared<Volume>(volume->getDimensions(), DataFormat<R>::get());
    auto newData =
        static_cast<R*>(newVolume->template getEditableRepresentation<VolumeRAM>()->getData());
    newVolume->setModelMatrix(volume->getModelMatrix());
    newVolume->setWorldMatrix(volume->getWorldMatrix());

    const dmat4 m{volume->getCoordinateTransformer().getDataToWorldMatrix()};
    const auto a = m * dvec4(0, 0, 0, 1);
    const auto b = m * dvec4(dvec3(1.0) / dvec3(volume->getDimensions() - size3_t(1)), 1);
    const auto spacing = dvec3(b - a);

    const auto o = glm::diagonal3x3(spacing);
    const Sampler s(volume, CoordinateSpace::World);

    const util::IndexMapper3D index{volume->getDimensions()};

    const auto resDim = dvec3(1.0) / dvec3(volume->getDimensions() - size3_t(1));
    const auto resSpace2 = dvec3(1.0) / (spacing * spacing);

    auto minval(std::numeric_limits<double>::max());
    auto maxval(std::numeric_limits<double>::lowest());

    auto func = [&](const size3_t& pos) {
        const dvec3 world{m * dvec4((dvec3(pos) + dvec3(0.5)) * resDim, 1.0)};

        const auto center = 2.0 * s.sample(world);
        const auto D2x = (s.sample(world + o[0]) + center - s.sample(world - o[0])) * resSpace2.x;
        const auto D2y = (s.sample(world + o[1]) + center - s.sample(world - o[1])) * resSpace2.y;
        const auto D2z = (s.sample(world + o[2]) + center - s.sample(world - o[2])) * resSpace2.z;
        const auto laplacian = center + D2x + D2y + D2z;

        for (size_t i = 0; i < comp; ++i) {
            minval = glm::min(minval, util::glmcomp(laplacian, i));
            maxval = glm::max(maxval, util::glmcomp(laplacian, i));
        }
        newData[index(pos)] = static_cast<R>(laplacian);
    };

    util::forEachVoxelParallel(*volume->getRepresentation<VolumeRAM>(), func);

    // Make range symmetric
    auto rangemax = std::max(std::abs(minval), std::abs(maxval));

    switch (postProcessing) {
        case VolumeLaplacianPostProcessing::Normalized:
            util::forEachVoxelParallel(
                *volume->getRepresentation<VolumeRAM>(), [&](const size3_t& pos) {
                    newData[index(pos)] = (newData[index(pos)] + R{static_cast<float>(rangemax)}) /
                                          R{static_cast<float>(2.0 * rangemax)};
                });
            newVolume->dataMap_.dataRange = dvec2(0.0, 1.0);
            newVolume->dataMap_.valueRange = dvec2(0.0, 1.0);
            break;
        case VolumeLaplacianPostProcessing::SignNormalized:
            util::forEachVoxelParallel(
                *volume->getRepresentation<VolumeRAM>(), [&](const size3_t& pos) {
                    newData[index(pos)] = (newData[index(pos)] + R{static_cast<float>(rangemax)}) /
                                              R{static_cast<float>(rangemax)} -
                                          R{1.0f};
                });
            newVolume->dataMap_.dataRange = dvec2(-1.0, 1.0);
            newVolume->dataMap_.valueRange = dvec2(-1.0, 1.0);
            break;
        case VolumeLaplacianPostProcessing::Scaled:
            util::forEachVoxelParallel(
                *volume->getRepresentation<VolumeRAM>(), [&](const size3_t& pos) {
                    newData[index(pos)] = newData[index(pos)] * R{static_cast<float>(scale)};
                });
            newVolume->dataMap_.dataRange = dvec2(-rangemax * scale, rangemax * scale);
            newVolume->dataMap_.valueRange = dvec2(-rangemax * scale, rangemax * scale);
            break;
        case VolumeLaplacianPostProcessing::None:
        default:
            newVolume->dataMap_.dataRange = dvec2(-rangemax, rangemax);
            newVolume->dataMap_.valueRange = dvec2(-rangemax, rangemax);
            break;
    }

    newVolume->axes = volume->axes;
    newVolume->dataMap_.valueAxis.name = "laplacian";
    newVolume->dataMap_.valueAxis.unit =
        volume->dataMap_.valueAxis.unit / volume->axes[0].unit / volume->axes[0].unit;

    return newVolume;
}
}  // namespace detail

}  // namespace util

}  // namespace inviwo
