/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017 Inviwo Foundation
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

#include <modules/python3/python3module.h>

#include <modules/python3/interface/pybuffer.h>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <modules/python3/pybindutils.h>

#include <glm/gtc/epsilon.hpp>

namespace inviwo {

namespace {
std::string getPath() {
    auto path = util::getInviwoApplication()->getModuleByType<Python3Module>()->getPath(
        ModulePath::UnitTests);
    return path + "/scripts/";
}
}  // namespace

TEST(Python3Scripts, GrabReturnValues) {
    PythonScriptDisk script(getPath() + "grabreturnvalue.py");

    bool status = false;
    script.run([&](pybind11::dict dict) {
        auto pyA = dict["a"];
        auto pyB = dict["b"];
        auto pyC = dict["c"];
        auto pyD = dict["d"];

        auto cA = pybind11::cast<int>(pyA);
        auto cB = pybind11::cast<float>(pyB);
        auto cC = pybind11::cast<std::string>(pyC);

        EXPECT_EQ(1, cA) << "Return value int";
        EXPECT_FLOAT_EQ(0.2f, cB) << "Return value float";
        EXPECT_STREQ("hello world", cC.c_str()) << "Return value string";

        auto pyList = pybind11::cast<pybind11::list>(pyD);
        EXPECT_EQ(1, pybind11::cast<int>(pyList[0])) << "Return value, int in list";
        EXPECT_EQ(2, pybind11::cast<int>(pyList[1])) << "Return value, int in list";
        EXPECT_STREQ("hello", pybind11::cast<std::string>(pyList[2]).c_str())
            << "Return value, string in list";

        status = true;
    });
    EXPECT_TRUE(status);
}

TEST(Python3Scripts, PassValues) {
    PythonScriptDisk script(getPath() + "passvalues.py");

    bool status = false;
    int a = 1;
    float b = 0.2f;
    std::string c = "hello world";
    std::vector<int> d({2, 3, 4});
    script.run({{"a", pybind11::cast(a)},
                {"b", pybind11::cast(b)},
                {"c", pybind11::cast(c)},
                {"d", pybind11::cast(d)}

               },
               [&](pybind11::dict dict) {

                   EXPECT_TRUE(pybind11::cast<bool>(dict["A"])) << "Pass value as int";
                   EXPECT_TRUE(pybind11::cast<bool>(dict["B"])) << "Pass value as float";
                   EXPECT_TRUE(pybind11::cast<bool>(dict["C"])) << "Pass value as string";
                   EXPECT_TRUE(pybind11::cast<bool>(dict["D1"])) << "Pass value as int list";
                   EXPECT_TRUE(pybind11::cast<bool>(dict["D2"])) << "Pass value as int list";
                   EXPECT_TRUE(pybind11::cast<bool>(dict["D3"])) << "Pass value as int list";

                   status = true;
               });

    EXPECT_TRUE(status);
}

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

TEST(Python3Scripts, GLMTest) {
    PythonScriptDisk script(getPath() + "glm.py");

    bool status = false;
    script.run([&](pybind11::dict dict) {
        auto v1 = pybind11::cast<vec3>(dict["v1"]);
        auto v2 = pybind11::cast<size2_t>(dict["v2"]);
        auto v3 = pybind11::cast<vec3>(dict["v3"]);
        auto v4 = pybind11::cast<vec4>(dict["v4"]);

        auto m1 = pybind11::cast<mat4>(dict["m1"]);
        auto m2 = pybind11::cast<mat3>(dict["m2"]);

        EXPECT_TRUE(
            glm::all(glm::epsilonEqual(vec3(1, 2, 3), v1, std::numeric_limits<float>::epsilon())));

        EXPECT_TRUE(glm::all(glm::equal(size2_t(4, 5), v2)));

        for (int col = 0; col < 4; col++) {
            for (int row = 0; row < 4; row++) {
                if (col == row) {
                    EXPECT_EQ(1, m1[col][row]);
                } else {
                    EXPECT_EQ(0, m1[col][row]);
                }
            }
        }

        EXPECT_EQ(0, m2[0][0]);
        EXPECT_EQ(1, m2[0][1]);
        EXPECT_EQ(0, m2[0][2]);

        EXPECT_EQ(-1, m2[1][0]);
        EXPECT_EQ(0, m2[1][1]);
        EXPECT_EQ(0, m2[1][2]);

        EXPECT_EQ(0, m2[2][0]);
        EXPECT_EQ(0, m2[2][1]);
        EXPECT_EQ(2, m2[2][2]);

        EXPECT_TRUE(
            glm::all(glm::epsilonEqual(vec3(-2, 1, 6), v3, std::numeric_limits<float>::epsilon())));

        EXPECT_TRUE(glm::all(
            glm::epsilonEqual(vec4(1, 2, 3, 4), v4, std::numeric_limits<float>::epsilon())));

        EXPECT_FLOAT_EQ(1.f, dict["x"].cast<float>());
        EXPECT_FLOAT_EQ(1.f, dict["r"].cast<float>());
        EXPECT_FLOAT_EQ(1.f, dict["s"].cast<float>());

        EXPECT_FLOAT_EQ(2.f, dict["y"].cast<float>());
        EXPECT_FLOAT_EQ(2.f, dict["g"].cast<float>());
        EXPECT_FLOAT_EQ(2.f, dict["t"].cast<float>());

        EXPECT_FLOAT_EQ(3.f, dict["z"].cast<float>());
        EXPECT_FLOAT_EQ(3.f, dict["b"].cast<float>());
        EXPECT_FLOAT_EQ(3.f, dict["p"].cast<float>());

        EXPECT_FLOAT_EQ(4.f, dict["w"].cast<float>());
        EXPECT_FLOAT_EQ(4.f, dict["a"].cast<float>());
        EXPECT_FLOAT_EQ(4.f, dict["q"].cast<float>());

        status = true;
    });

    EXPECT_TRUE(status);
}

TEST(Python3Scripts, OptionPropertyTest) {
    PythonScriptDisk script(getPath() + "option_property.py");

    bool status = false;
    script.run([&](pybind11::dict dict) {

        auto prop = dict["p"].cast<Property *>();
        ASSERT_TRUE(prop != nullptr);
        auto optionProperty = dynamic_cast<OptionPropertyInt *>(prop);
        ASSERT_TRUE(optionProperty != nullptr);

        EXPECT_STREQ("test", optionProperty->getIdentifier().c_str());
        EXPECT_STREQ("Test", optionProperty->getDisplayName().c_str());

        EXPECT_EQ(2, optionProperty->size());

        EXPECT_STREQ("a", optionProperty->getIdentifiers()[0].c_str());
        EXPECT_STREQ("A", optionProperty->getDisplayNames()[0].c_str());
        EXPECT_EQ(1, optionProperty->getValues()[0]);

        EXPECT_STREQ("b", optionProperty->getIdentifiers()[1].c_str());
        EXPECT_STREQ("B", optionProperty->getDisplayNames()[1].c_str());
        EXPECT_EQ(2, optionProperty->getValues()[1]);

        status = true;
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
        src << "a = np.array("<<numbers<<" , dtype = np." << GetParam() << " )" << std::endl;

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
TEST_P(DTypeTest, BufferTypeVec2Tests)  { testBuffer("[[1,2], [3,4], [5,6]]", 2); }
TEST_P(DTypeTest, BufferTypeVec3Tests)  { testBuffer("[[1,2,3], [4,5,6], [7,8,9]]", 3); }
TEST_P(DTypeTest, BufferTypeVec4Tests)  { testBuffer("[[1,2,3,4], [5,6,7,8], [9,10,11,12]]", 4); }


TEST_P(DTypeTest, LayerTypeScalarTests) { testLayer("[1,2,3,4]","(2,2)", 1); }
TEST_P(DTypeTest, LayerTypeVec2Tests) { testLayer("[[1,2], [3,4], [5,6] , [7,8]]", "(2,2,2)", 2); }
TEST_P(DTypeTest, LayerTypeVec3Tests) { testLayer("[[1,2,3], [4,5,6], [7,8,9] , [10,11,12]]", "(2,2,3)", 3); }
TEST_P(DTypeTest, LayerTypeVec4Tests) { testLayer("[[1,2,3,4], [5,6,7,8], [9,10,11,12], [13,14,15,16]]", "(2,2,4)", 4); }

TEST_P(DTypeTest, VolumeTypeScalarTests) { testVolume("[1,2,3,4,5,6,7,8]", "(2,2,2)", 1); }
TEST_P(DTypeTest, VolumeTypeVec2Tests) { testVolume("[[1,2], [3,4], [5,6] , [7,8] , [9, 10] , [11,12] , [13,14] , [15,16]]", "(2,2,2,2)", 2); }
TEST_P(DTypeTest, VolumeTypeVec3Tests) { testVolume("[[1,2,3], [4,5,6], [7,8,9] , [10,11,12],[13,14,15], [16,17,18], [19,20,21] , [22,23,24]]", "(2,2,2,3)", 3); }
TEST_P(DTypeTest, VolumeTypeVec4Tests) { testVolume("[[1,2,3,4], [5,6,7,8], [9,10,11,12], [13,14,15,16] , [17,18,19,20], [21,22,23,24], [25,26,27,28], [29,30,31,32]]", "(2,2,2,4)", 4); }

const static std::vector<std::string> dtypes = {{"float16"}, {"float32"}, {"float64"}, {"int8"},
                                                {"int16"},   {"int32"},   {"int64"},   {"uint8"},
                                                {"uint16"},  {"uint32"},  {"uint64"}};

INSTANTIATE_TEST_CASE_P(DefaultTypes, DTypeTest, ::testing::ValuesIn(dtypes));

}  // namespace inviwo
