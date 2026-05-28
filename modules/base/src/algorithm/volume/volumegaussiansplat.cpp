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

#include <modules/base/algorithm/volume/volumegaussiansplat.h>

#include <inviwo/core/datastructures/volume/volume.h>
#include <inviwo/core/datastructures/volume/volumeram.h>
#include <inviwo/core/datastructures/volume/volumeramprecision.h>
#include <inviwo/core/util/exception.h>
#include <inviwo/core/util/indexmapper.h>
#include <inviwo/core/util/threadutil.h>

#include <algorithm>
#include <cmath>
#include <future>
#include <vector>

#include <glm/common.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/mat3x3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

namespace inviwo {
namespace util {

std::string_view enumToStr(SplatKernel k) {
    switch (k) {
        case SplatKernel::Gaussian:
            return "Gaussian";
        case SplatKernel::Epanechnikov:
            return "Epanechnikov";
        case SplatKernel::Triangular:
            return "Triangular";
        case SplatKernel::Uniform:
            return "Uniform";
    }
    return "Unknown";
}

namespace {

// Evaluate the kernel given the squared world-space distance d2, the per-point bandwidth h and
// the support radius s = cutoff * h. Assumes d2 <= s*s; caller is responsible for the cutoff check.
template <SplatKernel K>
inline float evalKernel(float d2, float h, float s) {
    if constexpr (K == SplatKernel::Gaussian) {
        return std::exp(-0.5f * d2 / (h * h));
    } else if constexpr (K == SplatKernel::Epanechnikov) {
        const float t = d2 / (s * s);
        return 1.0f - t;
    } else if constexpr (K == SplatKernel::Triangular) {
        const float t = std::sqrt(d2) / s;
        return 1.0f - t;
    } else if constexpr (K == SplatKernel::Uniform) {
        return 1.0f;
    } else {
        return 0.0f;
    }
}

template <SplatKernel K>
void splatImpl(std::span<const vec3> worldPositions, std::span<const float> radii,
               const GaussianSplatSettings& settings, float* data) {

    const auto dims = settings.dimensions;
    const mat4 worldToData = glm::inverse(settings.modelMatrix);
    // basis matrix in world space: columns are the volume-axis vectors in world units
    const mat3 basisWorld{settings.modelMatrix};
    // Conservative voxel-size in world space along each axis (used to compute the
    // footprint AABB in index space from a world-space radius). For a non-orthogonal basis we use
    // the column lengths; the per-point AABB is still conservative because we recompute the exact
    // world-space distance per voxel.
    const vec3 voxelSizeWorld =
        vec3{glm::length(basisWorld[0]), glm::length(basisWorld[1]), glm::length(basisWorld[2])} /
        vec3{dims};

    const float defaultBandwidth = settings.defaultSigma;
    const float cutoff = settings.cutoff;
    const bool perPointRadii = !radii.empty();

    const std::size_t poolSize = util::getPoolSize();
    std::size_t jobs = settings.jobs;
    if (jobs == 0) {
        jobs = poolSize > 0 ? poolSize : 1;
    }
    jobs = std::min<std::size_t>(jobs, dims.z);
    if (jobs == 0) jobs = 1;

    auto processSlab = [&](std::size_t zStart, std::size_t zStop) {
        util::IndexMapper3D idx(dims);
        const auto dimsI = ivec3{dims};

        for (std::size_t p = 0; p < worldPositions.size(); ++p) {
            const float h = perPointRadii ? radii[p] : defaultBandwidth;
            if (!(h > 0.0f)) continue;
            const float support = cutoff * h;
            const float support2 = support * support;

            // Conservative footprint in index space
            const vec3 footprintWorld{support, support, support};
            const vec3 footprintIndex = footprintWorld / voxelSizeWorld;
            // world -> data [0,1]^3 -> index [0, dims)
            const vec3 centerData = vec3{worldToData * vec4{worldPositions[p], 1.0f}};
            const vec3 centerIndex = centerData * vec3{dims};

            const ivec3 lo = ivec3{glm::floor(centerIndex - footprintIndex)};
            const ivec3 hi = ivec3{glm::ceil(centerIndex + footprintIndex)} + ivec3{1};

            const int z0 = std::max<int>(lo.z, static_cast<int>(zStart));
            const int z1 = std::min<int>(hi.z, static_cast<int>(zStop));
            if (z0 >= z1) continue;

            const int y0 = std::max(lo.y, 0);
            const int y1 = std::min(hi.y, dimsI.y);
            if (y0 >= y1) continue;

            const int x0 = std::max(lo.x, 0);
            const int x1 = std::min(hi.x, dimsI.x);
            if (x0 >= x1) continue;

            const vec3 pw = worldPositions[p];

            for (int z = z0; z < z1; ++z) {
                for (int y = y0; y < y1; ++y) {
                    for (int x = x0; x < x1; ++x) {
                        const vec3 voxelIndex{static_cast<float>(x), static_cast<float>(y),
                                              static_cast<float>(z)};
                        const vec3 voxelWorld =
                            vec3{settings.modelMatrix *
                                 vec4{voxelIndex / vec3{dims}, 1.0f}};
                        const vec3 d = voxelWorld - pw;
                        const float d2 = glm::dot(d, d);
                        if (d2 > support2) continue;
                        const float contrib = evalKernel<K>(d2, h, support);
                        data[idx(size3_t{x, y, z})] += contrib;
                    }
                }
            }
        }
    };

    if (jobs == 1) {
        processSlab(0, dims.z);
        return;
    }

    std::vector<std::future<void>> futures;
    futures.reserve(jobs);
    for (std::size_t j = 0; j < jobs; ++j) {
        const std::size_t zStart = j * dims.z / jobs;
        const std::size_t zStop = (j + 1) * dims.z / jobs;
        if (zStart >= zStop) continue;
        futures.push_back(
            util::dispatchPool([=]() { processSlab(zStart, zStop); }));
    }
    for (auto& f : futures) f.wait();
}

}  // namespace

std::shared_ptr<Volume> gaussianSplat(std::span<const vec3> worldPositions,
                                      std::span<const float> radii,
                                      const GaussianSplatSettings& settings) {
    if (settings.dimensions.x == 0 || settings.dimensions.y == 0 || settings.dimensions.z == 0) {
        throw Exception("gaussianSplat: volume dimensions must be non-zero");
    }
    if (!(settings.defaultSigma > 0.0f)) {
        throw Exception("gaussianSplat: defaultSigma must be positive");
    }
    if (!(settings.cutoff > 0.0f)) {
        throw Exception("gaussianSplat: cutoff must be positive");
    }
    if (!radii.empty() && radii.size() != worldPositions.size()) {
        throw Exception("gaussianSplat: radii size does not match worldPositions size");
    }

    auto rep = std::make_shared<VolumeRAMPrecision<float>>(settings.dimensions);
    std::fill_n(rep->getDataTyped(), settings.dimensions.x * settings.dimensions.y *
                                         settings.dimensions.z,
                0.0f);

    if (!worldPositions.empty()) {
        float* data = rep->getDataTyped();
        switch (settings.kernel) {
            case SplatKernel::Gaussian:
                splatImpl<SplatKernel::Gaussian>(worldPositions, radii, settings, data);
                break;
            case SplatKernel::Epanechnikov:
                splatImpl<SplatKernel::Epanechnikov>(worldPositions, radii, settings, data);
                break;
            case SplatKernel::Triangular:
                splatImpl<SplatKernel::Triangular>(worldPositions, radii, settings, data);
                break;
            case SplatKernel::Uniform:
                splatImpl<SplatKernel::Uniform>(worldPositions, radii, settings, data);
                break;
        }
    }

    auto volume = std::make_shared<Volume>(rep);
    volume->setModelMatrix(dmat4{settings.modelMatrix});

    // Set a reasonable data range based on the actual contents
    const float* data = rep->getDataTyped();
    const std::size_t count =
        settings.dimensions.x * settings.dimensions.y * settings.dimensions.z;
    float maxVal = 0.0f;
    for (std::size_t i = 0; i < count; ++i) {
        maxVal = std::max(maxVal, data[i]);
    }
    if (maxVal <= 0.0f) maxVal = 1.0f;
    volume->dataMap.dataRange = dvec2{0.0, static_cast<double>(maxVal)};
    volume->dataMap.valueRange = volume->dataMap.dataRange;

    return volume;
}

}  // namespace util
}  // namespace inviwo
