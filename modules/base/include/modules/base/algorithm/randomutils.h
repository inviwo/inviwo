/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2019 Inviwo Foundation
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

#ifndef IVW_RANDOMUTILS_H
#define IVW_RANDOMUTILS_H

#include <modules/base/basemoduledefine.h>
#include <inviwo/core/common/inviwo.h>

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

namespace inviwo {

namespace util {

// Internal function to give the next power of 2 for a given integer ( 78 -> 128 )
static inline glm::u64 nextPow2(glm::u64 x) {
    if (x == 0) return 0;
    --x;
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;
    x |= x >> 32;
    return x + 1;
}

/**
 * Generate a sequence of length numberOfPoints of pseduo-random numbers on the open range (0 1).
 * @see https://en.wikipedia.org/wiki/Halton_sequence
 * @param base what base to use to generate fractions
 * @param numberOfPoints amount of points to generate
 */
template <typename T>
std::vector<T> haltonSequence(size_t base, size_t numberOfPoints) {
    std::vector<T> v;
    v.reserve(numberOfPoints);
    for (size_t i = 1; i <= numberOfPoints; i++) {
        T val(0);
        size_t denom = base;
        size_t a = i;
        while (a != 0) {
            auto rest = a % base;
            auto b = (T(1.0) / static_cast<T>(denom));
            val += rest * b;
            a -= rest;
            a /= base;
            denom *= base;
        }
        v.push_back(val);
    }
    return v;
}

/**
 * Generate an Image with sparse noise based on a pair of two Halton Sequences.
 * @see haltonSequence(size_t base, size_t numberOfPoints)
 * @see https://en.wikipedia.org/wiki/Halton_sequence
 * @param dims size of the resulting image.
 * @param numberOfPoints number of points to generate
 * @param baseX base used for the fractions to generate the x-values
 * @param baseY base used for the fractions to generate the y-values
 */
template <typename T>
std::shared_ptr<Image> haltonSequence(size2_t dims, size_t numberOfPoints, size_t baseX = 2,
                                      size_t baseY = 3) {

    std::shared_ptr<Image> img = std::make_shared<Image>(dims, DataFormat<T>::get());
    auto ram = static_cast<LayerRAMPrecision<T> *>(
        img->getColorLayer()->getEditableRepresentation<LayerRAM>());

    auto x = util::haltonSequence<T>(baseX, numberOfPoints);
    auto y = util::haltonSequence<T>(baseY, numberOfPoints);

    auto dimsf = vec2(dims - size2_t(1));

    util::IndexMapper2D index(dims);
    auto data = ram->getDataTyped();

    for (auto &&point : util::zip(x, y)) {
        auto coord = dimsf * vec2(get<0>(point), get<1>(point));
        data[index(coord)] = 1;
    }
    return img;
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
    std::shared_ptr<Volume> vol = std::make_shared<Volume>(dims, DataFormat<T>::get());
    auto ram = static_cast<VolumeRAMPrecision<T> *>(vol->getEditableRepresentation<VolumeRAM>());

    auto x = util::haltonSequence<T>(baseX, numberOfPoints);
    auto y = util::haltonSequence<T>(baseY, numberOfPoints);
    auto z = util::haltonSequence<T>(baseZ, numberOfPoints);

    auto dimsf = vec3(dims - size3_t(1));

    util::IndexMapper3D index(dims);
    auto data = ram->getDataTyped();

    for (auto &&point : util::zip(x, y, z)) {
        auto coord = dimsf * vec3(get<0>(point), get<1>(point), get<2>(point));
        data[index(coord)] = 1;
    }

    vol->dataMap_.dataRange = dvec2(0, 1);
    vol->dataMap_.valueRange = dvec2(0, 1);

    return vol;
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
 * std::numeric_limits::lowest()/::max()
 */
template <typename T, typename RNG,
          typename F = std::conditional_t<std::is_same_v<float, T>, float, double>>
inline static T randomNumber(RNG &rng, T min = detail::RandomNumberRangeValues::min<T>(),
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
void randomSequence(T *data, size_t numberOfElements, Rand &randomNumberGenerator = Rand(),
                    Dist &distribution = Dist(0, 1)) {
    std::generate(data, data + numberOfElements,
                  [&]() { return distribution(randomNumberGenerator); });
}

/**
 * Generate an Image with white noise based using C++ a given random number generator and
 * distribution.
 * @param dims Size of the output image
 * @param randomNumberGenerator the Random number generator to use, defaults to the Mersenne Twister
 * engine (std::mt19937)
 * @param distribution the distribution to use for the random numbers, defaults to
 * std::uniform_int/real_distribution between zero and one
 */
template <typename T, typename Rand = std::mt19937,
          typename Dist = typename std::conditional<std::is_integral<T>::value,
                                                    std::uniform_int_distribution<T>,
                                                    std::uniform_real_distribution<T>>::type>
std::shared_ptr<Image> randomImage(size2_t dims, Rand &randomNumberGenerator = Rand(),
                                   Dist &distribution = Dist(0, 1)) {
    std::shared_ptr<Image> img = std::make_shared<Image>(dims, DataFormat<T>::get());
    auto ram = static_cast<LayerRAMPrecision<T> *>(
        img->getColorLayer()->getEditableRepresentation<LayerRAM>());

    randomSequence(ram->getDataTyped(), dims.x * dims.y, randomNumberGenerator, distribution);

    return img;
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
std::shared_ptr<Volume> randomVolume(size3_t dims, Rand &randomNumberGenerator = Rand(),
                                     Dist &distribution = Dist(0, 1)) {
    std::shared_ptr<Volume> vol = std::make_shared<Volume>(dims, DataFormat<T>::get());
    auto ram = static_cast<VolumeRAMPrecision<T> *>(vol->getEditableRepresentation<VolumeRAM>());

    randomSequence(ram->getDataTyped(), dims.x * dims.y * dims.z, randomNumberGenerator,
                   distribution);

    vol->dataMap_.dataRange = dvec2(0, 1);
    vol->dataMap_.valueRange = dvec2(0, 1);

    return vol;
}

/**
 * Generate an Image with perlin noise, a cloud like noise using the sum of several white noise
 * images with different frequencies
 * @param dims Size of the output image
 * @param persistence controls the amplitude used in the different frequencies
 * @param startLevel controls the min level used. The level is determining the frequency to use
 * in each white noise image as 2^level
 * @param endLevel controlsthe max level used.
 * @param randomNumberGenerator the Random number generator to use, defaults to the Mersenne Twister
 * engine (std::mt19937)
 */
template <typename Rand = std::mt19937>
std::shared_ptr<Image> perlinNoise(size2_t dims, float persistence, size_t startLevel,
                                   size_t endLevel, Rand &randomNumberGenerator = Rand()) {
    std::shared_ptr<Image> img = std::make_shared<Image>(dims, DataFloat32::get());
    auto ram = static_cast<LayerRAMPrecision<float> *>(
        img->getColorLayer()->getEditableRepresentation<LayerRAM>());

    auto size = nextPow2(std::max(dims.x, dims.y));
    std::vector<std::shared_ptr<Image>> levels;
    std::vector<TemplateImageSampler<float, float>> samplers;
    auto currentSize = std::pow(2, startLevel);
    auto iterations = endLevel - startLevel + 1;
    float currentPersistance = 1;
    while (currentSize <= size && iterations--) {
        size2_t imgsize{static_cast<size_t>(currentSize)};
        auto dist = std::uniform_real_distribution<float>(-currentPersistance, currentPersistance);
        auto img1 = util::randomImage<float>(imgsize, randomNumberGenerator, dist);
        samplers.push_back(TemplateImageSampler<float, float>(img1.get()));
        levels.push_back(img1);
        currentSize *= 2;
        currentPersistance *= persistence;
    }

    auto data = ram->getDataTyped();
    float repri = 1.0f / size;
    util::IndexMapper2D index(dims);
    for (size_t y = 0; y < dims.y; y++) {
        for (size_t x = 0; x < dims.x; x++) {
            float v = 0;
            float X = x * repri;
            float Y = y * repri;
            for (auto &sampler : samplers) {
                v += sampler.sample(X, Y);
            }
            v = (v + 1.0f) / 2.0f;
            data[index(x, dims.y - 1 - y)] = glm::clamp(v, 0.0f, 1.0f);
        }
    }

    return img;
}

/**
 * Generate an Image with sparse noise based on the perlin noise algorith.
 * @see http://devmag.org.za/2009/05/03/poisson-disk-sampling/
 * @param dims Size of the output image
 * @param poissonDotsAlongX controlls the amount on points there is on average per line, set the
 * minimum distance between points
 * @param maxPoints a fallback variable to prevent generating to many points
 * @param randomNumberGenerator the Random number generator to use, defaults to the Mersenne Twister
 * engine (std::mt19937)
 */
template <typename Rand = std::mt19937>
std::shared_ptr<Image> poissonDisk(size2_t dims, size_t poissonDotsAlongX, size_t maxPoints,
                                   Rand &randomNumberGenerator = Rand()) {
    std::shared_ptr<Image> img = std::make_shared<Image>(dims, DataFloat32::get());

    std::uniform_int_distribution<int> rx(0, static_cast<int>(dims.x));
    std::uniform_int_distribution<int> ry(0, static_cast<int>(dims.y));
    std::uniform_real_distribution<float> rand(0, 1);

    float minDist = static_cast<float>(dims.x);
    minDist /= poissonDotsAlongX;  // min pixel distance between samples
    auto minDist2 = minDist * minDist;
    size2_t gridSize = size2_t(1) + size2_t(vec2(dims) * (1.0f / minDist));

    auto generateRandomPointAround = [&](const glm::i32vec2 &point) {
        auto radius = minDist * (rand(randomNumberGenerator) +
                                 1);  // random radius between mindist and 2 * mindist
        auto angle = 2 * M_PI * rand(randomNumberGenerator);  // random angle
        auto newX = point.x + radius * std::cos(angle);
        auto newY = point.y + radius * std::sin(angle);
        return glm::i32vec2(newX, newY);
    };

    auto gridImg = std::make_unique<Image>(gridSize, DataVec2Int32::get());
    auto grid = gridImg->getColorLayer()->getEditableRepresentation<LayerRAM>();

    auto imgData = static_cast<float *>(
        img->getColorLayer()->getEditableRepresentation<LayerRAM>()->getData());
    auto gridData = static_cast<glm::i32vec2 *>(grid->getData());
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
                imgData[imgIndex(newPoint)] = 1;
            }
        }
    }

    return img;
}

}  // namespace util
}  // namespace inviwo

#endif  // IVW_RANDOMUTILS_H
