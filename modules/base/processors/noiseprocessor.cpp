/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015 Inviwo Foundation
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

#include "noiseprocessor.h"
#include <inviwo/core/datastructures/image/imageram.h>
#include <inviwo/core/datastructures/image/image.h>
#include <inviwo/core/util/imagesampler.h>

namespace {
static inline int nextPow2(int x) {
    if (x == 0) return 0;
    --x;
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;
    return x + 1;
}
}

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
ProcessorClassIdentifier(NoiseProcessor, "org.inviwo.NoiseProcessor")
ProcessorDisplayName(NoiseProcessor, "NoiseProcessor")
ProcessorTags(NoiseProcessor, Tags::CPU);
ProcessorCategory(NoiseProcessor, "Data Creation");
ProcessorCodeState(NoiseProcessor, CODE_STATE_EXPERIMENTAL);

NoiseProcessor::NoiseProcessor()
    : Processor()
    , noise_("noise", DataFLOAT32::get(), false)
    , size_("size", "Size", ivec2(256), ivec2(32), ivec2(4096))
    , type_("type", "Type")
    , range_("range_","Range" , 0.0f, 1.0f, 0.0f, 1.0f)
    , levels_("levels", "Levels", 2, 8, 1, 16)
    , persistence_("persistence", "Persistence", 0.5f, 0.001f, 1.0f, 0.001f)
    , poissonDotsAlongX_("poissonDotsAlongX","Dots Along X",100,1,1024)
    , poissonMaxPoints_("poissonMaxPoints","Max Points" , 1 , 1000, 1000000)
    , randomness_("randomness", "Randomness")
    , useSameSeed_("useSameSeed", "Use same seed", true)
    , seed_("seed", "Seed", 1, 0, 1000)
    , rd_()
    , mt_(rd_()) {
    addPort(noise_);
    addProperty(size_);

    type_.addOption("random", "Random", NoiseType::Random);
    type_.addOption("perlin", "Perlin", NoiseType::Perlin);
    type_.addOption("poissonDisk", "Poisson Disk", NoiseType::PoissonDisk);
    type_.setCurrentStateAsDefault();
    addProperty(type_);
    addProperty(range_);
    addProperty(levels_);
    addProperty(persistence_);
    addProperty(poissonDotsAlongX_);
    addProperty(poissonMaxPoints_);

    auto typeOnChange = [&]() {
        range_.setVisible(type_.getSelectedValue() == NoiseType::Random);
        levels_.setVisible(type_.getSelectedValue() == NoiseType::Perlin);
        persistence_.setVisible(type_.getSelectedValue() == NoiseType::Perlin);
        poissonDotsAlongX_.setVisible(type_.getSelectedValue() == NoiseType::PoissonDisk);
        poissonMaxPoints_.setVisible(type_.getSelectedValue() == NoiseType::PoissonDisk);
    };

    type_.onChange(typeOnChange);
    
    addProperty(randomness_);
    randomness_.addProperty(useSameSeed_);
    randomness_.addProperty(seed_);
    useSameSeed_.onChange([&]() { seed_.setVisible(useSameSeed_.get()); });

    size_.onChange([&]() {
        auto s = std::max(size_.get().x, size_.get().y);
        s = nextPow2(s);
        auto l2 = log(s) / log(2.0f);
        levels_.setRangeMax(static_cast<int>(std::round(l2)));
    });

    typeOnChange();
}

NoiseProcessor::~NoiseProcessor() {}

void NoiseProcessor::process() {
    if (useSameSeed_.get()) {
        mt_.seed(seed_.get());
    }

    std::unique_ptr<Image> img = util::make_unique<Image>(size_.get(), DataFLOAT32::get());

    switch (type_.get()) {
        case NoiseType::Random:
            randomNoise(img.get(), range_.get().x, range_.get().y);
            break;
        case NoiseType::Perlin:
            perlinNoise(img.get());
            break;
        case NoiseType::PoissonDisk:
            poissonDisk(img.get());
            break;
    }

    noise_.setData(img.release());
}

void NoiseProcessor::randomNoise(Image *img, float minv, float maxv) {
    std::uniform_real_distribution<float> r(minv, maxv);
    size_t pixels = img->getDimensions().x * img->getDimensions().y;
    auto data = static_cast<float *>(
        img->getColorLayer()->getEditableRepresentation<LayerRAM>()->getData());
    for (size_t i = 0; i < pixels; i++) {
        data[i] = r(mt_);
    }
}

void NoiseProcessor::perlinNoise(Image *img) {
    auto size = nextPow2(std::max(size_.get().x, size_.get().y));
    std::vector<std::unique_ptr<Image>> levels;
    std::vector<TemplateImageSampler<float,float>> samplers;
    auto currentSize = std::pow(2, levels_.get().x);
    auto iterations = levels_.get().y - levels_.get().x + 1;
    float currentPersistance = 1;
    while (currentSize <= size && iterations--) {
        size2_t imgsize{static_cast<size_t>(currentSize)};
        auto img1 = util::make_unique<Image>(imgsize, DataFLOAT32::get());
        randomNoise(img1.get(), -currentPersistance, currentPersistance);
        samplers.push_back(TemplateImageSampler<float,float>(img1.get()));
        levels.push_back(std::move(img1));
        currentSize *= 2;
        currentPersistance *= persistence_.get();
    }

    auto data = static_cast<float *>(
        img->getColorLayer()->getEditableRepresentation<LayerRAM>()->getData());
    float repri = 1.0 / size;
    // size_t index = 0;
    util::IndexMapper2D index(size_.get());
#pragma omp parallel for
    for (long long y = 0; y < size_.get().y; y++) {
        for (long long x = 0; x < size_.get().x; x++) {
            float v = 0;
            float X = x * repri;
            float Y = y * repri;
            for (auto &sampler : samplers) {
                v += sampler.sample(X, Y);
            }
            v = (v + 1.0f) / 2.0f;
            data[index(x, size_.get().y - 1 - y)] = glm::clamp(v, 0.0f, 1.0f);
        }
    }
}

glm::i32vec2 generateRandomPointAround(const glm::i32vec2 &point, float mindist, std::uniform_real_distribution<float> &rand, std::mt19937 &mt)
{ //non-uniform, favours points closer to the inner ring, leads to denser packings
    auto r1 = rand(mt);
    auto r2 = rand(mt);
    
    //random radius between mindist and 2 * mindist
    auto radius = mindist * (r1 + 1);
    //random angle
    auto angle = 2 * M_PI * r2;
    //the new point is generated around the point (x, y)
    auto newX = point.x + radius * std::cos(angle);
    auto newY = point.y + radius * std::sin(angle);
    return glm::i32vec2(newX, newY);
}

/* 
Algorithm as describe by: 
http://devmag.org.za/2009/05/03/poisson-disk-sampling/
*/
void NoiseProcessor::poissonDisk(Image *img) {
    
    float minDist = size_.get().x;
    minDist /= poissonDotsAlongX_; //min pixel distance between samples
    auto minDist2 = minDist*minDist;
    size2_t gridSize = size2_t(1)+ size2_t(vec2(size_.get()) * (1.0f / minDist));

    auto gridImg = util::make_unique<Image>(gridSize, DataVec2INT32::get());
    auto grid = gridImg->getColorLayer()->getEditableRepresentation<LayerRAM>();

    auto imgData = static_cast<float*>(img->getColorLayer()->getEditableRepresentation<LayerRAM>()->getData());
    auto gridData = static_cast<glm::i32vec2*>(grid->getData());
    util::IndexMapper2D imgIndex(size_.get());
    util::IndexMapper2D gridIndex(gridSize);


    for (size_t i = 0; i < gridSize.x*gridSize.y; i++) {
        gridData[i] = glm::i32vec2(-2 * minDist);
    }

    std::uniform_int_distribution<int> rx(0, size_.get().x);
    std::uniform_int_distribution<int> ry(0, size_.get().y);
    std::uniform_real_distribution<float> rand(0, 1);

    std::vector<glm::i32vec2>  processList;
    std::vector<glm::i32vec2>  samplePoints;

    auto firstPoint = glm::i32vec2(rx(mt_), ry(mt_));
    processList.push_back(firstPoint);
    samplePoints.push_back(firstPoint);

    auto toGrid = [minDist](glm::i32vec2 p)->glm::i32vec2 { return glm::i32vec2(vec2(p)/minDist); };

    gridData[gridIndex(toGrid(firstPoint))] = firstPoint;

    int  someNumber = 30;

    while (processList.size() != 0 && samplePoints.size()  < static_cast<size_t>(poissonMaxPoints_)) {
        std::uniform_int_distribution<size_t> ri(0, processList.size()-1);
        auto i = ri(mt_);
        auto p = processList[i];
        processList.erase(processList.begin() + i);


        for (int j = 0; j < someNumber; j++) {
            auto newPoint = generateRandomPointAround(p, minDist, rand, mt_);
            if (newPoint.x < 0) continue;
            if (newPoint.y < 0) continue;
            if (newPoint.x >= size_.get().x) continue;
            if (newPoint.y >= size_.get().y) continue;

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
            }//*/
            if (!neighbourhood) {
                processList.push_back(newPoint);
                samplePoints.push_back(newPoint);
                auto idx = gridIndex(newGridPoint);
                gridData[idx] = newPoint;
                imgData[imgIndex(newPoint)] = 1;
            }
        }

    }

}

}  // namespace
