/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2026 Inviwo Foundation
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

#include <modules/base/basemoduledefine.h>

#include <inviwo/core/datastructures/unitsystem.h>
#include <inviwo/core/util/glmmat.h>
#include <inviwo/core/util/glmvec.h>

#include <cstddef>
#include <memory>
#include <span>
#include <string_view>
#include <functional>

namespace inviwo {
class Volume;

namespace util {

/**
 * Splatting kernels supported by splat. All kernels are evaluated in world space using
 * a per-point size @c s (either from the radii span or @c size).
 */
enum class SplatKernel : std::uint8_t {
    Gaussian,      //!< exp(-d^2 / (2 h^2)), truncated at d = cutoff*h
    Epanechnikov,  //!< max(0, 1 - (d / (cutoff*h))^2)
    Triangular,    //!< max(0, 1 - d / (cutoff*h))
    Uniform        //!< 1 if d < cutoff*h, else 0
};

IVW_MODULE_BASE_API std::string_view enumToStr(SplatKernel k);

struct IVW_MODULE_BASE_API SplatSettings {
    size3_t dimensions{64};  //!< Output volume dimensions (voxels)
    mat4 modelMatrix{1.0f};  //!< basis (columns 0..2) and offset (column 3) in world space

    std::array<Axis, 3> axes = util::defaultAxes<3>();
    Axis valueAxis = {};
    SplatKernel kernel{SplatKernel::Gaussian};
    float size{1.0f};
    float weight{1.0f};
    float error{0.01f};   //!< Used to calculate the support size in case of a Gaussian kernel
    std::size_t jobs{0};  //!< Number of Z-slabs to split the volume into; 0 = auto
};

struct IVW_MODULE_BASE_API SplatInput {
    std::span<const vec3> positions{};
    std::span<const float> sizes{};
    std::span<const float> weights{};

    mat4 pointTransform{1.0f};  //!< applied to each position before splatting, e.g. data to world
};

/**
 * Splat a set of points into a scalar volume using a chosen kernel.
 *
 * For each point @c p_i with size @c s_i the volume accumulates the kernel evaluated at the
 * world-space distance between the point and each voxel center inside the per-point footprint.
 * Points whose footprint lies entirely outside the volume are skipped.
 *
 * The work is parallelized by splitting the output volume into a number of disjoint Z-slabs. Each
 * worker iterates over all points but only writes voxels inside its slab, so no atomics or locks
 * are needed.
 *
 * @param positions      point positions, will be transformed with @c settings.pointTransform before
 *                       calculating distances
 * @param radii          per-point bandwidth in world units; if empty, @c defaultSigma is used for
 *                       every point. Must otherwise have the same size as @p positions.
 * @param weights        per-point weights to accumulate; if empty, all points are weighted equally.
 *                       Must otherwise have the same size as @p positions.
 * @param settings       output configuration, kernel selection and truncation
 * @return a single-channel float32 @c Volume with model matrix matching @c settings.modelMatrix
 *
 * @throws Exception if @p radii is non-empty and its size differs from @p positions, or if
 *         @p weights is non-empty and its size differs from @p positions, or if
 *         @c settings.dimensions has a zero component, or if @c settings.defaultSigma /
 *         @c settings.cutoff are non-positive.
 */
IVW_MODULE_BASE_API std::shared_ptr<Volume> splat(const SplatInput& input,
                                                  const SplatSettings& settings);

IVW_MODULE_BASE_API
std::pair<std::function<std::shared_ptr<Volume>(std::vector<vec2>)>,
          std::vector<std::function<vec2(const std::function<void(double)>&,
                                         const std::function<bool()>&)>>>
splatJobs(const SplatInput& input, const SplatSettings& settings);

IVW_MODULE_BASE_API
std::pair<std::function<std::shared_ptr<Volume>(std::vector<vec2>)>,
          std::vector<std::function<vec2(const std::function<void(double)>&,
                                         const std::function<bool()>&)>>>
splatJobs(std::span<const SplatInput> inputs, const SplatSettings& settings);


}  // namespace util
}  // namespace inviwo
