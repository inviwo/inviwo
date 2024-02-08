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

#include <modules/base/algorithm/volume/volumelaplacian.h>

#include <inviwo/core/datastructures/coordinatetransformer.h>           // for CoordinateSpace
#include <inviwo/core/datastructures/datamapper.h>                      // for DataMapper
#include <inviwo/core/datastructures/representationconverter.h>         // for RepresentationCon...
#include <inviwo/core/datastructures/representationconverterfactory.h>  // for RepresentationCon...
#include <inviwo/core/datastructures/unitsystem.h>                      // for Axis, Unit
#include <inviwo/core/datastructures/volume/volume.h>                   // for Volume
#include <inviwo/core/datastructures/volume/volumeram.h>                // for VolumeRAM
#include <inviwo/core/util/formatdispatching.h>                         // for dispatch, All
#include <inviwo/core/util/formats.h>                                   // for DataFormat
#include <inviwo/core/util/glmcomp.h>                                   // for glmcomp
#include <inviwo/core/util/glmmat.h>                                    // for dmat4
#include <inviwo/core/util/glmutils.h>                                  // for same_extent
#include <inviwo/core/util/glmvec.h>                                    // for dvec3, dvec2, siz...
#include <inviwo/core/util/indexmapper.h>                               // for IndexMapper3D
#include <inviwo/core/util/templatesampler.h>                           // for TemplateVolumeSam...
#include <inviwo/core/util/volumeramutils.h>                            // for forEachVoxelParallel

#include <functional>     // for __base
#include <unordered_map>  // for unordered_map
#include <vector>         // for vector
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
#include <glm/common.hpp>                // for max, min, mix
#include <glm/vec2.hpp>                  // for operator+, operator*

namespace inviwo {

std::shared_ptr<Volume> util::volumeLaplacian(std::shared_ptr<const Volume> volume,
                                              VolumeLaplacianPostProcessing postProcessing,
                                              double scale) {

    return dispatching::singleDispatch<std::shared_ptr<Volume>, dispatching::filter::All>(
        volume->getDataFormat()->getId(), [&]<typename T>() -> std::shared_ptr<Volume> {
            using R = typename util::same_extent<T, float>::type;

            auto srcRAM =
                static_cast<const VolumeRAMPrecision<T>*>(volume->getRepresentation<VolumeRAM>());

            const auto dims = srcRAM->getDimensions();

            auto dstRAM = std::make_shared<VolumeRAMPrecision<R>>(
                dims, swizzlemasks::rgba, InterpolationType::Linear, srcRAM->getWrapping());

            auto newVolume = std::make_shared<Volume>(dstRAM);
            newVolume->setModelMatrix(volume->getModelMatrix());
            newVolume->setWorldMatrix(volume->getWorldMatrix());
            newVolume->axes = volume->axes;
            newVolume->dataMap_.valueAxis.name = "laplacian";
            newVolume->dataMap_.valueAxis.unit =
                volume->dataMap_.valueAxis.unit / volume->axes[0].unit / volume->axes[0].unit;

            const dmat4 m{volume->getCoordinateTransformer().getDataToWorldMatrix()};
            const auto a = m * dvec4(0, 0, 0, 1);
            const auto b = m * dvec4(dvec3(1.0) / dvec3(volume->getDimensions() - size3_t(1)), 1);
            const auto spacing = dvec3(b - a);

            const auto o = glm::diagonal3x3(spacing);

            using Sampler = TemplateVolumeSampler<T, double, double>;
            const Sampler s(volume, CoordinateSpace::World);

            const util::IndexMapper3D index{volume->getDimensions()};

            const auto resDim = dvec3(1.0) / dvec3(volume->getDimensions() - size3_t(1));
            const auto resSpace2 = dvec3(1.0) / (spacing * spacing);

            auto minval(std::numeric_limits<double>::max());
            auto maxval(std::numeric_limits<double>::lowest());

            auto newData = dstRAM->getView();
            auto func = [&](const size3_t& pos) {
                const dvec3 world{m * dvec4((dvec3(pos) + dvec3(0.5)) * resDim, 1.0)};

                const auto center = 2.0 * s.sample(world);
                const auto D2x =
                    (s.sample(world + o[0]) + center - s.sample(world - o[0])) * resSpace2.x;
                const auto D2y =
                    (s.sample(world + o[1]) + center - s.sample(world - o[1])) * resSpace2.y;
                const auto D2z =
                    (s.sample(world + o[2]) + center - s.sample(world - o[2])) * resSpace2.z;
                const auto laplacian = center + D2x + D2y + D2z;

                if constexpr (1 < util::extent_v<T>) {
                    minval = glm::min(minval, glm::compMin(laplacian));
                    maxval = glm::max(maxval, glm::compMax(laplacian));
                } else {
                    minval = glm::min(minval, laplacian);
                    maxval = glm::max(maxval, laplacian);
                }

                newData[index(pos)] = static_cast<R>(laplacian);
            };

            util::forEachVoxelParallel(dims, func);

            // Make range symmetric
            auto rangeMax = std::max(std::abs(minval), std::abs(maxval));

            switch (postProcessing) {
                case VolumeLaplacianPostProcessing::Normalized:
                    util::forEachVoxelParallel(dims, [&](const size3_t& pos) {
                        newData[index(pos)] =
                            (newData[index(pos)] + R{static_cast<float>(rangeMax)}) /
                            R{static_cast<float>(2.0 * rangeMax)};
                    });
                    newVolume->dataMap_.dataRange = dvec2(0.0, 1.0);
                    newVolume->dataMap_.valueRange = dvec2(0.0, 1.0);
                    break;
                case VolumeLaplacianPostProcessing::SignNormalized:
                    util::forEachVoxelParallel(dims, [&](const size3_t& pos) {
                        newData[index(pos)] =
                            (newData[index(pos)] + R{static_cast<float>(rangeMax)}) /
                                R{static_cast<float>(rangeMax)} -
                            R{1.0f};
                    });
                    newVolume->dataMap_.dataRange = dvec2(-1.0, 1.0);
                    newVolume->dataMap_.valueRange = dvec2(-1.0, 1.0);
                    break;
                case VolumeLaplacianPostProcessing::Scaled:
                    util::forEachVoxelParallel(dims, [&](const size3_t& pos) {
                        newData[index(pos)] = newData[index(pos)] * R{static_cast<float>(scale)};
                    });
                    newVolume->dataMap_.dataRange = dvec2(-rangeMax * scale, rangeMax * scale);
                    newVolume->dataMap_.valueRange = dvec2(-rangeMax * scale, rangeMax * scale);
                    break;
                case VolumeLaplacianPostProcessing::None:
                default:
                    newVolume->dataMap_.dataRange = dvec2(-rangeMax, rangeMax);
                    newVolume->dataMap_.valueRange = dvec2(-rangeMax, rangeMax);
                    break;
            }

            return newVolume;
        });
}

}  // namespace inviwo
