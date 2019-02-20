/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2019 Inviwo Foundation
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

#ifndef IVW_VOLUMELAPLACIANALGO_H
#define IVW_VOLUMELAPLACIANALGO_H

#include <modules/base/basemoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/datastructures/volume/volume.h>
#include <inviwo/core/util/volumeramutils.h>
#include <inviwo/core/util/indexmapper.h>
#include <inviwo/core/util/templatesampler.h>

namespace inviwo {

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

    newVolume->dataMap_.valueUnit = "Laplacian";

    return newVolume;
}
}  // namespace detail

}  // namespace util

}  // namespace inviwo

#endif  // IVW_VOLUMELAPLACIANALGO_H
