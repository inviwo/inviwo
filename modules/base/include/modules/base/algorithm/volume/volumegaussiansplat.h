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

#include <inviwo/core/util/glmmat.h>
#include <inviwo/core/util/glmvec.h>

#include <cstddef>
#include <memory>
#include <span>
#include <string_view>

namespace inviwo {
class Volume;

namespace util {

/**
 * Splatting kernels supported by gaussianSplat. All kernels are evaluated in world space using
 * a per-point bandwidth @c h (either from the radii span or @c defaultSigma). The kernel has
 * compact support of @c cutoff * h (the Gaussian is truncated at that distance).
 */
enum class SplatKernel {
    Gaussian,      //!< exp(-d^2 / (2 h^2)), truncated at d = cutoff*h
    Epanechnikov,  //!< max(0, 1 - (d / (cutoff*h))^2)
    Triangular,    //!< max(0, 1 - d / (cutoff*h))
    Uniform        //!< 1 if d < cutoff*h, else 0
};

IVW_MODULE_BASE_API std::string_view enumToStr(SplatKernel k);

struct IVW_MODULE_BASE_API GaussianSplatSettings {
    size3_t dimensions{64};       //!< Output volume dimensions (voxels)
    mat4 modelMatrix{1.0f};       //!< basis (columns 0..2) and offset (column 3) in world space
    SplatKernel kernel{SplatKernel::Gaussian};
    float defaultSigma{0.05f};    //!< Bandwidth used when no per-point radii are provided
    float cutoff{3.0f};           //!< Kernel support radius in units of the per-point bandwidth
    std::size_t jobs{0};          //!< Number of Z-slabs to split the volume into; 0 = auto
};

/**
 * Splat a set of world-space points into a scalar volume using a chosen kernel.
 *
 * For each point @c p_i with bandwidth @c h_i the volume accumulates the kernel evaluated at the
 * world-space distance between the point and each voxel center inside the per-point footprint
 * @c [p_i - cutoff*h_i, p_i + cutoff*h_i]. Points whose footprint lies entirely outside the volume
 * are skipped.
 *
 * The work is parallelised by splitting the output volume into a number of disjoint Z-slabs. Each
 * worker iterates over all points but only writes voxels inside its slab, so no atomics or locks
 * are needed.
 *
 * @param worldPositions point positions in world space
 * @param radii          per-point bandwidth in world units; if empty, @c defaultSigma is used for
 *                       every point. Must otherwise have the same size as @p worldPositions.
 * @param settings       output configuration, kernel selection and truncation
 * @return a single-channel float32 @c Volume with model matrix matching @c settings.modelMatrix
 *
 * @throws Exception if @p radii is non-empty and its size differs from @p worldPositions, or if
 *         @c settings.dimensions has a zero component, or if @c settings.defaultSigma /
 *         @c settings.cutoff are non-positive.
 */
IVW_MODULE_BASE_API std::shared_ptr<Volume> gaussianSplat(std::span<const vec3> worldPositions,
                                                          std::span<const float> radii,
                                                          const GaussianSplatSettings& settings);

}  // namespace util
}  // namespace inviwo
