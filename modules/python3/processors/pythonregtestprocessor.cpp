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

#include <modules/python3/processors/pythonregtestprocessor.h>

#include <inviwo/core/datastructures/image/image.h>
#include <inviwo/core/datastructures/image/layer.h>
#include <inviwo/core/datastructures/image/layerram.h>
#include <inviwo/core/datastructures/image/layerramprecision.h>

#include <inviwo/core/datastructures/buffer/buffer.h>
#include <inviwo/core/datastructures/buffer/bufferram.h>
#include <inviwo/core/datastructures/buffer/bufferramprecision.h>

#include <modules/python3/python3module.h>

#include <modules/python3/interface/pybuffer.h>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <modules/python3/pybindutils.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo PythonRegTestProcessor::processorInfo_{
    "org.inviwo.PythonRegTestProcessor",  // Class identifier
    "Python Reg Test Processor",          // Display name
    "Undefined",                          // Category
    CodeState::Experimental,              // Code state
    Tags::None,                           // Tags
};
const ProcessorInfo PythonRegTestProcessor::getProcessorInfo() const { return processorInfo_; }

PythonRegTestProcessor::PythonRegTestProcessor()
    : Processor()
    , outport_("outport", false)

    , basePath_(InviwoApplication::getPtr()->getModuleByType<Python3Module>()->getPath(
                    ModulePath::Scripts) +
                "/tests/")

    , scriptGrabReturnValue_(basePath_ + "grabreturnvalue.py")
    , scriptPassValues_(basePath_ + "passvalues.py")
    , scriptSimpleBufferTest_(basePath_ + "simple_buffer_test.py")
    , scriptBufferTypes_(basePath_ + "buffer_types.py")
    , scriptLayerTypes_(basePath_ + "layer_types.py")
    , scriptVolumeTypes_(basePath_ + "volume_types.py")

{

    addPort(outport_);

    auto invl = [&]() { this->invalidate(InvalidationLevel::InvalidOutput); };

    scriptGrabReturnValue_.onChange(invl);
    scriptPassValues_.onChange(invl);
    scriptSimpleBufferTest_.onChange(invl);
    scriptBufferTypes_.onChange(invl);
    scriptLayerTypes_.onChange(invl);
    scriptVolumeTypes_.onChange(invl);
}

void PythonRegTestProcessor::process() {
    const static size_t imgSize = 80;
    auto img = std::make_shared<Image>(size2_t(imgSize, imgSize), DataVec4UInt8::get());
    data_ = static_cast<LayerRAMPrecision<DataVec4UInt8::type> *>(
                img->getColorLayer(0)->getEditableRepresentation<LayerRAM>())
                ->getDataTyped();

    for (int i = 0; i < imgSize * imgSize; i++) {
        data_[i] = DataVec4UInt8::type(255, 255, 0, 255);
    }

    testID_ = 0;

    // TEST 1  GrabReturnValue_
    {
        bool status = false;
        scriptGrabReturnValue_.run(
            {},
            [&](pybind11::dict dict) {
                auto pyA = dict["a"];
                auto pyB = dict["b"];
                auto pyC = dict["c"];
                auto pyD = dict["d"];

                auto cA = pybind11::cast<int>(pyA);
                auto cB = pybind11::cast<float>(pyB);
                auto cC = pybind11::cast<std::string>(pyC);

                expectEQ(1, cA, "Return value int");
                expectEQFloat(0.2f, cB, "Return value float");
                expectEQ<std::string>("hello world", cC, "Return value string");

                auto pyList = pybind11::cast<pybind11::list>(pyD);
                expectEQ(1, pybind11::cast<int>(pyList[0]), "Return value list");
                expectEQ(2, pybind11::cast<int>(pyList[1]), "Return value list");
                expectEQ<std::string>("hello", pybind11::cast<std::string>(pyList[2]),
                                      "Return value list");

                status = true;
            });
        expectEQ(true, status, "Parse return value");
    }

    {
        // Test 2 scriptPassValues_
        bool status = false;
        int a = 1;
        float b = 0.2f;
        std::string c = "hello world";
        std::vector<int> d({2, 3, 4});
        scriptPassValues_.run(
            {{"a", pybind11::cast(a)},
             {"b", pybind11::cast(b)},
             {"c", pybind11::cast(c)},
             {"d", pybind11::cast(d)}

            },
            [&](pybind11::dict dict) {

                expectEQ(true, pybind11::cast<bool>(dict["A"]), "Pass value as int");
                expectEQ(true, pybind11::cast<bool>(dict["B"]), "Pass value as float");
                expectEQ(true, pybind11::cast<bool>(dict["C"]), "Pass value as string");

                status = true;
            });

        expectEQ(true, status, "Parse return value");
    }

    {
        const static size_t bufferSize = 10;
        // Test 3: buffer test
        Buffer<int> intBuffer(bufferSize);

        intBuffer.getEditableRAMRepresentation()->getDataContainer()[1] = 1;
        intBuffer.getEditableRAMRepresentation()->getDataContainer()[3] = 3;

        bool status = false;
        scriptSimpleBufferTest_.run(
            {{"intBuffer", pybind11::cast(static_cast<BufferBase *>(&intBuffer),
                                          pybind11::return_value_policy::reference)}},
            [&](pybind11::dict dict) {
                status = true;

                expectEQ(1, pybind11::cast<int>(dict["a"]), "Simple buffer test: read value");
                expectEQ(3, pybind11::cast<int>(dict["b"]), "Simple buffer test: read value");

                auto vec = intBuffer.getEditableRAMRepresentation()->getDataContainer();
                for (size_t i = 0; i < bufferSize; i++) {
                    expectEQ<int>(static_cast<int>(i * i), vec[i], "Simple buffer test: write value");
                }
            });

        expectEQ(true, status, "Simple buffer test");
    }

    {
        // test buffer types

        bool status = false;
        scriptBufferTypes_.run([&](pybind11::dict dict) {
            auto listOfArrays = pybind11::cast<pybind11::list>(dict["arrs"]);
            for (auto &arrObj : listOfArrays) {
                auto arr = pybind11::cast<pybind11::array>(arrObj);
                auto buffer = pyutil::createBuffer(arr);
                buffer->getEditableRepresentation<BufferRAM>()->dispatch<void>([&](auto pBuffer) {
                    auto vec = pBuffer->getDataContainer();
                    int expected = 1;
                    for (auto v : vec) {
                        for (int i = 0; i < pBuffer->getDataFormat()->getComponents(); i++) {
                            expectEQ(expected++, (int)util::glmcomp(v, i), "Buffer creation");
                        }
                    }

                });
            }
            status = true;
        });
        expectEQ(true, status, "Buffer types creation test");
    }

    {
        // test layer types

        bool status = false;
        scriptLayerTypes_.run([&](pybind11::dict dict) {
            auto listOfArrays = pybind11::cast<pybind11::list>(dict["arrs"]);
            for (auto &arrObj : listOfArrays) {
                auto arr = pybind11::cast<pybind11::array>(arrObj);
                auto layer = pyutil::createLayer(arr);
                layer->getEditableRepresentation<LayerRAM>()->dispatch<void>([&](auto pLayer) {
                    auto dims = pLayer->getDimensions();
                    expectEQ(size2_t(2, 2), dims, "Create Layer: Dimensions");
                    auto data = pLayer->getDataTyped();
                    int expected = 1;
                    for (int j = 0; j < 4; j++) {
                        auto v = data[j];
                        for (int i = 0; i < pLayer->getDataFormat()->getComponents(); i++) {
                            expectEQ(expected++, (int)util::glmcomp(v, i), "Layer creation");
                        }
                    }

                });
            }
            status = true;
        });
        expectEQ(true, status, "Layer creation test");
    }

    {
        // test volume types

        bool status = false;
        scriptVolumeTypes_.run([&](pybind11::dict dict) {
            auto listOfArrays = pybind11::cast<pybind11::list>(dict["arrs"]);
            for (auto &arrObj : listOfArrays) {
                auto arr = pybind11::cast<pybind11::array>(arrObj);
                auto volume = pyutil::createVolume(arr);
                volume->getEditableRepresentation<VolumeRAM>()->dispatch<void>([&](auto pVolume) {
                    auto dims = pVolume->getDimensions();
                    expectEQ(size3_t(2, 2, 2), dims, "Create Volume: Dimensions");
                    auto data = pVolume->getDataTyped();
                    int expected = 1;
                    for (int j = 0; j < 8; j++) {
                        auto v = data[j];
                        for (int i = 0; i < pVolume->getDataFormat()->getComponents(); i++) {
                            expectEQ(expected++, (int)util::glmcomp(v, i), "Volume creation");
                        }
                    }
                });
            }
            status = true;
        });
        expectEQ(true, status, "Volume creation test");
    }

    // TODO test layer/volume of not square / cube sizes

    outport_.setData(img);
    LogInfo("Number of tests runed: " << testID_);
}

}  // namespace
