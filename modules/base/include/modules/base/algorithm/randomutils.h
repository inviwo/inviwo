/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2024 Inviwo Foundation
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

#include <inviwo/core/datastructures/image/image.h>
#include <inviwo/core/datastructures/image/layer.h>
#include <inviwo/core/datastructures/image/layerram.h>
#include <inviwo/core/datastructures/image/layerramprecision.h>

#include <inviwo/core/datastructures/volume/volume.h>
#include <inviwo/core/datastructures/volume/volumeram.h>
#include <inviwo/core/datastructures/volume/volumeramprecision.h>

#include <inviwo/core/util/zip.h>
#include <inviwo/core/util/imagesampler.h>

#include <random>
#include <numbers>
#include <bit>

namespace inviwo {

namespace util {

/**
 * Compute the @p n th point of the Halton sequence with base @p base on the open range (0 1).
 * @see https://en.wikipedia.org/wiki/Halton_sequence
 * @param base what base to use to generate fractions
 * @param n  index of the nth point of the Halton sequence
 * @return nth
 */
template <typename T>
T haltonSequence(size_t n, size_t base) {
    T val(0);
    size_t denom = base;
    size_t a = n;
    while (a != 0) {
        const auto rest = a % base;
        const auto b = (T(1.0) / static_cast<T>(denom));
        val += rest * b;
        a -= rest;
        a /= base;
        denom *= base;
    }
    return val;
}

/**
 * Generate a Layer with sparse noise based on a pair of two Halton Sequences.
 * @see haltonSequence(size_t base, size_t numberOfPoints)
 * @see https://en.wikipedia.org/wiki/Halton_sequence
 * @param dims size of the resulting layer.
 * @param numberOfPoints number of points to generate
 * @param baseX base used for the fractions to generate the x-values
 * @param baseY base used for the fractions to generate the y-values
 */
template <typename T>
std::shared_ptr<Layer> haltonSequence(size2_t dims, size_t numberOfPoints, size_t baseX = 2,
                                      size_t baseY = 3) {
    auto layerRam = std::make_shared<LayerRAMPrecision<T>>(LayerReprConfig{
        .dimensions = dims,
        .swizzleMask = swizzlemasks::defaultData(1),
    });

    const util::IndexMapper2D index(dims);
    auto data = layerRam->getView();

    const auto dimsf = vec2(dims - size2_t(1));
    for (size_t i = 1; i <= numberOfPoints; ++i) {
        auto coord = dimsf * vec2{haltonSequence<T>(i, baseX), haltonSequence<T>(i, baseY)};
        data[index(coord)] = 1;
    }
    return std::make_shared<Layer>(layerRam);
}
/**
 * Generate an Volume with sparse noise based on a three Halton Sequences.
 * @see haltonSequence(size_t base, size_t numberOfPoints)
 * @see https://en.wikipedia.org/wiki/Halton_sequence
 * @param dims size of the resulting Volume.
 * @param numberOfPoints number of points to generate
 * @param baseX base used for the fractions to generate the x-values
 * @param baseY base used for the fractions to generate the y-values
 * @param baseZ base used for the fractions to generate the z-values
 */
template <typename T>
std::shared_ptr<Volume> haltonSequence(size3_t dims, size_t numberOfPoints, size_t baseX = 2,
                                       size_t baseY = 3, size_t baseZ = 5) {
    auto volumeRam = std::make_shared<VolumeRAMPrecision<T>>(VolumeReprConfig{
        .dimensions = dims,
        .swizzleMask = swizzlemasks::defaultData(1),
    });

    const util::IndexMapper3D index(dims);
    auto data = volumeRam->getView();

    const auto dimsf = vec3(dims - size3_t(1));
    for (size_t i = 1; i <= numberOfPoints; ++i) {
        auto coord = dimsf * vec3{haltonSequence<T>(i, baseX), haltonSequence<T>(i, baseY),
                                  haltonSequence<T>(i, baseZ)};
        data[index(coord)] = 1;
    }
    return std::make_shared<Volume>(volumeRam);
}

namespace detail {
struct RandomNumberRangeValues {
    template <typename T>
    inline static constexpr T min() {
        if constexpr (std::is_floating_point_v<T>) {
            return 0;
        } else {
            return std::numeric_limits<T>::lowest();
        }
    }

    template <typename T>
    inline static constexpr T max() {
        if constexpr (std::is_floating_point_v<T>) {
            return 1;
        } else {
            return std::numeric_limits<T>::max();
        }
    }
};
}  // namespace detail

/**
 * Generates a random value of type T in the range [\p min, \p max).
 * Will use the distribution of the random number generator \p rng.
 * I.e when used with e.g. std::mt19937 it generates uniformly distributed number between \p min
 * and \p max.
 * \p min/\p max defaults to 0 and 1 for floating point types otherwise default to
 * std::numeric_limits::lowest()/std::numeric_limits::max()
 */
template <typename T, typename RNG,
          typename F = std::conditional_t<std::is_same_v<float, T>, float, double>>
inline static T randomNumber(RNG& rng, T min = detail::RandomNumberRangeValues::min<T>(),
                             T max = detail::RandomNumberRangeValues::max<T>()) {
    const auto t = static_cast<F>(rng() - RNG::min()) / static_cast<F>(RNG::max() - RNG::min());
    return static_cast<T>(min + t * (max - min));
}

/**
 * Fills a data container of type T with numberOfElements random numbers using the given random
 * number generator and distribution
 */
template <typename T, typename Rand = std::mt19937,
          typename Dist = typename std::conditional<std::is_integral<T>::value,
                                                    std::uniform_int_distribution<T>,
                                                    std::uniform_real_distribution<T>>::type>
void randomSequence(T* data, size_t numberOfElements, Rand& randomNumberGenerator = Rand(),
                    Dist& distribution = Dist(0, 1)) {
    std::generate(data, data + numberOfElements,
                  [&]() { return distribution(randomNumberGenerator); });
}

/**
 * Fills a span of type T with numberOfElements random numbers using the given random
 * number generator and distribution
 */
template <typename T, typename Rand = std::mt19937,
          typename Dist = typename std::conditional<std::is_integral<T>::value,
                                                    std::uniform_int_distribution<T>,
                                                    std::uniform_real_distribution<T>>::type>
void randomSequence(std::span<T> data, Rand& randomNumberGenerator = Rand(),
                    Dist& distribution = Dist(0, 1)) {
    std::generate(data.begin(), data.end(), [&]() { return distribution(randomNumberGenerator); });
}

/**
 * Generate a LayerRAMPrecision<T> with white noise based using a given random number
 * generator and distribution.
 * @tparam T     data type of the random numbers
 * @tparam Rand  random number engine, defaults to the Mersenne Twister engine (std::mt19937)
 * @tparam Dist  random number distribution, defaults to std::uniform_int/real_distribution between
 *               zero and one
 * @param dims   Size of the resulting layer
 * @param randomNumberGenerator the random number generator to use
 * @param distribution the distribution to use for the random numbers
 * @return LayerRAM with white noise
 */
template <typename T, typename Rand = std::mt19937,
          typename Dist = typename std::conditional<std::is_integral<T>::value,
                                                    std::uniform_int_distribution<T>,
                                                    std::uniform_real_distribution<T>>::type>
std::shared_ptr<LayerRAMPrecision<T>> randomLayerRAM(size2_t dims,
                                                     Rand& randomNumberGenerator = Rand(),
                                                     Dist& distribution = Dist(0, 1)) {
    auto layerRam = std::make_shared<LayerRAMPrecision<T>>(LayerReprConfig{
        .dimensions = dims,
        .swizzleMask = swizzlemasks::defaultData(1),
    });

    randomSequence(layerRam->getView(), randomNumberGenerator, distribution);
    return layerRam;
}

/**
 * Generate a Layer with white noise based using a given random number generator and
 * distribution.
 * @param dims Size of the output layer
 * @param randomNumberGenerator the random number generator to use, defaults to the Mersenne Twister
 * engine (std::mt19937)
 * @param distribution the distribution to use for the random numbers, defaults to
 * std::uniform_int/real_distribution between zero and one
 * @see randomLayerRAM
 */
template <typename T, typename Rand = std::mt19937,
          typename Dist = typename std::conditional<std::is_integral<T>::value,
                                                    std::uniform_int_distribution<T>,
                                                    std::uniform_real_distribution<T>>::type>
std::shared_ptr<Layer> randomLayer(size2_t dims, Rand& randomNumberGenerator = Rand(),
                                   Dist& distribution = Dist(0, 1)) {
    auto layer =
        std::make_shared<Layer>(randomLayerRAM<T>(dims, randomNumberGenerator, distribution));
    layer->dataMap.dataRange = dvec2{distribution.min(), distribution.max()};
    return layer;
}

/**
 * Generate an Volume with white noise based using C++ a given random number generator and
 * distribution.
 * @param dims Size of the output Volume
 * @param randomNumberGenerator the Random number generator to use, defaults to the Mersenne Twister
 * engine (std::mt19937)
 * @param distribution the distribution to use for the random numbers, defaults to
 * std::uniform_int/real_distribution between zero and one
 */
template <typename T, typename Rand = std::mt19937,
          typename Dist = typename std::conditional<std::is_integral<T>::value,
                                                    std::uniform_int_distribution<T>,
                                                    std::uniform_real_distribution<T>>::type>
std::shared_ptr<Volume> randomVolume(size3_t dims, Rand& randomNumberGenerator = Rand(),
                                     Dist& distribution = Dist(0, 1)) {
    auto volumeRam = std::make_shared<VolumeRAMPrecision<T>>(VolumeReprConfig{
        .dimensions = dims,
        .swizzleMask = swizzlemasks::defaultData(1),
    });

    randomSequence(volumeRam->getView(), randomNumberGenerator, distribution);

    return std::make_shared<Volume>(volumeRam);
}

/**
 * Generate a Layer with perlin noise, a cloud like noise using the sum of several white noise
 * layers with different frequencies
 * @param dims Size of the output layer
 * @param persistence controls the amplitude used in the different frequencies
 * @param startLevel controls the min level used. The level is determining the frequency to use
 * in each white noise layer as 2^level
 * @param endLevel controlsthe max level used.
 * @param randomNumberGenerator the Random number generator to use, defaults to the Mersenne Twister
 * engine (std::mt19937)
 */
template <typename Rand = std::mt19937>
std::shared_ptr<Layer> perlinNoise(size2_t dims, float persistence, size_t startLevel,
                                   size_t endLevel, Rand& randomNumberGenerator = Rand()) {
    const auto size = std::bit_ceil(std::max(dims.x, dims.y));
    std::vector<std::shared_ptr<LayerRAMPrecision<float>>> levels;
    std::vector<TemplateImageSampler<float, float>> samplers;
    auto currentSize = std::pow(2, startLevel);
    auto iterations = endLevel - startLevel + 1;
    float currentPersistance = 1;
    while (currentSize <= size && iterations--) {
        size2_t layerSize{static_cast<size_t>(currentSize)};
        auto dist = std::uniform_real_distribution<float>(-currentPersistance, currentPersistance);
        auto randomLayer = util::randomLayerRAM<float>(layerSize, randomNumberGenerator, dist);
        samplers.push_back(TemplateImageSampler<float, float>(randomLayer.get()));
        levels.push_back(randomLayer);
        currentSize *= 2;
        currentPersistance *= persistence;
    }

    auto layerRam = std::make_shared<LayerRAMPrecision<float>>(LayerReprConfig{
        .dimensions = dims,
        .swizzleMask = swizzlemasks::defaultData(1),
    });

    auto data = layerRam->getView();
    float repri = 1.0f / size;
    util::IndexMapper2D index(dims);
    for (size_t y = 0; y < dims.y; y++) {
        for (size_t x = 0; x < dims.x; x++) {
            float v = 0;
            float X = x * repri;
            float Y = y * repri;
            for (auto& sampler : samplers) {
                v += sampler.sample(X, Y);
            }
            v = (v + 1.0f) / 2.0f;
            data[index(x, dims.y - 1 - y)] = glm::clamp(v, 0.0f, 1.0f);
        }
    }
    return std::make_shared<Layer>(layerRam);
}

/**
 * Generate a Layer with sparse noise based on the perlin noise algorith.
 * @see http://devmag.org.za/2009/05/03/poisson-disk-sampling/
 * @param dims Size of the output layer
 * @param poissonDotsAlongX controlls the amount on points there is on average per line, set the
 * minimum distance between points
 * @param maxPoints a fallback variable to prevent generating to many points
 * @param randomNumberGenerator the Random number generator to use, defaults to the Mersenne Twister
 * engine (std::mt19937)
 */
template <typename Rand = std::mt19937>
std::shared_ptr<Layer> poissonDisk(size2_t dims, size_t poissonDotsAlongX, size_t maxPoints,
                                   Rand& randomNumberGenerator = Rand()) {
    std::uniform_int_distribution<int> rx(0, static_cast<int>(dims.x));
    std::uniform_int_distribution<int> ry(0, static_cast<int>(dims.y));
    std::uniform_real_distribution<float> rand(0, 1);

    float minDist = static_cast<float>(dims.x);
    minDist /= poissonDotsAlongX;  // min pixel distance between samples
    auto minDist2 = minDist * minDist;
    size2_t gridSize = size2_t(1) + size2_t(vec2(dims) * (1.0f / minDist));

    auto generateRandomPointAround = [&](const glm::i32vec2& point) {
        auto radius = minDist * (rand(randomNumberGenerator) +
                                 1);  // random radius between mindist and 2 * mindist
        auto angle = 2 * std::numbers::pi * rand(randomNumberGenerator);  // random angle
        auto newX = point.x + radius * std::cos(angle);
        auto newY = point.y + radius * std::sin(angle);
        return glm::i32vec2(newX, newY);
    };

    std::vector<glm::i32vec2> gridData(glm::compMul(dims), glm::i32vec2{0});
    util::IndexMapper2D imgIndex(dims);
    util::IndexMapper2D gridIndex(gridSize);

    for (size_t i = 0; i < gridSize.x * gridSize.y; i++) {
        gridData[i] = glm::i32vec2(static_cast<glm::i32>(-2 * minDist));
    }

    std::vector<glm::i32vec2> processList;
    std::vector<glm::i32vec2> samplePoints;

    glm::i32vec2 firstPoint;
    firstPoint.x = rx(randomNumberGenerator);
    firstPoint.y = ry(randomNumberGenerator);
    processList.push_back(firstPoint);
    samplePoints.push_back(firstPoint);

    auto toGrid = [minDist](glm::i32vec2 p) -> glm::i32vec2 {
        return glm::i32vec2(vec2(p) / minDist);
    };

    gridData[gridIndex(toGrid(firstPoint))] = firstPoint;

    int someNumber = 30;

    auto layerRam = std::make_shared<LayerRAMPrecision<float>>(LayerReprConfig{
        .dimensions = dims,
        .swizzleMask = swizzlemasks::defaultData(1),
    });
    auto layerData = layerRam->getView();

    while (processList.size() != 0 && samplePoints.size() < static_cast<size_t>(maxPoints)) {
        std::uniform_int_distribution<size_t> ri(0, processList.size() - 1);
        auto i = ri(randomNumberGenerator);
        auto p = processList[i];
        processList.erase(processList.begin() + i);

        for (int j = 0; j < someNumber; j++) {
            auto newPoint = generateRandomPointAround(p);
            if (glm::any(glm::lessThan(newPoint, glm::i32vec2(0))) ||
                glm::any(glm::greaterThanEqual(newPoint, glm::i32vec2(dims)))) {
                continue;
            }

            auto newGridPoint = toGrid(newPoint);
            bool neighbourhood = false;

            int startX = std::max(0, newGridPoint.x - 2);
            int startY = std::max(0, newGridPoint.y - 2);
            int endX = std::min(static_cast<int>(gridSize.x) - 1, newGridPoint.x + 2);
            int endY = std::min(static_cast<int>(gridSize.y) - 1, newGridPoint.y + 2);

            for (int x = startX; x <= endX && !neighbourhood; x++) {
                for (int y = startY; y <= endY && !neighbourhood; y++) {
                    auto p2 = gridData[gridIndex(glm::i32vec2(x, y))];
                    auto dist = glm::distance2(vec2(newPoint), vec2(p2));
                    if (dist < minDist2) {
                        neighbourhood = true;
                    }
                }
            }
            if (!neighbourhood) {
                processList.push_back(newPoint);
                samplePoints.push_back(newPoint);
                auto idx = gridIndex(newGridPoint);
                gridData[idx] = newPoint;
                layerData[imgIndex(newPoint)] = 1;
            }
        }
    }
    return std::make_shared<Layer>(layerRam);
}

}  // namespace util

}  // namespace inviwo
