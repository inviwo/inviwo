/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2024 Inviwo Foundation
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

#include <warn/push>
#include <warn/ignore/all>
#include <gtest/gtest.h>
#include <warn/pop>

#include <inviwo/core/datastructures/image/layer.h>
#include <inviwo/core/datastructures/image/layerram.h>
#include <inviwo/core/datastructures/image/layerramprecision.h>
#include <inviwo/core/datastructures/image/image.h>

namespace inviwo {

TEST(ImageTests, constructFromLayers) {

    auto colorLayerRAM = std::make_shared<LayerRAMPrecision<glm::u8vec4>>(
        size2_t{2, 2}, LayerType::Color, swizzlemasks::rgba, InterpolationType::Linear,
        wrapping2d::clampAll);
    colorLayerRAM->getDataTyped()[0] = glm::u8vec4(1, 0, 0, 1);
    colorLayerRAM->getDataTyped()[1] = glm::u8vec4(2, 0, 0, 1);
    colorLayerRAM->getDataTyped()[2] = glm::u8vec4(3, 0, 0, 1);
    colorLayerRAM->getDataTyped()[3] = glm::u8vec4(4, 0, 0, 1);

    auto colorLayer = std::make_shared<Layer>(colorLayerRAM);

    auto depthLayerRAM = std::make_shared<LayerRAMPrecision<float>>(
        size2_t{2, 2}, LayerType::Depth, swizzlemasks::depth, InterpolationType::Linear,
        wrapping2d::clampAll);
    depthLayerRAM->getDataTyped()[0] = 0.1f;
    depthLayerRAM->getDataTyped()[1] = 0.2f;
    depthLayerRAM->getDataTyped()[2] = 0.3f;
    depthLayerRAM->getDataTyped()[3] = 0.4f;

    auto depthLayer = std::make_shared<Layer>(depthLayerRAM);

    auto pickingLayerRAM = std::make_shared<LayerRAMPrecision<glm::u8vec4>>(
        size2_t{2, 2}, LayerType::Picking, swizzlemasks::rgba, InterpolationType::Linear,
        wrapping2d::clampAll);
    pickingLayerRAM->getDataTyped()[0] = glm::u8vec4(5, 0, 0, 1);
    pickingLayerRAM->getDataTyped()[1] = glm::u8vec4(6, 0, 0, 1);
    pickingLayerRAM->getDataTyped()[2] = glm::u8vec4(7, 0, 0, 1);
    pickingLayerRAM->getDataTyped()[3] = glm::u8vec4(8, 0, 0, 1);

    auto pickingLayer = std::make_shared<Layer>(pickingLayerRAM);

    auto image = std::make_shared<Image>(std::vector{colorLayer, depthLayer, pickingLayer});

    EXPECT_EQ(image->getDimensions(), size2_t(2));
    EXPECT_EQ(image->getNumberOfColorLayers(), 1);
    EXPECT_EQ(image->readPixel(size2_t{0, 0}, LayerType::Color, 0).x, 1.0);
    EXPECT_DOUBLE_EQ(image->readPixel(size2_t{0, 0}, LayerType::Depth, 0).x, 0.1f);
    EXPECT_EQ(image->readPixel(size2_t{0, 0}, LayerType::Picking, 0).x, 5.0);

    EXPECT_EQ(image->readPixel(size2_t{1, 0}, LayerType::Color, 0).x, 2.0);
    EXPECT_DOUBLE_EQ(image->readPixel(size2_t{1, 0}, LayerType::Depth, 0).x, 0.2f);
    EXPECT_EQ(image->readPixel(size2_t{1, 0}, LayerType::Picking, 0).x, 6.0);

    EXPECT_EQ(image->readPixel(size2_t{0, 1}, LayerType::Color, 0).x, 3.0);
    EXPECT_DOUBLE_EQ(image->readPixel(size2_t{0, 1}, LayerType::Depth, 0).x, 0.3f);
    EXPECT_EQ(image->readPixel(size2_t{0, 1}, LayerType::Picking, 0).x, 7.0);

    EXPECT_EQ(image->readPixel(size2_t{1, 1}, LayerType::Color, 0).x, 4.0);
    EXPECT_DOUBLE_EQ(image->readPixel(size2_t{1, 1}, LayerType::Depth, 0).x, 0.4f);
    EXPECT_EQ(image->readPixel(size2_t{1, 1}, LayerType::Picking, 0).x, 8.0);
}

}  // namespace inviwo
