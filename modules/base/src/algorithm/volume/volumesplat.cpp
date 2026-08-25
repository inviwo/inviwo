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

#include <modules/base/algorithm/volume/volumesplat.h>

#include <inviwo/core/datastructures/volume/volume.h>
#include <inviwo/core/datastructures/volume/volumeram.h>
#include <inviwo/core/datastructures/volume/volumeramprecision.h>
#include <inviwo/core/util/exception.h>
#include <inviwo/core/util/indexmapper.h>
#include <inviwo/core/util/threadutil.h>
#include <inviwo/core/util/glm.h>

#include <glm/common.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/mat3x3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

#include <algorithm>
#include <cmath>
#include <future>
#include <vector>
#include <numbers>

namespace inviwo::util {

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

// Rational approximation of erfinv (Winitzki, 2008)
double erfinv(double x) {
    // Clamp to valid domain (-1, 1)
    x = std::clamp(x, -1.0, 1.0);
    const double a = 0.147;  // Tuning constant
    const double ln1mx2 = std::log(1.0 - x * x);
    const double part1 = (2.0 / (std::numbers::pi * a)) + (ln1mx2 / 2.0);
    const double part2 = ln1mx2 / a;

    const double sign = (x >= 0) ? 1.0 : -1.0;
    return sign * std::sqrt(std::sqrt(part1 * part1 - part2) - part1);
}
double gaussian_support(double sigma, double epsilon) {
    return sigma * std::numbers::sqrt2 * erfinv(1.0 - epsilon);
}

// Evaluate the kernel given the squared world-space distance r2, the per-point size s
// Assumes r2 <= s*s; caller is responsible for the cutoff check.
template <SplatKernel K>
inline float evalKernel(float r2, float s) {
    if constexpr (K == SplatKernel::Gaussian) {
        return std::exp(-0.5f * r2 / (s * s));
    } else if constexpr (K == SplatKernel::Epanechnikov) {
        return 1.0f - r2 / (s * s);
    } else if constexpr (K == SplatKernel::Triangular) {
        return 1.0f - std::sqrt(r2) / s;
    } else if constexpr (K == SplatKernel::Uniform) {
        return 1.0f;
    } else {
        return 0.0f;
    }
}

template <SplatKernel K>
inline vec4 evalKernelGrad(vec3 r, float s) {
    const float r2 = glm::dot(r, r);
    float k = evalKernel<K>(r2, s);

    if constexpr (K == SplatKernel::Gaussian) {
        float f = -k / (s * s);
        return {k, r.x * f, r.y * f, r.z * f};
    } else if constexpr (K == SplatKernel::Epanechnikov) {
        float f = 2.0f / (s * s);
        return {k, r.x * f, r.y * f, r.z * f};
    } else if constexpr (K == SplatKernel::Triangular) {
        float f = 2.0f / (s * std::sqrt(r2));
        return {k, r.x * f, r.y * f, r.z * f};
    } else if constexpr (K == SplatKernel::Uniform) {
        return {k, 0.0f, 0.0f, 0.0f};
    } else {
        return {0.0f, 0.0f, 0.0f, 0.0f};
    }
}

template <SplatKernel K>
inline float kernelNormFactor(float s) {
    if constexpr (K == SplatKernel::Gaussian) {
        return std::numbers::inv_sqrtpi_v<float> / std::numbers::sqrt2_v<float> / s;
    } else if constexpr (K == SplatKernel::Epanechnikov) {
        return 3.0f / (s * 4.0f);
    } else if constexpr (K == SplatKernel::Triangular) {
        return 1.0f / s;
    } else if constexpr (K == SplatKernel::Uniform) {
        return 1.0f / 2.0f * s;
    } else {
        return 1.0f;
    }
}

template <SplatKernel K>
void splatImpl(const SplatInput& input, const SplatSettings& settings, mat4 indexToWorld,
               mat4 worldToIndex, float* data,
               std::vector<std::function<vec2(const std::function<void(double)>&,
                                              const std::function<bool()>&)>>& jobs) {

    const auto dims = settings.dimensions;

    // basis matrix in world space: columns are the volume-axis vectors in world units
    const mat3 basisWorld{settings.modelMatrix};
    // Conservative voxel-size in world space along each axis (used to compute the
    // footprint AABB in index space from a world-space radius). For a non-orthogonal basis we use
    // the column lengths; the per-point AABB is still conservative because we recompute the exact
    // world-space distance per voxel.
    const vec3 voxelSizeWorld =
        vec3{glm::length(basisWorld[0]), glm::length(basisWorld[1]), glm::length(basisWorld[2])} /
        vec3{dims};

    const mat4 pointTransform{input.pointTransform};

    const float size = settings.size;
    const float weight = settings.weight;
    const float error = settings.error;
    const bool perPointSize = !input.sizes.empty();
    const bool perPointWeight = !input.weights.empty();

    const std::size_t poolSize = util::getPoolSize();
    std::size_t nJobs = settings.jobs;
    if (nJobs == 0) {
        nJobs = poolSize > 0 ? poolSize : 1;
    }
    nJobs = std::min(nJobs, dims.z);
    if (nJobs == 0) nJobs = 1;

    auto processSlab = [&](std::size_t zStart, std::size_t zStop) {
        return [zStart, zStop, positions = input.positions, sizes = input.sizes,
                weights = input.weights, indexToWorld, worldToIndex, data, dims, size, weight,
                error, perPointSize, perPointWeight, voxelSizeWorld, pointTransform](
                   const std::function<void(double)>& progress, const std::function<bool()>& stop) {
            const util::IndexMapper3D idx(dims);
            const auto dimsI = ivec3{dims};

            auto minMax =
                vec2{std::numeric_limits<float>::max(), std::numeric_limits<float>::lowest()};

            for (std::size_t p = 0; p < positions.size(); ++p) {
                if (stop && stop()) return minMax;
                if (progress) {
                    progress(static_cast<double>(p) / static_cast<double>(positions.size()));
                }
                const vec3 pw = util::transformPos(pointTransform, positions[p]);
                const float s = (perPointSize ? sizes[p] : 1.0f) * size;
                const float w = (perPointWeight ? weights[p] : 1.0f) * weight;
                if (s <= 0.0f || w <= 0.0f) continue;

                const float support = [&]() {
                    if constexpr (K == SplatKernel::Gaussian) {
                        return static_cast<float>(gaussian_support(s, error));
                    } else {
                        (void)error;
                        return s;
                    }
                }();

                // Conservative footprint in index space
                const vec3 footprintWorld{support, support, support};
                const vec3 footprintIndex = footprintWorld / voxelSizeWorld;
                const vec3 centerIndex = vec3{worldToIndex * vec4{pw, 1.0f}};

                const auto lo = ivec3{glm::floor(centerIndex - footprintIndex)};
                const auto hi = ivec3{glm::ceil(centerIndex + footprintIndex)} + ivec3{1};

                const int z0 = std::max(lo.z, static_cast<int>(zStart));
                const int z1 = std::min(hi.z, static_cast<int>(zStop));
                if (z0 >= z1) continue;

                const int y0 = std::max(lo.y, 0);
                const int y1 = std::min(hi.y, dimsI.y);
                if (y0 >= y1) continue;

                const int x0 = std::max(lo.x, 0);
                const int x1 = std::min(hi.x, dimsI.x);
                if (x0 >= x1) continue;

                const float support2 = support * support;

                const auto f = w * kernelNormFactor<K>(s);
                for (int z = z0; z < z1; ++z) {
                    for (int y = y0; y < y1; ++y) {
                        for (int x = x0; x < x1; ++x) {
                            const auto voxelWorld = vec3{indexToWorld * vec4{ivec3{x, y, z}, 1.0f}};
                            const vec3 d = voxelWorld - pw;
                            const float d2 = glm::dot(d, d);
                            if (d2 > support2) continue;
                            const float contrib = f * evalKernel<K>(d2, s);
                            data[idx(size3_t{x, y, z})] += contrib;
                        }
                    }
                }
            }

            for (size_t z = zStart; z < zStop; ++z) {
                for (size_t y = 0; y < dims.y; ++y) {
                    for (size_t x = 0; x < dims.x; ++x) {
                        const float val = data[idx(size3_t{x, y, z})];
                        minMax.x = std::min(minMax.x, val);
                        minMax.y = std::max(minMax.y, val);
                    }
                }
            }
            return minMax;
        };
    };

    if (nJobs == 1) {
        jobs.emplace_back(processSlab(0, dims.z));
    } else {
        for (std::size_t j = 0; j < nJobs; ++j) {
            const std::size_t zStart = j * dims.z / nJobs;
            const std::size_t zStop = (j + 1) * dims.z / nJobs;
            if (zStart >= zStop) continue;
            jobs.emplace_back(processSlab(zStart, zStop));
        }
    }
}

}  // namespace

std::pair<std::function<std::shared_ptr<Volume>(std::vector<vec2>)>,
          std::vector<std::function<vec2(const std::function<void(double)>&,
                                         const std::function<bool()>&)>>>
splatJobs(const SplatInput& input, const SplatSettings& settings) {
    if (settings.dimensions.x == 0 || settings.dimensions.y == 0 || settings.dimensions.z == 0) {
        throw Exception("Volume dimensions must be non-zero");
    }
    if (settings.size < 0.0f) {
        throw Exception("Size must be positive");
    }
    if (settings.weight < 0.0f) {
        throw Exception("Weight must be positive");
    }
    if (settings.error < 0.0f) {
        throw Exception("Error must be positive");
    }
    if (!input.sizes.empty() && input.sizes.size() != input.positions.size()) {
        throw Exception("Sizes size does not match positions size");
    }
    if (!input.weights.empty() && input.weights.size() != input.positions.size()) {
        throw Exception("Weights size does not match positions size");
    }

    auto rep = std::make_shared<VolumeRAMPrecision<float>>(settings.dimensions);
    std::fill_n(rep->getDataTyped(),
                settings.dimensions.x * settings.dimensions.y * settings.dimensions.z, 0.0f);

    auto volume = std::make_shared<Volume>(rep);
    volume->setModelMatrix(dmat4{settings.modelMatrix});
    volume->axes = settings.axes;
    volume->dataMap.valueAxis = settings.valueAxis;

    const auto indexToWorld = volume->getCoordinateTransformer().getIndexToWorldMatrix();
    const auto worldToIndex = volume->getCoordinateTransformer().getWorldToIndexMatrix();

    const std::function<std::shared_ptr<Volume>(std::vector<vec2>)> collect =
        [volume](const std::vector<vec2>& minMaxes) {
            vec2 minMax{std::numeric_limits<float>::max(), std::numeric_limits<float>::lowest()};
            for (auto& val : minMaxes) {
                minMax.x = std::min(minMax.x, val.x);
                minMax.y = std::max(minMax.y, val.y);
            }
            volume->dataMap.dataRange = dvec2{minMax};
            volume->dataMap.valueRange = volume->dataMap.dataRange;
            return volume;
        };

    std::vector<
        std::function<vec2(const std::function<void(double)>&, const std::function<bool()>&)>>
        jobs;
    if (!input.positions.empty()) {
        float* data = rep->getDataTyped();

        switch (settings.kernel) {
            case SplatKernel::Gaussian:
                splatImpl<SplatKernel::Gaussian>(input, settings, indexToWorld, worldToIndex, data,
                                                 jobs);
                break;
            case SplatKernel::Epanechnikov:
                splatImpl<SplatKernel::Epanechnikov>(input, settings, indexToWorld, worldToIndex,
                                                     data, jobs);
                break;
            case SplatKernel::Triangular:
                splatImpl<SplatKernel::Triangular>(input, settings, indexToWorld, worldToIndex,
                                                   data, jobs);
                break;
            case SplatKernel::Uniform:
                splatImpl<SplatKernel::Uniform>(input, settings, indexToWorld, worldToIndex, data,
                                                jobs);
                break;
        }
    }

    return {collect, jobs};
}

std::pair<std::function<std::shared_ptr<Volume>(std::vector<vec2>)>,
          std::vector<std::function<vec2(const std::function<void(double)>&,
                                         const std::function<bool()>&)>>>
splatJobs(std::span<const SplatInput> inputs, const SplatSettings& settings) {
    if (settings.dimensions.x == 0 || settings.dimensions.y == 0 || settings.dimensions.z == 0) {
        throw Exception("Volume dimensions must be non-zero");
    }
    if (settings.size < 0.0f) {
        throw Exception("Size must be positive");
    }
    if (settings.weight < 0.0f) {
        throw Exception("Weight must be positive");
    }
    if (settings.error < 0.0f) {
        throw Exception("Error must be positive");
    }
    for (const auto& input : inputs) {
        if (!input.sizes.empty() && input.sizes.size() != input.positions.size()) {
            throw Exception("Sizes size does not match positions size");
        }
        if (!input.weights.empty() && input.weights.size() != input.positions.size()) {
            throw Exception("Weights size does not match positions size");
        }
    }

    auto rep = std::make_shared<VolumeRAMPrecision<float>>(settings.dimensions);
    std::fill_n(rep->getDataTyped(),
                settings.dimensions.x * settings.dimensions.y * settings.dimensions.z, 0.0f);

    auto volume = std::make_shared<Volume>(rep);
    volume->setModelMatrix(dmat4{settings.modelMatrix});
    volume->axes = settings.axes;
    volume->dataMap.valueAxis = settings.valueAxis;

    const auto indexToWorld = volume->getCoordinateTransformer().getIndexToWorldMatrix();
    const auto worldToIndex = volume->getCoordinateTransformer().getWorldToIndexMatrix();

    const std::function<std::shared_ptr<Volume>(std::vector<vec2>)> collect =
        [volume](const std::vector<vec2>& minMaxes) {
            vec2 minMax{std::numeric_limits<float>::max(), std::numeric_limits<float>::lowest()};
            for (auto& val : minMaxes) {
                minMax.x = std::min(minMax.x, val.x);
                minMax.y = std::max(minMax.y, val.y);
            }
            volume->dataMap.dataRange = dvec2{minMax};
            volume->dataMap.valueRange = volume->dataMap.dataRange;
            return volume;
        };

    std::vector<
        std::function<vec2(const std::function<void(double)>&, const std::function<bool()>&)>>
        jobs;

    for (const auto& input : inputs) {
        if (!input.positions.empty()) {
            float* data = rep->getDataTyped();

            switch (settings.kernel) {
                case SplatKernel::Gaussian:
                    splatImpl<SplatKernel::Gaussian>(input, settings, indexToWorld, worldToIndex,
                                                     data, jobs);
                    break;
                case SplatKernel::Epanechnikov:
                    splatImpl<SplatKernel::Epanechnikov>(input, settings, indexToWorld,
                                                         worldToIndex, data, jobs);
                    break;
                case SplatKernel::Triangular:
                    splatImpl<SplatKernel::Triangular>(input, settings, indexToWorld, worldToIndex,
                                                       data, jobs);
                    break;
                case SplatKernel::Uniform:
                    splatImpl<SplatKernel::Uniform>(input, settings, indexToWorld, worldToIndex,
                                                    data, jobs);
                    break;
            }
        }
    }

    return {collect, jobs};
}

std::shared_ptr<Volume> splat(const SplatInput& input, const SplatSettings& settings) {

    auto [collect, jobs] = splatJobs(input, settings);

    if (util::getPoolSize() > 0) {
        std::vector<std::future<vec2>> futures;
        for (auto& job : jobs) {
            futures.emplace_back(util::dispatchPool([job]() { return job(nullptr, nullptr); }));
        }
        return collect(futures | std::views::transform([](auto& f) { return f.get(); }) |
                       std::ranges::to<std::vector>());
    } else {
        std::vector<vec2> minMaxes;
        for (auto& job : jobs) {
            minMaxes.push_back(job(nullptr, nullptr));
        }
        return collect(minMaxes);
    }
}

}  // namespace inviwo::util
