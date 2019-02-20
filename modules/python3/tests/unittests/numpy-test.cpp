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

#include <warn/push>
#include <warn/ignore/all>
#include <gtest/gtest.h>
#include <warn/pop>

#include <inviwo/core/common/inviwoapplication.h>
#include <modules/python3/python3module.h>
#include <modules/python3/pythonscript.h>
#include <modules/python3/pybindutils.h>

#include <inviwo/core/datastructures/image/image.h>
#include <inviwo/core/datastructures/image/layer.h>
#include <inviwo/core/datastructures/image/layerram.h>
#include <inviwo/core/datastructures/image/layerramprecision.h>

#include <inviwo/core/datastructures/buffer/buffer.h>
#include <inviwo/core/datastructures/buffer/bufferram.h>
#include <inviwo/core/datastructures/buffer/bufferramprecision.h>

#include <inviwo/core/datastructures/volume/volume.h>
#include <inviwo/core/datastructures/volume/volumeram.h>
#include <inviwo/core/datastructures/volume/volumeramprecision.h>

#include <inviwo/core/properties/optionproperty.h>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <glm/gtc/epsilon.hpp>

namespace inviwo {

namespace {
std::string getPath() {
    auto path = util::getInviwoApplication()->getModuleByType<Python3Module>()->getPath(
        ModulePath::UnitTests);
    return path + "/scripts/";
}
}  // namespace

TEST(Python3Scripts, SimpleBufferTest) {
    PythonScriptDisk script(getPath() + "simple_buffer_test.py");

    const static size_t bufferSize = 10;
    // Test 3: buffer test
    Buffer<int> intBuffer(bufferSize);

    intBuffer.getEditableRAMRepresentation()->getDataContainer()[1] = 1;
    intBuffer.getEditableRAMRepresentation()->getDataContainer()[3] = 3;

    bool status = false;
    script.run({{"intBuffer", pybind11::cast(static_cast<BufferBase *>(&intBuffer),
                                             pybind11::return_value_policy::reference)}},
               [&](pybind11::dict dict) {
                   status = true;

                   EXPECT_EQ(1, pybind11::cast<int>(dict["a"])) << "Simple buffer test: read value";
                   EXPECT_EQ(3, pybind11::cast<int>(dict["b"])) << "Simple buffer test: read value";

                   auto vec = intBuffer.getEditableRAMRepresentation()->getDataContainer();
                   for (size_t i = 0; i < bufferSize; i++) {
                       EXPECT_EQ(static_cast<int>(i * i), vec[i])
                           << "Simple buffer test: write value";
                   }
               });

    EXPECT_TRUE(status);
}

class DTypeTest : public ::testing::TestWithParam<std::string> {
protected:
    virtual void SetUp() {}
    virtual void TearDown() {}

    void testBuffer(std::string numbers, size_t components) {
        std::stringstream src;
        src << "import numpy as np" << std::endl;
        src << "a = np.array(" << numbers << " , dtype = np." << GetParam() << " )" << std::endl;

        PythonScript s;
        s.setSource(src.str());
        bool status = false;
        s.run([&](pybind11::dict dict) {
            ASSERT_TRUE(dict.contains("a"));

            auto ob = dict["a"];
            ASSERT_TRUE(pybind11::isinstance<pybind11::array>(ob));
            auto arr = pybind11::cast<pybind11::array>(ob);
            auto buffer = pyutil::createBuffer(arr);
            EXPECT_EQ(3, buffer->getSize());
            EXPECT_EQ(components, buffer->getDataFormat()->getComponents());

            buffer->getEditableRepresentation<BufferRAM>()->dispatch<void>([&](auto pBuffer) {
                auto &vec = pBuffer->getDataContainer();
                int expected = 1;
                for (const auto &v : vec) {
                    for (size_t i = 0; i < pBuffer->getDataFormat()->getComponents(); i++) {
                        EXPECT_EQ(expected++, (int)util::glmcomp(v, i));
                    }
                }
            });

            status = true;
        });

        EXPECT_TRUE(status);
    }

    void testLayer(std::string numbers, std::string shape, size_t components) {
        std::stringstream src;
        src << "import numpy as np" << std::endl;
        src << "a = np.array(" << numbers << ", dtype=np." << GetParam() << ")" << std::endl;
        src << "a.shape = " << shape << std::endl;

        PythonScript s;
        s.setSource(src.str());
        bool status = false;
        s.run([&](pybind11::dict dict) {
            ASSERT_TRUE(dict.contains("a"));

            auto ob = dict["a"];
            ASSERT_TRUE(pybind11::isinstance<pybind11::array>(ob));

            auto arr = pybind11::cast<pybind11::array>(ob);
            auto layer = pyutil::createLayer(arr);
            EXPECT_EQ(components, layer->getDataFormat()->getComponents());
            layer->getEditableRepresentation<LayerRAM>()->dispatch<void>([&](auto pLayer) {
                auto dims = pLayer->getDimensions();
                EXPECT_EQ(size2_t(2, 2), dims);
                auto data = pLayer->getDataTyped();
                int expected = 1;
                for (int j = 0; j < 4; j++) {
                    auto v = data[j];
                    for (size_t i = 0; i < pLayer->getDataFormat()->getComponents(); i++) {
                        EXPECT_EQ(expected++, (int)util::glmcomp(v, i));
                    }
                }
            });

            status = true;
        });

        EXPECT_TRUE(status);
    }
    void testVolume(std::string numbers, std::string shape, size_t components) {

        std::stringstream src;
        src << "import numpy as np" << std::endl;
        src << "a = np.array(" << numbers << ", dtype=np." << GetParam() << ")" << std::endl;
        src << "a.shape = " << shape << std::endl;

        PythonScript s;
        s.setSource(src.str());
        bool status = false;
        s.run([&](pybind11::dict dict) {
            ASSERT_TRUE(dict.contains("a"));

            auto ob = dict["a"];
            ASSERT_TRUE(pybind11::isinstance<pybind11::array>(ob));

            auto arr = pybind11::cast<pybind11::array>(ob);
            auto volume = pyutil::createVolume(arr);
            EXPECT_EQ(components, volume->getDataFormat()->getComponents());
            volume->getEditableRepresentation<VolumeRAM>()->dispatch<void>([&](auto pLayer) {
                auto dims = pLayer->getDimensions();
                EXPECT_EQ(size3_t(2, 2, 2), dims);
                auto data = pLayer->getDataTyped();
                int expected = 1;
                for (int j = 0; j < 4; j++) {
                    auto v = data[j];
                    for (size_t i = 0; i < pLayer->getDataFormat()->getComponents(); i++) {
                        EXPECT_EQ(expected++, (int)util::glmcomp(v, i));
                    }
                }
            });

            status = true;
        });

        EXPECT_TRUE(status);
    }
};

TEST_P(DTypeTest, BufferTypeScalarTests) { testBuffer("[1,2,3]", 1); }
TEST_P(DTypeTest, BufferTypeVec2Tests) { testBuffer("[[1,2], [3,4], [5,6]]", 2); }
TEST_P(DTypeTest, BufferTypeVec3Tests) { testBuffer("[[1,2,3], [4,5,6], [7,8,9]]", 3); }
TEST_P(DTypeTest, BufferTypeVec4Tests) { testBuffer("[[1,2,3,4], [5,6,7,8], [9,10,11,12]]", 4); }

TEST_P(DTypeTest, LayerTypeScalarTests) { testLayer("[1,2,3,4]", "(2,2)", 1); }
TEST_P(DTypeTest, LayerTypeVec2Tests) { testLayer("[[1,2], [3,4], [5,6] , [7,8]]", "(2,2,2)", 2); }
TEST_P(DTypeTest, LayerTypeVec3Tests) {
    testLayer("[[1,2,3], [4,5,6], [7,8,9] , [10,11,12]]", "(2,2,3)", 3);
}
TEST_P(DTypeTest, LayerTypeVec4Tests) {
    testLayer("[[1,2,3,4], [5,6,7,8], [9,10,11,12], [13,14,15,16]]", "(2,2,4)", 4);
}

TEST_P(DTypeTest, VolumeTypeScalarTests) { testVolume("[1,2,3,4,5,6,7,8]", "(2,2,2)", 1); }
TEST_P(DTypeTest, VolumeTypeVec2Tests) {
    testVolume("[[1,2], [3,4], [5,6] , [7,8] , [9, 10] , [11,12] , [13,14] , [15,16]]", "(2,2,2,2)",
               2);
}
TEST_P(DTypeTest, VolumeTypeVec3Tests) {
    testVolume(
        "[[1,2,3], [4,5,6], [7,8,9] , [10,11,12],[13,14,15], [16,17,18], [19,20,21] , [22,23,24]]",
        "(2,2,2,3)", 3);
}
TEST_P(DTypeTest, VolumeTypeVec4Tests) {
    testVolume(
        "[[1,2,3,4], [5,6,7,8], [9,10,11,12], [13,14,15,16] , [17,18,19,20], [21,22,23,24], "
        "[25,26,27,28], [29,30,31,32]]",
        "(2,2,2,4)", 4);
}

const static std::vector<std::string> dtypes = {{"float16"}, {"float32"}, {"float64"}, {"int8"},
                                                {"int16"},   {"int32"},   {"int64"},   {"uint8"},
                                                {"uint16"},  {"uint32"},  {"uint64"}};

INSTANTIATE_TEST_SUITE_P(DefaultTypes, DTypeTest, ::testing::ValuesIn(dtypes));

}  // namespace inviwo
