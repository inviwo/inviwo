/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2023-2025 Inviwo Foundation
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

#include <inviwo/core/common/inviwoapplication.h>
#include <modules/python3/python3module.h>
#include <modules/python3/pythonscript.h>
#include <modules/python3/pybindutils.h>
#include <modules/python3/volumepy.h>
#include <modules/python3/layerpy.h>

#include <inviwo/core/datastructures/volume/volume.h>
#include <inviwo/core/datastructures/volume/volumeram.h>
#include <inviwo/core/datastructures/volume/volumeramprecision.h>
#include <inviwo/core/datastructures/image/layer.h>
#include <inviwo/core/datastructures/image/layerram.h>
#include <inviwo/core/datastructures/image/layerramprecision.h>
#include <inviwo/core/util/glmcomp.h>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <glm/gtc/epsilon.hpp>

#include <array>
#include <algorithm>

namespace inviwo {

TEST(Python3Representations, LayerPy2RAM) {
    // create a 4 x 3 layer with two identical channels
    // voxel values increase in x direction
    PythonScript s;
    s.setSource(R"(
import numpy as np
arr = np.repeat(np.arange(12, dtype=np.uint8), 2)
arr.shape = (3, 4, 2)
)");

    bool status = false;
    s.run([&](pybind11::dict dict) {
        ASSERT_TRUE(dict.contains("arr"));

        auto ob = dict["arr"];
        ASSERT_TRUE(pybind11::isinstance<pybind11::array>(ob));
        auto arr = pybind11::cast<pybind11::array>(ob);

        auto layerpy = std::make_shared<LayerPy>(arr);
        EXPECT_EQ(size2_t(4, 3), layerpy->getDimensions());
        EXPECT_EQ(2, layerpy->getDataFormat()->getComponents());

        LayerPy2RAMConverter converter;
        auto layerram = converter.createFrom(layerpy);

        EXPECT_EQ(size2_t(4, 3), layerram->getDimensions());
        EXPECT_EQ(2, layerram->getDataFormat()->getComponents());

        layerram->dispatch<void>([&](auto pLayer) {
            auto dims = pLayer->getDimensions();
            auto data = pLayer->getDataTyped();

            int expected = 0;
            for (size_t i = 0; i < glm::compMul(dims); ++i) {
                EXPECT_EQ(expected, static_cast<int>(util::glmcomp(data[i], 0)));
                EXPECT_EQ(expected, static_cast<int>(util::glmcomp(data[i], 1)));
                ++expected;
            }
        });

        status = true;
    });
}

TEST(Python3Representations, LayerRAM2Py) {
    // a 4 x 3 x 1 volume with two identical channels
    // voxel values increase in x direction
    std::array<int, 24> data = {0, 0, 1, 1, 2, 2, 3, 3, 4,  4,  5,  5,
                                6, 6, 7, 7, 8, 8, 9, 9, 10, 10, 11, 11};
    const size2_t dims{4, 3};

    auto layerRAM = std::make_shared<LayerRAMPrecision<glm::ivec2>>(dims);
    memcpy(layerRAM->getDataTyped(), data.data(), data.size() * sizeof(int));

    LayerRAM2PyConverter converter;
    auto layerpy = converter.createFrom(layerRAM);
    EXPECT_EQ(dims, layerpy->getDimensions()) << "LayerPy dimensions";

    const auto& arr = layerpy->data();
    auto pyformat = pyutil::getDataFormat(2, layerpy->data());

    EXPECT_EQ(3, arr.ndim()) << "LayerPy numpy ndims";
    EXPECT_EQ(dims, size2_t(arr.shape(1), arr.shape(0))) << "LayerPy numpy dimensions";
    EXPECT_EQ("Vec2INT32", pyformat->getString()) << "LayerPy data format";

    // check correct, consecutive ordering of the pybind array
    const pybind11::array_t<int> d = layerpy->data();
    std::span<const int> s{d.data(0), 24};
    EXPECT_TRUE(std::equal(data.begin(), data.end(), s.begin()))
        << "difference between input and LayerPy data";

    // check correct, consecutive ordering of the pybind array within python
    PythonScript script;
    script.setSource(R"(
import numpy as np
result = np.array_equal(expected, arr.flatten())
)");
    bool status = false;
    script.run({{"expected", pybind11::cast(data)}, {"arr", layerpy->data()}},
               [&](pybind11::dict dict) {
                   EXPECT_TRUE(pybind11::cast<bool>(dict["result"]))
                       << "difference between input and LayerPy data";
                   status = true;
               });
    EXPECT_TRUE(status);
}

TEST(Python3Representations, VolumePy2RAM) {
    // create a 4 x 3 x 1 volume with two identical channels
    // voxel values increase in x direction
    PythonScript s;
    s.setSource(R"(
import numpy as np
arr = np.repeat(np.arange(12, dtype=np.uint8), 2)
arr.shape = (1, 3, 4, 2)
)");

    bool status = false;
    s.run([&](pybind11::dict dict) {
        ASSERT_TRUE(dict.contains("arr"));

        auto ob = dict["arr"];
        ASSERT_TRUE(pybind11::isinstance<pybind11::array>(ob));
        auto arr = pybind11::cast<pybind11::array>(ob);

        auto volumepy = std::make_shared<VolumePy>(arr);
        EXPECT_EQ(size3_t(4, 3, 1), volumepy->getDimensions());
        EXPECT_EQ(2, volumepy->getDataFormat()->getComponents());

        VolumePy2RAMConverter converter;
        auto volumeram = converter.createFrom(volumepy);

        EXPECT_EQ(size3_t(4, 3, 1), volumeram->getDimensions());
        EXPECT_EQ(2, volumeram->getDataFormat()->getComponents());

        volumeram->dispatch<void>([&](auto pVolume) {
            auto dims = pVolume->getDimensions();
            auto data = pVolume->getDataTyped();

            int expected = 0;
            for (size_t i = 0; i < glm::compMul(dims); ++i) {
                EXPECT_EQ(expected, static_cast<int>(util::glmcomp(data[i], 0)));
                EXPECT_EQ(expected, static_cast<int>(util::glmcomp(data[i], 1)));
                ++expected;
            }
        });

        status = true;
    });
}

TEST(Python3Representations, VolumeRAM2Py) {
    // a 4 x 3 x 1 volume with two identical channels
    // voxel values increase in x direction
    std::array<int, 24> data = {0, 0, 1, 1, 2, 2, 3, 3, 4,  4,  5,  5,
                                6, 6, 7, 7, 8, 8, 9, 9, 10, 10, 11, 11};
    const size3_t dims{4, 3, 1};

    auto volumeRAM = std::make_shared<VolumeRAMPrecision<glm::ivec2>>(dims);
    memcpy(volumeRAM->getDataTyped(), data.data(), data.size() * sizeof(int));

    VolumeRAM2PyConverter converter;
    auto volumepy = converter.createFrom(volumeRAM);
    EXPECT_EQ(dims, volumepy->getDimensions()) << "VolumePy dimensions";

    const auto& arr = volumepy->data();
    auto pyformat = pyutil::getDataFormat(2, volumepy->data());

    EXPECT_EQ(4, arr.ndim()) << "VolumePy numpy ndims";
    EXPECT_EQ(dims, size3_t(arr.shape(2), arr.shape(1), arr.shape(0)))
        << "VolumePy numpy dimensions";
    EXPECT_EQ("Vec2INT32", pyformat->getString()) << "VolumePy data format";

    // check correct, consecutive ordering of the pybind array
    const pybind11::array_t<int> d = volumepy->data();
    std::span<const int> s{d.data(0), 24};
    EXPECT_TRUE(std::equal(data.begin(), data.end(), s.begin()))
        << "difference between input and VolumePy data";

    // check correct, consecutive ordering of the pybind array within python
    PythonScript script;
    script.setSource(R"(
import numpy as np
result = np.array_equal(expected, arr.flatten())
)");
    bool status = false;
    script.run({{"expected", pybind11::cast(data)}, {"arr", volumepy->data()}},
               [&](pybind11::dict dict) {
                   EXPECT_TRUE(pybind11::cast<bool>(dict["result"]))
                       << "difference between input and VolumePy data";
                   status = true;
               });
    EXPECT_TRUE(status);
}

}  // namespace inviwo
